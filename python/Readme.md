# ultr4rick — Python host tools

Host-side Python for driving an Ultr4rick v2 board running the **embarke**
RP2350B firmware (`../embarke/`) over USB serial, and for turning its raw ADC
traces into ultrasonic NDT (non-destructive testing) measurements: pulse
tuning, gain sweeps, back-wall echo detection, thickness estimation, and HDF5
storage. (Formerly the `pic0rick` package; `Pic0rick` remains as an alias of
`Ultr4rick`.)

> Works with both embarke firmware builds (`embarke/build.sh`):
> - the **stdio** build (`embarke` UF2, USB `2e8a:0009`): text REPL
>   (`write dac`, `start acq`, `read`), via `Ultr4rick.dac/pulse_adc_trigger/read`;
> - the **DSP** build (`embarke-dsp` UF2, USB `cafe:4011`): binary `U4RK`
>   frames, via `ultr4rick.dsp` and `Ultr4rick.status/capture/read_fft/read_raw`,
>   streaming and the on-board DSP self-test.
>
> Board pin map: `../embarke/PINMAP.md`.

## Layout

```
python/
├── ultr4rick/
│   ├── device.py            # Ultr4rick — serial driver for the board
│   ├── dsp.py               # U4RK binary frame reader, status parser, self-test check
│   └── ndt_acquisition.py   # UltrasonicAcquisition — capture + analysis + HDF5
├── example_dsp.py             # DSP-build script: status, self-test, captures
├── example_dsp.ipynb          # DSP-build step-by-step notebook
├── example_simple.ipynb       # minimal capture
├── example_acquisition.ipynb  # live-hardware NDT workflow (needs a board)
└── example_process.ipynb      # offline analysis of a saved HDF5 (no hardware)
```

## Python modules

### `ultr4rick/device.py` — the hardware driver
`Ultr4rick` opens the CDC serial port (115200 baud). `_find_port()` picks the
board by USB ID (`cafe:4011` DSP build, then `2e8a:0009` stdio build) and
falls back to the first `/dev/ttyACM*`·`/dev/ttyUSB*` / `COM*` /
`tty.usbmodem*`.

| Method | Firmware command | Build |
|---|---|---|
| `version()` | `version` | both |
| `reboot_dfu()` | `reboot-dfu` (→ `RP2350` UF2 drive) | both |
| `write_mux(v)` / `set_mux()` / `clear_mux()` | `write mux <hex>` / `set mux` / `clear mux` | both |
| `dac(N)` | `write dac N` | stdio |
| `pulse_adc_trigger(pon, poff, damp)` | `start acq …` | stdio |
| `read()` | `read` (8000 hex samples) | stdio |
| `status()` | `status` (parsed by `dsp.parse_status`) | DSP |
| `help()` / `command(line)` | any command → its `OK`/`ERR` line | DSP |
| `set_gain(N)` | `dac write N` | DSP |
| `configure_pulse(neg, damp, pos, order)` | `pulse config …` (min 40 ns each) | DSP |
| `arm_pulser()` / `disarm_pulser()` | `pulser arm` / `pulser disarm` | DSP |
| `read_raw()` / `read_fft()` / `capture("alaw")` | `read_raw` / `read_fft` / `acq alaw` | DSP |
| `set_alaw_reference(ref)` | `dsp scale <ref>` | DSP |
| `selftest()` | `dsp selftest` (21 frames; check with `dsp.check_selftest`) | DSP |
| `stream_start(type, hz)` / `stream_frames(n)` / `stream_stop()` | `stream start …` / `stream stop` | DSP |

Pulser: the board drives two lines, A (GPIO22) and B (GPIO23): A = negative,
B = positive, A+B = damping, none = idle. `negative_ns`/`positive_ns`/`damp_ns`
set each phase (8 ns resolution). The MAX14866 mux is on the board, so the mux
commands are always available.

`Fech = 60e6` (60 MS/s) is stored on the instance. `verbose`/`logging` options
echo or append the raw serial exchange to a log file.

### `ultr4rick/ndt_acquisition.py` — capture + analysis + storage
The workhorse. Importing `ultr4rick.device` is **lazy** (via `get_probe()`), so
this module is fully usable for offline analysis on a machine with no hardware.

- **`UltrasonicAcquisition`** — a dataclass bundling one trace with its metadata
  (`Fech`, `pon/poff/damp`, `gain`, `target`, transducer `piezo_*`). Key members:
  - `from_probe(...)` — trigger the board (DSP build: `set_gain` →
    `configure_pulse` → `arm_pulser` → `read_raw` → `disarm_pulser`), decode the samples to a normalized `[-1, 1]` signal (`(code-512)/512`),
    and optionally cache to HDF5 keyed on `(gain, pon, poff, damp, target,
    piezo_id)` (a cache hit skips the hardware).
  - `signal_filtered` — zero-phase Butterworth band-pass around the probe's
    centre frequency/bandwidth.
  - `detect_echoes(...)` — rectify + smooth, find the main echo in a time
    window, then track periodic back-wall echoes at `Δt = 2·thickness/c`;
    returns peak times/amplitudes, a measured `Δt`, a back-computed thickness,
    and a labelled figure.
  - `calibrate(...)` — sweep `pon = poff`, save every shot, and report the pulse
    width that maximises the windowed echo amplitude (with a 3-panel figure).
  - `amplitude(...)`, `plot(...)`, `label(...)`, `info(h5_path)`.
- **HDF5 I/O** — `save_h5`, `load_h5`, and per-acquisition `save()`; each trace
  is a group with the signal dataset (gzip) plus scalar attributes, deduplicated
  by the same key.

## Notebooks

### `example_acquisition.ipynb` — live hardware
End-to-end workflow against a connected board and a 1018-steel 5-step
calibration block with a 5 MHz probe: gain sweeps, `calibrate()` for the best
`pon/poff`, then `from_probe()` + `detect_echoes()` at several block thicknesses
(10/15/25 mm) to recover thickness from the echo train, and `info()` to
summarize the stored HDF5.

### `example_process.ipynb` — offline analysis
No hardware: `load_h5(...)` a saved capture, extract the emitted pulse, and study
it (autocorrelation / side-lobe structure) with NumPy/SciPy. Good starting point
for signal-processing work on existing data.

## Dependencies

`pyserial`, `numpy`, `scipy`, `h5py`, `matplotlib` (+ `jupyter`): see the repo-root
`requirements.txt`. Start Jupyter from `python/` so `import ultr4rick` resolves:

```bash
python3 -m venv .venv && .venv/bin/pip install -r requirements.txt   # at repo root
cd python && ../.venv/bin/jupyter lab
```

## Example data

The notebooks read and write `data/` (relative to `python/`), e.g.
`data/calibration_block_5steps_1018steel.h5`; see `data/README.md`.

## Quick start (offline)

```python
from ultr4rick.ndt_acquisition import load_h5
acqs = load_h5("data/calibration_block_5steps_1018steel.h5")
acqs[0].plot()          # raw (+ filtered if piezo_* set)
```

Live capture instead:

```python
from ultr4rick.ndt_acquisition import UltrasonicAcquisition
a = UltrasonicAcquisition.from_probe(Fech=60e6, gain=350, pon=70, poff=70, damp=6000,
                                     piezo_central_freq=5e6, piezo_bandwidth=4e6)
a.detect_echoes(start_us=35, end_us=55, target_thickness=0.010, speed_of_sound=5950)
```
