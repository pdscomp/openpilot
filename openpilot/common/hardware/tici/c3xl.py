from __future__ import annotations

import time
from collections.abc import Callable
from typing import Any


SCHEMA_VERSION = 1
MODEL_PATH = "/sys/firmware/devicetree/base/model"
CMDLINE_PATH = "/proc/cmdline"
PANDA_USB_VIDS = (0xBBAA, 0x3801)
PANDA_USB_PIDS = (0xDDEE, 0xDDCC)
MISSING_HW_TYPE_ENDPOINT = b"\xff\x00\xc1\x3e\xde\xad\xd0\x0d"
F4_HW_TYPES = (0x06,)
H7_HW_TYPES = (0x07, 0x09, 0x0A, 0xB1)

PandaSnapshot = dict[str, Any]
PandaProbe = Callable[[], list[PandaSnapshot]]


def _normalize_model(model: str) -> str:
  return model.strip("\x00 \t\r\n\v\f").removeprefix("comma ")


def read_device_model(path: str = MODEL_PATH) -> str:
  try:
    with open(path) as f:
      return f.read()
  except OSError:
    return ""


def read_device_serial(path: str = CMDLINE_PATH) -> str:
  try:
    with open(path) as f:
      fields = dict(part.split("=", 1) for part in f.read().split() if "=" in part)
    return fields.get("androidboot.serialno", "")
  except OSError:
    return ""


def _get_params(params=None):
  if params is not None:
    return params
  from openpilot.common.params import Params
  return Params()


def _read_mode(params) -> int | None:
  try:
    mode = params.get("HardwareC3XLMode", return_default=True)
    return mode if mode in (-1, 0, 1) else None
  except (TypeError, ValueError):
    return None


def is_c3xl(model: str | None = None, params=None) -> bool:
  """Return true only for an explicit local force on exact tici hardware."""
  model = _normalize_model(model if model is not None else read_device_model())
  return model == "tici" and _read_mode(_get_params(params)) == 1


def _decision(result: str, model: str, *, active: bool = False, samples: int = 0,
              snapshot: PandaSnapshot | None = None, snapshots: list[PandaSnapshot] | None = None,
              reason: str = "") -> dict[str, Any]:
  decision: dict[str, Any] = {
    "result": result,
    "model": model,
    "active": active,
    "samples": samples,
    "reason": reason or result,
  }
  if snapshot is not None:
    decision.update({
      "panda_serial": snapshot["serial"],
      "transport": snapshot["transport"],
      "mcu": snapshot["mcu"],
    })
  if snapshots is not None:
    decision["pandas"] = [{key: item.get(key) for key in ("serial", "transport", "mcu", "bootstub")} for item in snapshots]
  return decision


def classify_c3xl(model: str, mode: int | None, sample_pandas: PandaProbe | None = None, *,
                   max_samples: int = 15, interval: float = 0.5, deadline: float = 7.5,
                   sleep: Callable[[float], None] = time.sleep,
                   monotonic: Callable[[], float] = time.monotonic) -> dict[str, Any]:
  """Classify C3XL policy. Only manual force activates clone behavior."""
  model = _normalize_model(model)
  if model != "tici":
    return _decision("out_of_scope", model)
  if mode not in (-1, 0, 1):
    return _decision("invalid_mode", model)
  if mode == -1:
    return _decision("disabled", model)
  if mode == 1:
    return _decision("forced", model, active=True)

  end = monotonic() + deadline
  previous: tuple[str, str, str] | None = None
  consecutive = 0
  attempts = 0
  last_reason = "no_panda"
  last_snapshots: list[PandaSnapshot] = []

  if sample_pandas is None:
    def probe():
      return probe_usb_pandas(deadline=end, monotonic=monotonic)
  else:
    probe = sample_pandas

  for attempt in range(max_samples):
    if monotonic() >= end:
      last_reason = "timeout"
      break

    snapshots = probe()
    attempts += 1
    last_snapshots = snapshots
    if monotonic() >= end:
      last_reason = "timeout"
      break
    if len(snapshots) != 1:
      previous, consecutive = None, 0
      last_reason = "no_panda" if not snapshots else "multiple_pandas"
    else:
      snapshot = snapshots[0]
      if snapshot.get("bootstub"):
        previous, consecutive = None, 0
        last_reason = "bootstub"
      elif snapshot.get("mcu") not in ("f4", "h7"):
        previous, consecutive = None, 0
        last_reason = "unknown_mcu"
      else:
        identity = (snapshot["serial"], snapshot["transport"], snapshot["mcu"])
        consecutive = consecutive + 1 if identity == previous else 1
        previous = identity
        last_reason = "mixed"
        if consecutive >= 3:
          result = "candidate_f4" if snapshot["mcu"] == "f4" else "stock_h7"
          return _decision(result, model, samples=consecutive, snapshot=snapshot, snapshots=snapshots)

    remaining = end - monotonic()
    if attempt + 1 < max_samples and remaining > 0:
      sleep(min(interval, remaining))

  return _decision("uncertain", model, samples=attempts, snapshots=last_snapshots, reason=last_reason)


