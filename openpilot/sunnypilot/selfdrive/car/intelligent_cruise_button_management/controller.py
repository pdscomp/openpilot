"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
import numpy as np

from openpilot.cereal import custom
from opendbc.car.structs import car
from opendbc.car import structs
from opendbc.sunnypilot.car.icbm_actuation_profile import get_actuation_profile
from openpilot.common.constants import CV
from openpilot.common.realtime import DT_CTRL
from openpilot.common.swaglog import cloudlog
from openpilot.sunnypilot.selfdrive.car.intelligent_cruise_button_management.helpers import get_minimum_set_speed
from openpilot.sunnypilot.selfdrive.car.cruise_ext import CRUISE_BUTTON_TIMER, update_manual_button_timers

ButtonType = car.CarState.ButtonEvent.Type
LongitudinalPlanSource = custom.LongitudinalPlanSP.LongitudinalPlanSource
State = custom.IntelligentCruiseButtonManagement.IntelligentCruiseButtonManagementState
SendButtonState = custom.IntelligentCruiseButtonManagement.SendButtonState
SessionState = custom.LongitudinalPlanSP.SpeedLimit.AssistState

INACTIVE_TIMER = 0.4
# After a genuine driver press the servo yields in the opposing direction for this long:
# the driver just chose a speed and an immediate synthesized walk-back reads as a fight
# (route 126 t=341: a +5 was reverted within 1.4 s). SET+ parks down-moves, SET- parks
# up-moves (a refused re-anchor otherwise restores the baseline right over a fresh -5);
# a press in the opposing direction cancels the other grace. Accountability mirrors the
# gas override: the press is the driver's, so no urgency carve-out.
DRIVER_PRESS_GRACE_T = 3.0
DRIVER_PRESS_GRACE_FRAMES = int(DRIVER_PRESS_GRACE_T / DT_CTRL)
# Reaction deadband in display units, applied only while a limiter (SCC/SLA) drives the
# plan; those targets jitter 1-2 units frame to frame and an undamped servo ping-pongs
# SET+/SET- around the noise. A cruise-source target is the driver setpoint, a stable
# integer, so track it exactly: a dash residual from a dropped press self-heals instead
# of stranding the dash low.
REACT_DEADBAND = 2
# The error must persist this long before acting, so a single-frame target glitch
# (e.g. a bad map sample) or a momentary dip can't trigger a button burst.
REACT_TIMER = 0.3
# Down moves act after REACT_TIMER. Up moves on decel_needs_stable_setpoint cars wait for
# the target to hold still this long first: limiter dips arrive in trains, and restoring
# between them churns the dash and delays the next decel on an ECU that will not commit
# while the set speed is moving. The quiet window also gives card time to adopt the dash
# after a driver press before the servo could chase a stale target. Decel-overshoot
# release is exempt; its slow monotonic rise is measured as tolerated.
# Sized from an 11-route / 57k-frame sweep of the recorded target streams: the churn
# suppression is all bought in the first second (regret 67.7% -> 27.0% at 1.0 s; 3.0 s
# only reaches 26.2% while nearly doubling the speed lost to the wait).
RESTORE_QUIET_TIME = 1.0
RESTORE_QUIET_FRAMES = int(RESTORE_QUIET_TIME / DT_CTRL)

