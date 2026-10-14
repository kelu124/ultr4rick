---
name: ultr4rick-board-test
description: Talk to a flashed Ultr4rick board over its USB CDC command line and run the host capture/self-test tool (tools/ultr4rick_capture.py). Use after flashing, to check firmware version/status, run the DSP self-test, capture raw/envelope/A-law frames, or run stream stability tests. ultr4rick repo only.
---

# Ultr4rick on-board testing

The full test guide and wire protocol are in `firmware/ULTR4RICK_PROTOCOL.md`. Read it before
changing frame formats or test acceptance criteria.

## Host environment

The system Python has no scipy. Use a venv in the scratchpad:

```bash
python3 -m venv $SCRATCH/venv && $SCRATCH/venv/bin/pip install -r firmware/tools/requirements.txt
```

## Quick status (safe, no pulser/ADC)

```bash
python3 -c "
import serial,time
s=serial.Serial('/dev/ttyACM0',115200,timeout=1); time.sleep(0.3); s.reset_input_buffer()
s.write(b'status\r\n'); time.sleep(0.5); print(s.read(4096).decode()); s.close()"
```

The response should show `board=ultr4rick package=RP2350B firmware=1.5 pulser=disarmed`.

## DSP self-test (primary functional test)

```bash
$SCRATCH/venv/bin/python firmware/tools/ultr4rick_capture.py --port /dev/ttyACM0 --selftest --output $SCRATCH/selftest
```

It should report 21/21 frames and 7 result lines with no `ERROR`. The tool rejects firmware other than `1.5`.
Run `status` afterwards to read `stages_us` / `worst_us`.

## Captures / streaming

```bash
... ultr4rick_capture.py --port /dev/ttyACM0 --mode raw|envelope|alaw --output DIR
... ultr4rick_capture.py --port /dev/ttyACM0 --mode alaw --rate 70 --frames 4200 --timeout 30 --output DIR
```

While streaming, the firmware sends only binary frames. Text replies resume after `stream stop`.

## Safety rules

- **Never** send `pulser arm` unless the user explicitly asks for a pulser test. If you arm it, always
  `pulser disarm` afterwards. GPIO27/28 drive the HV pulser.
- Assume the high-voltage supply may be connected. Without being asked, stick to the `status`, `help`, `dsp selftest`,
  `acq raw` and `read` commands.
- `dac write` needs the GPIO22→SCLK1 and GPIO23→MOSI1 jumpers fitted.

## Known baseline (2026-10-12, RP2350B, system GCC 10.3)

Self-test passes (nrms ≤ 8.4e-8). DSP timings were `stages_us=558/6601/163/6327/1072/441`, `worst_us≈15.6 ms`.
That is **above** the 14.3 ms frame period of the 70 Hz A-law limit, so a 70 Hz stream may drop
frames on this board. Verify with the 60 s stream test before relying on 70 Hz.

## ADC baseline, no input (2026-10-12, pulser disarmed)

Three raw captures gave a mean of about **517.8 counts**: roughly +6 above mid-scale, with noise of 0.9 counts rms.
The header `adc_dc_mean` matches `raw.npy.mean()` exactly.
**Watch for this:** in the first capture after idle, samples 0–54 read 1012–1020 (near full scale) and pull the mean up to
524. Later captures are clean. Check for this start-of-frame transient before trusting the first ~64 samples.
To compute the mean yourself: `np.load(DIR/'raw.npy').mean()`, or read `adc_dc_mean` in `headers.json`.
