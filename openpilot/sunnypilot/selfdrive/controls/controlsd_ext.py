"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
import time

import openpilot.cereal.messaging as messaging
from openpilot.cereal import log, custom

from opendbc.car import structs
from openpilot.common.params import Params
from openpilot.common.swaglog import cloudlog
from openpilot.sunnypilot import PARAMS_UPDATE_PERIOD
from openpilot.sunnypilot.livedelay.helpers import get_lat_delay
from openpilot.sunnypilot.modeld_v2.modeld_base import ModelStateBase
from openpilot.sunnypilot.selfdrive.controls.lib.blinker_pause_lateral import BlinkerPauseLateral
from openpilot.sunnypilot.selfdrive.controls.lib.lane_change_smoothing import LaneChangeSmoothing
from openpilot.sunnypilot.selfdrive.controls.lib.latcontrol_torque_v0 import LatControlTorque as LatControlTorqueV0
from openpilot.sunnypilot.selfdrive.controls.lib.latcontrol_torque_v2 import LatControlTorque as LatControlTorqueV2
from openpilot.sunnypilot.selfdrive.controls.lib.torque_tune import resolved_tune_version
from openpilot.sunnypilot.selfdrive.controls.lib.turn_assist import TurnAssistController


# TI recommended-settings seeds. Unset params only — explicit user picks persist.
# Only keys that DIFFER from upstream defaults belong here; unset-only seeding a key to
# its stock default is a dead write. Stock already covers:
# LagdToggle=1, LateralJerkTorqueController=0, CustomTorqueParams=0, TorqueParamsOverrideEnabled=0.
TI_TUNE_SEEDS = {
  "LiveTorqueParamsToggle": True,       # "Self-Tune": defaults OFF upstream, we want it learning
  "SpeedDependentTorqueToggle": True,   # "Speed-Dependent Self-Tune": defaults OFF upstream
  "LaneChangeSmoothing": True,          # "Smooth Lane Changes": defaults OFF upstream
  "LaneChangeSmoothingPace": 8,         # upstream default 5 (~5.8 s); 8 ≈ 4 s glide
  "LowSpeedTurnAssist": True,           # TI cars steer at standstill, so low-speed assist works
}
# No CX-8 delay override: the owner A/B'd "Live Delay" (the lagd learner) against a fixed
# 0.05 software delay and reports it performs significantly better — stock LagdToggle=1 wins.
TI_PLATFORMS = ("MAZDA_CX5_2022", "MAZDA_CX8_2022")


def seed_ti_defaults(params: Params, CP) -> None:
  """TI cars get torque enforcement plus our recommended lateral defaults, seeded once.
  Unset params only — anything the user explicitly picked is left alone."""
  if not params.get_bool("TorqueInterceptorEnabled"):
    return
  if params.get("EnforceTorqueControl") is None:
    # enforcement off pins v0 via the resolver; the tune resolves to the declared 2.0
    # default once enforced. block=True: resolved_tune_version reads it right after, same call.
    params.put_bool("EnforceTorqueControl", True, block=True)
  if CP.carFingerprint not in TI_PLATFORMS:
    return
  for key, value in TI_TUNE_SEEDS.items():
    if params.get(key) is None:
      params.put(key, value, block=True)


