"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

# Which torque controller an unset TorqueControlTune selects. This is easy to get wrong by
# dropping `return_default=True` from the params read: params_keys.h declares "2.0" (v2), but
# a bare params.get() returns None for an unset param, and float(None) raises — or, guarded,
# silently falls through to the upstream controller. Nothing says the car dropped off v2.
#
# The v0 constructor is patched out: these tests pin the branch that gets taken, not the
# controller's behavior, and building the real one pulls in NNLC model loading.

from types import SimpleNamespace
from unittest.mock import MagicMock

import pytest

from opendbc.car.structs import car
from openpilot.cereal import custom
from openpilot.common.params import Params
from openpilot.common.prefix import OpenpilotPrefix
from openpilot.sunnypilot.selfdrive.controls import controlsd_ext
from openpilot.sunnypilot.selfdrive.controls.controlsd_ext import ControlsExt

V0 = "v0"
V1 = "v1"  # stands in for the `lac` upstream controller controlsd passes in
V2 = "v2"


@pytest.fixture
def ctx(monkeypatch):
  monkeypatch.setattr(controlsd_ext, "LatControlTorqueV0", lambda *a, **k: V0)
  monkeypatch.setattr(controlsd_ext, "LatControlTorqueV2", lambda *a, **k: V2)
  with OpenpilotPrefix():
    params = Params()
    CP = car.CarParams.new_message(steerControlType="torque")
    CP.lateralTuning.init('torque')
    controls = SimpleNamespace(params=params, CP=CP.as_reader(),
                               CP_SP=custom.CarParamsSP.new_message().as_reader())
    yield params, controls


def select(controls):
  return ControlsExt.initialize_lateral_control(controls, V1, MagicMock(), 0.01)


class TestTorqueTuneSelection:
  def test_unset_selects_v2(self, ctx):
    """The declared default in params_keys.h is 2.0 — an unset param must honor it, so a
    fresh install (and every car seeded into torque control) drives on the v2 tune."""
    params, controls = ctx
    params.put_bool("EnforceTorqueControl", True, block=True)
    params.remove("TorqueControlTune")
    assert select(controls) == V2

  @pytest.mark.parametrize(("version", "expected"), [(0.0, V0), (1.0, V1), (2.0, V2)])
  def test_explicit_version_is_honored(self, ctx, version, expected):
    params, controls = ctx
    params.put_bool("EnforceTorqueControl", True, block=True)
    params.put("TorqueControlTune", version, block=True)
    assert select(controls) == expected

  def test_every_declared_version_is_wired(self, ctx):
    """The versions file is what the UI selectors and the sunnylink schema offer, while
    initialize_lateral_control decides what is constructible. A version added to the file
    but not wired here would surface in every selector and silently run v1."""
    from openpilot.sunnypilot.selfdrive.controls.lib.torque_tune import load_versions

    wired = {0.0: V0, 1.0: V1, 2.0: V2}
    declared = {float(info["version"]) for info in load_versions().values()}
    assert declared == set(wired), "declared tune versions must match the wired controllers"

    params, controls = ctx
    params.put_bool("EnforceTorqueControl", True, block=True)
    for version, expected in wired.items():
      params.put("TorqueControlTune", version, block=True)
      assert select(controls) == expected

  @pytest.mark.parametrize("version", [1.0, 2.0])
  def test_torque_control_not_enforced_still_uses_v0_for_torque_cars(self, ctx, version):
    """Pre-existing behavior worth pinning: torque-tuned cars get v0 even with the toggle off.
    For 2.0 this is also the structural NNLC exclusion: enabling NNLC disables
    EnforceTorqueControl (ui_state/_cleanup_unsupported_params), so a stored v2 selection can
    never construct the v2 controller alongside NNLC."""
    params, controls = ctx
    params.put_bool("EnforceTorqueControl", False, block=True)
    params.put("TorqueControlTune", version, block=True)
    assert select(controls) == V0

  def test_ui_default_matches_what_controls_runs(self, ctx):
    """For an unset param the MICI selector lights up the declared default (the widget itself
    is pinned by test_torque_tune_unset_is_v2) — that version must be the one
    initialize_lateral_control picks, or the UI claims a tune the car isn't running."""
    from openpilot.selfdrive.ui.sunnypilot.mici.layouts.steering import SteeringLayoutMici

    params, controls = ctx
    params.put_bool("EnforceTorqueControl", True, block=True)
    params.remove("TorqueControlTune")

    shown = float(params.get("TorqueControlTune", return_default=True))
    assert shown in set(SteeringLayoutMici._load_torque_versions().values()), \
      "the declared default must be a version the selectors offer"
    assert {0.0: V0, 1.0: V1, 2.0: V2}[shown] == select(controls)