# Deceleration overshoot: a stock ACC's deceleration scales with the gap between the dash
# set speed and the ACTUAL speed, not the target: commanding dash = target produces almost
# nothing until the car is already several mph over it, so it arrives at curves hot. When the
# planner demands deceleration, command the dash below vEgo by the gap that yields the
# requested decel, capped at the planner target from above (down-only: a stale command
# fail-safes to the car slowing). The command tracks vEgo down through the maneuver and rises
# back to the target on its own as the car converges and aTarget relaxes.
# The mechanism is brand-agnostic; the response curve is not. To enable a brand, measure its
# achieved decel vs (dash - vEgo) gap from logs and add an inverse map entry here.
DECEL_OVERSHOOT_PARAMS = {
  # Mazda CX-5 2022, 422k hands-off cruise samples across 447 rlog segments:
  # ~0.09 m/s^2 per mph of gap, dead below ~2 mph, saturating near -0.75 m/s^2 by ~9 mph
  'mazda': {
    'decel_bp': [0.02, 0.09, 0.26, 0.44, 0.73],  # desired decel magnitude, m/s^2
    # Required gap below vEgo, mph, carrying a lead over the steady-state inverse. The gap
    # the ECU actually sees is not the one commanded here: the lever's rise is limited by
    # the dash walk (~4 mph/s measured), not by DECEL_OVERSHOOT_RISE, so a maneuver spends
    # its first seconds at a gap well short of the request -- route 135 measured 2.65 s
    # median from a limiter taking the source to the car pulling -0.5 m/s^2. Commanding the
    # deeper gap up front pays that walk back; the request falls as the car converges, so
    # the lever still lets go on its own.
    'gap_v': [2.0, 4.0, 6.0, 8.5, 10.0],
    'max_gap': 10.,  # mph; the response saturates, going deeper buys nothing
    'min_decel': 0.15,  # m/s^2; leave gentle coast-downs to the stock behavior
  },
}
# Apply fast (the curve is coming), release slowly so the command doesn't pump between the
# ECU's discrete coast/downshift/brake stages.
DECEL_OVERSHOOT_RISE = 10.  # mph/s
DECEL_OVERSHOOT_RELEASE = 3.  # mph/s
DECEL_OVERSHOOT_SOURCES = (LongitudinalPlanSource.sccVision, LongitudinalPlanSource.sccMap,
                           LongitudinalPlanSource.speedLimitAssist)

# The sustained 10 Hz stream registers on the ECU as paced discrete presses, never as a
# held button (all-routes census: 149/149 stream-driven dash steps were 1 mph -- the
# wheel's own button-up frames interleave with the forged ones, so the ECU never sees an
# unbroken hold). At ~4 mph/s it is still the fastest walk available, so it takes any
# move with a real distance to cover; discrete taps take the small remainder, where the
# stream's in-flight frames would overshoot and ping-pong around the target.
FAST_MODE_MIN = 3  # display units of remaining error to run the stream
# If the dash never moves under the stream, this ECU is not registering it at all;
# taps are the proven fallback for the rest of the drive.
FAST_STALL_T = 1.5  # s

TAP_BUTTONS = {
  State.increasing: SendButtonState.increase,
  State.decreasing: SendButtonState.decrease,
}
HOLD_BUTTONS = {
  State.increasing: SendButtonState.increaseHold,
  State.decreasing: SendButtonState.decreaseHold,
}


