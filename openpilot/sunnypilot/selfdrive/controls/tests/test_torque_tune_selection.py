"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

# Which torque controller an unset TorqueControlTune selects. This is easy to get wrong by
# dropping `return_default=True` from the params read: params_keys.h declares "0.0" (v0), but
# a bare params.get() returns None for an unset param, and `None == 0.0` is False — which
# silently selects the newest tune instead. Nothing errors; the car just steers on v1.
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
  def test_unset_selects_v0(self, ctx):
    """The declared default in params_keys.h is 0.0 — an unset param must honor it."""
    params, controls = ctx
    params.put_bool("EnforceTorqueControl", True, block=True)
    params.remove("TorqueControlTune")
    assert select(controls) == V0

  @pytest.mark.parametrize(("version", "expected"), [(0.0, V0), (1.0, V1), (2.0, V2)])
  def test_explicit_version_is_honored(self, ctx, version, expected):
    params, controls = ctx
    params.put_bool("EnforceTorqueControl", True, block=True)
    params.put("TorqueControlTune", version, block=True)
    assert select(controls) == expected

  def test_torque_control_not_enforced_still_uses_v0_for_torque_cars(self, ctx):
    """Pre-existing behavior worth pinning: torque-tuned cars get v0 even with the toggle off."""
    params, controls = ctx
    params.put_bool("EnforceTorqueControl", False, block=True)
    params.put("TorqueControlTune", 1.0, block=True)
    assert select(controls) == V0

  def test_ui_default_option_matches_what_controls_runs(self, ctx):
    """The MICI selector shows the first (oldest) version for an unset param — it must be the
    same tune initialize_lateral_control picks, or the UI claims a tune the car isn't running."""
    from openpilot.selfdrive.ui.sunnypilot.mici.layouts.steering import SteeringLayoutMici

    params, controls = ctx
    versions = SteeringLayoutMici._load_torque_versions()
    shown_version = next(iter(versions.values()))  # oldest-first ordering

    params.put_bool("EnforceTorqueControl", True, block=True)
    params.remove("TorqueControlTune")
    assert (select(controls) == V0) is (shown_version == 0.0)


class TestTorqueTuneTiSeed:
  """TI cars get v2 + torque-control enforcement seeded once; explicit picks persist."""

  def test_ti_on_unset_seeds_v2_and_enforce(self, ctx):
    params, controls = ctx
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    assert select(controls) == V2
    assert params.get_bool("EnforceTorqueControl") is True
    assert params.get("TorqueControlTune", return_default=True) == 2.0  # seed persisted

  def test_ti_on_explicit_v0_persists(self, ctx):
    params, controls = ctx
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    params.put("TorqueControlTune", 0.0, block=True)
    assert select(controls) == V0
    assert params.get("TorqueControlTune", return_default=True) == 0.0

  def test_ti_on_explicit_v1_persists(self, ctx):
    params, controls = ctx
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    params.put("TorqueControlTune", 1.0, block=True)
    assert select(controls) == V1

  def test_ti_on_enforce_explicitly_off_stays_off(self, ctx):
    """A user who explicitly disabled enforcement keeps it — the v0 pin then applies."""
    params, controls = ctx
    params.put_bool("TorqueInterceptorEnabled", True, block=True)
    params.put_bool("EnforceTorqueControl", False, block=True)
    assert select(controls) == V0
    assert params.get_bool("EnforceTorqueControl") is False

  def test_ti_off_unset_seeds_nothing(self, ctx):
    params, controls = ctx
    assert select(controls) == V0
    assert params.get("TorqueControlTune") is None
    assert params.get("EnforceTorqueControl") is None


class _ExtStub:
  """LatControlTorqueExt stand-in: pass-through, never overrides output."""
  def __init__(self, *a, **k):
    self.overrides_output = False
  def update_override_torque_params(self, torque_params):
    return False
  def update_limits(self):
    pass
  def update(self, CS, VM, pid, params, ff, pid_log, setpoint, measurement, *a):
    return pid_log, 0.0


@pytest.fixture
def v2_lac(monkeypatch):
  """Real LatControlTorque v2 with the extension stubbed out (identity torque<->lataccel maps)."""
  from openpilot.sunnypilot.selfdrive.controls.lib import latcontrol_torque_v2
  monkeypatch.setattr(latcontrol_torque_v2, "LatControlTorqueExt", _ExtStub)
  CP = car.CarParams.new_message(steerControlType="torque")
  CP.lateralTuning.init('torque')
  CI = SimpleNamespace(torque_from_lateral_accel=lambda: (lambda la, tp: la),
                       lateral_accel_from_torque=lambda: (lambda t, tp: t))
  lac = latcontrol_torque_v2.LatControlTorque(CP.as_reader(), custom.CarParamsSP.new_message().as_reader(), CI, 0.01)
  return lac, latcontrol_torque_v2


