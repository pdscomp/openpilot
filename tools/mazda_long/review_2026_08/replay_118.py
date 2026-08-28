"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of zoompilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.

Replay route 118 seg 9 (the 2026-08-27 SCBS latch at t+580.77) through the CarController.

This is the release that motivated the deferred-pulse design. On the build that recorded it
the emission was a byte-level twin of a stock latched release -- 9-frame pulse, command pinned
at -1 raw until the body dropped GEAR.BRAKE_HOLD, stock's +25 raw/frame ramp, real lead
advertised and departing -- and the camera still latched the fault 80 ms in. That made it the
fourth pulse out of four this port has ever emitted with a healthy camera, and the fourth latch.

So the check here is not a grammar check any more: it is that the release now puts NO
RESUME_UNLATCHING on the wire at all, because the body let go inside the deferral window.

Caveat this replay cannot escape: the body's GEAR.BRAKE_HOLD trace is the one it recorded
while we were pulsing at it. It shows the body releasing 30 ms after the pulse began -- well
inside RESUME_PULSE_DEFER_T -- but it cannot prove the body would have let go with no pulse
at all. That is the one question only a drive can answer, and the fallback exists for it.
"""
import sys, os, glob
sys.path.insert(0, "/Users/zeph/Developer/experiments/sunnypilot_proj/sunnypilot")
sys.path.insert(0, "/Users/zeph/Developer/experiments/sunnypilot_proj/sunnypilot/opendbc_repo")
sys.path.append("/Users/zeph/Developer/experiments/sunnypilot_proj/sunnypilot/tools/mazda_long")
os.chdir("/Users/zeph/Developer/experiments/sunnypilot_proj/sunnypilot")
from openpilot.tools.lib.logreader import LogReader
from replay_standstill_hold import build_controller, decode_cmd, mock_inputs

SEGS = sorted(glob.glob("tools/mazda_long/test_data/route_118/rlog_seg*.zst"),
              key=lambda p: int(p.split("seg")[1].split(".")[0]))
LATCH_T = 580.77


def replay():
  rows = []
  cc = lead = None
  brake_hold = False
  t0 = None
  for p in SEGS:
    for m in LogReader(p):
      w = m.which()
      t = m.logMonoTime * 1e-9
      if t0 is None:
        t0 = t
      tr = t - t0
      if tr > LATCH_T + 6.0:
        break
      if w == "carControl":
        cc = m.carControl
      elif w == "carControlSP":
        lo = m.carControlSP.leadOne
        lead = (lo.dRel, lo.vRel)
      elif w == "can":
        for c in m.can:
          if c.address == 0x228 and c.src == 0:
            brake_hold = bool((bytes(c.dat)[2] >> 4) & 1)
      elif w == "carState" and cc is not None:
        rows.append((tr, cc, m.carState, lead, brake_hold))

  ctrl = build_controller()
  out = []
  for tr, cc, cs, lead, brake_hold in rows:
    control, control_sp, carstate = mock_inputs(cc, cs, brake_hold, lead)
    sends = ctrl.update_longitudinal(control, control_sp, carstate)
    ctrl.frame += 1
    info = next((d for a, d, b in sends if a == 0x21b and b == 0), None)
    if info is not None:
      out.append((tr, decode_cmd(info), (info[5] >> 2) & 1, (info[6] >> 6) & 1, brake_hold))
  return out, ctrl


if __name__ == "__main__":
  out, ctrl = replay()
  win = [r for r in out if LATCH_T - 2.0 <= r[0] <= LATCH_T + 3.0]
  pulses = [r for r in win if r[3]]
  print(f"route 118 release window: {len(win)} frames, "
        f"latched_release={ctrl.stop_and_go.latched_release}")
  print(f"RESUME_UNLATCHING frames emitted: {len(pulses)}  "
        f"({'PASS - nothing for the camera to fault' if not pulses else 'FAIL - still pulsing'})")
  last = None
  for tr, c, s, u, h in win:
    key = (s, u, h)
    if key != last or u:
      print(f"  {tr:8.2f}  cmd={c:+6d} stop={s} unl={u} brake_hold={int(h)}")
      last = key
