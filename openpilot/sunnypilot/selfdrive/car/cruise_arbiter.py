"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Cruise arbiter: single owner of button meaning and the SLA session on non-pcm cars
(everything that is not pcm-op-long).

Runs at 100 Hz inside card, in the same frame as the button events and the setpoint
writer. Every +/- press is classified exactly once, into one intent, from the
pre-frame session snapshot; everything downstream (the v_cruise increment path, the
reconciler, the plannerd mirror, the ICBM servo) consumes the classification or the
published session instead of re-interpreting buttons. This replaces the previous
arrangement where four modules independently interpreted the same press across three
processes, bridged by wall-clock latches sized to the slowest consumer.

The session is published on carStateSP.cruiseSession; announceCounter makes 100 Hz
alert-worthy transitions visible to 20 Hz consumers without sampling loss.

A pending confirm prompt freezes speed at three altitudes, each with a distinct job:
  1. out of an active session, the session cap (v_cap) holds the old target, so the
     plan min() cannot release the dash toward the baseline while the driver is
     deciding. Prompting from idle publishes no cap: there is no old target to hold,
     and a cap equal to the baseline would still relabel the plan source as a limiter;
  2. the ICBM servo parks (controller.prompt_frozen) with its restore patience held
     at zero, so a decline/timeout still waits out a full quiet window;
  3. card vetoes button emission with same-frame state (gate_send_button), because
     the servo's view of the session is one message hop stale.
