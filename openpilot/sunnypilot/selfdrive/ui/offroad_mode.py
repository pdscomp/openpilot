"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Force-offroad ("Always Offroad") request path.

Entering offroad is deferred through OffroadModeRequested: card runs the stock-ECU
hand-back first and then grants OffroadMode (see
openpilot/sunnypilot/selfdrive/car/alpha_long_toggle.py). hardwared grants the request
directly when there is no onroad session to hand back from, or if card does not finish
in time. Leaving offroad needs no coordination and clears OffroadMode directly.
"""
from openpilot.common.params import Params


def request_offroad_mode(params: Params, enable: bool) -> None:
  if enable:
    params.put_bool("OffroadModeRequested", True)
  else:
    params.put_bool("OffroadMode", False)
