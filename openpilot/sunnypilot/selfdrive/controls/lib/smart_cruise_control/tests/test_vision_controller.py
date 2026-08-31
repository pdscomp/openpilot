"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

The controller is exercised with synthetic roads: kappa(s) profiles rendered into the
time-indexed model arrays the way the model would report them at a given speed. The
activation distances asserted here follow from the platform limits in limits.py; if
those constants move, the geometry in these tests moves with them.
"""
import contextlib
from typing import Any

import numpy as np

import openpilot.cereal.messaging as messaging
from openpilot.cereal import custom, log
from openpilot.common.params import Params
from openpilot.common.realtime import DT_MDL
from openpilot.selfdrive.car.cruise import V_CRUISE_UNSET
from openpilot.selfdrive.modeld.constants import ModelConstants
from opendbc.car import structs
from openpilot.sunnypilot.selfdrive.controls.lib.smart_cruise_control import MIN_V
from openpilot.sunnypilot.selfdrive.controls.lib.smart_cruise_control import vision_controller
from openpilot.sunnypilot.selfdrive.controls.lib.smart_cruise_control.vision_controller import SmartCruiseControlVision
from openpilot.common.test import OpenpilotTestCase

VisionState = custom.LongitudinalPlanSP.SmartCruiseControl.VisionState

V_EGO = 20.
SETPOINT = 20.
CURVE_KAPPA = 0.02  # r = 50 m -> allowed 10 m/s at the 2.0 ceiling
CURVE_V = 10.


def make_cp(op_long: bool = True) -> structs.CarParams:
  return structs.CarParams(brand="mazda", openpilotLongitudinalControl=op_long,
                           longitudinalActuatorDelay=0.36)


# What the model does to curvature with range, measured over 26 apexes on route 135 (see
# vision_controller._KAPPA_BIAS_GAIN). A test road rendered without it is a perfect sensor,
# which the correction is deliberately a no-op against.
ATTENUATION_D = [0., 30., 50., 70., 90., 110., 130., 200.]
ATTENUATION = [1.0, 0.94, 0.88, 0.79, 0.66, 0.55, 0.30, 0.30]


def model_for_road(v: float, kappa_fn, v_model: float | None = None, attenuate: bool = False):
  """Render kappa(s) into model arrays as the model would report driving it at speed v.

  v_model lets the model's own velocity plan differ from v (a planned slowdown); the yaw
  rate follows the planned velocity, exactly as the model reports it.

  attenuate applies the measured range under-read, so the road reaches the controller the
  way the real model would report it rather than as perfect geometry.
  """
  t = np.array(ModelConstants.T_IDXS)
  s = v * t
  vm = v if v_model is None else v_model
  if attenuate:
    reported = kappa_fn

    def kappa_fn(si, _true=reported):
      return _true(si) * float(np.interp(si, ATTENUATION_D, ATTENUATION))

  model = messaging.new_message('modelV2')
  position = log.XYZTData.new_message()
  position.x = [float(si) for si in s]
  position.y = [0.0] * len(t)
  model.modelV2.position = position
  velocity = log.XYZTData.new_message()
  velocity.x = [float(vm)] * len(t)
  model.modelV2.velocity = velocity
  orientation_rate = log.XYZTData.new_message()
  orientation_rate.z = [float(kappa_fn(si) * vm) for si in s]
  model.modelV2.orientationRate = orientation_rate
  return model


def curve_at(d_curve: float, kappa: float = CURVE_KAPPA):
  return lambda s: kappa if s >= d_curve else 0.


@contextlib.contextmanager
def patch_gain(gain):
  """Run with a different far-field curvature correction, to isolate what it buys."""
  saved = vision_controller._KAPPA_BIAS_GAIN
  vision_controller._KAPPA_BIAS_GAIN = gain
  try:
    yield
  finally:
    vision_controller._KAPPA_BIAS_GAIN = saved


class TestSmartCruiseControlVision(OpenpilotTestCase):

  def setup_method(self):
    self.params = Params()
    self.params.put_bool("SmartCruiseControlVision", True, block=True)
    self.scc_v = SmartCruiseControlVision(make_cp())

  def make_sm(self, v: float, kappa_fn, cur_curvature: float = 0., v_model: float | None = None,
              attenuate: bool = False) -> Any:
    controls_state = messaging.new_message('controlsState')
    controls_state.controlsState.curvature = float(cur_curvature)
    return {'modelV2': model_for_road(v, kappa_fn, v_model, attenuate).modelV2,
            'controlsState': controls_state.controlsState}

  def run_road(self, v: float, kappa_fn, n: int = 3, cur_curvature: float = 0.,
               v_model: float | None = None, setpoint: float = SETPOINT,
               enabled: bool = True, override: bool = False, scc=None, attenuate: bool = False):
    scc = scc or self.scc_v
    sm = self.make_sm(v, kappa_fn, cur_curvature, v_model, attenuate)
    for _ in range(n):
      scc.update(sm, enabled, override, v, 0., setpoint)
    return scc

  # -- lifecycle -------------------------------------------------------------

  def test_initial_state(self):
    assert self.scc_v.state == VisionState.disabled
    assert not self.scc_v.is_active
    assert self.scc_v.output_v_target == V_CRUISE_UNSET
    assert self.scc_v.output_a_target == 0.

  def test_param_disable(self):
    self.params.put_bool("SmartCruiseControlVision", False, block=True)
    self.scc_v.enabled = False
    self.run_road(V_EGO, curve_at(50.))
    assert self.scc_v.state == VisionState.disabled

  def test_long_disabled(self):
    self.run_road(V_EGO, curve_at(50.), enabled=False)
    assert self.scc_v.state == VisionState.disabled
    assert self.scc_v.output_v_target == V_CRUISE_UNSET

  def test_override_suspends_control(self):
    self.run_road(V_EGO, curve_at(50.), override=True)
    assert self.scc_v.state == VisionState.overriding
    assert self.scc_v.output_v_target == V_CRUISE_UNSET

  # -- hold the set speed ----------------------------------------------------

  def test_straight_road_never_acts(self):
    self.run_road(V_EGO, lambda s: 0., n=10)
    assert self.scc_v.state == VisionState.enabled
    assert self.scc_v.output_v_target == V_CRUISE_UNSET
    assert self.scc_v.a_required == 0.

  def test_distant_curve_holds_set_speed(self):
    # a curve 185 m out, as the model actually reports one at that range: even with the
    # under-read corrected it asks well under the 0.7 * 1.2 commit, so the car holds
    self.run_road(V_EGO, curve_at(185., kappa=0.012), attenuate=True)
    assert self.scc_v.state == VisionState.enabled
    assert self.scc_v.output_v_target == V_CRUISE_UNSET
    assert 0. < self.scc_v.a_required < 0.84

  # -- brake at the budget ---------------------------------------------------

  def test_curve_inside_braking_distance_engages(self):
    self.run_road(V_EGO, curve_at(100.))
    assert self.scc_v.state == VisionState.entering
    assert self.scc_v.is_active
    # target leads v_ego by the required decel, capped by the profile
    assert MIN_V < self.scc_v.output_v_target < V_EGO - 0.5
    assert self.scc_v.output_a_target < 0.

  def test_a_target_is_jerk_ramped(self):
    sm = self.make_sm(V_EGO, curve_at(100.))
    prev = 0.
    j = self.scc_v.limits.jerk(V_EGO)
    for i in range(45):
      self.scc_v.update(sm, True, False, V_EGO, 0., SETPOINT)
      a = self.scc_v.output_a_target
      if i:
        assert a <= prev + 1e-9
        assert prev - a <= j * DT_MDL + 1e-6
      prev = a
    assert prev < -1.0  # converged to a real decel request, not the old smear

  def test_planned_slowdown_does_not_lower_the_estimate(self):
    # The old lat-acc form used the model's velocity plan, so a planned slowdown lowered
    # the prediction below the abort threshold mid-braking. Geometry divides it back out.
    self.run_road(V_EGO, curve_at(100.), v_model=0.7 * V_EGO)
    assert self.scc_v.is_active

  def test_slowing_toward_the_curve_stays_committed(self):
    self.run_road(V_EGO, curve_at(100.))
    assert self.scc_v.is_active
    self.run_road(14., curve_at(40.), n=1)
    assert self.scc_v.is_active
    assert self.scc_v.solver_active

  # -- arrive at the correct speed -------------------------------------------

  def test_holds_allowed_speed_inside_the_curve(self):
    # approach a touch fast, curve at the bumper
    self.run_road(12., curve_at(0.), cur_curvature=CURVE_KAPPA)
    assert self.scc_v.is_active
    # settled at the allowed speed: hold it, do not re-accelerate toward the setpoint
    self.run_road(CURVE_V, curve_at(0.), cur_curvature=CURVE_KAPPA, n=2)
    assert self.scc_v.state == VisionState.turning
    assert abs(self.scc_v.output_v_target - CURVE_V) < 1.0

  def test_releases_when_the_road_straightens(self):
    self.run_road(12., curve_at(0.), cur_curvature=CURVE_KAPPA)
    assert self.scc_v.is_active
    self.run_road(CURVE_V, lambda s: 0., cur_curvature=0., n=3)
    assert self.scc_v.state == VisionState.enabled
    assert self.scc_v.output_v_target == V_CRUISE_UNSET

  def test_hairpin_floors_at_min_v(self):
    # kappa 0.12 allows 4.1 m/s, below the 20 km/h operating floor
    self.run_road(6., curve_at(0., kappa=0.12), cur_curvature=0.12)
    assert self.scc_v.is_active
    assert self.scc_v.output_v_target == MIN_V

  # -- far-field curvature bias ----------------------------------------------

  def test_recovers_an_attenuated_far_corner(self):
    # a corner the model reports at 55% of its real curvature: the correction pulls the
    # planned speed back toward the truth instead of planning for the corner it was told
    road = curve_at(110., kappa=0.012)
    self.run_road(V_EGO, road, attenuate=True)
    truth = (2.0 * 0.95 / 0.012) ** 0.5
    with patch_gain([1.0] * len(vision_controller._KAPPA_BIAS_GAIN)):
      raw = SmartCruiseControlVision(make_cp())
      self.run_road(V_EGO, road, scc=raw, attenuate=True)
    # as reported the corner looks far faster than it is; corrected it lands much closer
    assert raw.v_dip_ahead > truth + 4.
    assert self.scc_v.v_dip_ahead < raw.v_dip_ahead - 2.
    assert self.scc_v.v_dip_ahead > truth  # under-corrects: the cap is deliberate

  def test_bias_correction_commits_earlier(self):
    # the whole point: a corner the reported geometry leaves under the commit gate is
    # already worth braking for once the model's under-read is undone
    road = curve_at(110., kappa=0.013)
    self.run_road(V_EGO, road, attenuate=True)
    assert self.scc_v.is_active

    with patch_gain([1.0] * len(vision_controller._KAPPA_BIAS_GAIN)):
      raw = SmartCruiseControlVision(make_cp())
      self.run_road(V_EGO, road, scc=raw, attenuate=True)
    assert not raw.is_active
    assert self.scc_v.a_required > raw.a_required

  def test_near_field_is_never_outvoted(self):
    # a constant-radius curve the car is already in: the far half of the SAME curve is
    # attenuated, so correcting it would settle the car below the speed the road requires.
    # The near floor holds it at the true allowed speed.
    self.run_road(12., curve_at(0.), cur_curvature=CURVE_KAPPA, attenuate=True)
    self.run_road(CURVE_V, curve_at(0.), cur_curvature=CURVE_KAPPA, n=2, attenuate=True)
    # the near field plans at the margin, and that is exactly where it settles
    near_allowed = (2.0 * 0.95 / CURVE_KAPPA) ** 0.5
    assert abs(self.scc_v.output_v_target - near_allowed) < 0.05
    # uncorrected the far half of the same curve reads gentler, so nothing drags it under
    with patch_gain([1.0] * len(vision_controller._KAPPA_BIAS_GAIN)):
      raw = SmartCruiseControlVision(make_cp())
      self.run_road(CURVE_V, curve_at(0.), cur_curvature=CURVE_KAPPA, n=3, scc=raw, attenuate=True)
    assert self.scc_v.output_v_target >= raw.output_v_target - 0.05

  def test_near_floor_does_not_block_braking_for_a_tighter_corner(self):
    # the floor only applies once the near field is what binds; a gentle bend under the
    # nose must not stop the car braking for a hairpin beyond it
    def road(s):
      return 0.008 if s < 90. else 0.06
    self.run_road(V_EGO, road, n=5)
    assert self.scc_v.is_active
    assert self.scc_v.output_v_target < V_EGO - 2.

  def test_straight_road_is_unaffected_by_the_gain(self):
    # a gain on a kappa of zero is still zero; no false braking is bought with it
    self.run_road(V_EGO, lambda s: 0., n=10)
    assert self.scc_v.a_required == 0.
    assert self.scc_v.output_v_target == V_CRUISE_UNSET

  def test_published_decel_stays_within_the_clip(self):
    # required_decel screams through its distance floor when a constraint sits inside the
    # actuation lead, and the gain puts more constraints there. The publication cap owns it.
    stock = SmartCruiseControlVision(make_cp(op_long=False))
    self.run_road(V_EGO, curve_at(100., kappa=0.012), n=60, scc=stock, attenuate=True)
    assert stock.output_a_target >= -2.0
    assert stock.output_v_target >= MIN_V

  # -- per-path budgets ------------------------------------------------------

  def test_stock_path_commits_earlier_and_prepositions_the_dip(self):
    # same road: op-long (1.2 budget, 0.36 s lead) still holds; stock ACC (0.75 budget,
    # response + dash traversal lead) is already inside its braking distance
    road = curve_at(140., kappa=0.014)
    self.run_road(V_EGO, road, attenuate=True)
    assert not self.scc_v.is_active

    stock = SmartCruiseControlVision(make_cp(op_long=False))
    self.run_road(V_EGO, road, scc=stock, attenuate=True)
    assert stock.is_active
    # the dash cannot track a profile; it gets sent to the dip itself
    assert abs(stock.output_v_target - stock.v_dip_ahead) < 1.0
    assert stock.v_dip_ahead < V_EGO


class TestLookaheadWire(OpenpilotTestCase):
  """v_ahead_min feeds the ICBM restore gate: 0 must mean exactly "no lookahead"."""

  def setup_method(self):
    self.params = Params()
    self.params.put_bool("SmartCruiseControlVision", True, block=True)
    self.scc_v = SmartCruiseControlVision(make_cp())

  def step(self, v=V_EGO, kappa_fn=lambda s: 0., long_enabled=True):
    sm = {'modelV2': model_for_road(v, kappa_fn).modelV2,
          'controlsState': messaging.new_message('controlsState').controlsState}
    self.scc_v.update(sm, long_enabled, False, v, 0., SETPOINT)

  def test_clear_road_caps_at_unset(self):
    self.step()
    assert self.scc_v.v_ahead_min == 255.

  def test_dip_passes_through(self):
    self.step(kappa_fn=curve_at(60.))
    assert 0. < self.scc_v.v_ahead_min < SETPOINT

  def test_long_disabled_reports_no_lookahead(self):
    self.step(kappa_fn=curve_at(60.))
    self.step(long_enabled=False)
    assert self.scc_v.v_ahead_min == 0.

  def test_toggle_off_reports_no_lookahead(self):
    self.params.put_bool("SmartCruiseControlVision", False, block=True)
    scc = SmartCruiseControlVision(make_cp())
    sm = {'modelV2': model_for_road(V_EGO, curve_at(60.)).modelV2,
          'controlsState': messaging.new_message('controlsState').controlsState}
    scc.update(sm, True, False, V_EGO, 0., SETPOINT)
    assert scc.v_ahead_min == 0.
