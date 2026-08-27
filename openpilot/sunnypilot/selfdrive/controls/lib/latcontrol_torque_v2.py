"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
import math
import numpy as np
from collections import deque

from openpilot.cereal import log
from opendbc.car.lateral import get_friction
from openpilot.common.constants import ACCELERATION_DUE_TO_GRAVITY
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.swaglog import cloudlog
from openpilot.selfdrive.controls.lib.drive_helpers import MIN_SPEED

from openpilot.sunnypilot.selfdrive.controls.lib.latcontrol_torque_v0 import (
  LatControlTorque as LatControlTorqueV0,
  FRICTION_THRESHOLD,
  LP_FILTER_CUTOFF_HZ,
)

# v2 keeps v0's error correction in lateral acceleration space and its extension
# boundary (speed-dependent torque owns the feedforward params) unchanged. It
# reworks the setpoint path, the low-speed error gain, the friction input shaping,
# and the integrator policy around them.

VERSION = 2

# distinct from opendbc.car.lateral.MAX_LATERAL_JERK and drive_helpers.MAX_LATERAL_JERK,
# which are curvature rate limits — this one only clips the setpoint-lead jerk
MAX_SETPOINT_LATERAL_JERK = 2.5  # m/s^3
UNWIND_JERK_THRESHOLD = -1.0  # m/s^3, setpoint rate below this while near zero means the plan is unwinding
UNWIND_LAT_ACCEL_NEAR_ZERO = 0.3  # m/s^2
MIN_LATERAL_CONTROL_SPEED = 0.3  # m/s
STEER_RELEASE_I_DECAY = 0.8  # one-shot integrator decay on steering-press release

# StarPilot's low-speed error boost (their LOW_SPEED_X/Y, verbatim): the PID error is
# scaled by 1 + lsf/kp, ~+45% at 5 m/s fading to ~+3% at 30, closing low-speed tracking
# error faster than the KP ladder alone. Normalizing by the scheduled KP keeps the added
# proportional authority roughly absolute across the ladder instead of compounding with
# it. Applied to the PID error only — the friction input keeps the unboosted error, so
# the stiction kick is unchanged (replay-validated orthogonal, 2026-08-27).
LOW_SPEED_X = [0, 10, 20, 30]  # m/s
LOW_SPEED_Y = [12, 10.5, 8, 5]

# Roll compensation and latAccelOffset are lateral-accel-domain corrections; below
# walking pace the desired lateral accel is ~0, so an unfaded road-crown term dominates
# the whole feedforward and actively unwinds a held wheel at pull-away.
FF_ROLL_OFFSET_FADE_BP = [0.5, 2.5]  # m/s
FF_ROLL_OFFSET_FADE_V = [0.0, 1.0]

# Small planner jerk changes around the lane center can repeatedly re-trigger the
# friction compensation term. Keep this correction out of the center band while
# leaving actual turn-in and unwind commands unchanged.
CENTER_CHATTER_JERK_DEADZONE_SPEED_BP = [0.0, 5.0, 12.0, 25.0]  # m/s
CENTER_CHATTER_JERK_DEADZONE_SPEED_V = [0.08, 0.12, 0.18, 0.18]  # m/s^3
CENTER_CHATTER_JERK_DEADZONE_LAT_ACCEL_BP = [0.0, 0.18, 0.35]  # m/s^2
CENTER_CHATTER_JERK_DEADZONE_LAT_ACCEL_V = [1.0, 1.0, 0.0]


def get_center_chatter_jerk_deadzone(v_ego, setpoint):
  """Small-signal jerk deadzone for the friction input (see the constants above)."""
  center_weight = np.interp(abs(setpoint), CENTER_CHATTER_JERK_DEADZONE_LAT_ACCEL_BP,
                            CENTER_CHATTER_JERK_DEADZONE_LAT_ACCEL_V)
  if center_weight == 0.0:  # in a real turn most of the time: skip the second interp
    return 0.0
  speed_deadzone = np.interp(max(v_ego, 0.0), CENTER_CHATTER_JERK_DEADZONE_SPEED_BP,
                             CENTER_CHATTER_JERK_DEADZONE_SPEED_V)
  return float(speed_deadzone * center_weight)


