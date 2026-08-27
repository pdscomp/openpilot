#!/usr/bin/env python3
"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Tests for the one-time steer-to-zero Mazda torque-control default seeding.
"""

from opendbc.car.structs import car
from opendbc.car.mazda.values import MazdaFlags

from openpilot.sunnypilot.selfdrive.car.interfaces import _seed_mazda_torque_defaults

CarParams = car.CarParams

SEEDED_KEYS = ("EnforceTorqueControl", "LiveTorqueParamsToggle", "SpeedDependentTorqueToggle")


class FakeParams:
  """Minimal dict-backed Params stand-in (avoids the stale on-disk params_pyx for new keys)."""
  def __init__(self, initial=None):
    self._store = dict(initial or {})

  def get_bool(self, key):
    return bool(self._store.get(key, False))

  def get(self, key):
    return self._store.get(key)

  def put_bool(self, key, val):
    self._store[key] = bool(val)


def _cx5_eps_cp():
  return CarParams(brand="mazda", flags=MazdaFlags.STEER_TO_ZERO.value)


def _pre_2022_mazda_cp():
  return CarParams(brand="mazda")


def _non_mazda_cp():
  return CarParams(brand="toyota", flags=MazdaFlags.STEER_TO_ZERO.value)


def _ti_cp():
  return CarParams(brand="mazda", flags=MazdaFlags.TORQUE_INTERCEPTOR.value, minSteerSpeed=0.0)


class TestMazdaTorqueDefaultsSeed:
  def test_steer_to_zero_mazda_gets_defaults(self):
    params = FakeParams()
    _seed_mazda_torque_defaults(_cx5_eps_cp(), params)
    for key in SEEDED_KEYS:
      assert params.get_bool(key) is True
    assert params.get_bool("MazdaTorqueDefaultsApplied") is True

  def test_pre_2022_mazda_not_seeded(self):
    params = FakeParams()
    _seed_mazda_torque_defaults(_pre_2022_mazda_cp(), params)
    for key in SEEDED_KEYS:
      assert params.get_bool(key) is False
    assert params.get_bool("MazdaTorqueDefaultsApplied") is False

  def test_non_mazda_not_seeded(self):
    params = FakeParams()
    _seed_mazda_torque_defaults(_non_mazda_cp(), params)
    for key in SEEDED_KEYS:
      assert params.get_bool(key) is False
    assert params.get_bool("MazdaTorqueDefaultsApplied") is False

  def test_ti_zero_speed_does_not_seed_steer_to_zero_defaults(self):
    params = FakeParams()
    _seed_mazda_torque_defaults(_ti_cp(), params)
    for key in SEEDED_KEYS:
      assert params.get_bool(key) is False
    assert params.get_bool("MazdaTorqueDefaultsApplied") is False

  def test_idempotent_respects_user_override(self):
    # Already applied once, and the user has since turned the toggles back off.
    params = FakeParams({"MazdaTorqueDefaultsApplied": True})
    _seed_mazda_torque_defaults(_cx5_eps_cp(), params)
    for key in SEEDED_KEYS:
      assert params.get_bool(key) is False  # not re-seeded

  def test_explicit_off_before_first_seed_is_never_overridden(self):
    params = FakeParams({"EnforceTorqueControl": False})
    _seed_mazda_torque_defaults(_cx5_eps_cp(), params)
    assert params.get_bool("EnforceTorqueControl") is False  # explicit pick wins
    assert params.get_bool("LiveTorqueParamsToggle") is True  # unset keys still seed
    assert params.get_bool("MazdaTorqueDefaultsApplied") is True