def diagnose_c3xl(model: str | None = None, params=None, sample_pandas: PandaProbe | None = None, *,
                   device_serial: str | None = None, **classify_kwargs) -> dict[str, Any]:
  """Run and persist a diagnostic classification without enabling clone behavior."""
  p = _get_params(params)
  decision = classify_c3xl(model if model is not None else read_device_model(), _read_mode(p),
                            sample_pandas, **classify_kwargs)
  if decision["model"] == "tici":
    evidence = {
      "schema": SCHEMA_VERSION,
      "device_serial": device_serial if device_serial is not None else read_device_serial(),
      **decision,
    }
    p.put("HardwareC3XLEvidence", evidence, block=True)
  return decision


def probe_usb_pandas(*, deadline: float | None = None,
                     monotonic: Callable[[], float] = time.monotonic) -> list[PandaSnapshot]:
  """Passively read USB Panda identity. No reset, claim, write, DFU, or flash."""
  try:
    import usb1
  except ImportError:
    return []

  snapshots: list[PandaSnapshot] = []
  try:
    with usb1.USBContext() as context:
      for device in context.getDeviceList(skip_on_error=True):
        try:
          vid, pid = device.getVendorID(), device.getProductID()
        except Exception:
          continue
        if vid not in PANDA_USB_VIDS or pid not in PANDA_USB_PIDS:
          continue

        try:
          serial = device.getSerialNumber()
        except Exception:
          continue
        if not isinstance(serial, str) or len(serial) != 24:
          continue

        bootstub = (pid & 0xF0) == 0xE0
        snapshot: PandaSnapshot = {"serial": serial, "transport": "usb", "mcu": "unknown", "bootstub": bootstub}
        bcd = None
        raw_type = b""
        read_ok = False
        try:
          bcd = device.getbcdDevice()
          remaining = None if deadline is None else deadline - monotonic()
          if remaining is None or remaining > 0:
            handle = device.open()
            try:
              remaining = None if deadline is None else deadline - monotonic()
              if remaining is None or remaining > 0:
                timeout_ms = 1000 if remaining is None else max(1, int(remaining * 1000))
                raw_type = bytes(handle.controlRead(0xC0, 0xC1, 0, 0, 0x40, timeout_ms))
                read_ok = True
            finally:
              handle.close()
        except Exception:
          read_ok = False

        if read_ok:
          hw_type = raw_type[0] if len(raw_type) == 1 else None
          if bootstub and raw_type.startswith(MISSING_HW_TYPE_ENDPOINT):
            hw_type = bcd >> 8 if bcd not in (None, 0x2300) else 0x06
          snapshot["mcu"] = "f4" if hw_type in F4_HW_TYPES else "h7" if hw_type in H7_HW_TYPES else "unknown"
        snapshots.append(snapshot)
  except Exception:
    return []
  return snapshots


if __name__ == "__main__":
  import json
  print(json.dumps(diagnose_c3xl(), sort_keys=True))
