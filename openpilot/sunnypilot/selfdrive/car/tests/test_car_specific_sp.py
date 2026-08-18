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


def test_ti_not_ready_event_raises_when_feedback_unhealthy():
  cse = CarSpecificEventsSP(_mazda_cp(ti=True), structs.CarParamsSP())
  cs_sp = custom.CarStateSP.new_message(torqueInterceptorReady=False)
  events_sp = cse.update(structs.CarState(), cs_sp, Events())
  assert events_sp.has(EventNameSP.torqueInterceptorNotReady)


def test_ti_not_ready_event_stays_clear_when_healthy_or_feature_off():
  cs_sp_ready = custom.CarStateSP.new_message(torqueInterceptorReady=True)
  cse = CarSpecificEventsSP(_mazda_cp(ti=True), structs.CarParamsSP())
  assert not cse.update(structs.CarState(), cs_sp_ready, Events()).has(EventNameSP.torqueInterceptorNotReady)

  cs_sp_unready = custom.CarStateSP.new_message(torqueInterceptorReady=False)
  cse = CarSpecificEventsSP(_mazda_cp(ti=False), structs.CarParamsSP())
  assert not cse.update(structs.CarState(), cs_sp_unready, Events()).has(EventNameSP.torqueInterceptorNotReady)
