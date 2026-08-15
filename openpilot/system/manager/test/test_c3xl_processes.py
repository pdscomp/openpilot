from types import SimpleNamespace

from openpilot.system.manager import process_config


def test_dmonitoring_model_requires_a_driver_camera_outside_forced_c3xl(monkeypatch):
  monkeypatch.setattr(process_config, "is_c3xl_runtime", lambda *, params: True)
  assert not process_config.dmonitoring_model(True, SimpleNamespace(get_bool=lambda _: False), SimpleNamespace())

  monkeypatch.setattr(process_config, "is_c3xl_runtime", lambda *, params: False)
  assert process_config.dmonitoring_model(True, SimpleNamespace(get_bool=lambda _: False), SimpleNamespace())
