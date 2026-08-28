"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
from openpilot.cereal import custom
from openpilot.selfdrive.car.helpers import convert_carControlSP


class TestConvertCarControlSP:
  def test_capnp_only_substructs_do_not_crash(self):
    """capnp CarControlSP can carry substructs (e.g. turnAssist) that have no field on
    the opendbc dataclass; the converter must drop them instead of passing them as kwargs."""
    msg = custom.CarControlSP.new_message()
    msg.turnAssist.holdCurvature = 0.1
    msg.laneChangeSmoothing.jerkFactor = 0.5
    msg.mads.enabled = True
    msg.leadOne.dRel = 12.5

    out = convert_carControlSP(msg.as_reader())
    assert out.mads.enabled is True
    assert abs(out.leadOne.dRel - 12.5) < 1e-6
