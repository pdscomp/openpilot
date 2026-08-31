"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""

import pytest

from openpilot.cereal import custom
from opendbc.car.car_helpers import interfaces
from opendbc.car.structs import car as car_struct
from opendbc.car.rivian.values import CAR as RIVIAN
from opendbc.car.tesla.values import CAR as TESLA
from opendbc.car.toyota.values import CAR as TOYOTA
from openpilot.common.constants import CV
from openpilot.common.params import Params
from openpilot.common.realtime import DT_MDL
from openpilot.selfdrive.car.cruise import V_CRUISE_UNSET
from openpilot.sunnypilot import PARAMS_UPDATE_PERIOD
from openpilot.sunnypilot.selfdrive.car import interfaces as sunnypilot_interfaces
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit import PCM_LONG_REQUIRED_MAX_SET_SPEED
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit.common import Mode
from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit.speed_limit_assist import SpeedLimitAssist, \
  PRE_ACTIVE_GUARD_PERIOD, ACTIVE_STATES
from openpilot.sunnypilot.selfdrive.car.cruise_arbiter import CruiseArbiter, \
  PRE_ACTIVE_GUARD_PERIOD as ARBITER_PROMPT_PERIOD, DISABLED_GUARD_PERIOD as ARBITER_GUARD_PERIOD
from openpilot.common.realtime import DT_CTRL
from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP

SpeedLimitAssistState = custom.LongitudinalPlanSP.SpeedLimit.AssistState
ButtonType = car_struct.CarState.ButtonEvent.Type

ALL_STATES = tuple(SpeedLimitAssistState.schema.enumerants.values())

SPEED_LIMITS = {
  'residential': 25 * CV.MPH_TO_MS,  # 25 mph
  'city': 35 * CV.MPH_TO_MS,         # 35 mph
  'highway': 65 * CV.MPH_TO_MS,      # 65 mph
  'freeway': 80 * CV.MPH_TO_MS,      # 80 mph
}

DEFAULT_CAR = TOYOTA.TOYOTA_RAV4_TSS2


@pytest.fixture
def car_name(request):
  return getattr(request, "param", DEFAULT_CAR)


@pytest.fixture(autouse=True)
def set_car_name_on_instance(request, car_name):
  instance = getattr(request, "instance", None)
  if instance:
    instance.car_name = car_name


