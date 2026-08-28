"""TI auto-detect: 0x24A-on-bus-1 sniffing and the unset-only auto-enable decision."""
from types import SimpleNamespace

from openpilot.common.params import Params
from openpilot.common.prefix import OpenpilotPrefix
from openpilot.selfdrive.car.card import ti_present, should_auto_enable_ti
from opendbc.car.mazda.values import MazdaFlags

TI_ADDR = 0x24A


def _pkt(*frames):
  return SimpleNamespace(can=[SimpleNamespace(address=a, src=s) for a, s in frames])


class TestBackendBootState:
  """manager.py:66-67 calls finalize_ti_enable + enforce_backend_state at every boot against
  real Params. test_manager.py monkeypatches both away, so only this exercises the boot
  contract: every key backend.py touches must exist in params_keys.h."""

  def test_manager_boot_backend_path(self):
    from openpilot.common.api.backend import enforce_backend_state, finalize_ti_enable, enable_ti, is_konik_locked
    with OpenpilotPrefix():
      params = Params()
      finalize_ti_enable(params)
      enforce_backend_state(params)
      assert not is_konik_locked(params)
      enable_ti(params)
      assert params.get_bool("TorqueInterceptorEnabled")
      assert is_konik_locked(params)
      finalize_ti_enable(params)  # still a clean no-op with TI on


class TestTiPresent:
  def test_seen_on_bus_one(self):
    assert ti_present([_pkt((0x100, 0), (TI_ADDR, 1))])

  def test_wrong_bus_is_not_a_detection(self):
    assert not ti_present([_pkt((TI_ADDR, 0), (TI_ADDR, 2))])

  def test_absent(self):
    assert not ti_present([_pkt((0x100, 1), (0x200, 1))])
    assert not ti_present([_pkt()])


class TestShouldAutoEnableTi:
  GEN1 = MazdaFlags.GEN1.value

  def _cp(self, flags=GEN1, fingerprint="MAZDA_CX5_2022"):
    return SimpleNamespace(flags=flags, carFingerprint=fingerprint)

  def test_gen1_unset_enables(self):
    with OpenpilotPrefix():
      assert should_auto_enable_ti(True, self._cp(), Params())

  def test_explicit_off_is_user_intent(self):
    with OpenpilotPrefix():
      params = Params()
      params.put_bool("TorqueInterceptorEnabled", False, block=True)
      assert not should_auto_enable_ti(True, self._cp(), params)

  def test_explicit_on_needs_no_autoenable(self):
    with OpenpilotPrefix():
      params = Params()
      params.put_bool("TorqueInterceptorEnabled", True, block=True)
      assert not should_auto_enable_ti(True, self._cp(), params)

  def test_not_seen_no_enable(self):
    with OpenpilotPrefix():
      assert not should_auto_enable_ti(False, self._cp(), Params())

  def test_non_gen1_never_enables(self):
    with OpenpilotPrefix():
      assert not should_auto_enable_ti(True, self._cp(flags=0), Params())

  def test_cx8_left_to_the_intrinsic_latch(self):
    with OpenpilotPrefix():
      assert not should_auto_enable_ti(True, self._cp(fingerprint="MAZDA_CX8_2022"), Params())
