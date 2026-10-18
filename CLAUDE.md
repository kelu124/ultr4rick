# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Ultr4rick is a higher-power successor to the pic0rick open ultrasound board. It is built on an **RP2350B**
(48-GPIO package) with a 10-bit 60 MS/s ADC (AD9215), an A/B-driven bipolar HV pulser, a SEPIC HV supply, a MAX14866
HV mux, an MCP4812 DAC (AFE gain), an LP5562 RGB LED driver on I2C and a uSD slot.

- `firmware/` is the Pico SDK C firmware for the RP2350B. It started as a fork of pic0rick 0.1.11 and identifies
  as Ultr4rick: acquisition, pulser, DAC/mux, Hilbert-envelope DSP and the USB CDC protocol.
  `firmware/PINMAP.md` is the source of truth for every GPIO.
- `python/ultr4rick/` is the host library (serial driver, `U4RK` frame reader, self-test check, NDT analysis +
  HDF5). Notebooks in `python/` write to `python/data/`. Dependencies are in the root `requirements.txt`.
- `hardware/` holds the KiCad project for the v2 board (schematic sheets per subsystem, PCB, gerbers).
- `docs/` is the versioned user documentation (`docs/version.yaml`): introduction, setup and first acquisitions,
  firmware, Python library, repository map. Every code change must be reflected there (`ultr4rick-docs`).
- Licenses (`LICENSE.md`, `LICENSES/`): hardware TAPR OHL 1.0, firmware and Python GPL-3.0-or-later, docs CC BY-SA 4.0.
- `TODO.md` (root) is the maintainer's hardware to-do list. `.claude/TODO.md` and `.claude/DONE.md` track Claude's work.

The previous Ultr4rick 1.5 firmware, its `ULTR4RICK_PROTOCOL.md` and `tools/ultr4rick_capture.py`, and the v1
hardware are in git history at `4da14a5`.

## Commands

The procedures live in project skills under `.claude/skills/`:

- `ultr4rick-firmware`: pin map, pulser encoding, shared SPI bus, safety invariants, firmware layout.
- `ultr4rick-build-flash`: build both variants and flash without sudo (`flash.sh`, which uses `reboot-dfu`).
- `ultr4rick-board-test`: drive the board from `python/ultr4rick`: status, self-test, captures, streaming, safety.
- `ultr4rick-changelog`: required change logging (`.claude/CHANGELOG.md`, `.claude/commits/`, `.claude/TODO.md`/`DONE.md`).
- `ultr4rick-versioning`: firmware, Python and docs `major.minor.patch` versions. Claude bumps the patch only.
- `ultr4rick-docs`: keep `docs/` in step with every code change, and version it.

Short version:

```bash
cd firmware && export PICO_SDK_PATH=$HOME/.pico-sdk/sdk/2.3.0
./build.sh -DPICO_TOOLCHAIN_PATH=/usr    # -> dist/ultr4rick-v<ver>-{ultr4rick,ultr4rick-dsp}.uf2
../.claude/skills/ultr4rick-build-flash/flash.sh --no-build    # flashes the DSP build
```

There are no unit tests. The DSP is verified **on hardware**: `Ultr4rick.selftest()` + `dsp.check_selftest()`
compare the board's envelope with `scipy.signal.hilbert` (pass at normalized RMS ≤ 1e-4).

## Firmware architecture (`firmware/`)

There are two build variants of the same hardware layer, selected by `-DDSP`:

- **stdio** (`main.c`, default): single-threaded SDK `stdio_usb` text REPL (`start acq`, `read`, `write dac`,
  `write/set/clear mux`, `version`, `reboot-dfu`). Raw 8000-sample captures. USB `2e8a:0009`.
- **DSP** (`main_dsp.c`, `-DDSP=ON`): raw TinyUSB on one CDC that carries both text replies and binary frames. USB
  `cafe:4011`, strings `Ultr4rick…`, serial `U4RK0001`.
  - Core 0 runs a cooperative superloop: USB task, command parsing, stream scheduling, capture polling and frame
    output. Nothing may block it. A USB disconnect cancels the session, and stale jobs are dropped by `session_id`.
  - Core 1 (`dsp/pipeline.c`) runs the DSP and writes header + CRC32 into one of 2 output slots. Back-pressure is
    handled by counting drops, never by blocking.

Shared modules:
- `hw/acquisition.c` + `.pio`: PIO0 generates the 60 MHz ADC clock on GPIO0 and samples D0–D9 on GPIO1–10. DMA moves
  the samples into the capture buffer.
