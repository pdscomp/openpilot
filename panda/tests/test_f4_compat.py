import pytest

from opendbc.car.structs import CarParams
from panda import Panda
from panda.python.constants import McuType
from panda.tests.libpanda import libpanda_py


class FakeHandle:
  def __init__(self, hw_type):
    self.hw_type = hw_type
    self.writes = []

  def controlRead(self, *_args):
    return self.hw_type

  def controlWrite(self, *args, **_kwargs):
    self.writes.append(args)

  def close(self):
    pass


def panda_for_type(hw_type, *, bcd=None, assume_f4=False):
  panda = object.__new__(Panda)
  panda._handle = FakeHandle(hw_type)
  panda._bcd_hw_type = bcd
  panda._assume_f4_mcu = assume_f4
  panda._handle_open = True
  panda._context = None
  return panda


def test_mcu_type_detection():
  assert panda_for_type(Panda.HW_TYPE_DOS).get_mcu_type() == McuType.F4
  assert panda_for_type(Panda.HW_TYPE_RED_PANDA).get_mcu_type() == McuType.H7
  assert panda_for_type(Panda.HW_TYPE_TRES).get_mcu_type() == McuType.H7
  assert panda_for_type(Panda.HW_TYPE_CUATRO).get_mcu_type() == McuType.H7
  assert McuType.F4.config.app_fn == "panda.bin.signed"
  assert McuType.H7.config.app_fn == "panda_h7.bin.signed"


def test_old_bootstub_fallbacks_are_f4_only():
  invalid_endpoint = b"\xff\x00\xc1\x3e\xde\xad\xd0\x0d"
  assert panda_for_type(invalid_endpoint, bcd=Panda.HW_TYPE_DOS).get_mcu_type() == McuType.F4
  oldest_bootstub = panda_for_type(invalid_endpoint, assume_f4=True)
  assert oldest_bootstub.get_type() == Panda.HW_TYPE_DOS
  assert oldest_bootstub.get_mcu_type() == McuType.F4
  with pytest.raises(ValueError):
    panda_for_type(invalid_endpoint).get_mcu_type()


def test_old_bootstub_can_reset_into_dfu():
  panda = panda_for_type(b"\xff\x00\xc1\x3e\xde\xad\xd0\x0d", assume_f4=True)
  panda.reset(enter_bootloader=True, reconnect=False)
  assert panda._handle.writes[0][1] == 0xd1


def test_f4_rejects_canfd_safety_mode():
  safety = libpanda_py.libpanda.test_get_supported_safety_mode
  assert safety(CarParams.SafetyModel.hyundaiCanfd, False) == CarParams.SafetyModel.silent
  assert safety(CarParams.SafetyModel.toyota, False) == CarParams.SafetyModel.toyota
  assert safety(CarParams.SafetyModel.hyundaiCanfd, True) == CarParams.SafetyModel.hyundaiCanfd


def test_f4_rejects_can_fd_before_transport():
  panda = panda_for_type(Panda.HW_TYPE_DOS)
  panda._mcu_type = McuType.F4
  panda.can_version = Panda.CAN_PACKET_VERSION
  panda._handle = None
  with pytest.raises(ValueError, match="CAN-FD"):
    panda.can_send_many([(0x123, b"123456789", 0)])
  with pytest.raises(ValueError, match="CAN-FD"):
    panda.can_send_many([(0x123, b"12345678", 0)], fd=True)