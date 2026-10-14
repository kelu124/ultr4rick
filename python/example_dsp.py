#!/usr/bin/env python3
"""Example: drive the Ultr4rick RP2350B DSP firmware from Python.

Targets the ``-DDSP`` firmware build (``firmware/build.sh`` ->
``ultr4rick-dsp`` UF2). Exercises the API:

  * Ultr4rick.status()           -> parsed status dict
  * Ultr4rick.selftest()         -> on-board DSP self-test vs scipy
  * Ultr4rick.read_raw()         -> 8000-sample raw ADC frame
  * Ultr4rick.read_fft()         -> 4096-sample Hilbert-envelope frame
  * pulser arm + a pulsed capture (only with --arm)

Run against a connected board (auto-detects the USB-CDC port):

    python example_dsp.py                 # summary to stdout
    python example_dsp.py --save out/     # also save raw.npy / envelope.npy
    python example_dsp.py --plot out/     # also save a PNG (needs matplotlib)

Requires: pyserial, numpy (matplotlib only for --plot).
"""

import argparse
import os
import sys

import numpy as np

from ultr4rick import dsp
from ultr4rick.device import Ultr4rick


def summarize(name, frame):
    s = frame.samples()
    hdr = frame.header
    print(f"  {name:9s} type={hdr.payload_name:8s} samples={hdr.sample_count:5d} "
          f"bytes={hdr.payload_bytes:5d} seq={hdr.sequence}")
    if hdr.payload_type == 2:  # envelope float32
        print(f"            envelope peak={float(s.max()):.1f} "
              f"dc_mean={hdr.adc_dc_mean:.1f}")
    else:                      # raw uint16 / alaw uint8
        print(f"            min/mean/max = {int(s.min())}/{int(s.mean())}/"
              f"{int(s.max())}")
    return s


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", help="serial port (default: auto-detect)")
    ap.add_argument("--save", metavar="DIR",
                    help="save raw.npy / envelope.npy into DIR")
    ap.add_argument("--plot", metavar="DIR",
                    help="save capture.png into DIR (needs matplotlib)")
    ap.add_argument("--gain", type=int, default=300,
                    help="TGC DAC value 0..1023 (default 300)")
    ap.add_argument("--arm", action="store_true",
                    help="also arm the pulser for one pulsed capture "
                         "(fires the HV pulser!)")
    args = ap.parse_args(argv)

    probe = Ultr4rick(port=args.port, verbose=False)

    # 1) Identify the firmware. status() only works on a DSP build.
    try:
        st = probe.status()
    except ValueError:
        print("This board is not running a DSP (-DDSP) firmware build "
              "(no status line). Flash the ultr4rick-dsp UF2.",
              file=sys.stderr)
        return 2
    print("Board:", st.get("board"), st.get("package"),
          "| firmware", st.get("firmware"), "| backend", st.get("dsp_backend"))

    # 2) DSP self-test: synthetic vectors through the on-board Hilbert chain.
    results = dsp.check_selftest(probe.selftest())
    for name, (nrms, ok) in results.items():
        print(f"  selftest {name:12s} nrms={nrms:.2e} {'ok' if ok else 'FAIL'}")

    # 3) Set the TGC gain. The DSP build's command is `dac write <n>` (note:
    #    the stdio build uses `write dac <n>` / Ultr4rick.dac()).
    print(" ", probe.command(f"dac write {args.gain}"))

    # 4) One-shot captures with the pulser disarmed (idle input).
    print("Captures (pulser disarmed):")
    raw = summarize("read_raw", probe.read_raw())
    env = summarize("read_fft", probe.read_fft())

    # 5) Optional pulsed acquisition: configure, arm, capture, disarm.
    env_pulsed = None
    if args.arm:
        probe.command("pulse config 96 6000 96 neg-first")
        probe.command("pulser arm")
        try:
            print("Captures (pulser armed):")
            env_pulsed = summarize("read_fft", probe.read_fft())
        finally:
            probe.command("pulser disarm")

    # 6) Optional save / plot.
    if args.save:
        os.makedirs(args.save, exist_ok=True)
        np.save(os.path.join(args.save, "raw.npy"), raw)
        np.save(os.path.join(args.save, "envelope.npy"), env)
        print("Saved raw.npy / envelope.npy to", args.save)

    if args.plot:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        os.makedirs(args.plot, exist_ok=True)
        fig, ax = plt.subplots(2, 1, figsize=(9, 6))
        ax[0].plot(raw); ax[0].set_title("read_raw (8000 ADC samples)")
        ax[1].plot(env, label="idle")
        if env_pulsed is not None:
            ax[1].plot(env_pulsed, label="pulser armed")
        ax[1].set_title("read_fft (4096-pt Hilbert envelope)"); ax[1].legend()
        fig.tight_layout()
        out = os.path.join(args.plot, "capture.png")
        fig.savefig(out, dpi=110)
        print("Saved", out)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