class LatControlTorque(LatControlTorqueV0):
  def __init__(self, CP, CP_SP, CI, dt):
    super().__init__(CP, CP_SP, CI, dt)
    # Stores CURVATURE, scaled by the current v^2 on read — buffered lateral accel keeps the
    # old speed's v^2 and reads as phantom tracking error whenever speed changes in the delay
    # window. Same length as v0's (unused here) buffer so the delay clamp stays valid.
    self.curvature_request_buffer = deque([0.] * self.lat_accel_request_buffer_len, maxlen=self.lat_accel_request_buffer_len)
    self.jerk_filter = FirstOrderFilter(0.0, 1 / (2 * np.pi * LP_FILTER_CUTOFF_HZ), self.dt)
    self.low_speed_pid_threshold = max(CP.minSteerSpeed, MIN_LATERAL_CONTROL_SPEED)
    self.prev_steering_pressed = False
    self.prev_setpoint = 0.0

    # The extension's override controllers (jerk-aware, NNLC) recompute error/feedforward/
    # friction in torque space and step the shared PID themselves, silently replacing v2's
    # friction shaping and integrator policy while the setpoint changes still flow through.
    # Disable them regardless of the params; NNLC is already excluded structurally (enabling
    # it disables EnforceTorqueControl, which forces v0) — this covers the params disagreeing.
    cloudlog.info("LatControlTorque v2: extension output overrides (jerk-aware/NNLC) disabled")
    self.extension.disable_output_overrides()
    self.update_limits()  # an override controller may have retuned the shared PID to torque-space limits

  def update(self, active, CS, VM, params, steer_limited_by_safety, desired_curvature, calibrated_pose, curvature_limited, lat_delay):
    # Override torque params from extension
    if self.extension.update_override_torque_params(self.torque_params):
      self.update_limits()

    pid_log = log.ControlsState.LateralTorqueState.new_message()
    pid_log.version = VERSION

    measured_curvature = -VM.calc_curvature(math.radians(CS.steeringAngleDeg - params.angleOffsetDeg), CS.vEgo, params.roll)
    measurement = measured_curvature * CS.vEgo ** 2
    future_desired_lateral_accel = desired_curvature * CS.vEgo ** 2

    if not active:
      output_torque = 0.0
      pid_log.active = False
      # Keep the request buffer and rate state primed with the live command instead of
      # letting them go stale. Re-engaging with a wound wheel against a stale buffer puts
      # the setpoint ~lat_delay behind the measurement, and the low-speed gains turn that
      # lag into an unwind shove. The integrator is deliberately NOT cleared here: MADS
      # cycles lateral often, and the release decay + unwind freeze below replace a blunt
      # reset.
      self.curvature_request_buffer.append(desired_curvature)
      self.previous_measurement = measurement
      self.measurement_rate_filter.x = 0.0
      self.jerk_filter.x = 0.0
      self.prev_setpoint = future_desired_lateral_accel
    else:
      # One-shot integrator decay on steering-press release, so hand-back after an
      # override is neutral instead of a kick
      if self.prev_steering_pressed and not CS.steeringPressed:
        self.pid.i *= STEER_RELEASE_I_DECAY

      roll_offset_fade = float(np.interp(CS.vEgo, FF_ROLL_OFFSET_FADE_BP, FF_ROLL_OFFSET_FADE_V))
      roll_compensation = params.roll * ACCELERATION_DUE_TO_GRAVITY * roll_offset_fade
      curvature_deadzone = abs(VM.calc_curvature(math.radians(self.steering_angle_deadzone_deg), CS.vEgo, 0.0))
      lateral_accel_deadzone = curvature_deadzone * CS.vEgo ** 2

      delay_frames = int(np.clip(lat_delay / self.dt, 1, self.lat_accel_request_buffer_len))
      expected_lateral_accel = self.curvature_request_buffer[-delay_frames] * CS.vEgo ** 2
      self.curvature_request_buffer.append(desired_curvature)
      gravity_adjusted_future_lateral_accel = future_desired_lateral_accel - roll_compensation

      raw_lateral_jerk = (future_desired_lateral_accel - expected_lateral_accel) / max(lat_delay, self.dt)
      raw_lateral_jerk = min(max(raw_lateral_jerk, -MAX_SETPOINT_LATERAL_JERK), MAX_SETPOINT_LATERAL_JERK)
      # the filter is a convex combination of clipped inputs, so its output needs no second clip
      desired_lateral_jerk = self.jerk_filter.update(raw_lateral_jerk)

      # first-order lead: delayed request plus one lat_delay of planned jerk, so turn-in
      # starts a steering delay early instead of chasing the request. The clip and low-pass
      # above keep a lagd mis-estimate from over-leading the setpoint. With the plan holding
      # still the jerk term is zero and this collapses back to v0's setpoint algebra.
      setpoint = expected_lateral_accel + desired_lateral_jerk * lat_delay

      measurement_rate = self.measurement_rate_filter.update((measurement - self.previous_measurement) / self.dt)
      self.previous_measurement = measurement

      # Freeze the integrator while the plan is unwinding through center: integrating the
      # transient error there is what sticks the wheel past straight
      setpoint_rate = (setpoint - self.prev_setpoint) / self.dt
      unwind_detected = setpoint_rate < UNWIND_JERK_THRESHOLD and abs(setpoint) < UNWIND_LAT_ACCEL_NEAR_ZERO
      self.prev_setpoint = setpoint

      error = setpoint - measurement

      # low-speed error boost (see the constants above); interp the schedule directly
      # rather than reading pid.k_p, which still holds the previous frame's speed here
      low_speed_factor = (np.interp(CS.vEgo, LOW_SPEED_X, LOW_SPEED_Y) / max(CS.vEgo, MIN_SPEED)) ** 2
      current_kp = np.interp(CS.vEgo, self.pid._k_p[0], self.pid._k_p[1])
      error *= 1.0 + low_speed_factor / max(current_kp, 1e-3)

      # do error correction in lateral acceleration space, convert at end to handle non-linear torque responses correctly
      pid_log.error = float(error)
      ff = gravity_adjusted_future_lateral_accel
      # latAccelOffset corrects roll compensation bias from device roll misalignment relative
      # to car roll; it fades with the roll compensation it corrects
      ff -= self.torque_params.latAccelOffset * roll_offset_fade

      # The friction term sees the deadzoned jerk in place of the raw jerk contribution, so
      # tiny planner wobble at lane center cannot sign-flip it; the PID error above is unchanged
      friction_jerk_deadzone = get_center_chatter_jerk_deadzone(CS.vEgo, setpoint)
      friction_jerk = math.copysign(max(abs(desired_lateral_jerk) - friction_jerk_deadzone, 0.0), desired_lateral_jerk)
      friction_error = expected_lateral_accel + friction_jerk * lat_delay - measurement
      ff += get_friction(friction_error, lateral_accel_deadzone, FRICTION_THRESHOLD, self.torque_params)

      # Keyed to max(minSteerSpeed, 0.3) instead of v0's vEgo < 5: the integrator keeps
      # working at creep speeds, where controlsd only allows lateral on steer-at-standstill
      # cars anyway
      if CS.vEgo < self.low_speed_pid_threshold:
        self.pid.reset()
      freeze_integrator = (steer_limited_by_safety or CS.steeringPressed or
                           CS.vEgo < self.low_speed_pid_threshold or unwind_detected)
      if self.extension.overrides_output:
        # Unreachable while __init__ disables the output overrides; kept as the guard
        # against a future override controller double-integrating the shared PID.
        output_torque = 0.0
      else:
        output_lataccel = self.pid.update(pid_log.error,
                                         -measurement_rate,
                                          feedforward=ff,
                                          speed=CS.vEgo,
                                          freeze_integrator=freeze_integrator)
        output_torque = self.torque_from_lateral_accel(output_lataccel, self.torque_params)

      # Lateral acceleration torque controller extension updates
      # Overrides pid_log.error and output_torque
      pid_log, output_torque = self.extension.update(CS, VM, self.pid, params, ff, pid_log, setpoint, measurement, calibrated_pose, roll_compensation,
                                                     future_desired_lateral_accel, measurement, lateral_accel_deadzone, gravity_adjusted_future_lateral_accel,
                                                     desired_curvature, measured_curvature, steer_limited_by_safety, output_torque)

      pid_log.active = True
      pid_log.p = float(self.pid.p)
      pid_log.i = float(self.pid.i)
      pid_log.d = float(self.pid.d)
      pid_log.f = float(self.pid.f)
      pid_log.output = float(-output_torque)
      pid_log.actualLateralAccel = float(measurement)
      pid_log.desiredLateralAccel = float(setpoint)
      pid_log.desiredLateralJerk = float(desired_lateral_jerk)
      pid_log.saturated = bool(self._check_saturation(self.steer_max - abs(output_torque) < 1e-3, CS, steer_limited_by_safety, curvature_limited))

    self.prev_steering_pressed = CS.steeringPressed

    # TODO left is positive in this convention
    return -output_torque, 0.0, pid_log
