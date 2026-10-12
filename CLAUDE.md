# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Ultr4rick is a higher-power successor to the pic0rick open ultrasound board. It is built on an **RP2350B**
(48-GPIO package) with a 10-bit 60 MS/s ADC, a bipolar HV pulser, a SEPIC HV supply, a MUX, an MCP4812 DAC (AFE gain) and a uSD slot.

- `firmware/` is the Pico SDK C firmware: acquisition, Hilbert-envelope DSP and the USB CDC protocol.
- `firmware/tools/ultr4rick_capture.py` is the host-side capture and self-test tool (numpy/scipy/pyserial).
- `firmware/ULTR4RICK_PROTOCOL.md` is the authoritative test guide, command reference and binary frame format.
- `hardware/` holds the KiCad project (schematic sheets per subsystem, PCB, gerbers). `hardware_v2/` is an untracked
  working copy of the next revision.

## Commands

The build/flash and test procedures live in project skills under `.claude/skills/`:

- `ultr4rick-build-flash`: configure, build and flash without sudo (`flash.sh`).
- `ultr4rick-board-test`: serial CLI, DSP self-test, captures and the safety rules.
- `ultr4rick-changelog`: the required change-logging convention (see below).

Short version:

```bash
cd firmware && export PICO_SDK_PATH=$HOME/.pico-sdk/sdk/2.3.0
cmake -S . -B build -G Ninja -DPICO_TOOLCHAIN_PATH=/usr && ninja -C build   # -> build/adc-pulse.uf2
../.claude/skills/ultr4rick-build-flash/flash.sh --no-build
```

There are no unit tests. The DSP is verified **on hardware** with `ultr4rick_capture.py --selftest`, which compares
the board's output against `scipy.signal.hilbert`.

## Firmware architecture (`firmware/ultr4rick/`)

The firmware is dual-core and fixed at 4096 samples per frame. The shared constants and pin map are in `u4rk.h`.

- **Core 0** (`main.c`) runs a cooperative superloop: TinyUSB task, command-line parsing, stream scheduling,
  capture polling and frame output. No other task should block it. USB writes are async
  (`usb_transport.c`). A USB disconnect cancels the session. Jobs and frames carry a `session_id`, so stale
  work from a closed host connection is dropped rather than sent.
- **Acquisition** (`acquisition.c` + `acquisition.pio`): PIO generates the 60 MHz ADC clock on GPIO0 and samples
  D0–D9 on GPIO1–10. DMA moves the samples into one of two raw buffers. `pulser.pio` drives GPIO27/28
  (P+/PDAMP) with 8 ns tick resolution, synchronised with the capture start.
- **Core 1** (`pipeline.c`) runs `process_job` from a `job_queue`. It unpacks the raw data, then runs the DSP and writes the header plus
  CRC32 into one of 2 output slots, which go back to core 0 via `output_ready_queue`. Back-pressure is handled by counting
  drops (processing or USB), never by blocking. The drop counts are reported in every header.
- **DSP** (`dsp.c`): CMSIS-DSP `arm_rfft_fast_f32` forward transform, analytic-signal mask, inverse transform, magnitude, then
  optional A-law (A=87.6) against a runtime reference. Only the float32 transform sources of CMSIS-DSP are
  compiled (`cmsisdsp_u4rk` in CMakeLists). Don't link the whole library.
- **Protocol** (`protocol.c`): a fixed 64-byte little-endian header (`U4RK` magic). If you change it, update
  `ULTR4RICK_PROTOCOL.md` and `tools/ultr4rick_capture.py` too, and bump the firmware version: the host tool checks
  for `1.5` exactly (`pico_set_program_version` in CMakeLists).

Board definition: `firmware/boards/ultr4rick.h`. It deliberately does *not* inherit `pico2.h`, because it targets RP2350B,
W25Q128 16 MiB flash and a 12 MHz crystal. USB IDs are `cafe:4011`.

## Hardware safety invariants

- The pulser pins must be `00` at boot. `u4rk_acquisition_init()` runs first in `main()`. The pulser is disarmed on
  USB disconnect, DMA fault, stream stop and reset. Keep all of these paths intact.
- Never arm the pulser (`pulser arm`) on real hardware unless the user explicitly asks for it.
- Pin conflicts: GPIO22/23 are used for the DAC SPI only when the jumpers to SCLK1/MOSI1 are fitted. MAX14866 control is
  intentionally absent from the RP2350B firmware.

## Known open issue

The exact 4096-point Hilbert takes ~15.6 ms worst case on the RP2350B at the default clock. That exceeds the 70 Hz A-law
limit (14.3 ms per frame) as well as the original 200 Hz/4.5 ms target. See `.claude/CHANGELOG.md` for measurements.

## Change logging (required)

Log every change in `.claude/CHANGELOG.md` with a per-commit detail note in `.claude/commits/`. The
format is in the `ultr4rick-changelog` skill. Only commit when asked, and branch off `main` first.
