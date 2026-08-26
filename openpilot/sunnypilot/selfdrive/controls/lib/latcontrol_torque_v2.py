"""
Torque controller v2: latcontrol_torque_v0 plus lateral-oscillation dampers.

The four dampers are ported from firestar5683's StarPilot v2 torque controller
(generic path only; StarPilot's per-brand tuned paths are NOT ported):
  https://github.com/firestar5683/StarPilot/blob/StarPilot/selfdrive/controls/lib/latcontrol_torque.py
  history: https://github.com/firestar5683/StarPilot/commits/StarPilot/selfdrive/controls/lib/latcontrol_torque.py
StarPilot is MIT-licensed (fork of FrogAi/FrogPilot); dampers credited to firestar5683.

  1. Setpoint jerk clamp: desired lateral jerk is low-pass filtered and clipped to
     MAX_LAT_JERK_UP, so the commanded lateral acceleration can't change faster than
     the EPS can actually slew torque. Kills command-side overshoot/ringing in curves.
  2. Measurement-rate clamp: same clip on the filtered measurement derivative fed to
     the PID error-rate path. Kills D-path spikes from road noise.
  3. Integrator decay on driver-override release (STEER_RELEASE_I_DECAY):
     no snap-back when the driver lets go.
  4. Unwind freeze: integrator freezes while the setpoint unwinds through near-zero
     accel, stopping the I-term pumping back and forth through center.

Gains are unchanged from v0 (KP 1.0 / KI 0.3) — the dampers are the experiment.
Note: v0's measurement-rate filter constant (LP_FILTER_CUTOFF_HZ) is kept rather than
StarPilot's (1/(2*pi*(MAX_LAT_JERK_UP-0.5))) — the clip is the damper, and our
measurement path was already tuned around the 1.2 Hz filter.
"""
import math
import numpy as np
from collections import deque

from openpilot.cereal import log
from opendbc.car.lateral import get_friction
from openpilot.common.constants import ACCELERATION_DUE_TO_GRAVITY
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.selfdrive.controls.lib.latcontrol import LatControl
from openpilot.common.pid import PIDController

from openpilot.sunnypilot.selfdrive.controls.lib.latcontrol_torque_ext import LatControlTorqueExt

# At higher speeds (25+mph) we can assume:
# Lateral acceleration achieved by a specific car correlates to
# torque applied to the steering rack. It does not correlate to
# wheel slip, or to speed.

# This controller applies torque to achieve desired lateral
# accelerations. To compensate for the low speed effects the
# proportional gain is increased at low speeds by the PID controller.
# Additionally, there is friction in the steering wheel that needs
# to be overcome to move it at all, this is compensated for too.

KP = 1.0
KI = 0.3
KD = 0.0
INTERP_SPEEDS = [1, 1.5, 2.0, 3.0, 5, 7.5, 10, 15, 30]
KP_INTERP = [250, 120, 65, 30, 11.5, 5.5, 3.5, 2.0, KP]

LP_FILTER_CUTOFF_HZ = 1.2
LAT_ACCEL_REQUEST_BUFFER_SECONDS = 1.0
FRICTION_THRESHOLD = 0.3
VERSION = 2

# StarPilot v2 damper constants (firestar5683) — see module docstring
MAX_LAT_JERK_UP = 2.5            # m/s^3, clip for desired jerk and measurement rate
JERK_GAIN = 0.22                 # friction input anticipates this much filtered jerk
STEER_RELEASE_I_DECAY = 0.8      # integrator kept after driver-override release
UNWIND_D_DES_THRESHOLD = -1.0    # m/s^2/s, setpoint unwinding faster than this...
UNWIND_LAT_ACCEL_NEAR_ZERO = 0.3 # ...while within this of zero accel = freeze integrator


