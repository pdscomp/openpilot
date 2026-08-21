from openpilot.cereal import custom
from opendbc.car import structs
from opendbc.car.mazda.values import MazdaFlags
from openpilot.selfdrive.selfdrived.events import Events
from openpilot.sunnypilot.selfdrive.car.car_specific import CarSpecificEventsSP

EventNameSP = custom.OnroadEventSP.EventName


def _mazda_cp(ti: bool) -> structs.CarParams:
  CP = structs.CarParams(carFingerprint="MAZDA_CX5_2022", brand="mazda")
  if ti:
    CP.flags |= MazdaFlags.TORQUE_INTERCEPTOR.value
  return CP


def _cs(vEgo: float) -> structs.CarState:
  return structs.CarState(vEgo=vEgo)


def _sp(ready: bool):
  return custom.CarStateSP.new_message(torqueInterceptorReady=ready)


def test_ti_not_ready_event_raises_when_unhealthy_at_speed_sustained():
  cse = CarSpecificEventsSP(_mazda_cp(ti=True), structs.CarParamsSP())
  for _ in range(200):
    assert not cse.update(_cs(20.0), _sp(False), Events()).has(EventNameSP.torqueInterceptorNotReady)
  assert cse.update(_cs(20.0), _sp(False), Events()).has(EventNameSP.torqueInterceptorNotReady)


def test_ti_not_ready_event_stays_clear_when_healthy_or_feature_off():
  cse = CarSpecificEventsSP(_mazda_cp(ti=True), structs.CarParamsSP())
  for _ in range(300):
    assert not cse.update(_cs(20.0), _sp(True), Events()).has(EventNameSP.torqueInterceptorNotReady)

  cse = CarSpecificEventsSP(_mazda_cp(ti=False), structs.CarParamsSP())
  for _ in range(300):
    assert not cse.update(_cs(20.0), _sp(False), Events()).has(EventNameSP.torqueInterceptorNotReady)


def test_ti_not_ready_stays_silent_at_low_speed_and_standstill():
  # TI drops out of RUN at low speed/standstill by design (self-protection);
  # that must never raise the fault alert
  cse = CarSpecificEventsSP(_mazda_cp(ti=True), structs.CarParamsSP())
  for v in (0.0, 5.0, 9.9, 10.0):
    for _ in range(300):
      assert not cse.update(_cs(v), _sp(False), Events()).has(EventNameSP.torqueInterceptorNotReady)


def test_ti_not_ready_counter_resets_on_recovery():
  cse = CarSpecificEventsSP(_mazda_cp(ti=True), structs.CarParamsSP())
  for _ in range(150):
    cse.update(_cs(20.0), _sp(False), Events())
  cse.update(_cs(20.0), _sp(True), Events())  # brief recovery
  for _ in range(200):
    assert not cse.update(_cs(20.0), _sp(False), Events()).has(EventNameSP.torqueInterceptorNotReady)
  assert cse.update(_cs(20.0), _sp(False), Events()).has(EventNameSP.torqueInterceptorNotReady)