"""
from dataclasses import dataclass

import numpy as np

from openpilot.cereal import custom
from opendbc.car import structs
from opendbc.car.structs import car
from openpilot.common.constants import CV
from openpilot.common.realtime import DT_CTRL
from openpilot.sunnypilot.selfdrive.car.intelligent_cruise_button_management.helpers import get_minimum_set_speed
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit import ACTIVE_STATES
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit.common import Mode
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit.helpers import compare_cluster_target, confirm_needed_for_change

ButtonType = car.CarState.ButtonEvent.Type
SessionState = custom.LongitudinalPlanSP.SpeedLimit.AssistState
CruiseIntent = custom.CarStateSP.CruiseSession.CruiseIntent

# canonical for the card-side SP stack (cruise_ext imports from here); a fourth home is
# still one too many, but selfdrive.car.cruise cannot be imported without a cycle
V_CRUISE_UNSET = 255.
V_CRUISE_MAX = 145  # kph

# All timers are 100 Hz frame counts (DT_CTRL).
DISABLED_GUARD_PERIOD = 0.5   # s after engagement before the session may form
PRE_ACTIVE_GUARD_PERIOD = 5.  # s a confirm prompt stays open
# resolve a prompt press at long-press duration instead of waiting for release (the ECU
# is already grid-stepping); one frame before cruise.py's first repeat tick
# (CRUISE_LONG_PRESS = 50, not importable here without a cycle) so the tick is already
# owned when it fires
LONG_PRESS_FRAMES = 50 - 1

PLUS_BUTTONS = (ButtonType.accelCruise, ButtonType.resumeCruise)
MINUS_BUTTONS = (ButtonType.decelCruise, ButtonType.setCruise)

# per-press classification, decided at the press edge from the pre-frame snapshot
_PRESS_NORMAL = 0   # plain increment/decrement press
_PRESS_DISMISS = 1  # started while the session was active: owned, ends the session
_PRESS_PROMPT = 2   # started while a confirm prompt was open: resolves at release/tick


@dataclass
class _Press:
  cls: int
  frames: int = 0
  resolved: bool = False   # prompt press answered (confirm); owned from then on
  released: bool = False   # kept through the release frame for press_owned, swept next


class CruiseArbiter:
  def __init__(self, CP, CP_SP):
    # everything the old non-pcm SLA machine served: stock-ACC button cars (ICBM,
    # pcmCruise and not pcmCruiseSpeed) AND op-long ports without pcmCruise, where the
    # buttons and setpoint are openpilot's own. Only pcm-op-long cars keep the plannerd
    # machine. The dash-specific pieces (reconciler, servo freeze, card veto) gate
    # themselves on the ICBM config independently.
    self.applicable = not (CP.openpilotLongitudinalControl and CP.pcmCruise)

    # session
    self.state = SessionState.disabled
    self.state_prev_frame = SessionState.disabled  # snapshot from before this frame's step
    self.v_cap = V_CRUISE_UNSET  # m/s; session target while active, frozen hold while prompting
    self.last_intent = CruiseIntent.none
    self.announce_counter = 0

    # params, refreshed off the RT path (card params thread)
    self.enabled = False   # SpeedLimitMode == assist
    self.is_metric = False

    # resolver inputs (from longitudinalPlanSP, updated at plan rate)
    self._speed_limit = 0.
    self._speed_limit_prev = 0.
    self._slf = 0.  # speedLimitFinalLast, m/s
    self._has_limit = False

    # machine state
    self.long_enabled_prev = False
    self.long_engaged_timer = 0
    self.pre_active_timer = 0
    self._driver_dismissed = False
    self._cluster_conv = 0
    self._cluster_conv_prev = 0

    # press tracking, keyed by raw enumerant int (capnp _DynamicEnum instances do not
    # hash-match the raw ints cruise.py passes into press_owned)
    self._press: dict[int, _Press] = {}
    # set for the frame an accel-confirm adopts the limit; consumed by the helper to
    # kill the reconcile window before the reconciler runs
    self.adopted_this_frame = False

  # ---- params (called from card's params thread, never the 100 Hz path) -------------
  def read_params(self, params):
    if not self.applicable:
      return
    self.enabled = params.get("SpeedLimitMode", return_default=True) == Mode.assist
    self.is_metric = params.get_bool("IsMetric")

  # ---- resolver inputs (card, on longitudinalPlanSP updates) ------------------------
  def update_limit(self, LP_SP):
    if not self.applicable:
      return
    resolver = LP_SP.speedLimit.resolver
    self._speed_limit = float(resolver.speedLimit)
    self._slf = float(resolver.speedLimitFinalLast)
    self._has_limit = bool(resolver.speedLimitValid or resolver.speedLimitLastValid)

  # ---- helpers ----------------------------------------------------------------------
  @property
  def _conv(self) -> float:
    return CV.MS_TO_KPH if self.is_metric else CV.MS_TO_MPH

  @property
  def prompting(self) -> bool:
    return self.state == SessionState.preActive

  @property
  def session_active(self) -> bool:
    return self.state in ACTIVE_STATES

  def _target_conv(self) -> int:
    return round(self._slf * self._conv)

  @property
  def target_kph(self) -> float:
    # the limit rounded to a display integer, expressed in kph (the setpoint's unit)
    conv = 1. if self.is_metric else CV.KPH_TO_MPH
    return round(self._slf * CV.MS_TO_KPH * conv) / conv

  @property
  def slf_kph(self) -> float:
    return self._slf * CV.MS_TO_KPH

  @property
  def _limit_changed(self) -> bool:
    return self._has_limit and self._speed_limit != self._speed_limit_prev

  def _set_state(self, state, announce=False):
    self.state = state
    if announce:
      self.announce_counter += 1

  def _enter_prompt(self):
    # Freeze the plan for the length of the prompt: out of an active session the hold is
    # the session's last cap (the dash stays put instead of restoring un-confirmed).
    # Idle there is nothing to hold -- the baseline IS the dash -- so publish no cap at
    # all. Holding the cluster instead is a no-op numerically but NOT in the plan's
    # min(): the display-unit round-trip (round to whole mph/kph, divide back) lands a
    # few mm/s under v_cruise, so SLA wins the min() by rounding error alone and relabels
    # longitudinalPlanSource as a limiter. That relabel arms ICBM's decel overshoot
    # against an ordinary cruise convergence, and the servo's own prompt freeze then
    # stores the resulting command until the prompt times out and dumps it as a SET-
    # burst. Pre-2026-07-26 this compared exactly equal and cruise won the tie; keep the
    # property explicit instead of resting it on float luck.
    was_session = self.state in ACTIVE_STATES or self.v_cap < V_CRUISE_UNSET
    hold = self.v_cap if was_session else V_CRUISE_UNSET
    self._set_state(SessionState.preActive)
    self.v_cap = float(hold)
    self.pre_active_timer = int(PRE_ACTIVE_GUARD_PERIOD / DT_CTRL)

  def _activate(self, from_prompt: bool):
    # announce when the activation resolves a confirm prompt or a walk is about to
    # happen; activating because the setpoint already matches the limit is silent
    announce = from_prompt or self._target_conv() != self._cluster_conv
    self._set_state(SessionState.active, announce=announce)

  # ---- press classification ----------------------------------------------------------
  def _classify_presses(self, CS, v_cruise_kph: float) -> float:
    """Consume button edges; decide intents from the pre-frame session snapshot.

    Returns v_cruise_kph, possibly raised by an upward confirm adoption."""
    self.adopted_this_frame = False
    if self._press:
      # releases stayed through their frame for press_owned; sweep them now
      self._press = {btn: p for btn, p in self._press.items() if not p.released}

    for b in CS.buttonEvents:
      if b.type not in PLUS_BUTTONS and b.type not in MINUS_BUTTONS:
        continue
      btn = b.type.raw

      if b.pressed:
        if self.state_prev_frame in ACTIVE_STATES:
          # a press on an active session dismisses it at the press edge; the whole
          # press is owned (its ECU step re-anchors via the reconciler, never counted
          # here). SLA re-arms on the next limit change.
          self._press[btn] = _Press(_PRESS_DISMISS)
          self._set_state(SessionState.inactive)
          self._driver_dismissed = True
          self.last_intent = CruiseIntent.dismiss
        elif self.state_prev_frame == SessionState.preActive:
          self._press[btn] = _Press(_PRESS_PROMPT)
        else:
          self._press[btn] = _Press(_PRESS_NORMAL)

      else:  # release
        press = self._press.get(btn)
        if press is None:
          continue
        if press.cls == _PRESS_PROMPT and not press.resolved:
          v_cruise_kph = self._resolve_prompt_press(btn, press, v_cruise_kph)
        if press.cls == _PRESS_NORMAL and self.last_intent == CruiseIntent.none:
          self.last_intent = CruiseIntent.increment if btn in PLUS_BUTTONS else CruiseIntent.decrement
        press.released = True

    # long-press ticks: a prompt press that reaches long-press duration resolves at the
    # first tick instead of waiting for release (the ECU is already grid-stepping)
    for btn, press in self._press.items():
      if press.released:
        continue
      press.frames += 1
      if press.cls == _PRESS_PROMPT and not press.resolved and press.frames >= LONG_PRESS_FRAMES:
        v_cruise_kph = self._resolve_prompt_press(btn, press, v_cruise_kph)

    return v_cruise_kph

  def _resolve_prompt_press(self, button: int, press: _Press, v_cruise_kph: float) -> float:
    if self.state != SessionState.preActive:
      # the prompt resolved some other way (timeout, dial-to-target) while the press was
      # in flight; treat as a plain press
      press.cls = _PRESS_NORMAL
      return v_cruise_kph

    req_plus, req_minus = compare_cluster_target(self._cluster_conv / self._conv, self._slf, self.is_metric)
    is_plus = button in PLUS_BUTTONS

    if (req_plus and is_plus) or (req_minus and not is_plus):
      # confirm. An upward confirm means "take me to the limit": raise the setpoint to
      # the target (never lower it: a baseline above the limit stays, and the active
      # session caps the plan instead).
      press.resolved = True
      self.last_intent = CruiseIntent.confirm
      if is_plus and self.target_kph > v_cruise_kph:
        v_cruise_kph = float(np.clip(round(self.target_kph, 1), get_minimum_set_speed(self.is_metric), V_CRUISE_MAX))
        self.adopted_this_frame = True
      self._activate(from_prompt=True)
    else:
      # a press against the confirm direction declines: the session ends at once so the
      # frozen hold releases and the prompt stops shadowing the driver's dialing. The
      # press still counts as a normal increment.
      press.cls = _PRESS_NORMAL
      self.last_intent = CruiseIntent.decline
      self._set_state(SessionState.inactive)

    return v_cruise_kph

  def press_owned(self, button_type: int) -> bool:
    """True when this press must not increment v_cruise (confirm- or dismiss-owned).
    Takes the raw enumerant int (as cruise.py's button paths carry); valid for release
    events and long-press repeat ticks in the same frame."""
    press = self._press.get(button_type)
    if press is None:
      return False
    if press.cls == _PRESS_DISMISS:
      return True
    return press.cls == _PRESS_PROMPT and press.resolved

  # ---- main step (card 100 Hz) -------------------------------------------------------
  def step(self, CS, long_enabled: bool, v_cruise_kph: float, v_cruise_cluster_kph: float) -> float:
    """Pinned sub-step order:
      (1) snapshot the pre-frame session state
      (2) classify press edges/ticks from the snapshot (dismiss/confirm/decline; an
          upward confirm adopts the limit into v_cruise and flags the reconcile kill)
      (3) step the session machine (engage guards, limit changes, dial-to-target,
          prompt timeout)
      (4) refresh the published cap
    The caller then runs the increment path (consulting press_owned) and the
    reconciler (consulting prompting/adopted_this_frame), in that order."""
    if not self.applicable:
      return v_cruise_kph

    self.state_prev_frame = self.state
    conv = CV.KPH_TO_MS * self._conv
    self._cluster_conv_prev = self._cluster_conv
    self._cluster_conv = round(v_cruise_cluster_kph * conv) if v_cruise_cluster_kph not in (V_CRUISE_UNSET, -1) else 0
    self.last_intent = CruiseIntent.none

    v_cruise_kph = self._classify_presses(CS, v_cruise_kph)

    self.long_engaged_timer = max(0, self.long_engaged_timer - 1)
    self.pre_active_timer = max(0, self.pre_active_timer - 1)

    if self.state != SessionState.disabled:
      if not long_enabled or not self.enabled:
        self._set_state(SessionState.disabled)
        self._driver_dismissed = False

      elif self.state in ACTIVE_STATES:
        # dismiss is handled at the press edge in classification
        if self._limit_changed and confirm_needed_for_change(self._cluster_conv, self._target_conv(), self.is_metric):
          self._enter_prompt()
        elif self._limit_changed and self._target_conv() != self._cluster_conv:
          # CST auto-apply: the new target takes over without confirmation; announce
          # only when it changes something
          self.announce_counter += 1

      elif self.state == SessionState.preActive:
        # confirm/decline are handled at release/tick in classification
        if self._target_conv() == self._cluster_conv:
          self._activate(from_prompt=True)  # dialing onto the target answers the prompt
        elif self.pre_active_timer <= 0:
          self._set_state(SessionState.inactive)

      elif self.state == SessionState.inactive:
        if self._limit_changed:
          self._driver_dismissed = False
          self._enter_prompt()
        elif not self._driver_dismissed and self._has_limit and self._target_conv() == self._cluster_conv \
             and not self._press:
          # dial-to-target latches only once the press is over: latching mid-hold would
          # cap a driver who is dialing past the limit
          self._activate(from_prompt=False)

    else:  # DISABLED
      if long_enabled and self.enabled:
        if not self.long_enabled_prev or self._cluster_conv != self._cluster_conv_prev:
          self.long_engaged_timer = int(DISABLED_GUARD_PERIOD / DT_CTRL)
        elif self.long_engaged_timer <= 0:
          if self._has_limit and self._target_conv() == self._cluster_conv:
            self._activate(from_prompt=False)
          elif self._has_limit:
            self._enter_prompt()
          else:
            self._set_state(SessionState.inactive)

    # published cap: session target while active, the frozen hold while prompting
    if self.state in ACTIVE_STATES:
      self.v_cap = float(self._slf) if self._has_limit else V_CRUISE_UNSET
    elif self.state != SessionState.preActive:  # a prompt keeps its frozen hold
      self.v_cap = V_CRUISE_UNSET

    self._speed_limit_prev = self._speed_limit
    self.long_enabled_prev = long_enabled
    return v_cruise_kph

  # ---- publishing / output gating ----------------------------------------------------
  def fill_msg(self, cs_sp) -> None:
    if not self.applicable:
      return
    session = cs_sp.cruiseSession
    session.state = self.state
    session.vCap = float(self.v_cap)
    session.lastIntent = self.last_intent
    session.announceCounter = self.announce_counter

  def gate_send_button(self, CC_SP) -> None:
    """Authoritative emission gate, called by card just before CI.apply: the servo's
    own prompt freeze is one message hop stale, so a button frame could otherwise
    escape at prompt onset."""
    if self.applicable and self.prompting:
      CC_SP.intelligentCruiseButtonManagement.sendButton = structs.IntelligentCruiseButtonManagement.SendButtonState.none
