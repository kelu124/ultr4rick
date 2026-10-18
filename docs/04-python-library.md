# 4. Python library

The `ultr4rick` package in [`python/ultr4rick/`](../python/ultr4rick/) drives the board over USB serial and turns
its traces into measurements. Its version is in
[`python/ultr4rick/version.yaml`](../python/ultr4rick/version.yaml) and available as `ultr4rick.__version__`. This
page describes library **0.1.13**.

## Install

```bash
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt    # at the repo root
cd python && ../.venv/bin/python -c "import ultr4rick; print(ultr4rick.__version__)"
```

The library is not installed as a package: run your scripts and notebooks from `python/`, or add `python/` to
`sys.path`.

| Module | Role | Needs a board |
|---|---|---|
| `ultr4rick.device` | `Ultr4rick`, the serial driver | yes |
| `ultr4rick.dsp` | binary frame reader, status parser, A-law decoder, self-test check | no |
| `ultr4rick.ndt_acquisition` | `UltrasonicAcquisition`: capture, echo detection, thickness, HDF5 storage | only for `from_probe()` / `calibrate()` |

## `ultr4rick.device.Ultr4rick`

```python
from ultr4rick.device import Ultr4rick
probe = Ultr4rick(port=None, verbose=True, logging=False, log_file=".log")
```

- `port=None` finds the board by USB ID (`cafe:4011` DSP build, then `2e8a:0009` stdio build), then falls back to
  the first `/dev/ttyACM*`, `/dev/ttyUSB*`, `COM*` or `/dev/tty.usbmodem*`.
