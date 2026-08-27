"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Lane-change smoothing: a user-paced lateral jerk limit on automatic lane changes.

Only the jerk (curvature rate) is tightened: capping lateral accel would strangle the
end-of-maneuver arrest and let the car glide past the new lane center before it can
build enough counter-curvature. The arrest itself gets proportional extra authority
(a P-pursuit on how far the command lags the model) so a gentle entry never turns
into a wheel-snap or an overshoot at the end.

Ported from StarPilot (github.com/firestar5683/StarPilot, controlsd.py); the tuning
constants and the failure evidence cited beside them are theirs, from rlog-driven
iteration on their fleet.
"""
import math

from openpilot.cereal import log
from openpilot.common.params import Params
from openpilot.common.realtime import DT_CTRL
from openpilot.selfdrive.controls.lib.drive_helpers import MAX_LATERAL_JERK, smooth_value

LaneChangeState = log.LaneChangeState

# Pace setting: 1 = gentlest (~8 s lane change), 9 = quickest (~3.6 s). The jerk factor
# comes from a sinusoidal lane-change profile (peak jerk j = pi^3 * W / T^3 for a lane
# width W crossed in time T) with 1.3x headroom.
PACE_MIN, PACE_MAX, PACE_DEFAULT = 1, 9, 5
LANE_WIDTH = 3.5      # m
PACE_T_FASTEST = 3.0  # s, the sinusoidal profile time at the (hypothetical) pace 10
PACE_T_SPAN = 5.0     # s added to the profile time going from pace 10 to pace 1
JERK_HEADROOM = 1.3

# After a smoothed lane change ends, ramp the curvature limits back to stock over this
# time so the final recenter correction is shaped instead of stepping through unclamped.
SMOOTH_RELEASE_T = 2.0

# Cap on the extra jerk factor granted while the model is unwinding lane-change
# curvature (the arrest and any correction back toward center). Entry gentleness is
# comfort, but arrest speed is a correctness constraint: a slow symmetric cap lets the
# car glide past the new lane center. 0.6 (~ pace-5 rate) fully tracks the arrest
# demand seen in logs, 40% below stock.
ARREST_JERK_FLOOR = 0.6
# The extra unwind authority is proportional to how far the command lags the model
# (rate = lag/tau, a P-pursuit), NOT a fixed fast rate: a boolean-gated floor engages
# as a bang-bang switch right at the maneuver crest — where the slow entry command
# meets a model that already peaked and is diving — snapping the wheel ~2 deg in
# 0.2 s. With pursuit, lag ~0 at the crest so the rate crosses zero smoothly, and
# noise-scale lag inside the deadband gets no boost (kills the fast-down/slow-up
# sawtooth).
ARREST_PURSUIT_TAU = 0.2    # s
ARREST_GAP_DEADBAND = 5e-5  # 1/m
ARREST_RISE_TAU = 0.2


def read_pace(params) -> int:
  """The clamped pace setting — the single sanitization point for every consumer
  (controller, desire helper, settings badges)."""
  return min(max(int(params.get("LaneChangeSmoothingPace", return_default=True)), PACE_MIN), PACE_MAX)


def pace_profile_time(pace: int) -> float:
  """Target lane-change duration for a pace setting (sinusoidal profile)."""
  return PACE_T_FASTEST + (10 - pace) * PACE_T_SPAN / 9.0


def pace_jerk_factor(pace: int) -> float:
  t_target = pace_profile_time(pace)
  j_req = (math.pi ** 3) * LANE_WIDTH / (t_target ** 3)
  return min(1.0, j_req * JERK_HEADROOM / MAX_LATERAL_JERK)


def lane_change_time_extra(pace: int) -> float:
  """Seconds added to DesireHelper's lane-change timeout, so a gentle maneuver is not
  aborted mid-change by the stock cap."""
  return (10 - pace) * 2.0 / 9.0


class LaneChangeSmoothing:
  """Stateful jerk-factor source for clip_curvature during automatic lane changes."""

  def __init__(self):
    self.params = Params()
    self.enabled = False
    self.set_jerk = 1.0
    self.smooth_release = 0.0
    self.entry_sign = 0.0
    self.arrest_jerk_factor = 1.0
    self.get_params()

  def get_params(self) -> None:
    self.enabled = self.params.get_bool("LaneChangeSmoothing")
    self.set_jerk = pace_jerk_factor(read_pace(self.params))

  def reset(self) -> None:
    self.smooth_release = 0.0
    self.entry_sign = 0.0
    self.arrest_jerk_factor = 1.0

  def update(self, CS, model_v2, new_desired_curvature: float, prev_desired_curvature: float) -> float:
    """Returns the jerk factor for clip_curvature (1.0 = stock limits)."""
    if not self.enabled:
      self.reset()
      return 1.0

    jerk_factor = 1.0
    in_lane_change = model_v2.meta.laneChangeState in (LaneChangeState.laneChangeStarting,
                                                       LaneChangeState.laneChangeFinishing)
    # Hold the tight jerk limit for the whole maneuver, then taper back to stock so the
    # model's recenter step and mid-change corrections stay shaped instead of passing
    # through a mostly-relaxed clamp.
    if in_lane_change:
      self.smooth_release = SMOOTH_RELEASE_T
      if self.entry_sign == 0.0 and abs(new_desired_curvature - prev_desired_curvature) > 2e-4:
        self.entry_sign = math.copysign(1.0, new_desired_curvature - prev_desired_curvature)
    else:
      self.smooth_release = max(self.smooth_release - DT_CTRL, 0.0)
      if self.smooth_release <= 0.0:
        self.entry_sign = 0.0
    if self.smooth_release > 0.0:
      release = 1.0 - self.smooth_release / SMOOTH_RELEASE_T  # 0 in maneuver -> 1 after
      jerk_factor = self.set_jerk + (1.0 - self.set_jerk) * release
      step = new_desired_curvature - prev_desired_curvature
      model_unwinding = self.entry_sign != 0.0 and abs(step) > ARREST_GAP_DEADBAND and \
          math.copysign(1.0, step) == -self.entry_sign
      if model_unwinding:
        v_lim = max(CS.vEgo, 1.0)
        gap = max(abs(step) - ARREST_GAP_DEADBAND, 0.0)
        jf_gap = (gap / ARREST_PURSUIT_TAU) * v_lim ** 2 / MAX_LATERAL_JERK
        arrest_cap = ARREST_JERK_FLOOR + (1.0 - ARREST_JERK_FLOOR) * release
        jerk_factor = max(jerk_factor, min(arrest_cap, jerk_factor + jf_gap))
      if jerk_factor > self.arrest_jerk_factor:
        jerk_factor = float(smooth_value(jerk_factor, self.arrest_jerk_factor, ARREST_RISE_TAU, DT_CTRL))
    self.arrest_jerk_factor = jerk_factor
    return jerk_factor