- `hw/pulser.pio`: one PIO1 SM drives GPIO22/23 = {A, B}: `00` idle, A negative, B positive, A+B damping. The tick
  is 8 ns and each phase needs at least 40 ns. It starts with the capture.
- `hw/spi_bus.c`: SPI1 on GPIO26/27, shared by `hw/dac.c` (MCP4812, CS GPIO11) and `max/max14866.c` (nLE GPIO21,
  SET 28, CLR 29). The MAX14866 has no chip select: shift 16 bits, then pulse nLE. Never pulse nLE first.
- `dsp/dsp.c`: CMSIS-DSP `arm_rfft_fast_f32` forward transform, analytic mask, inverse transform, magnitude, then
  optional A-law (A=87.6). Only the float32 transform sources are compiled (`cmsisdsp_u4rk`). Don't link the whole
  library.
- `dsp/protocol.c`: a fixed 64-byte little-endian header with `U4RK` magic. If you change it, update
  `python/ultr4rick/dsp.py` (`HEADER`, `MAGIC`) and bump the versions (see `ultr4rick-versioning`).
- I2C0 (GPIO24/25) and the uSD pins (GPIO30/31, 40–43) are defined in `hw/u4rk.h` but not driven yet.

The image runs from SRAM (`pico_set_binary_type(copy_to_ram)`, about 270 KB of 520 KB used). Keep an eye on SRAM when adding
buffers. The board definition is `firmware/boards/ultr4rick.h` (`PICO_BOARD=ultr4rick`, forced in CMake). It deliberately
does *not* inherit `pico2.h`: it targets RP2350B, W25Q128 16 MiB flash and a 12 MHz crystal.

## Hardware safety invariants

- Pulser A/B must be `00` at boot. `u4rk_acquisition_init()` runs first in both `main()`s, before the USB wait.
  The pulser is disarmed on USB disconnect, DMA fault, stream stop and reset. Keep all of these paths intact.
- Never arm the pulser (`pulser arm`) on real hardware unless the user explicitly asks for it. Always disarm
  afterwards, in a `finally`.
- Only one process can own `/dev/ttyACM0`. Check `fuser -v /dev/ttyACM0` before blaming the firmware for lost bytes.

## Known open issues

- DSP time: running from SRAM (`copy_to_ram`, firmware 0.1.15) brought the envelope down to 6.55 ms (7.3 ms worst with
  A-law). That is well inside the 70 Hz A-law period, but still over the 4.5 ms target. Don't go back to XIP execution:
  from flash the timing depended on code layout (13.4–20.3 ms). A 50 Hz envelope stream is USB-bound.
- The first capture after idle has a start-of-frame transient. Discard it or check it before trusting it.
- In the v2 schematic, the MCP4725 (U2) has SDA/SCL swapped. See `TODO.md`.

## Change logging and versions (required)

Log every change in `.claude/CHANGELOG.md`, with a per-commit note in `.claude/commits/`, and keep `.claude/TODO.md`
/ `.claude/DONE.md` current (`ultr4rick-changelog` skill). When committing, bump the patch of every component whose
files changed (`ultr4rick-versioning` skill); major/minor belong to the maintainer. Every code change also updates
the `docs/` pages it affects and bumps the docs patch (`ultr4rick-docs` skill). Commit after each major step.
Commits that bump neither the firmware nor the Python version (no `firmware/` or `python/` change; a docs bump is
fine) may go straight to `main` and be pushed.
Anything that bumps the firmware or Python version goes on a branch off `main`; don't commit it to `main`, push or
merge it without being asked.

## Working with the maintainer

- Do the real thing: build, flash and test on the board attached to this machine (`/dev/ttyACM0`), and report the
  measured numbers. An edit that hasn't been run on hardware isn't done; say so if you couldn't verify it.
- Keep project know-how in this repo (`CLAUDE.md`, `.claude/skills/`, `.claude/TODO.md`), not only in private memory,
  so any session can pick it up.
- Branches: `main` is the integration branch. Start a new branch off `main` per piece of work that bumps a version;
  once it's merged and the maintainer asks to trim, delete it locally and on `origin`.
- Ask before killing processes you didn't start (e.g. a serial monitor from another session holding the port).
- KiCad must be closed before moving or renaming anything under `hardware/`.
- Related boards: pic0rick (RP2040, this firmware's ancestor) and we_gate (ECP5 FPGA) live in other repos.
