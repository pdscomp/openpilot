import sys
from pathlib import Path
from types import SimpleNamespace

from openpilot.common.params import ParamKeyFlag, ParamKeyType, Params
from openpilot.common.hardware.tici.c3xl import classify_c3xl, diagnose_c3xl, is_c3xl, is_c3xl_runtime, latch_c3xl_runtime, probe_usb_pandas


F4 = {"serial": "f4", "transport": "usb", "mcu": "f4", "bootstub": False}
H7 = {"serial": "h7", "transport": "usb", "mcu": "h7", "bootstub": False}


def sequence(*samples):
  samples = iter(samples)
  return lambda: next(samples, [])


def test_params_are_local_non_backup_controls(tmp_path):
  params = Params(str(tmp_path))
  assert params.get("HardwareC3XLMode", return_default=True) == 0
  assert params.get("HardwareC3XLEvidence", return_default=True) == {}
  assert params.get_type("HardwareC3XLMode") == ParamKeyType.INT
  assert params.get_type("HardwareC3XLEvidence") == ParamKeyType.JSON
  assert b"HardwareC3XLMode" not in params.all_keys(ParamKeyFlag.BACKUP)
  assert b"HardwareC3XLEvidence" not in params.all_keys(ParamKeyFlag.BACKUP)
  assert b"HardwareC3XLRuntimeMode" not in params.all_keys(ParamKeyFlag.BACKUP)
  assert b"HardwareC3XLRuntimeMode" in params.all_keys(ParamKeyFlag.CLEAR_ON_MANAGER_START)
  keys_source = (Path(__file__).parents[1] / "params_keys.h").read_text()
  assert '{"HardwareC3XLEvidence", {PERSISTENT | DONT_LOG, JSON, "{}"}}' in keys_source


def test_manual_mode_is_tici_only_and_environment_is_ignored(tmp_path, monkeypatch):
  params = Params(str(tmp_path))
  monkeypatch.setenv("C3XL", "1")

  assert not is_c3xl("tici", params)
  params.put("HardwareC3XLMode", 1, block=True)
  assert is_c3xl("tici", params)
  assert not is_c3xl("tizi", params)
  assert not is_c3xl("mici", params)
  assert not is_c3xl("notcomma tici", params)
  assert not is_c3xl("foo comma tici", params)
  assert is_c3xl("comma tici", params)
  assert is_c3xl("\ttici\t", params)


def test_runtime_mode_is_manager_latched(tmp_path):
  params = Params(str(tmp_path))
  params.put("HardwareC3XLMode", 1, block=True)
  assert latch_c3xl_runtime("tici", params)
  assert is_c3xl_runtime("tici", params)

  params.put("HardwareC3XLMode", 0, block=True)
  assert is_c3xl_runtime("tici", params)
  assert not is_c3xl_runtime("tizi", params)

  assert not latch_c3xl_runtime("tici", params)
  assert not is_c3xl_runtime("tici", params)


def test_model_is_normalized_once_at_policy_boundary(tmp_path, monkeypatch):
  params = Params(str(tmp_path))
  params.put("HardwareC3XLMode", 1, block=True)
  monkeypatch.setattr("openpilot.common.hardware.tici.c3xl.read_device_model", lambda: "comma comma tici")
  probes = 0

  def probe():
    nonlocal probes
    probes += 1
    return [F4]

  assert not is_c3xl(params=params)
  assert classify_c3xl("comma comma tici", 1)["result"] == "out_of_scope"
  decision = diagnose_c3xl(params=params, sample_pandas=probe)
  assert decision["result"] == "out_of_scope"
  assert probes == 0
  assert params.get("HardwareC3XLEvidence") is None


