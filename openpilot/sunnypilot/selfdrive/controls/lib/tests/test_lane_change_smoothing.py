"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
from types import SimpleNamespace

import pytest

from openpilot.cereal import log
from openpilot.common.params import Params
from openpilot.common.test import OpenpilotTestCase
from openpilot.selfdrive.controls.lib.drive_helpers import clip_curvature, MAX_LATERAL_JERK
from openpilot.sunnypilot.selfdrive.controls.lib.lane_change_smoothing import (
  LaneChangeSmoothing, pace_jerk_factor, pace_profile_time, lane_change_time_extra,
  PACE_MIN, PACE_MAX, ARREST_JERK_FLOOR, SMOOTH_RELEASE_T,
)

DT = 0.01


def make_cs(v_ego):
  return SimpleNamespace(vEgo=v_ego)


def make_model(state=log.LaneChangeState.off):
  return SimpleNamespace(meta=SimpleNamespace(laneChangeState=state))


IN_CHANGE = make_model(log.LaneChangeState.laneChangeStarting)
FINISHING = make_model(log.LaneChangeState.laneChangeFinishing)
OFF = make_model()


class TestPaceMapping:
  def test_profile_time_span(self):
    # pace 1 is the gentlest (~8 s), pace 9 just under stock timing
    assert pace_profile_time(PACE_MIN) == pytest.approx(8.0, abs=0.1)
    assert pace_profile_time(PACE_MAX) == pytest.approx(3.6, abs=0.1)

  def test_jerk_factor_monotonic_and_bounded(self):
    factors = [pace_jerk_factor(p) for p in range(PACE_MIN, PACE_MAX + 1)]
    assert all(0.0 < f <= 1.0 for f in factors)
    assert factors == sorted(factors)

  def test_pace5_matches_sinusoid(self):
    # j = pi^3 * 3.5 / t^3 * 1.3 headroom over the 5.0 ISO limit
    t = pace_profile_time(5)
    expected = (3.141592653589793 ** 3) * 3.5 / t ** 3 * 1.3 / MAX_LATERAL_JERK
    assert pace_jerk_factor(5) == pytest.approx(expected)

  def test_time_extra(self):
    assert lane_change_time_extra(PACE_MIN) == pytest.approx(2.0)
    assert lane_change_time_extra(9) == pytest.approx(2.0 / 9.0)


class TestLaneChangeSmoothing(OpenpilotTestCase):
  def setup_method(self):
    self.params = Params()
    self.params.put_bool("LaneChangeSmoothing", True, block=True)
    self.lcs = LaneChangeSmoothing()
    self.lcs.get_params()

  def test_disabled_returns_stock(self):
    self.params.put_bool("LaneChangeSmoothing", False, block=True)
    self.lcs.get_params()
    assert self.lcs.update(make_cs(15.0), IN_CHANGE, 0.002, 0.0) == 1.0

  def test_stock_outside_lane_change(self):
    assert self.lcs.update(make_cs(15.0), OFF, 0.002, 0.0) == 1.0

  def test_clamped_during_maneuver(self):
    jf = self.lcs.update(make_cs(15.0), IN_CHANGE, 0.001, 0.0)
    assert jf == pytest.approx(self.lcs.set_jerk)
    assert jf < 0.2  # pace 5 is a real clamp

  def test_taper_back_to_stock_after_maneuver(self):
    self.lcs.update(make_cs(15.0), IN_CHANGE, 0.001, 0.0)
    jfs = [self.lcs.update(make_cs(15.0), OFF, 0.0, 0.0) for _ in range(int(SMOOTH_RELEASE_T / DT) + 10)]
    assert jfs[-1] == 1.0
    assert all(b >= a - 1e-9 for a, b in zip(jfs[:-1], jfs[1:], strict=True))  # monotonic release

  def test_arrest_gets_extra_authority(self):
    # entry establishes the sign, then the model unwinds: the pursuit must raise the
    # factor above the entry clamp, up to the arrest floor
    self.lcs.update(make_cs(15.0), IN_CHANGE, 0.001, 0.0)
    jf = self.lcs.set_jerk
    for _ in range(100):
      jf = self.lcs.update(make_cs(15.0), IN_CHANGE, -0.002, 0.0)
    assert jf > self.lcs.set_jerk * 2
    assert jf <= ARREST_JERK_FLOOR + 1e-6

  def test_arrest_deadband_rejects_noise(self):
    self.lcs.update(make_cs(15.0), IN_CHANGE, 0.001, 0.0)
    jf = self.lcs.update(make_cs(15.0), IN_CHANGE, -0.00003, 0.0)
    assert jf == pytest.approx(self.lcs.set_jerk)

  def test_arrest_rise_is_smoothed(self):
    # the boost must ramp with the rise tau, not step
    self.lcs.update(make_cs(15.0), IN_CHANGE, 0.001, 0.0)
    jf1 = self.lcs.update(make_cs(15.0), IN_CHANGE, -0.005, 0.0)
    jf2 = self.lcs.update(make_cs(15.0), IN_CHANGE, -0.005, 0.0)
    assert jf1 < ARREST_JERK_FLOOR * 0.5  # far from the cap on the first boosted frame
    assert jf2 > jf1

  def test_finishing_state_keeps_clamp(self):
    jf = self.lcs.update(make_cs(15.0), FINISHING, 0.001, 0.0)
    assert jf == pytest.approx(self.lcs.set_jerk)

  def test_clip_curvature_scales_with_factor(self):
    full, _ = clip_curvature(15.0, 0.0, 0.01, 0.0)
    half, _ = clip_curvature(15.0, 0.0, 0.01, 0.0, jerk_factor=0.5)
    assert half == pytest.approx(full / 2)
    stock, _ = clip_curvature(15.0, 0.0, 0.01, 0.0, jerk_factor=1.0)
    assert stock == full