class IntelligentCruiseButtonManagement:
  def __init__(self, CP: structs.CarParams, CP_SP: structs.CarParamsSP):
    self.CP = CP
    self.CP_SP = CP_SP
    self.profile = get_actuation_profile(CP.brand)

    self.v_target = 0
    self.v_cruise_cluster = 0
    self.v_cruise_min = 0
    self.cruise_button = SendButtonState.none
    self.state = State.inactive
    self.pre_active_timer = 0
    self.restore_quiet_timer = 0
    self.v_target_prev = 0
    self.v_target_raw = 0
    self.v_target_raw_prev = 0
    self.react_deadband = REACT_DEADBAND
    self.lookahead_valid = False
    self.dip_ahead = False
    self.down_grace_timer = 0
    self.up_grace_timer = 0

    self.is_ready = False
    self.is_ready_prev = False
    self.is_metric = False
    # a pending SLA confirm prompt freezes the servo: the plan cap already holds, but
    # the dash must not move at all while the driver is being asked (card additionally
    # vetoes emission with same-frame session state; this gate is one hop stale)
    self.prompt_frozen = False
    self.decel_overshoot_enabled = False
    self.overshoot_mph = 0.0
    self.overshoot_params = DECEL_OVERSHOOT_PARAMS.get(CP.brand)
    self.limiter_active = False

    # Fast-walk stream execution
    self.fast_active = False
    self.fast_stall_frames = 0
    self.fast_last_cluster = 0
    self.fast_faulted = False  # set for the drive when the stream never moves the dash

    self.cruise_button_timers = dict(CRUISE_BUTTON_TIMER)

  def update_decel_overshoot(self, CS: car.CarState, LP_SP: custom.LongitudinalPlanSP) -> float:
    if self.overshoot_params is None:
      return 0.0

    p = self.overshoot_params
    want = 0.0
    # Never integrate a command the servo cannot emit: is_ready covers a driver press,
    # prompt_frozen a pending confirm. Both block emission, so winding up behind them
    # only banks a stale gap to dump the moment the block lifts. Nothing is lost by
    # waiting -- a limiter still asking for decel rebuilds at DECEL_OVERSHOOT_RISE, ~0.5 s
    # to a full gap, well inside the REACT_TIMER the servo owes before it acts anyway.
    if (self.decel_overshoot_enabled and self.is_ready and not self.prompt_frozen
        and self.down_grace_timer <= 0
        and LP_SP.longitudinalPlanSource in DECEL_OVERSHOOT_SOURCES
        and LP_SP.aTarget < -p['min_decel'] and CS.vEgo > LP_SP.vTarget):
      want = min(float(np.interp(-LP_SP.aTarget, p['decel_bp'], p['gap_v'])), p['max_gap'])

    if want > self.overshoot_mph:
      self.overshoot_mph = min(want, self.overshoot_mph + DECEL_OVERSHOOT_RISE * DT_CTRL)
    else:
      # release gently only while the limiter is live (aTarget flaps between the ECU's
      # decel stages mid-maneuver); once the plan is back on cruise the residual only
      # holds the dash down and stalls the restore, so drop it at the build rate
      release = DECEL_OVERSHOOT_RELEASE if self.limiter_active else DECEL_OVERSHOOT_RISE
      self.overshoot_mph = max(want, self.overshoot_mph - release * DT_CTRL)

    return self.overshoot_mph

  def update_calculations(self, CS: car.CarState, LP_SP: custom.LongitudinalPlanSP) -> None:
    speed_conv = CV.MS_TO_KPH if self.is_metric else CV.MS_TO_MPH

    self.limiter_active = LP_SP.longitudinalPlanSource != LongitudinalPlanSource.cruise

    v_target_ms = LP_SP.vTarget
    overshoot_ms = self.update_decel_overshoot(CS, LP_SP) * CV.MPH_TO_MS
    if overshoot_ms > 0:
      # command relative to actual speed so the ECU sees the gap that produces the requested
      # decel; never above the planner target, and never more than the gap below it
      v_target_ms = min(v_target_ms, max(CS.vEgo, LP_SP.vTarget) - overshoot_ms)

    self.v_target_prev = self.v_target
    self.v_target = round(v_target_ms * speed_conv)
    # The plan's own target, before the overshoot lever: restore intent is judged against
    # this. The lever's decay is self-inflicted motion and must not look like a moving plan.
    self.v_target_raw_prev = self.v_target_raw
    self.v_target_raw = round(LP_SP.vTarget * speed_conv)
    self.v_cruise_min = get_minimum_set_speed(self.is_metric)
    self.v_cruise_cluster = round(CS.cruiseState.speedCluster * speed_conv)

    # Exact tracking against the (stable) driver setpoint; jitter band against limiters.
    # Overshoot keeps the limiter band: its command moves by design.
    self.react_deadband = REACT_DEADBAND if self.limiter_active or self.overshoot_mph > 0 else 1

    # Vision lookahead for the restore gate. 0 = no lookahead (feature off, long disabled,
    # or no model) and the servo falls back to the stillness heuristic below.
    v_ahead_min = LP_SP.smartCruiseControl.vision.vAheadMin
    self.lookahead_valid = v_ahead_min > 0.
    self.dip_ahead = self.lookahead_valid and v_ahead_min * speed_conv < self.v_target_raw - self.react_deadband

  def update_restore_quiet_timer(self) -> None:
    # how long an up-error has persisted against a still PLAN target; plan-target motion,
    # the error closing, or a pending confirm prompt resets it. Holding the timer at zero
    # through the prompt means a decline or timeout still waits out a FULL quiet window
    # before any restore: the prompt must not pre-pay the servo's patience.
    # Keyed on v_target_raw, not the overshoot-adjusted command: the lever's slow release
    # after a limiter ends moved v_target every few frames and pinned this timer at zero
    # until the decay finished (route 126: 4.1 s of extra post-curve braking).
    up_error = self.v_target_raw - self.v_cruise_cluster
    if self.prompt_frozen:
      self.restore_quiet_timer = 0
    elif up_error >= self.react_deadband and self.v_target_raw == self.v_target_raw_prev:
      self.restore_quiet_timer += 1
    else:
      self.restore_quiet_timer = 0

  def plan_fast_mode(self) -> None:
    # Run the stream while the remaining error is worth it; taps take the remainder.
    # Evaluated every frame against the live dash. No grid or metric assumptions: the
    # stream is just presses, so it is valid wherever taps are.
    remaining = abs(self.v_target - self.v_cruise_cluster)
    use_fast = not self.fast_faulted and remaining >= FAST_MODE_MIN

    if use_fast and not self.fast_active:
      self.fast_active = True
      self.fast_stall_frames = 0
      self.fast_last_cluster = self.v_cruise_cluster
    elif self.fast_active:
      if remaining < FAST_MODE_MIN:
        self.fast_active = False
      elif self.v_cruise_cluster != self.fast_last_cluster:
        self.fast_last_cluster = self.v_cruise_cluster
        self.fast_stall_frames = 0
      else:
        self.fast_stall_frames += 1
        if self.fast_stall_frames * DT_CTRL > FAST_STALL_T:
          self.fast_faulted = True
          self.fast_active = False
          cloudlog.event("icbm_fast_mode_fallback", brand=self.CP.brand)

  def update_state_machine(self) -> custom.IntelligentCruiseButtonManagement.SendButtonState:
    self.pre_active_timer = max(0, self.pre_active_timer - 1)
    self.update_restore_quiet_timer()

    # a pending confirm prompt parks any move; transitions out of holding are gated below
    if self.prompt_frozen and self.state in (State.preActive, State.increasing, State.decreasing):
      self.state = State.holding

    # HOLDING, ACCELERATING, DECELERATING, PRE_ACTIVE
    if self.state != State.inactive:
      if not self.is_ready:
        self.state = State.inactive

      else:
        # Up-moves need the quiet window on decel_needs_stable_setpoint cars, on EVERY
        # entry path: the preActive route (taken after any driver press resets
        # readiness) used to bypass it straight into increasing, letting the servo
        # chase a stale target before card had settled the press's own effects.
        # The overshoot exemption only covers the overshoot command's own slow release
        # (still limiter-sourced); residual overshoot after a source flip back to cruise
        # must not bypass the quiet window into a full baseline restore.
        # With a valid vision lookahead the profile replaces the stillness heuristic
        # outright: restore immediately when nothing ahead binds below the target, and
        # hold while a dip is coming, however quiet the target is -- restoring between
        # bends accelerates the car into the next apex (route 126: 3 of 8 over-ceiling
        # apexes were restore-fed).
        if self.lookahead_valid:
          up_allowed = not self.dip_ahead
        else:
          up_allowed = ((self.overshoot_mph > 0 and self.limiter_active)
                        or not self.profile.decel_needs_stable_setpoint
                        or self.restore_quiet_timer >= RESTORE_QUIET_FRAMES)
        up_allowed = up_allowed and self.up_grace_timer <= 0

        # Down-moves skip the quiet window because a limiter's decel is urgent; that is
        # only true while the limiter is live. The overshoot is a lever, not a
        # destination, so a residual gap left over after a source flip back to cruise
        # must not start a fresh descent (mirror of up_allowed's residual carve-out).
        # Without overshoot in play a down-move is a plain setpoint correction (a dash
        # residual from a dropped press) and stays unconditional. A fresh driver SET+
        # press parks all down-moves for the grace window: the dash is the driver's for
        # a beat, and the plan keeps publishing its cap regardless.
        down_allowed = (self.limiter_active or self.overshoot_mph <= 0) and self.down_grace_timer <= 0

        # PRE_ACTIVE
        if self.state == State.preActive:
          if self.pre_active_timer <= 0:
            if self.v_target - self.v_cruise_cluster >= self.react_deadband and up_allowed:
              self.state = State.increasing

            elif self.v_cruise_cluster - self.v_target >= self.react_deadband \
                 and self.v_cruise_cluster > self.v_cruise_min and down_allowed:
              self.state = State.decreasing

            else:
              self.state = State.holding

        # HOLDING
        elif self.state == State.holding and not self.prompt_frozen:
          down_pending = self.v_cruise_cluster - self.v_target >= self.react_deadband and down_allowed
          up_pending = self.v_target - self.v_cruise_cluster >= self.react_deadband
          if down_pending or (up_pending and up_allowed):
            self.pre_active_timer = int(REACT_TIMER / DT_CTRL)
            self.state = State.preActive

        # ACCELERATING
        elif self.state == State.increasing:
          # a dip appearing mid-restore aborts it: the commit gate trails the profile,
          # and stepping up until the limiter takes the source feeds the next apex
          if self.v_target <= self.v_cruise_cluster or self.dip_ahead:
            self.state = State.holding

        # DECELERATING
        elif self.state == State.decreasing:
          if self.v_target >= self.v_cruise_cluster or self.v_cruise_cluster <= self.v_cruise_min:
            self.state = State.holding

    # INACTIVE
    elif self.state == State.inactive:
      if self.is_ready and not self.is_ready_prev:
        self.pre_active_timer = int(INACTIVE_TIMER / DT_CTRL)
        self.state = State.preActive

    if self.state in TAP_BUTTONS:
      self.plan_fast_mode()
      send_button = HOLD_BUTTONS[self.state] if self.fast_active else TAP_BUTTONS[self.state]
    else:
      self.fast_active = False
      send_button = SendButtonState.none

    return send_button

  def update_readiness(self, CS: car.CarState, CC: car.CarControl) -> None:
    update_manual_button_timers(CS, self.cruise_button_timers)

    ready = CC.enabled and not CC.cruiseControl.override and not CC.cruiseControl.cancel and not CC.cruiseControl.resume
    button_pressed = any(self.cruise_button_timers[k] > 0 for k in self.cruise_button_timers)

    # buttonEvents carry only the wheel's own presses (forged frames echo on src 128+ and
    # never reach carState), so this cannot latch on the servo's own sends
    if self.cruise_button_timers[ButtonType.accelCruise] > 0:
      self.down_grace_timer = DRIVER_PRESS_GRACE_FRAMES
      self.up_grace_timer = 0
    elif self.cruise_button_timers[ButtonType.decelCruise] > 0:
      self.up_grace_timer = DRIVER_PRESS_GRACE_FRAMES
      self.down_grace_timer = 0
    else:
      self.down_grace_timer = max(0, self.down_grace_timer - 1)
      self.up_grace_timer = max(0, self.up_grace_timer - 1)

    self.is_ready = ready and not button_pressed

  def run(self, CS: car.CarState, CC: car.CarControl, LP_SP: custom.LongitudinalPlanSP, is_metric: bool,
          decel_overshoot_enabled: bool = False, session_state=SessionState.disabled) -> None:
    if self.CP_SP.pcmCruiseSpeed:
      return

    self.is_metric = is_metric
    self.decel_overshoot_enabled = decel_overshoot_enabled
    self.prompt_frozen = session_state == SessionState.preActive

    self.update_calculations(CS, LP_SP)
    self.update_readiness(CS, CC)

    self.cruise_button = self.update_state_machine()

    self.is_ready_prev = self.is_ready