class TestSpeedLimitAssist:

  def setup_method(self, method):
    self.params = Params()
    self.reset_custom_params()
    self.events_sp = EventsSP()
    CI = self._setup_platform(self.car_name)
    self.sla = SpeedLimitAssist(CI.CP, CI.CP_SP)
    self.sla.pre_active_timer = int(PRE_ACTIVE_GUARD_PERIOD / DT_MDL)
    self.pcm_long_max_set_speed = PCM_LONG_REQUIRED_MAX_SET_SPEED[self.sla.is_metric][1]  # use 80 MPH for now
    self.speed_conv = CV.MS_TO_KPH if self.sla.is_metric else CV.MS_TO_MPH

  def teardown_method(self, method):
    self.reset_state()

  def _setup_platform(self, car_name):
    CarInterface = interfaces[car_name]
    CP = CarInterface.get_non_essential_params(car_name)
    CP_SP = CarInterface.get_non_essential_params_sp(CP, car_name)
    CI = CarInterface(CP, CP_SP)
    CI.CP.openpilotLongitudinalControl = True  # always assume it's openpilot longitudinal
    sunnypilot_interfaces.setup_interfaces(CI, self.params)
    return CI

  def reset_custom_params(self):
    self.params.put("IsReleaseSpBranch", True, block=True)
    self.params.put("SpeedLimitMode", int(Mode.assist), block=True)
    self.params.put_bool("IsMetric", False, block=True)
    self.params.put("SpeedLimitOffsetType", 0, block=True)
    self.params.put("SpeedLimitValueOffset", 0, block=True)

  def reset_state(self):
    self.sla.state = SpeedLimitAssistState.disabled
    self.sla.frame = -1
    self.sla.last_op_engaged_frame = 0
    self.sla.op_engaged = False
    self.sla.op_engaged_prev = False
    self.sla._speed_limit = 0.
    self.sla.speed_limit_prev = 0.
    self.sla.last_valid_speed_limit_offsetted = 0.
    self.sla._distance = 0.
    self.events_sp.clear()

  def initialize_active_state(self, initialize_v_cruise):
    self.sla.state = SpeedLimitAssistState.active
    self.sla.v_cruise_cluster = initialize_v_cruise
    self.sla.v_cruise_cluster_prev = initialize_v_cruise
    self.sla.prev_v_cruise_cluster_conv = round(initialize_v_cruise * self.speed_conv)

  def test_initial_state(self):
    assert self.sla.state == SpeedLimitAssistState.disabled
    assert not self.sla.is_enabled
    assert not self.sla.is_active
    assert V_CRUISE_UNSET == self.sla.get_v_target_from_control()

  @pytest.mark.parametrize("car_name", [RIVIAN.RIVIAN_R1, TESLA.TESLA_MODEL_Y], indirect=True)
  def test_disallowed_brands(self, car_name):
    """
      Speed Limit Assist is disabled for the following brands and conditions:
      - All Tesla and is a release branch;
      - All Rivian
    """
    assert not self.sla.enabled

    # stay disallowed even when the param may have changed from somewhere else
    self.params.put("SpeedLimitMode", int(Mode.assist), block=True)
    for _ in range(int(PARAMS_UPDATE_PERIOD / DT_MDL)):
      self.sla.update(True, False, SPEED_LIMITS['city'], 0, SPEED_LIMITS['highway'], SPEED_LIMITS['city'],
                      SPEED_LIMITS['city'], True, 0, self.events_sp)
    assert not self.sla.enabled

  def test_disabled(self):
    self.params.put("SpeedLimitMode", int(Mode.off), block=True)
    for _ in range(int(10. / DT_MDL)):
      self.sla.update(True, False, SPEED_LIMITS['city'], 0, SPEED_LIMITS['highway'], SPEED_LIMITS['city'], SPEED_LIMITS['city'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.disabled

  def test_transition_disabled_to_preactive(self):
    for _ in range(int(3. / DT_MDL)):
      self.sla.update(True, False, SPEED_LIMITS['city'], 0, SPEED_LIMITS['highway'], SPEED_LIMITS['city'], SPEED_LIMITS['city'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.preActive
    assert self.sla.is_enabled and not self.sla.is_active

  def test_transition_disabled_to_pending_no_speed_limit_not_max_initial_set_speed(self):
    for _ in range(int(3. / DT_MDL)):
      self.sla.update(True, False, SPEED_LIMITS['highway'], 0, SPEED_LIMITS['city'], 0, 0, False, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.pending
    assert self.sla.is_enabled and not self.sla.is_active

  def test_preactive_to_active_with_max_speed_confirmation(self):
    self.sla.state = SpeedLimitAssistState.preActive
    self.sla.update(True, False, SPEED_LIMITS['city'], 0, self.pcm_long_max_set_speed, SPEED_LIMITS['highway'],
                    SPEED_LIMITS['highway'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.active
    assert self.sla.is_enabled and self.sla.is_active
    assert self.sla.output_v_target == SPEED_LIMITS['highway']

  def test_preactive_timeout_to_inactive(self):
    self.sla.state = SpeedLimitAssistState.preActive
    self.sla.update(True, False, SPEED_LIMITS['city'], 0, SPEED_LIMITS['highway'], SPEED_LIMITS['city'], SPEED_LIMITS['city'], True, 0, self.events_sp)

    for _ in range(int(PRE_ACTIVE_GUARD_PERIOD / DT_MDL)):
      self.sla.update(True, False, SPEED_LIMITS['city'], 0, SPEED_LIMITS['highway'], SPEED_LIMITS['city'], SPEED_LIMITS['city'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.inactive

  def test_preactive_to_pending_no_speed_limit(self):
    self.sla.state = SpeedLimitAssistState.preActive
    self.sla.update(True, False, SPEED_LIMITS['highway'], 0, self.pcm_long_max_set_speed, 0, 0, False, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.pending
    assert self.sla.is_enabled and not self.sla.is_active

  def test_pending_to_active_when_speed_limit_available(self):
    self.sla.state = SpeedLimitAssistState.pending
    self.sla.v_cruise_cluster_prev = self.pcm_long_max_set_speed
    self.sla.prev_v_cruise_cluster_conv = round(self.pcm_long_max_set_speed * self.speed_conv)

    self.sla.update(True, False, SPEED_LIMITS['highway'], 0, self.pcm_long_max_set_speed,
                    SPEED_LIMITS['highway'], SPEED_LIMITS['highway'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.active

  def test_pending_to_adapting_when_below_speed_limit(self):
    self.sla.state = SpeedLimitAssistState.pending
    self.sla.v_cruise_cluster_prev = self.pcm_long_max_set_speed
    self.sla.prev_v_cruise_cluster_conv = round(self.pcm_long_max_set_speed * self.speed_conv)

    self.sla.update(True, False, SPEED_LIMITS['highway'] + 5, 0, self.pcm_long_max_set_speed,
                    SPEED_LIMITS['highway'], SPEED_LIMITS['highway'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.adapting
    assert self.sla.is_enabled and self.sla.is_active

  def test_active_to_adapting_transition(self):
    self.initialize_active_state(self.pcm_long_max_set_speed)

    self.sla.update(True, False, SPEED_LIMITS['highway'] + 2, 0, self.pcm_long_max_set_speed, SPEED_LIMITS['highway'],
                    SPEED_LIMITS['highway'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.adapting

  def test_adapting_to_active_transition(self):
    self.sla.state = SpeedLimitAssistState.adapting
    self.sla.v_cruise_cluster_prev = self.pcm_long_max_set_speed
    self.sla.prev_v_cruise_cluster_conv = round(self.pcm_long_max_set_speed * self.speed_conv)

    self.sla.update(True, False, SPEED_LIMITS['city'], 0, self.pcm_long_max_set_speed, SPEED_LIMITS['highway'],
                    SPEED_LIMITS['highway'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.active

  def test_manual_cruise_change_detection(self):
    self.sla.state = SpeedLimitAssistState.active
    expected_cruise = SPEED_LIMITS['highway']
    self.sla.v_cruise_cluster_prev = expected_cruise

    different_cruise = SPEED_LIMITS['highway'] + 5
    self.sla.update(True, False, SPEED_LIMITS['city'], 0, different_cruise, SPEED_LIMITS['city'], SPEED_LIMITS['city'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.inactive

  # TODO-SP: test lower CST cases
  def test_rapid_speed_limit_changes(self):
    self.initialize_active_state(self.pcm_long_max_set_speed)
    speed_limits = [SPEED_LIMITS['highway'], SPEED_LIMITS['freeway']]

    for _, speed_limit in enumerate(speed_limits):
      self.sla.update(True, False, speed_limit, 0, self.pcm_long_max_set_speed, speed_limit, speed_limit, True, 0, self.events_sp)
    assert self.sla.state in ACTIVE_STATES

  def test_invalid_speed_limits_handling(self):
    self.initialize_active_state(self.pcm_long_max_set_speed)

    invalid_limits = [-10, 0, 200 * CV.MPH_TO_MS]

    for invalid_limit in invalid_limits:
      self.sla.update(True, False, SPEED_LIMITS['city'], 0, self.pcm_long_max_set_speed, invalid_limit, SPEED_LIMITS['city'], True, 0, self.events_sp)
      assert isinstance(self.sla.output_v_target, (int, float))
      assert self.sla.output_v_target == V_CRUISE_UNSET or self.sla.output_v_target > 0

  def test_stale_data_handling(self):
    self.initialize_active_state(self.pcm_long_max_set_speed)
    old_speed_limit = SPEED_LIMITS['city']

    self.sla.update(True, False, SPEED_LIMITS['city'], 0, self.pcm_long_max_set_speed, 0, old_speed_limit, True, 0, self.events_sp)
    assert self.sla.state in ACTIVE_STATES
    assert self.sla.output_v_target == old_speed_limit

  def test_distance_based_adapting(self):
    self.sla.state = SpeedLimitAssistState.adapting
    self.sla.v_cruise_cluster_prev = self.pcm_long_max_set_speed
    self.sla.prev_v_cruise_cluster_conv = round(self.pcm_long_max_set_speed * self.speed_conv)

    distance = 100.0
    current_speed = SPEED_LIMITS['freeway']
    target_speed = SPEED_LIMITS['highway']

    self.sla.update(True, False, current_speed, 0, self.pcm_long_max_set_speed, target_speed, target_speed, True, distance, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.adapting
    assert self.sla.output_v_target == target_speed
    # required decel to the sign: (limit^2 - v_ego^2) / (2 * d), reached through the
    # publication ramp
    expected = (target_speed ** 2 - current_speed ** 2) / (2. * distance)
    for _ in range(40):
      self.sla.update(True, False, current_speed, 0, self.pcm_long_max_set_speed, target_speed, target_speed, True, distance, self.events_sp)
    assert self.sla.output_a_target == pytest.approx(max(expected, -2.0), abs=1e-3)

  def test_long_disengaged_to_disabled(self):
    self.initialize_active_state(self.pcm_long_max_set_speed)

    self.sla.update(False, False, SPEED_LIMITS['city'], 0, self.pcm_long_max_set_speed, SPEED_LIMITS['city'],
                    SPEED_LIMITS['city'], True, 0, self.events_sp)
    assert self.sla.state == SpeedLimitAssistState.disabled
    assert self.sla.output_v_target == V_CRUISE_UNSET

  def test_maintain_states_with_no_changes(self):
    """Test that states are maintained when no significant changes occur"""
    test_states = [
      SpeedLimitAssistState.preActive,
      SpeedLimitAssistState.pending,
      SpeedLimitAssistState.active,
      SpeedLimitAssistState.adapting
    ]

    for state in test_states:
      self.sla.state = state
      self.sla.op_engaged = True

      initial_state = state

      self.sla.update(True, False, SPEED_LIMITS['city'], 0, self.pcm_long_max_set_speed, SPEED_LIMITS['city'], SPEED_LIMITS['city'], True, 0, self.events_sp)

      assert self.sla.state in ALL_STATES  # Sanity check

      if initial_state == SpeedLimitAssistState.preActive:
        assert self.sla.state in [SpeedLimitAssistState.preActive, SpeedLimitAssistState.active]
      elif initial_state in ACTIVE_STATES:
        assert self.sla.state in ACTIVE_STATES


class TestCruiseArbiterNonPcm:
  """Stock-ACC (button-actuated) cars: pcmCruise=True, openpilotLongitudinalControl=False,
  pcmCruiseSpeed=False. The session runs in the card-side cruise arbiter, synchronous
  with the buttons: presses are classified once at their edges (dismiss at press,
  confirm/decline at release), so the confirm press's own dash step and ICBM-injected
  presses can never tear a session down (the seg16 F1 bug class)."""

  MPH = CV.MPH_TO_MS

  def setup_method(self, method):
    self.params = Params()
    self.params.put("IsReleaseSpBranch", True, block=True)
    self.params.put("SpeedLimitMode", int(Mode.assist), block=True)
    self.params.put_bool("IsMetric", False, block=True)

    CarInterface = interfaces[DEFAULT_CAR]
    CP = CarInterface.get_non_essential_params(DEFAULT_CAR)
    CP_SP = CarInterface.get_non_essential_params_sp(CP, DEFAULT_CAR)
    CP.openpilotLongitudinalControl = False
    CP.pcmCruise = True
    CP_SP.pcmCruiseSpeed = False
    self.arb = CruiseArbiter(CP, CP_SP)
    self.arb.read_params(self.params)
    assert self.arb.applicable
    self.v_cruise_kph = 60 * CV.MPH_TO_KPH

  def _lp(self, limit_mph, has_limit=True):
    lp = custom.LongitudinalPlanSP()
    lp.speedLimit.resolver.speedLimit = limit_mph * self.MPH
    lp.speedLimit.resolver.speedLimitFinalLast = limit_mph * self.MPH
    lp.speedLimit.resolver.speedLimitLastValid = has_limit
    return lp

  def frame(self, cluster_mph, limit_mph, has_limit=True, events=None):
    self.arb.update_limit(self._lp(limit_mph, has_limit))
    CS = car_struct.CarState()
    CS.buttonEvents = events or []
    self.v_cruise_kph = self.arb.step(CS, True, self.v_cruise_kph, cluster_mph * CV.MPH_TO_KPH)

  def press(self, button_type, cluster_mph, limit_mph, has_limit=True):
    self.frame(cluster_mph, limit_mph, has_limit,
               [car_struct.CarState.ButtonEvent(type=button_type, pressed=True)])
    self.frame(cluster_mph, limit_mph, has_limit,
               [car_struct.CarState.ButtonEvent(type=button_type, pressed=False)])

  def go_pre_active(self, cluster_mph, limit_mph):
    self.arb.state = SpeedLimitAssistState.preActive
    self.arb.pre_active_timer = int(ARBITER_PROMPT_PERIOD / DT_CTRL)
    self.frame(cluster_mph, limit_mph)
    assert self.arb.state == SpeedLimitAssistState.preActive

  def test_confirm_press_sticks(self):
    """F1 regression: one + press confirms; neither the press's own dash step nor ICBM
    walking the dash afterward may tear the session down."""
    self.go_pre_active(cluster_mph=40, limit_mph=45)

    self.press(ButtonType.accelCruise, 40, 45)
    assert self.arb.state == SpeedLimitAssistState.active

    # the ECU applies the confirm press's own +1 next frame
    self.frame(41, 45)
    assert self.arb.state == SpeedLimitAssistState.active
    # ICBM walks the dash to the target across the following seconds
    for cluster in (42, 43, 44, 45):
      self.frame(cluster, 45)
      assert self.arb.state == SpeedLimitAssistState.active

  def test_up_confirm_adopts_setpoint(self):
    """+ on a prompt above the setpoint raises v_cruise to the limit (never lowers it);
    the confirm press itself must not also increment."""
    self.v_cruise_kph = 40 * CV.MPH_TO_KPH
    self.go_pre_active(cluster_mph=40, limit_mph=45)

    self.press(ButtonType.accelCruise, 40, 45)
    assert self.arb.state == SpeedLimitAssistState.active
    assert round(self.v_cruise_kph * CV.KPH_TO_MPH) == 45

  def test_down_confirm_keeps_baseline(self):
    self.go_pre_active(cluster_mph=60, limit_mph=45)
    self.press(ButtonType.decelCruise, 60, 45)
    assert self.arb.state == SpeedLimitAssistState.active
    assert round(self.v_cruise_kph * CV.KPH_TO_MPH) == 60

  def test_wrong_direction_press_declines(self):
    """A release against the confirm direction is a decline: the session ends right away
    (instead of a prompt lingering over a driver who is dialing the other way), the press
    still increments, and the next limit change re-prompts normally."""
    self.go_pre_active(cluster_mph=40, limit_mph=45)

    self.press(ButtonType.decelCruise, 40, 45)  # limit is above: requires +
    assert self.arb.state == SpeedLimitAssistState.inactive
    assert not self.arb.press_owned(ButtonType.decelCruise)

    # declining is not a dismissal: a new limit re-prompts
    self.frame(40, 35)
    assert self.arb.state == SpeedLimitAssistState.preActive

  def test_settled_press_deactivates(self):
    """Settled at the limit, a press hands the buttons back at the press edge."""
    self.go_pre_active(cluster_mph=50, limit_mph=45)
    self.press(ButtonType.decelCruise, 50, 45)
    assert self.arb.state == SpeedLimitAssistState.active
    self.frame(45, 45)  # ICBM finished the move

    self.frame(45, 45, events=[car_struct.CarState.ButtonEvent(type=ButtonType.accelCruise, pressed=True)])
    assert self.arb.state == SpeedLimitAssistState.inactive
    assert self.arb.press_owned(ButtonType.accelCruise)  # the ECU step re-anchors, no increment
    self.frame(45, 45, events=[car_struct.CarState.ButtonEvent(type=ButtonType.accelCruise, pressed=False)])
    assert self.arb.press_owned(ButtonType.accelCruise)

  def test_mid_move_press_aborts(self):
    """A + press while ICBM is still walking the dash down aborts the session; the servo
    then restores the driver's setpoint because the plan min releases."""
    self.go_pre_active(cluster_mph=60, limit_mph=45)
    self.press(ButtonType.decelCruise, 60, 45)
    assert self.arb.state == SpeedLimitAssistState.active

    self.press(ButtonType.accelCruise, 52, 45)  # mid-walk
    assert self.arb.state == SpeedLimitAssistState.inactive

  def test_rearms_on_next_limit_change(self):
    self.test_mid_move_press_aborts()

    self.frame(60, 45)  # same limit: stays down
    assert self.arb.state == SpeedLimitAssistState.inactive
    self.frame(60, 35)  # new limit posted -> new session
    assert self.arb.state == SpeedLimitAssistState.preActive

  def test_dial_to_target_confirms(self):
    """Reaching the limit by hand is a confirmation (upstream semantics)."""
    self.go_pre_active(cluster_mph=50, limit_mph=45)
    self.frame(45, 45)
    assert self.arb.state == SpeedLimitAssistState.active

  def test_settled_dismissal_does_not_reactivate(self):
    """After a settled-press dismissal the cluster still equals the limit until the ECU's
    own +-1 step lands; the dial-to-target auto-confirm must not re-arm and fight the
    driver. Dismissal holds until the next limit change."""
    self.test_settled_press_deactivates()

    for cluster in (45, 45, 46, 46, 46):
      self.frame(cluster, 45)
      assert self.arb.state == SpeedLimitAssistState.inactive

    self.frame(46, 35)  # a genuinely new limit re-arms
    assert self.arb.state == SpeedLimitAssistState.preActive

  def test_limit_dropout_holds_session(self):
    """Map dropout mid-session: the state survives; the cap releases only through the
    resolver's own last-limit semantics."""
    self.go_pre_active(cluster_mph=50, limit_mph=45)
    self.press(ButtonType.decelCruise, 50, 45)
    assert self.arb.state == SpeedLimitAssistState.active

    self.frame(45, 45, has_limit=False)
    assert self.arb.state == SpeedLimitAssistState.active

  def test_prompt_freezes_cap_out_of_session(self):
    """A limit change mid-session prompts and freezes the plan cap at the old session
    target until answered; a decline releases it."""
    self.go_pre_active(cluster_mph=48, limit_mph=40)
    self.press(ButtonType.decelCruise, 48, 40)
    assert self.arb.state == SpeedLimitAssistState.active
    self.frame(40, 40)
    old_cap = self.arb.v_cap
    assert abs(old_cap - 40 * self.MPH) < 0.1

    self.frame(40, 45)  # limit rises: prompt
    assert self.arb.state == SpeedLimitAssistState.preActive
    for _ in range(50):
      self.frame(40, 45)
      assert self.arb.state == SpeedLimitAssistState.preActive
      assert self.arb.v_cap == old_cap, "prompt must freeze the plan cap"

    self.press(ButtonType.accelCruise, 40, 45)  # confirm up
    assert self.arb.state == SpeedLimitAssistState.active
    assert abs(self.arb.v_cap - 45 * self.MPH) < 0.1

  def test_prompt_times_out(self):
    self.go_pre_active(cluster_mph=50, limit_mph=45)
    for _ in range(int(ARBITER_PROMPT_PERIOD / DT_CTRL) + 2):
      self.frame(50, 45)
    assert self.arb.state == SpeedLimitAssistState.inactive

  def test_wall_clock_timers(self):
    """The arbiter runs at 100 Hz: the prompt window and engage guard must still be
    their wall-clock durations."""
    assert int(ARBITER_PROMPT_PERIOD / DT_CTRL) == 500   # 5 s at 100 Hz
    assert int(ARBITER_GUARD_PERIOD / DT_CTRL) == 50     # 0.5 s at 100 Hz

  def test_applicability_matches_planner_selection(self):
    """The arbiter must cover exactly the cars plannerd mirrors (everything that is not
    pcm-op-long): stock-ACC button cars AND op-long ports without pcmCruise. A mismatch
    leaves a car mirroring a permanently disabled session."""
    CarInterface = interfaces[DEFAULT_CAR]
    CP = CarInterface.get_non_essential_params(DEFAULT_CAR)
    CP_SP = CarInterface.get_non_essential_params_sp(CP, DEFAULT_CAR)

    for op_long, pcm_cruise, pcm_cruise_speed in [(False, True, False),  # ICBM (Mazda)
                                                  (True, False, True),   # op-long, no pcmCruise (most ports)
                                                  (True, True, True)]:   # pcm-op-long (plannerd machine)
      CP.openpilotLongitudinalControl = op_long
      CP.pcmCruise = pcm_cruise
      CP_SP.pcmCruiseSpeed = pcm_cruise_speed
      arb = CruiseArbiter(CP, CP_SP)
      mirrored_by_plannerd = not (op_long and pcm_cruise)
      assert arb.applicable == mirrored_by_plannerd, (op_long, pcm_cruise, pcm_cruise_speed)

  def test_op_long_non_pcm_car_confirms(self):
    """Op-long without pcmCruise (e.g. most Hyundai/Honda ports): buttons are
    openpilot's own; the arbiter session and confirm flow must work there too."""
    CarInterface = interfaces[DEFAULT_CAR]
    CP = CarInterface.get_non_essential_params(DEFAULT_CAR)
    CP_SP = CarInterface.get_non_essential_params_sp(CP, DEFAULT_CAR)
    CP.openpilotLongitudinalControl = True
    CP.pcmCruise = False
    CP_SP.pcmCruiseSpeed = True
    self.arb = CruiseArbiter(CP, CP_SP)
    self.arb.read_params(self.params)
    assert self.arb.applicable
    self.v_cruise_kph = 40 * CV.MPH_TO_KPH

    self.go_pre_active(cluster_mph=40, limit_mph=45)
    self.press(ButtonType.accelCruise, 40, 45)
    assert self.arb.state == SpeedLimitAssistState.active
    assert round(self.v_cruise_kph * CV.KPH_TO_MPH) == 45


class TestAssistMirrorDefaultMessage:
  """plannerd ignores carStateSP in its health checks, so the mirror can be fed capnp's
  default CruiseSession (vCap=0.0) before the first card message arrives or in process
  replay without a carStateSP pub. That 0 must not become the plan target."""

  def _mirror(self):
    from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit.assist_mirror import SpeedLimitAssistMirror
    return SpeedLimitAssistMirror(None, None)

  def test_default_session_yields_unset(self):
    from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP
    session = custom.CarStateSP.new_message().cruiseSession
    mirror = self._mirror()
    mirror.update(session, v_ego=25.0, distance=0.0, a_ego=0.0, events_sp=EventsSP())
    assert mirror.output_v_target == V_CRUISE_UNSET

  def test_real_cap_passes_through(self):
    from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP
    session = custom.CarStateSP.new_message().cruiseSession
    session.vCap = 22.5
    mirror = self._mirror()
    mirror.update(session, v_ego=20.0, distance=0.0, a_ego=0.0, events_sp=EventsSP())
    assert mirror.output_v_target == pytest.approx(22.5)

  def test_active_cap_below_v_ego_publishes_required_decel(self):
    from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP
    session = custom.CarStateSP.new_message().cruiseSession
    session.state = SpeedLimitAssistState.adapting
    session.vCap = 20.0
    mirror = self._mirror()
    # 25 -> 20 m/s over 150 m needs (400 - 625) / 300 = -0.75; the publication ramp
    # walks there at 2 m/s3, so run it to convergence
    for _ in range(20):
      mirror.update(session, v_ego=25.0, distance=150.0, a_ego=0.0, events_sp=EventsSP())
    assert mirror.output_a_target == pytest.approx(-0.75)

  def test_inactive_session_tracks_a_ego(self):
    from openpilot.sunnypilot.selfdrive.selfdrived.events import EventsSP
    session = custom.CarStateSP.new_message().cruiseSession
    session.vCap = 20.0  # cap present but session not active
    mirror = self._mirror()
    mirror.update(session, v_ego=25.0, distance=150.0, a_ego=-0.2, events_sp=EventsSP())
    assert mirror.output_a_target == pytest.approx(-0.2)