class LatControlTorque(LatControl):
  def __init__(self, CP, CP_SP, CI, dt):
    super().__init__(CP, CP_SP, CI, dt)
    self.torque_params = CP.lateralTuning.torque.as_builder()
    self.torque_from_lateral_accel = CI.torque_from_lateral_accel()
    self.lateral_accel_from_torque = CI.lateral_accel_from_torque()
    self.pid = PIDController([INTERP_SPEEDS, KP_INTERP], KI, KD, rate=1/self.dt)
    self.update_limits()
    self.steering_angle_deadzone_deg = self.torque_params.steeringAngleDeadzoneDeg
    self.lat_accel_request_buffer_len = int(LAT_ACCEL_REQUEST_BUFFER_SECONDS / self.dt)
    self.lat_accel_request_buffer = deque([0.] * self.lat_accel_request_buffer_len , maxlen=self.lat_accel_request_buffer_len)
    self.previous_measurement = 0.0
    self.measurement_rate_filter = FirstOrderFilter(0.0, 1 / (2 * np.pi * LP_FILTER_CUTOFF_HZ), self.dt)

    # damper state
    self.jerk_filter = FirstOrderFilter(0.0, 1 / (2 * np.pi * LP_FILTER_CUTOFF_HZ), self.dt)
    self.prev_desired_lateral_accel = 0.0
    self.prev_steering_pressed = False

    self.extension = LatControlTorqueExt(self, CP, CP_SP, CI)
    self.update_limits()  # the __init__ call above ran before the extension existed

  def update_torque_parameters(self, latAccelFactor, latAccelOffset, friction):
    self.torque_params.latAccelFactor = latAccelFactor
    self.torque_params.latAccelOffset = latAccelOffset
    self.torque_params.friction = friction
    self.update_limits()

  def update_limits(self):
    self.pid.set_limits(self.lateral_accel_from_torque(self.steer_max, self.torque_params),
                        self.lateral_accel_from_torque(-self.steer_max, self.torque_params))
    # torque-space extension controllers need +-steer_max instead; re-assert on every reset
    # path (live params, per-frame override) or they run with lat-accel-space limits.
    # hasattr: the first call happens in __init__ before the extension exists
    if hasattr(self, 'extension'):
      self.extension.update_limits()

  def update(self, active, CS, VM, params, steer_limited_by_safety, desired_curvature, calibrated_pose, curvature_limited, lat_delay):
    # Override torque params from extension
    if self.extension.update_override_torque_params(self.torque_params):
      self.update_limits()

    pid_log = log.ControlsState.LateralTorqueState.new_message()
    pid_log.version = VERSION
    if not active:
      output_torque = 0.0
      pid_log.active = False
      # damper state must not go stale while inactive, or re-engagement transients
      # hit the clamps and feel like a dead zone
      self.jerk_filter.x = 0.0
      self.measurement_rate_filter.x = 0.0
      self.previous_measurement = 0.0
      self.prev_desired_lateral_accel = 0.0
      self.lat_accel_request_buffer = deque([0.] * self.lat_accel_request_buffer_len, maxlen=self.lat_accel_request_buffer_len)
    else:
      # damper 3: decay the integrator on driver-override release (no snap-back)
      if self.prev_steering_pressed and not CS.steeringPressed:
        self.pid.i *= STEER_RELEASE_I_DECAY

      measured_curvature = -VM.calc_curvature(math.radians(CS.steeringAngleDeg - params.angleOffsetDeg), CS.vEgo, params.roll)
      roll_compensation = params.roll * ACCELERATION_DUE_TO_GRAVITY
      curvature_deadzone = abs(VM.calc_curvature(math.radians(self.steering_angle_deadzone_deg), CS.vEgo, 0.0))
      lateral_accel_deadzone = curvature_deadzone * CS.vEgo ** 2

      delay_frames = int(np.clip(lat_delay / self.dt, 1, self.lat_accel_request_buffer_len))
      expected_lateral_accel = self.lat_accel_request_buffer[-delay_frames]
      future_desired_lateral_accel = desired_curvature * CS.vEgo ** 2
      self.lat_accel_request_buffer.append(future_desired_lateral_accel)
      gravity_adjusted_future_lateral_accel = future_desired_lateral_accel - roll_compensation

      # damper 1: filter + clamp desired jerk so the setpoint can't whip (StarPilot v2)
      raw_lateral_jerk = (future_desired_lateral_accel - expected_lateral_accel) / max(lat_delay, self.dt)
      raw_lateral_jerk = np.clip(raw_lateral_jerk, -MAX_LAT_JERK_UP, MAX_LAT_JERK_UP)
      desired_lateral_jerk = np.clip(self.jerk_filter.update(raw_lateral_jerk), -MAX_LAT_JERK_UP, MAX_LAT_JERK_UP)

      measurement = measured_curvature * CS.vEgo ** 2
      # damper 2: clamp the measurement derivative used by the PID error-rate path
      measurement_rate = self.measurement_rate_filter.update((measurement - self.previous_measurement) / self.dt)
      measurement_rate = np.clip(measurement_rate, -MAX_LAT_JERK_UP, MAX_LAT_JERK_UP)
      self.previous_measurement = measurement

      setpoint = lat_delay * desired_lateral_jerk + expected_lateral_accel

      # damper 4: detect setpoint unwinding through near-zero accel (anti center-pumping)
      desired_lateral_accel_rate = (setpoint - self.prev_desired_lateral_accel) / self.dt
      unwind_detected = (desired_lateral_accel_rate < UNWIND_D_DES_THRESHOLD and
                         abs(setpoint) < UNWIND_LAT_ACCEL_NEAR_ZERO)
      self.prev_desired_lateral_accel = setpoint

      error = setpoint - measurement

      # do error correction in lateral acceleration space, convert at end to handle non-linear torque responses correctly
      pid_log.error = float(error)
      ff = gravity_adjusted_future_lateral_accel
      # latAccelOffset corrects roll compensation bias from device roll misalignment relative to car roll
      ff -= self.torque_params.latAccelOffset
      # friction anticipates filtered jerk (StarPilot v2 JERK_GAIN)
      ff += get_friction(error + JERK_GAIN * desired_lateral_jerk, lateral_accel_deadzone, FRICTION_THRESHOLD, self.torque_params)

      freeze_integrator = steer_limited_by_safety or CS.steeringPressed or CS.vEgo < 5 or unwind_detected
      if self.extension.overrides_output:
        # the extension runs its own torque-space pid.update on the shared PID; a stock
        # update here would also integrate the lat-accel-space error into the integrator
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
      pid_log.output = float(-output_torque) # TODO: log lat accel?
      pid_log.actualLateralAccel = float(measurement)
      pid_log.desiredLateralAccel = float(setpoint)
      pid_log.desiredLateralJerk = float(desired_lateral_jerk)
      pid_log.saturated = bool(self._check_saturation(self.steer_max - abs(output_torque) < 1e-3, CS, steer_limited_by_safety, curvature_limited))

    self.prev_steering_pressed = CS.steeringPressed

    # TODO left is positive in this convention
    return -output_torque, 0.0, pid_log
