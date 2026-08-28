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
    # Plain interp — STEER_MAX normalization is NOT used here. The carcontroller
    # handles STEER_MAX scaling of the integer command. LAF blends smoothly between
    # bins, giving the PID gradual headroom transition across the STEER_MAX cliff.
    if self._speed_dep_active and self._speed_dep_speed_bp:
      new_lat_accel_factor = float(np.interp(self._last_vego, self._speed_dep_speed_bp, self._speed_dep_lat_accel_factor_bp))
      new_fric = float(np.interp(self._last_vego, self._speed_dep_speed_bp, self._speed_dep_friction_bp))
      if new_lat_accel_factor != torque_params.latAccelFactor or new_fric != torque_params.friction:
        torque_params.latAccelFactor = new_lat_accel_factor
        torque_params.friction = new_fric
        changed = True

    return changed