def _drive(lac, desired_curvature, frames=1, steering_pressed=False, v_ego=20.0):
  CS = SimpleNamespace(vEgo=v_ego, steeringAngleDeg=0.0, steeringPressed=steering_pressed)
  VM = SimpleNamespace(calc_curvature=lambda angle, v, roll: angle)
  params = SimpleNamespace(angleOffsetDeg=0.0, roll=0.0)
  logs = [lac.update(True, CS, VM, params, False, desired_curvature, None, False, 0.2)[2] for _ in range(frames)]
  return logs


class TestTorqueV2Dampers:
  def test_gains_match_starpilot_generic_path(self, v2_lac):
    """Pins the deliberate v0 divergence: KP 0.6 / KI 0.35 (StarPilot v2 generic, CX-8 owner-confirmed)."""
    lac, mod = v2_lac
    assert (mod.KP, mod.KI) == (0.6, 0.35)
    lac.pid.speed = 30.0  # top of the interp schedule = steady-state KP
    assert lac.pid.k_p == 0.6

  def test_desired_jerk_is_clamped(self, v2_lac):
    """A 4 m/s^2 step at 0.2 s delay is raw 20 m/s^3 jerk; v2 must never log above the clip."""
    lac, mod = v2_lac
    logs = _drive(lac, 0.01, frames=50)  # 0.01 * 20^2 = 4 m/s^2
    assert max(l.desiredLateralJerk for l in logs) <= mod.MAX_LAT_JERK_UP + 1e-9
    assert max(l.desiredLateralJerk for l in logs) > 1.0  # clamp engaged, not just dead

  def test_v0_same_step_exceeds_clip(self, v2_lac):
    """Discrimination check: v0's unclamped jerk on the same step is ~20 m/s^3."""
    _, mod2 = v2_lac
    import openpilot.sunnypilot.selfdrive.controls.lib.latcontrol_torque_v0 as v0mod
    orig = v0mod.LatControlTorqueExt
    v0mod.LatControlTorqueExt = _ExtStub
    try:
      CP = car.CarParams.new_message(steerControlType="torque")
      CP.lateralTuning.init('torque')
      CI = SimpleNamespace(torque_from_lateral_accel=lambda: (lambda la, tp: la),
                           lateral_accel_from_torque=lambda: (lambda t, tp: t))
      lac0 = v0mod.LatControlTorque(CP.as_reader(), custom.CarParamsSP.new_message().as_reader(), CI, 0.01)
      logs = _drive(lac0, 0.01, frames=1)
      assert logs[0].desiredLateralJerk > 4 * mod2.MAX_LAT_JERK_UP
    finally:
      v0mod.LatControlTorqueExt = orig

  def test_integrator_decays_on_override_release(self, v2_lac):
    lac, mod = v2_lac
    _drive(lac, 0.0005, frames=30)  # gentle curve: p+ff stays under the limit so i accumulates
    assert abs(lac.pid.i) > 0
    _drive(lac, 0.0005, frames=2, steering_pressed=True)  # freeze while pressed
    i_before = lac.pid.i
    release_log = _drive(lac, 0.0005, frames=1)[0]  # release edge decays, then frame integrates
    expected = mod.STEER_RELEASE_I_DECAY * i_before + mod.KI * 0.01 * release_log.error
    assert lac.pid.i == pytest.approx(expected, rel=1e-6)

  def test_integrator_freezes_during_unwind(self, v2_lac):
    """Setpoint unwinding fast through near-zero accel must freeze the integrator."""
    lac, mod = v2_lac
    # craft an unwind frame: filtered jerk strongly negative, setpoint crossing near zero
    lac.jerk_filter.x = -mod.MAX_LAT_JERK_UP
    lac.prev_desired_lateral_accel = 0.25
    lac.lat_accel_request_buffer.clear()
    lac.lat_accel_request_buffer.extend([0.25] * lac.lat_accel_request_buffer_len)
    lac.pid.i = 1.0
    _drive(lac, 0.25 / 20.0 ** 2)  # future == expected == 0.25 -> raw jerk ~0, filter stays near clip
    assert lac.pid.i == 1.0  # frozen: unwind detected
    # control: let the jerk filter settle near 0, then the same frame integrates normally
    lac.pid.i = 0.05
    for _ in range(150):
      _drive(lac, 0.25 / 20.0 ** 2)
      lac.prev_desired_lateral_accel = 0.25
    assert lac.pid.i != 0.05
