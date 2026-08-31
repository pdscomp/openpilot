"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Per-car deceleration planning limits for the curve and limit speed planners.

A car is either openpilot-long or stock ACC for the whole drive, so the budget and the
actuation lead are picked once at init. The numbers are measured, not aspirational:
what each path can actually deliver decides where the braking point goes.
See docs/curve-and-limit-planning.md.
"""
from dataclasses import dataclass

import numpy as np

from opendbc.car import structs
from opendbc.sunnypilot.car.icbm_actuation_profile import get_actuation_profile

# openpilot long's cruise candidate is clip(v_cruise - v_ego, A_CRUISE_MIN, max) with a
# jerk clip from J_CRUISE_VALS. Mirrored from selfdrive/controls/lib/longitudinal_planner.py,
# which cannot be imported from here (it imports the SP planner overlay, which imports this
# package).
_OP_LONG_A_BUDGET = 1.2
_OP_LONG_J_BP = [0., 10., 25., 40.]
_OP_LONG_J_VALS = [1.6, 1.2, 0.8, 0.6]

# Stock ACC decelerates with the gap between the dash set speed and actual speed; the
# response saturates per brand. mazda: DECEL_OVERSHOOT_PARAMS saturation near -0.75 m/s2
# by ~9 mph of gap, from 422k hands-off cruise samples. Deeper gaps buy nothing, so a
# plan assuming more simply arrives hot.
_STOCK_A_BUDGET = {'mazda': 0.75}
# Unmeasured brands keep roughly today's effective ramp; a smaller budget only means
# braking starts earlier, which is the safe direction to be wrong in.
_STOCK_A_BUDGET_DEFAULT = 0.5
# Time from a stable lowered set speed to the ECU actually decelerating. Estimate, erring
# large (earlier braking) until measured from overshoot episodes.
_STOCK_RESPONSE_T = 1.0

_MPH_PER_MS = 2.23694

# What the servo's button stream actually moves the dash at. The wheel keeps broadcasting
# its genuine button-up frames, so forged hold frames interleave and register as discrete
# presses, never as a held button: route 126 measured 294/294 dash steps at 1 mph (zero
# grid snaps), 4.1 mph/s under hold frames and 3.8 mph/s under taps. The native 5 mph
# grid timing only applies to a physical hold and must not size the actuation lead.
_SERVO_WALK_RATE = {'mazda': 4.0}  # mph/s, measured

# Shared solver gate: a constraint binds once the decel it requires reaches this fraction
# of the budget. Below 1.0 leaves headroom for slope and curvature error; swept against
# the corpus together with the vision planning margin (see vision_controller.py).
COMMIT_FRAC = 0.7


@dataclass(frozen=True)
class PlanningLimits:
  a_budget: float  # deliverable deceleration, m/s2, positive
  t_lead: float  # fixed actuation lead, s
  op_long: bool
  # stock path only: the dash has to be walked down before the ECU sees the new set speed
  walk_rate: float = 5.  # display units per second the servo actually achieves

  def jerk(self, v_ego: float) -> float:
    """The consumer's own jerk limit easing into a_budget; 0 where the ECU self-smooths."""
    if not self.op_long:
      return 0.
    return float(np.interp(v_ego, _OP_LONG_J_BP, _OP_LONG_J_VALS))

  def dash_traversal_time(self, delta_v_ms: float) -> float:
    """Seconds of dash walking to lower the set speed by delta_v (stock path only).

    Display units are taken as mph: the measured rates are imperial-only so far, and for a
    lead estimate the ~1.6x metric error is inside the response-time uncertainty anyway.
    Uses the measured servo walk rate, not the native hold grid: synthesized holds
    register as discrete presses (see _SERVO_WALK_RATE).
    """
    if self.op_long or delta_v_ms <= 0.:
      return 0.
    return delta_v_ms * _MPH_PER_MS / max(self.walk_rate, 1.)


def get_planning_limits(CP: structs.CarParams) -> PlanningLimits:
  if CP.openpilotLongitudinalControl:
    return PlanningLimits(a_budget=_OP_LONG_A_BUDGET, t_lead=float(CP.longitudinalActuatorDelay), op_long=True)

  profile = get_actuation_profile(CP.brand)
  return PlanningLimits(a_budget=_STOCK_A_BUDGET.get(CP.brand, _STOCK_A_BUDGET_DEFAULT),
                        t_lead=_STOCK_RESPONSE_T, op_long=False,
                        walk_rate=_SERVO_WALK_RATE.get(CP.brand, profile.tap_rate_hz))
