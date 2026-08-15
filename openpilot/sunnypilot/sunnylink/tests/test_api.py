from types import SimpleNamespace

import pytest

from openpilot.sunnypilot.sunnylink.api import SunnylinkApi, UNREGISTERED_SUNNYLINK_DONGLE_ID
from openpilot.sunnypilot.sunnylink.utils import get_sunnylink_status


@pytest.mark.parametrize("dongle_id", [None, "", UNREGISTERED_SUNNYLINK_DONGLE_ID])
def test_registration_waits_for_a_real_comma_dongle_id(dongle_id):
  api = object.__new__(SunnylinkApi)
  api.params = SimpleNamespace(get=lambda _: dongle_id)
  api.dongle_id = None
  api._status_update = lambda message: None

  assert api.register_device() is None


def test_registration_defers_without_an_imei():
  api = object.__new__(SunnylinkApi)
  api.params = SimpleNamespace(get=lambda _: "comma-dongle")
  api.dongle_id = None
  api._resolve_imeis = lambda: ("", "")
  api._status_update = lambda message: None

  assert api.register_device() is None


@pytest.mark.parametrize(("dongle_id", "registered"), [
  (None, False),
  ("", False),
  (UNREGISTERED_SUNNYLINK_DONGLE_ID, False),
  ("sunnylink-dongle", True),
])
def test_empty_sunnylink_id_is_not_registered(dongle_id, registered):
  params = SimpleNamespace(get=lambda _: dongle_id, get_bool=lambda _: True)
  _, is_registered, _ = get_sunnylink_status(params)
  assert is_registered is registered
