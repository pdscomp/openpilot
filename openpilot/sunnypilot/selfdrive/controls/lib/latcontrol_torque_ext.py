"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

import numpy as np

from openpilot.sunnypilot.selfdrive.controls.lib.nnlc.nnlc import NeuralNetworkLateralControl
from openpilot.sunnypilot.selfdrive.controls.lib.latcontrol_torque_ext_override import LatControlTorqueExtOverride


class LatControlTorqueExt(NeuralNetworkLateralControl, LatControlTorqueExtOverride):
  def __init__(self, lac_torque, CP, CP_SP, CI):
    NeuralNetworkLateralControl.__init__(self, lac_torque, CP, CP_SP, CI)
    LatControlTorqueExtOverride.__init__(self, CP)
    self._output_overrides_disabled = False

  def disable_output_overrides(self):
    """Permanently neutralize the override controllers (jerk-aware, NNLC, and any future
    sibling) for a host controller that owns its own friction shaping and integrator
    policy. Speed-dependent torque (update_override_torque_params) is unaffected. The
    caller must re-run the host's update_limits(): an override controller may already
    have retuned the shared PID to torque-space limits at construction."""
    self._output_overrides_disabled = True

  @property
  def overrides_output(self) -> bool:
    return not self._output_overrides_disabled and super().overrides_output

  def update_limits(self):
    # the extension's only limit work is the override controllers' torque-space retune
    if self._output_overrides_disabled:
      return
    super().update_limits()

  def update(self, CS, VM, pid, params, ff, pid_log, setpoint, measurement, calibrated_pose, roll_compensation,
             desired_lateral_accel, actual_lateral_accel, lateral_accel_deadzone, gravity_adjusted_lateral_accel,
             desired_curvature, actual_curvature, steer_limited_by_safety, output_torque):
    # Store vEgo for update_override_torque_params (which runs before this, next frame)
    self._last_vego = CS.vEgo
    self._ff = ff
    self._pid = pid
    self._pid_log = pid_log
    self._setpoint = setpoint
    self._measurement = measurement
    self._roll_compensation = roll_compensation
    self._lateral_accel_deadzone = lateral_accel_deadzone
    self._desired_lateral_accel = desired_lateral_accel
    self._actual_lateral_accel = actual_lateral_accel
    self._desired_curvature = desired_curvature
    self._actual_curvature = actual_curvature
    self._gravity_adjusted_lateral_accel = gravity_adjusted_lateral_accel
    self._steer_limited_by_safety = steer_limited_by_safety
    self._output_torque = output_torque

    if self._output_overrides_disabled:
      return self._pid_log, self._output_torque

    self.update_calculations(CS, VM, desired_lateral_accel)
    self.update_jerk_aware_torque_control(CS, roll_compensation, gravity_adjusted_lateral_accel)
    self.update_neural_network_feedforward(CS, params, calibrated_pose)

    return self._pid_log, self._output_torque

  def disable_speed_dep_torque(self):
    """The single speed-dep deactivation path. Restores the CP tune so the controller
    doesn't keep running on the last interpolated values forever — matching what
    upstream does when useParams is false (live params simply stop applying)."""
    if not self._speed_dep_active:
      return
    self._speed_dep_active = False
    tune = self.CP.lateralTuning.torque
    self.lac_torque.torque_params.latAccelFactor = tune.latAccelFactor
    self.lac_torque.torque_params.latAccelOffset = tune.latAccelOffset
    self.lac_torque.torque_params.friction = tune.friction
    self.lac_torque.update_limits()

  def update_speed_dep_torque(self, tp):
    """Apply speed-dependent learned values from torqued.
    Uses learned values for valid bins. For invalid bins, falls back to
    TOML seed values if available for this car, otherwise global filtered.
    A message with useParams off or no bins deactivates uniformly — both are
    "torqued no longer stands behind these values" and must not leave the
    controller on stale tables (useParams flips off mid-drive when the driver
    enables the manual override)."""
    if not tp.useParams or not tp.speedBinCenters:
      self.disable_speed_dep_torque()
      return
    speed_bp = list(tp.speedBinCenters)

    factors = list(tp.speedBinLatAccelFactors)
    frictions = list(tp.speedBinFrictions)
    valid_bp = list(tp.speedBinValid)

    if self._speed_dep_car_cfg is None:
      from opendbc.sunnypilot.car.interfaces import get_speed_dep_config_for_car
      self._speed_dep_car_cfg = get_speed_dep_config_for_car(self.CP)
    cfg = self._speed_dep_car_cfg
    seed_lafs = cfg.get('laf_bp')
    seed_frictions = cfg.get('friction_bp')
    if (seed_lafs and seed_frictions and
        len(seed_lafs) == len(speed_bp) and len(seed_frictions) == len(speed_bp)):
      fallback_factors = seed_lafs
      fallback_frictions = seed_frictions
    else:
      global_factor = tp.latAccelFactorFiltered
      global_fric = tp.frictionCoefficientFiltered
      fallback_factors = [global_factor] * len(speed_bp)
      fallback_frictions = [global_fric] * len(speed_bp)

    self._speed_dep_active = True
    self._speed_dep_speed_bp = speed_bp
    self._speed_dep_lat_accel_factor_bp = [factors[i] if valid_bp[i] else fallback_factors[i] for i in range(len(speed_bp))]
    self._speed_dep_friction_bp = [frictions[i] if valid_bp[i] else fallback_frictions[i] for i in range(len(speed_bp))]

    # Per-count LAF table for platforms with a speed-dependent STEER_MAX (see the
    # per-frame interp in update_override_torque_params). Learned and seed values alike
    # were measured under this car's schedule, so one conversion covers both. Rebuilt on
    # every torqued message: bin validity flips move values between learned and fallback.
    schedule = cfg.get('steer_max_schedule')
    self._speed_dep_steer_max_schedule = schedule
    if schedule:
      sm_bp, sm_v = schedule
      self._speed_dep_laf_per_count_bp = [laf / float(np.interp(c, sm_bp, sm_v))
                                          for laf, c in zip(self._speed_dep_lat_accel_factor_bp, speed_bp, strict=True)]
    else:
      self._speed_dep_laf_per_count_bp = []

    # Set global filtered values for PID limits baseline. Per-frame speed-dep
    # interpolation in update_override_torque_params overwrites on next frame.
    self.lac_torque.torque_params.latAccelFactor = tp.latAccelFactorFiltered
    self.lac_torque.torque_params.latAccelOffset = tp.latAccelOffsetFiltered
    self.lac_torque.torque_params.friction = tp.frictionCoefficientFiltered
    self.lac_torque.update_limits()