def _with_fingerprint(controls, fingerprint: str):
  CP = car.CarParams.new_message(steerControlType="torque", carFingerprint=fingerprint)
  CP.lateralTuning.init('torque')
  controls.CP = CP.as_reader()


def _assert_seeds(params, seeds):
  for key, value in seeds.items():
    assert params.get(key) == value, key  # typed params layer: get returns bool/int/float


class TestTorqueTuneTiSeed:
  """TI cars get torque enforcement plus the recommended lateral bundle, seeded into unset
  params only. The CX-8 table additionally parks the (stalled) delay learner at the
  owner-validated fixed delay."""

  def test_ti_on_unset_seeds_enforce_and_resolves_v2(self, ctx):
    params, controls = ctx
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    assert select(controls) == V2
    assert params.get_bool("EnforceTorqueControl")

  def test_explicit_enforce_off_persists(self, ctx):
    params, controls = ctx
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    params.put_bool("EnforceTorqueControl", False, block=True)
    assert select(controls) == V0
    assert not params.get_bool("EnforceTorqueControl")

  def test_ti_off_seeds_nothing(self, ctx):
    params, controls = ctx
    _with_fingerprint(controls, "MAZDA_CX8_2022")
    select(controls)
    assert params.get("EnforceTorqueControl") is None
    assert params.get("LiveTorqueParamsToggle") is None

  def test_cx8_gets_full_bundle(self, ctx):
    params, controls = ctx
    _with_fingerprint(controls, "MAZDA_CX8_2022")
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    select(controls)
    _assert_seeds(params, controlsd_ext.TI_TUNE_SEEDS_CX8)

  def test_cx8_explicit_delay_pick_persists(self, ctx):
    params, controls = ctx
    _with_fingerprint(controls, "MAZDA_CX8_2022")
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    params.put_bool("LagdToggle", True, block=True)
    select(controls)
    assert params.get_bool("LagdToggle")

  def test_cx5_gets_base_bundle_without_delay_pair(self, ctx):
    params, controls = ctx
    _with_fingerprint(controls, "MAZDA_CX5_2022")
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    select(controls)
    _assert_seeds(params, controlsd_ext.TI_TUNE_SEEDS)
    assert params.get("LagdToggle") is None
    assert params.get("LagdToggleDelay") is None

  def test_non_seeded_platform_gets_enforce_but_no_bundle(self, ctx):
    params, controls = ctx  # fixture fingerprint is a non-seeded platform ("")
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    select(controls)
    assert params.get_bool("EnforceTorqueControl")
    assert params.get("LiveTorqueParamsToggle") is None


class TestPerCarGainOverride:
  """latcontrol_torque prefers CP.lateralTuning.torque.kp/ki when a platform sets them
  (the CX-8's owner-validated 0.6/0.35); unset fields keep the module defaults."""

  def _controller(self, kp=0.0, ki=0.0):
    from openpilot.selfdrive.controls.lib.latcontrol_torque import LatControlTorque
    CP = car.CarParams.new_message(steerControlType="torque")
    CP.lateralTuning.init('torque')
    CP.lateralTuning.torque.kp = kp
    CP.lateralTuning.torque.ki = ki
    return LatControlTorque(CP.as_reader(), custom.CarParamsSP.new_message().as_reader(), MagicMock(), 0.01)

  def test_platform_gains_are_used_when_set(self):
    ctrl = self._controller(kp=0.6, ki=0.35)
    assert ctrl.pid._k_p[1][-1] == pytest.approx(0.6)
    assert ctrl.pid._k_i[1][0] == pytest.approx(0.35)

  def test_unset_falls_back_to_module_defaults(self):
    from openpilot.selfdrive.controls.lib import latcontrol_torque
    ctrl = self._controller()
    assert ctrl.pid._k_p[1][-1] == latcontrol_torque.KP
    assert ctrl.pid._k_i[1][0] == latcontrol_torque.KI
    assert ctrl.pid._k_p[1][:-1] == latcontrol_torque.KP_INTERP[:-1]  # schedule below the top end untouched
