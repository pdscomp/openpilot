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
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit import ACTIVE_STATES, ENABLED_STATES, V_CRUISE_UNSET
from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP

EventNameSP = custom.OnroadEventSP.EventName
SessionState = custom.LongitudinalPlanSP.SpeedLimit.AssistState


class SpeedLimitAssistMirror:
  pcm_op_long = False

  def __init__(self, CP, CP_SP):
    self.state = SessionState.disabled
    self.output_v_target = V_CRUISE_UNSET
    self.output_a_target = 0.
    self._announce_seen: int | None = None  # sync on first update (plannerd restarts)

  @property
  def is_enabled(self) -> bool:
    return self.state in ENABLED_STATES

  @property
  def is_active(self) -> bool:
    return self.state in ACTIVE_STATES

  def update(self, session, a_ego: float, events_sp: EventsSP) -> None:
    self.state = session.state
    # The arbiter publishes vCap as a real target, a frozen hold, or V_CRUISE_UNSET —
    # never 0. A 0 can only be capnp's float default from a not-yet-received carStateSP,
    # and without this guard it would win the plan min() as a full-stop target.
    v_cap = float(session.vCap)
    self.output_v_target = v_cap if v_cap > 0.0 else V_CRUISE_UNSET
    self.output_a_target = a_ego

    if self.state == SessionState.preActive:
      events_sp.add(EventNameSP.speedLimitPreActive)

    announce = int(session.announceCounter)
    if self._announce_seen is not None and announce != self._announce_seen:
      events_sp.add(EventNameSP.speedLimitActive)
    self._announce_seen = announce
