#!/usr/bin/env python3
import openpilot.cereal.messaging as messaging
from openpilot.common.params import Params
from openpilot.common.realtime import Ratekeeper, config_realtime_process
from openpilot.common.hardware.tici.c3xl import is_c3xl_runtime
from openpilot.selfdrive.monitoring.policy import DriverMonitoring


def dmonitoringd_thread():
  config_realtime_process([0, 1, 2, 3], 5)

  params = Params()
  c3xl = is_c3xl_runtime(params=params)
  pm = messaging.PubMaster(['driverMonitoringState'])
  services = ['carState', 'selfdriveState', 'carControl'] if c3xl else [
    'driverStateV2', 'liveCalibration', 'carState', 'selfdriveState', 'modelV2', 'carControl']
  sm = messaging.SubMaster(services, poll=None if c3xl else 'driverStateV2', frequency=20 if c3xl else None)

  DM = DriverMonitoring(rhd_saved=params.get_bool("IsRhdDetected"), always_on=params.get_bool("AlwaysOnDM"))
  demo_mode=False
  rk = Ratekeeper(20, print_delay_threshold=None) if c3xl else None

  # C3XL runs genuine wheel-touch monitoring at 20Hz; stock remains model-driven.
  while True:
    sm.update(0 if c3xl else 100)
    if not c3xl and not sm.updated['driverStateV2']:
      # iterate when model has new output
      continue

    valid = sm.all_checks()
    if c3xl:
      if valid:
        DM.run_wheeltouch_step(sm)
    elif demo_mode and sm.valid['driverStateV2']:
      DM.run_step(sm, demo=True)
    elif valid:
      DM.run_step(sm, demo=demo_mode)

    # publish
    dat = DM.get_state_packet(valid=valid)
    pm.send('driverMonitoringState', dat)

    # load live always-on toggle
    if sm.frame % 40 == 1:
      DM.always_on = params.get_bool("AlwaysOnDM")
      demo_mode = params.get_bool("IsDriverViewEnabled")

    # save rhd virtual toggle every 5 mins
    if (not c3xl and sm['driverStateV2'].frameId % 6000 == 0 and not demo_mode and
     DM.wheelpos_offsetter.filtered_stat.n > DM.settings._WHEELPOS_FILTER_MIN_COUNT and
     DM.wheel_on_right == (DM.wheelpos_offsetter.filtered_stat.M > DM.settings._WHEELPOS_THRESHOLD)):
      params.put_bool("IsRhdDetected", DM.wheel_on_right)

    if rk is not None:
      rk.keep_time()

def main():
  dmonitoringd_thread()


if __name__ == '__main__':
  main()