def test_non_tici_never_probes_or_writes_evidence(tmp_path):
  params = Params(str(tmp_path))
  params.put("HardwareC3XLMode", 1, block=True)
  calls = 0

  def probe():
    nonlocal calls
    calls += 1
    return [F4]

  decision = diagnose_c3xl("tizi", params, probe, sleep=lambda _: None)
  assert decision["result"] == "out_of_scope"
  assert not decision["active"]
  assert calls == 0
  assert params.get("HardwareC3XLEvidence") is None


def test_force_disable_and_invalid_modes_fail_closed():
  assert classify_c3xl("tici", 1)["active"]
  assert classify_c3xl("tici", -1)["result"] == "disabled"
  assert classify_c3xl("tici", 2)["result"] == "invalid_mode"
  assert not classify_c3xl("tici", 2)["active"]


def test_stable_f4_is_diagnostic_only():
  decision = classify_c3xl("tici", 0, sequence([F4], [F4], [F4]), sleep=lambda _: None)
  assert decision["result"] == "candidate_f4"
  assert decision["samples"] == 3
  assert not decision["active"]


def test_stable_h7_remains_stock():
  decision = classify_c3xl("tici", 0, sequence([H7], [H7], [H7]), sleep=lambda _: None)
  assert decision["result"] == "stock_h7"
  assert not decision["active"]


def test_identity_must_be_three_consecutive_matches():
  f4_other = F4 | {"serial": "other"}
  decision = classify_c3xl("tici", 0, sequence([F4], [F4], [f4_other], [f4_other], [f4_other]),
                            max_samples=5, sleep=lambda _: None)
  assert decision["result"] == "candidate_f4"
  assert decision["panda_serial"] == "other"
  assert decision["samples"] == 3


def test_uncertain_evidence_never_activates():
  bootstub = F4 | {"bootstub": True}
  cases = (
    sequence([], [], []),
    sequence([F4, H7], [F4, H7], [F4, H7]),
    sequence([bootstub], [bootstub], [bootstub]),
    sequence([F4], [H7], [F4]),
  )
  for probe in cases:
    decision = classify_c3xl("tici", 0, probe, max_samples=3, sleep=lambda _: None)
    assert decision["result"] == "uncertain"
    assert not decision["active"]


def test_deadline_clamps_sleep():
  now = 0.0

  def monotonic():
    return now

  def sleep(duration):
    nonlocal now
    now += duration

  decision = classify_c3xl("tici", 0, list, deadline=0.75, interval=0.5,
                            monotonic=monotonic, sleep=sleep)
  assert now == 0.75
  assert decision["reason"] == "timeout"


def test_late_third_match_does_not_classify():
  now = 0.0
  calls = 0

  def probe():
    nonlocal calls, now
    calls += 1
    if calls == 3:
      now = 1.01
    return [F4]

  decision = classify_c3xl("tici", 0, probe, max_samples=3, interval=0,
                            deadline=1.0, monotonic=lambda: now, sleep=lambda _: None)
  assert decision["result"] == "uncertain"
  assert decision["reason"] == "timeout"
  assert not decision["active"]


def test_diagnostic_evidence_is_persisted_without_enabling(tmp_path):
  params = Params(str(tmp_path))
  decision = diagnose_c3xl("tici", params, sequence([F4], [F4], [F4]),
                            device_serial="device", sleep=lambda _: None)
  evidence = params.get("HardwareC3XLEvidence")

  assert decision["result"] == "candidate_f4"
  assert evidence["schema"] == 1
  assert evidence["device_serial"] == "device"
  assert evidence["result"] == "candidate_f4"
  assert evidence["pandas"] == [F4]
  assert not is_c3xl("tici", params)
  assert "timestamp" not in evidence


def test_uncertain_evidence_preserves_current_pandas(tmp_path):
  params = Params(str(tmp_path))
  bootstub = F4 | {"bootstub": True}
  diagnose_c3xl("tici", params, sequence([bootstub]), max_samples=1,
                 device_serial="device", sleep=lambda _: None)
  evidence = params.get("HardwareC3XLEvidence")
  assert evidence["result"] == "uncertain"
  assert evidence["reason"] == "bootstub"
  assert evidence["pandas"] == [bootstub]
  assert "timestamp" not in evidence