class ControlsExt(ModelStateBase):
  def __init__(self, CP: structs.CarParams, params: Params):
    ModelStateBase.__init__(self)
    self.CP = CP
    self.params = params
    self._param_update_time: float = 0.0
    self.blinker_pause_lateral = BlinkerPauseLateral()
    self.turn_assist = TurnAssistController(CP)
    self.lane_change_smoothing = LaneChangeSmoothing()

    cloudlog.info("controlsd_ext is waiting for CarParamsSP")
    self.CP_SP = messaging.log_from_bytes(params.get("CarParamsSP", block=True), custom.CarParamsSP)
    cloudlog.info("controlsd_ext got CarParamsSP")

    self.sm_services_ext = ['radarState', 'selfdriveStateSP']
    self.pm_services_ext = ['carControlSP']

  def initialize_lateral_control(self, lac, CI, dt):
    seed_ti_defaults(self.params, self.CP)
    # the enforce-off v0 forcing and the unset-param default both live in the resolver,
    # shared with the settings UIs so they gate on the tune that will actually run
    version = resolved_tune_version(self.params, self.CP.lateralTuning.which() == 'torque')
    if version == 0.0:  # v0
      return LatControlTorqueV0(self.CP, self.CP_SP, CI, dt)
    elif version == 2.0:  # v2
      return LatControlTorqueV2(self.CP, self.CP_SP, CI, dt)
    else:
      return lac

  def get_params_sp(self, sm: messaging.SubMaster) -> None:
    if time.monotonic() - self._param_update_time > PARAMS_UPDATE_PERIOD:
      self.blinker_pause_lateral.get_params()
      self.turn_assist.get_params()
      self.lane_change_smoothing.get_params()

      if self.CP.lateralTuning.which() == 'torque':
        self.lat_delay = get_lat_delay(self.params, sm["lateralDelay"].lateralDelay)

      self._param_update_time = time.monotonic()

  def get_lat_active(self, sm: messaging.SubMaster) -> bool:
    if self.blinker_pause_lateral.update(sm['carState']):
      return False

    ss_sp = sm['selfdriveStateSP']
    if ss_sp.mads.available:
      return bool(ss_sp.mads.active)

    # MADS not available, use stock state to engage
    return bool(sm['selfdriveState'].active)

  def update_lateral_assist(self, sm: messaging.SubMaster, lat_active: bool, new_desired_curvature: float,
                            prev_desired_curvature: float, current_curvature: float) -> tuple[float, float]:
    """Low-speed turn assist + lane-change smoothing over the desired curvature,
    returning (new_desired_curvature, jerk_factor) for clip_curvature. The lateral
    maneuver mode's scripted commands must pass through the stock clip untouched."""
    if sm.valid['lateralManeuverPlan']:
      return new_desired_curvature, 1.0
    CS = sm['carState']
    model_v2 = sm['modelV2']
    new_desired_curvature = self.turn_assist.update(CS, lat_active, model_v2, new_desired_curvature, current_curvature)
    jerk_factor = self.lane_change_smoothing.update(CS, model_v2, new_desired_curvature, prev_desired_curvature)
    return new_desired_curvature, jerk_factor

  @staticmethod
  def get_lead_data(_lead, src: log.RadarState.LeadData) -> None:
    _lead.dRel = src.dRel
    _lead.yRel = src.yRel
    _lead.vRel = src.vRel
    _lead.aRel = src.deprecated.aRel
    _lead.vLead = src.vLead
    _lead.dPath = src.deprecated.dPath
    _lead.vLat = src.deprecated.vLat
    _lead.vLeadK = src.vLeadK
    _lead.aLeadK = src.aLeadK
    _lead.fcw = src.deprecated.fcw
    _lead.status = src.present
    _lead.aLeadTau = src.aLeadTau
    _lead.modelProb = src.modelProb
    _lead.radar = src.radar
    _lead.radarTrackId = src.radarTrackId

  def state_control_ext(self, sm: messaging.SubMaster) -> custom.CarControlSP:
    CC_SP = custom.CarControlSP.new_message()

    self.get_lead_data(CC_SP.leadOne, sm['radarState'].leadOne)
    self.get_lead_data(CC_SP.leadTwo, sm['radarState'].leadTwo)

    # MADS state
    mads_src = sm['selfdriveStateSP'].mads
    CC_SP.mads.state = mads_src.state
    CC_SP.mads.enabled = mads_src.enabled
    CC_SP.mads.active = mads_src.active
    CC_SP.mads.available = mads_src.available

    # ICBM state
    icbm_src = sm['selfdriveStateSP'].intelligentCruiseButtonManagement
    CC_SP.intelligentCruiseButtonManagement.state = icbm_src.state
    CC_SP.intelligentCruiseButtonManagement.sendButton = icbm_src.sendButton
    CC_SP.intelligentCruiseButtonManagement.vTarget = icbm_src.vTarget

    # lateral assist telemetry, for offline validation of the turn hold and pace clamp
    CC_SP.turnAssist.holdCurvature = float(self.turn_assist.hold)
    CC_SP.turnAssist.leadCurvature = float(self.turn_assist.lead_applied)
    CC_SP.laneChangeSmoothing.jerkFactor = float(self.lane_change_smoothing.arrest_jerk_factor)

    return CC_SP

  @staticmethod
  def publish_ext(CC_SP: custom.CarControlSP, sm: messaging.SubMaster, pm: messaging.PubMaster) -> None:
    cc_sp_send = messaging.new_message('carControlSP')
    cc_sp_send.valid = sm['carState'].canValid
    cc_sp_send.carControlSP = CC_SP

    pm.send('carControlSP', cc_sp_send)

  def run_ext(self, sm: messaging.SubMaster, pm: messaging.PubMaster) -> None:
    CC_SP = self.state_control_ext(sm)
    self.publish_ext(CC_SP, sm, pm)

    # Speed-dependent torque: apply per-bin learned values to the lateral controller
    if (self.CP.lateralTuning.which() == 'torque'
        and sm.updated.get('lateralTorqueParameters', False)
        and sm.all_checks(['lateralTorqueParameters'])):
      tp = sm['lateralTorqueParameters']
      if tp.useParams and hasattr(self.LaC, 'extension'):
        self.LaC.extension.update_speed_dep_torque(tp)
