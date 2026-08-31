"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Plannerd-side mirror of the card cruise arbiter's SLA session (non-pcm cars).

The session machine runs in card at 100 Hz, in the same frame as the buttons and the
setpoint. Plannerd only needs three things from it: the plan cap for min() source
selection, the assist state for the UI wire format, and the alert events. The mirror
reads carStateSP.cruiseSession and reproduces the exact surface SpeedLimitAssist used
to provide here, so longitudinalPlanSP consumers are unchanged.

Events: speedLimitPreActive is level-driven (the prompt alert must persist for the
whole window), speedLimitActive fires on announce-counter deltas — the counter is
bumped by card at 100 Hz and never un-bumps, so a 20 Hz reader cannot miss it.
"""
from openpilot.cereal import custom
from openpilot.selfdrive.controls.lib.drive_helpers import CONTROL_N
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.common.realtime import DT_MDL
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit import ACTIVE_STATES, ENABLED_STATES, V_CRUISE_UNSET
from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP

EventNameSP = custom.OnroadEventSP.EventName
SessionState = custom.LongitudinalPlanSP.SpeedLimit.AssistState

# Publication shaping shared by the limiter sources (see docs/curve-and-limit-planning.md)
_A_PUB_MIN = -2.0  # m/s2
_PUB_JERK = 2.0  # m/s3
# past the sign (distance 0) the decel degrades to a pull over the control horizon,
# matching SpeedLimitAssist's active-state form
_T_ACTIVE = float(ModelConstants.T_IDXS[CONTROL_N])


class SpeedLimitAssistMirror:
  pcm_op_long = False

  def __init__(self, CP, CP_SP):
    self.state = SessionState.disabled
    self.output_v_target = V_CRUISE_UNSET
    self.output_a_target = 0.
    self._a_out = 0.
    self._announce_seen: int | None = None  # sync on first update (plannerd restarts)

  @property
  def is_enabled(self) -> bool:
    return self.state in ENABLED_STATES

  @property
  def is_active(self) -> bool:
    return self.state in ACTIVE_STATES

  def update(self, session, v_ego: float, distance: float, a_ego: float, events_sp: EventsSP) -> None:
    self.state = session.state
    # The arbiter publishes vCap as a real target, a frozen hold, or V_CRUISE_UNSET —
    # never 0. A 0 can only be capnp's float default from a not-yet-received carStateSP,
    # and without this guard it would win the plan min() as a full-stop target.
    v_cap = float(session.vCap)
    self.output_v_target = v_cap if v_cap > 0.0 else V_CRUISE_UNSET

    # The decel actually required to arrive at the cap: this is what keys ICBM's overshoot
    # gap on stock ACC, so a_ego here means map limits never brake the real car. The
    # resolver's distance lives in plannerd on both paths, so compute locally; the card
    # wire stays unchanged. Ramped: the plan aTarget seeds mpc.set_cur_state.
    if self.is_active and 0.0 < v_cap < v_ego:
      d_eff = max(distance, v_ego * _T_ACTIVE)
      a_des = max((v_cap ** 2 - v_ego ** 2) / (2. * d_eff), _A_PUB_MIN)
      step = _PUB_JERK * DT_MDL
      self._a_out = min(max(a_des, self._a_out - step), self._a_out + step)
    else:
      self._a_out = a_ego
    self.output_a_target = self._a_out

    if self.state == SessionState.preActive:
      events_sp.add(EventNameSP.speedLimitPreActive)

    announce = int(session.announceCounter)
    if self._announce_seen is not None and announce != self._announce_seen:
      events_sp.add(EventNameSP.speedLimitActive)
    self._announce_seen = announce
