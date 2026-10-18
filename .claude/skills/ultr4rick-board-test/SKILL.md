---
name: ultr4rick-board-test
description: Talk to a flashed Ultr4rick board over USB CDC with the python/ultr4rick library — version/status, DSP self-test against scipy, raw/envelope/A-law captures, streaming, and (only when asked) pulsed captures. Use after flashing or to reproduce hardware behaviour. ultr4rick repo only.
---

# Ultr4rick on-board testing

The host library is `python/ultr4rick` (`device.Ultr4rick`, `dsp`). Run scripts with `python/` as the working
directory so `import ultr4rick` resolves.

## Host environment

The system Python has no scipy. Use a venv in the scratchpad:

```bash
python3 -m venv $SCRATCH/venv && $SCRATCH/venv/bin/pip install -r requirements.txt   # repo root
```

## Before talking to the board
- `lsusb | grep -E 'cafe:4011|2e8a:0009'`: DSP build vs stdio build.
- `fuser -v /dev/ttyACM0` must show no other process. A serial monitor from another session once stole bytes, which
  made frames time out and the version parse come back partial. Ask before killing anything you didn't start.

## Standard check (safe: pulser stays disarmed)

```python
from ultr4rick.device import Ultr4rick
from ultr4rick import dsp
p = Ultr4rick(verbose=False)            # prints "Firmware <fw> - library <lib>"
print(p.versions())                      # (firmware_version, library_version), read at connection
print(p.version())                       # version, board ultr4rick, chip RP2350B, mux enabled, build hash
st = p.status(); assert not st["pulser_armed"]; print(st["raw"])
print(p.command("dac write 300"), p.command("write mux 0"), p.command("clear mux"))
print(dsp.check_selftest(p.selftest()))  # 7 cases, each (nrms, True); pass at nrms <= 1e-4
raw, env, alaw = p.read_raw(), p.read_fft(), p.capture("alaw")
p.stream_start("envelope", 20); frames = list(p.stream_frames(20)); print(p.stream_stop())   # drops=0
p.close()
```

`status` should show `board=ultr4rick package=RP2350B firmware=<version> pulser=disarmed`. While streaming the
firmware sends only binary frames; text replies resume after `stream stop`. Max rates: raw 100, envelope 50,
A-law 70 Hz.

## Pulsed capture (only when the user explicitly asks)

```python
p.command("pulse config 96 6000 96 neg-first")
try:
    p.command("pulser arm"); raw = p.read_raw(); env = p.read_fft()
finally:
    p.command("pulser disarm")
assert not p.status()["pulser_armed"]
```

`python/example_dsp.ipynb` section 15 does this and plots idle vs pulsed (`python/data/pulsed_capture.png`).

## Safety rules
- **Never** arm the pulser unless the user explicitly asks for a pulser test, and always disarm in a `finally`.
  GPIO22/23 drive the HV pulser.
- Assume the high-voltage supply may be connected.

## Known results
- 2026-10-18, maintainer's run of `example_dsp.ipynb` mux section with elements 1–7 (`e199deb`,
  `python/data/mux_elements.png`): echo trains on element 2 (~13 µs spacing) and element 4 (~20 µs), nothing on the
  others. Transducers can also go on the SMA `PULSER_OUT`/`AFE_IN` connectors with a tee, mux cleared
  (`docs/images/v0/v0_working.jpg`).
- 2026-10-14, firmware 0.1.15 (runs from SRAM):
  - One-shot envelope `dsp_us` 6.55 ms (FFTs 2.3/2.6 ms), worst with A-law 7.3 ms.
  - Streams: 20 Hz envelope and 70 Hz A-law sustain with ≤ 1 drop. 50 Hz envelope drops ~25% (USB-bound).
  - `status` `drops` is cumulative since boot: compare before/after.
- 2026-10-14, firmware 0.1.12 DSP:
  - Self-test 7/7, worst nRMS 8.4e-8.
  - Idle raw mean ~517 counts, σ 0.9.
  - DSP worst 14.2 ms.
  - Stream: 20 frames at 20 Hz, 0 drops.
- First capture after idle has a start-of-frame transient (raw min/max 208/892 seen). Discard it before trusting the data.
- Pulsed, gain 300, 96/6000/96 neg-first:
  - The excitation starts at 0.92 µs and the receiver saturates until about 6.2 µs.
  - Echoes about every 41.3 µs, all saturated at gain 300. Use gain ~50–150.
