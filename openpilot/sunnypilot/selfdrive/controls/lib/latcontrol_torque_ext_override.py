"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

import numpy as np

from openpilot.common.params import Params


class LatControlTorqueExtOverride:
  def __init__(self, CP):
    self.CP = CP
    self.params = Params()
    self.enforce_torque_control_toggle = self.params.get_bool("EnforceTorqueControl")  # only during init
    self.torque_override_enabled = self.params.get_bool("TorqueParamsOverrideEnabled")
    self.frame = -1
    # cached at the 3 s poll below; preloaded so the values are valid from frame 0
    self._override_lat_accel_factor = float(self.params.get("TorqueParamsOverrideLatAccelFactor", return_default=True))
    self._override_friction = float(self.params.get("TorqueParamsOverrideFriction", return_default=True))

    # Speed-dep state (set by LatControlTorqueExt subclass)
    self._speed_dep_active = False
    self._speed_dep_speed_bp = []
    self._speed_dep_lat_accel_factor_bp = []
    self._speed_dep_friction_bp = []
    # Per-count LAF interp (platforms with a speed-dependent STEER_MAX): the schedule is
    # (speed_bp, steer_max_v) from the car's speed-dep config, and the per-count table is
    # the LAF table divided by the schedule at each bin center. None/empty on flat cars.
    self._speed_dep_steer_max_schedule = None
    self._speed_dep_laf_per_count_bp = []
    self._speed_dep_car_cfg = None
    self._last_vego = 0.0

  def update_override_torque_params(self, torque_params) -> bool:
    changed = False

    # Manual override first: it must own the params on EVERY frame, or the per-frame
    # speed-dep interpolation below out-writes it 299 frames out of 300. The params
    # store is only polled at 3 s cadence; the cached values apply each frame.
    if self.enforce_torque_control_toggle:
      self.frame += 1
      if self.frame % 300 == 0:
        self.torque_override_enabled = self.params.get_bool("TorqueParamsOverrideEnabled")
        if self.torque_override_enabled:
          self._override_lat_accel_factor = float(self.params.get("TorqueParamsOverrideLatAccelFactor", return_default=True))
          self._override_friction = float(self.params.get("TorqueParamsOverrideFriction", return_default=True))

      if self.torque_override_enabled:
        if torque_params.latAccelFactor != self._override_lat_accel_factor or torque_params.friction != self._override_friction:
          torque_params.latAccelFactor = self._override_lat_accel_factor
          torque_params.friction = self._override_friction
          changed = True
        return changed

    # Speed-dep latAccelFactor and friction: interpolate by current speed each frame.
    # On a platform with a speed-dependent STEER_MAX, bin LAF values are normalized units
    # learned under one scale each, so they interp in per-count space and rescale by the
    # schedule at the current speed: the physical counts curve is smooth, and the scale's
    # step lands exactly where the carcontroller applies it (14.2-14.5 m/s on the CX-5)
    # instead of being smeared across the whole bin span (~+18% torque below the cliff,
    # ~-19% above). Friction stays a plain interp of normalized values: it is applied as
    # friction/LAF, so the LAF step cancels the STEER_MAX step and its counts arrive
    # smooth on their own (CX-5 learned bins: 79.5 vs 80.2 counts at the cliff edges).
    if self._speed_dep_active and self._speed_dep_speed_bp:
      if self._speed_dep_steer_max_schedule and self._speed_dep_laf_per_count_bp:
        sm_bp, sm_v = self._speed_dep_steer_max_schedule
        new_lat_accel_factor = float(np.interp(self._last_vego, self._speed_dep_speed_bp, self._speed_dep_laf_per_count_bp) *
                                     np.interp(self._last_vego, sm_bp, sm_v))
      else:
        new_lat_accel_factor = float(np.interp(self._last_vego, self._speed_dep_speed_bp, self._speed_dep_lat_accel_factor_bp))
      new_fric = float(np.interp(self._last_vego, self._speed_dep_speed_bp, self._speed_dep_friction_bp))
      if new_lat_accel_factor != torque_params.latAccelFactor or new_fric != torque_params.friction:
        torque_params.latAccelFactor = new_lat_accel_factor
        torque_params.friction = new_fric
        changed = True

    return changed