def test_usb_probe_is_read_only(monkeypatch):
  class Handle:
    closed = False

    def controlRead(self, *args):
      assert args[:5] == (0xC0, 0xC1, 0, 0, 0x40)
      assert 0 < args[5] <= 1000
      return b"\x06"

    def close(self):
      self.closed = True

  handle = Handle()

  class Device:
    getVendorID = staticmethod(lambda: 0xBBAA)
    getProductID = staticmethod(lambda: 0xDDCC)
    getSerialNumber = staticmethod(lambda: "a" * 24)
    getbcdDevice = staticmethod(lambda: 0x0600)
    open = staticmethod(lambda: handle)

  class Context:
    def __enter__(self):
      return self

    def __exit__(self, *_):
      pass

    def getDeviceList(self, *, skip_on_error):
      assert skip_on_error
      return [Device()]

  monkeypatch.setitem(sys.modules, "usb1", SimpleNamespace(USBContext=Context))
  assert probe_usb_pandas() == [{"serial": "a" * 24, "transport": "usb", "mcu": "f4", "bootstub": False}]
  assert handle.closed


def test_usb_probe_retains_valid_panda_when_read_fails(monkeypatch):
  class Handle:
    def __init__(self, result):
      self.result = result

    def controlRead(self, *_):
      if isinstance(self.result, Exception):
        raise self.result
      return self.result

    def close(self):
      pass

  class Device:
    def __init__(self, serial, result):
      self.serial = serial
      self.handle = Handle(result)

    getVendorID = staticmethod(lambda: 0xBBAA)
    getProductID = staticmethod(lambda: 0xDDCC)
    getbcdDevice = staticmethod(lambda: 0x0600)

    def getSerialNumber(self):
      return self.serial

    def open(self):
      return self.handle

  devices = [Device("a" * 24, b"\x06"), Device("b" * 24, TimeoutError())]

  class Context:
    def __enter__(self):
      return self

    def __exit__(self, *_):
      pass

    def getDeviceList(self, *, skip_on_error):
      return devices

  monkeypatch.setitem(sys.modules, "usb1", SimpleNamespace(USBContext=Context))
  snapshots = probe_usb_pandas()
  assert snapshots == [F4 | {"serial": "a" * 24},
                       {"serial": "b" * 24, "transport": "usb", "mcu": "unknown", "bootstub": False}]
  decision = classify_c3xl("tici", 0, lambda: snapshots, max_samples=1, sleep=lambda _: None)
  assert decision["result"] == "uncertain"
  assert decision["reason"] == "multiple_pandas"
  assert decision["pandas"] == snapshots


def test_usb_probe_does_not_read_after_open_crosses_deadline(monkeypatch):
  now = 0.9

  class Handle:
    closed = False

    def controlRead(self, *_):
      raise AssertionError("read after deadline")

    def close(self):
      self.closed = True
      raise RuntimeError("close failed")

  handle = Handle()

  class Device:
    getVendorID = staticmethod(lambda: 0xBBAA)
    getProductID = staticmethod(lambda: 0xDDCC)
    getSerialNumber = staticmethod(lambda: "a" * 24)
    getbcdDevice = staticmethod(lambda: 0x0600)

    @staticmethod
    def open():
      nonlocal now
      now = 1.1
      return handle

  class Context:
    def __enter__(self):
      return self

    def __exit__(self, *_):
      pass

    def getDeviceList(self, *, skip_on_error):
      return [Device()]

  monkeypatch.setitem(sys.modules, "usb1", SimpleNamespace(USBContext=Context))
  assert probe_usb_pandas(deadline=1.0, monotonic=lambda: now) == [
    {"serial": "a" * 24, "transport": "usb", "mcu": "unknown", "bootstub": False},
  ]
  assert handle.closed