- When it connects, it reads the firmware version and prints both versions:
  `Firmware 0.1.15 - library 0.1.13`. They are kept as `probe.firmware_version` and `probe.library_version`
  (strings; the firmware one is `"unknown"` if the board didn't answer), and returned by `probe.versions()`.
- `probe.Fech` is the sampling rate, 60e6.
- `verbose` prints the serial replies, `logging` appends them to `log_file`.
- `Pic0rick` is kept as an alias of `Ultr4rick`.

### Methods for both builds

| Method | Firmware command | Notes |
|---|---|---|
| `version()` | `version` | dict: `version`, `changes`, `board`, `chip`, `mux`, `mux_enabled`, `build`, `release`, `raw` |
| `versions()` | (none) | `(firmware_version, library_version)`, read at connection |
| `reboot_dfu()` | `reboot-dfu` | the board comes back as the `RP2350` drive; closes the port |
| `write_mux(value)` | `write mux <hex>` | any pattern 0..0xFFFF, as an int or a hex string |
| `set_one_mux(k)` | `write mux …` | element *k* (1..8) on TX and RX, all other switches open |
| `mux_element_mask(k, tx=True, rx=True)` | (none) | static: the pattern for element *k* |
| `set_mux()` / `clear_mux()` | `set mux` / `clear mux` | all switches closed / open |
| `close()` | (none) | closes the serial port |

### Methods for the DSP build

| Method | Firmware command | Returns |
|---|---|---|
| `status()` | `status` | dict (`dsp.parse_status`): `pulser_armed`, `dac`, `drops`, stage times, `raw`, … |
| `help()` / `command(line)` | any | the `OK …` line; raises `RuntimeError` on `ERR`, `TimeoutError` on no reply |
| `set_gain(n)` | `dac write n` | receive gain 0..1023 |
| `configure_pulse(neg_ns, damp_ns, pos_ns, order="pos-first")` | `pulse config …` | minimum 40 ns per phase |
| `arm_pulser()` / `disarm_pulser()` | `pulser arm` / `pulser disarm` | |
| `read_raw()` | `read_raw` | `dsp.Frame`, 8000 uint16 samples |
| `read_fft()` | `read_fft` | `dsp.Frame`, 4096 float32 envelope samples |
| `capture(payload)` | `acq raw\|envelope\|alaw` | `dsp.Frame`; `probe.last_reply` keeps the `OK` line |
| `set_alaw_reference(ref)` | `dsp scale ref` | |
| `selftest()` | `dsp selftest` | 21 frames, check them with `dsp.check_selftest()` |
| `stream_start(payload, rate_hz)` / `stream_frames(n)` / `stream_stop()` | `stream start …` / `stream stop` | `stream_frames` yields frames |

### Methods for the stdio build

| Method | Firmware command |
|---|---|
| `dac(n)` | `write dac n` |
| `pulse_adc_trigger(pon=200, poff=200, damp=2000)` | `start acq …` (fires the pulser) |
| `read()` | `read`, 8000 samples as hex text |

### Mux

The 8 elements each have a TX and an RX switch (see the table in [Firmware](03-firmware.md#gain-dac-and-mux-on-the-shared-spi-bus-hwspi_busc-hwdacc-maxmax14866c)):

```python
probe.set_one_mux(2)                          # element 2 on TX and RX: 0x000C
probe.write_mux(0x00C0)                       # any pattern: here element 4 on TX and RX
probe.write_mux("FFFF")                       # hex strings work too
m = Ultr4rick.mux_element_mask
probe.write_mux(m(1, rx=False) | m(2, tx=False))   # pitch-catch: TX on 1, RX on 2 -> 0x0006
probe.clear_mux()                             # all open
```

`write_mux` raises `ValueError` outside 0..0xFFFF, and `set_one_mux` outside 1..8.

## `ultr4rick.dsp`

| Name | What it does |
|---|---|
| `FrameReader(stream)` / `read_frame()` | reads one `U4RK` frame from the serial port, checks magic, size and CRC32 |
| `Frame.header` / `Frame.samples()` | the parsed `FrameHeader` and the payload as a NumPy array |
| `FrameHeader` | the 64-byte header fields: `payload_type`, `flags`, `sequence`, `sample_count`, `adc_dc_mean`, `envelope_peak`, `alaw_reference`, `negative_ns`, `damp_ns`, `positive_ns`, `dropped_frames`, … |
| `alaw_decode(bytes, reference)` | A-law bytes back to envelope amplitude in ADC counts |
| `check_selftest(frames, tolerance=1e-4)` | compares the board's envelopes with `scipy.signal.hilbert` |
| `parse_status(line)` / `describe_status(status)` | parses / pretty-prints the `status` line |
| `FLAG_*`, `PAYLOAD_NAMES`, `SELFTEST_NAMES`, `HEADER`, `MAGIC` | protocol constants |

## `ultr4rick.ndt_acquisition`

For non-destructive testing. It only imports the device driver when it needs it, so it works offline.

```python
from ultr4rick.ndt_acquisition import UltrasonicAcquisition, load_h5
a = UltrasonicAcquisition.from_probe(Fech=60e6, gain=350, pon=70, poff=70, damp=6000,
                                     piezo_central_freq=5e6, piezo_bandwidth=4e6)
a.detect_echoes(start_us=35, end_us=55, target_thickness=0.010, speed_of_sound=5950)
```

- `from_probe(...)`: sets the gain and pulse, arms, reads a raw capture, disarms, and normalises the signal to
  [-1, 1]. Results can be cached in an HDF5 file, keyed on the settings.
- `signal_filtered`: zero-phase band-pass around the probe's centre frequency.
- `detect_echoes(...)`: finds the main echo and the back-wall echo train, and computes the thickness.
- `calibrate(...)`: sweeps the pulse width and reports the one that maximises the echo.
- `amplitude`, `plot`, `label`, `info(h5_path)`, `save()`, `save_h5`, `load_h5`.

## Examples

| File | What it shows | Board |
|---|---|---|
| `python/example_dsp.ipynb` | every DSP-build feature step by step: status, gain, pulser, raw / envelope / A-law, self-test, mux elements 2 and 4, idle vs. pulsed | yes |
| `python/example_dsp.py` | the same as a script | yes |
| `python/example_simple.ipynb` | a minimal capture | yes |
| `python/example_acquisition.ipynb` | thickness of a 5-step 1018 steel block with a 5 MHz probe | yes |
| `python/example_process.ipynb` | analysis of a saved HDF5 file | no |

The notebooks write to `python/data/` (see [`python/data/README.md`](../python/data/README.md)).
