# Changelog — ultr4rick firmware

Forked from the pic0rick firmware at 0.1.11; earlier entries are pic0rick history.

All notable changes to the firmware (`firmware/`). Versions follow the `A.B.C`
scheme in `version.yaml` (major.minor.patch); the patch is bumped on every
change to `firmware/` source. Newest first.

This file and `version.yaml` are the version record — editing them does not
itself trigger a version bump. Every firmware version bump must add an entry
here (see `docs/claude/README.md`).

## 0.1.14 — 2026-10-14
- **Status LED (LP5562 on I2C0, GPIO24/25):** new `hw/led.c`. The RGB LED
  shows the pipeline stage: white boot, green idle, yellow capture, blue DSP
  (core 1), magenta USB transmit, red fault (held 0.5 s). The W channel (LED2)
  is on while a host has the ttyACM open (CDC DTR) and off when it drops.
  All channels are at 5% PWM (`U4RK_LED_LEVEL_PERCENT`), linear.
- I2C0 runs at 100 kHz on the RP2350 internal pull-ups, because the v2 board
  has no external ones. The LP5562 is reset at boot; the read-back of the
  auto-incremented B/G/R write must match, otherwise `led=absent`.
- Core 0 only, with writes only on changes (about 0.5 ms each). The stdio REPL
  now polls input with a 10 ms timeout, so the LED follows the port state.
- `status` gains `led=ok|absent`.

## 0.1.13 — 2026-10-14
- **Repo cleanup, no functional change:** the `embarke/` tree replaced the old
  `firmware/` (the previous Ultr4rick 1.5 firmware is in git history at
  `4da14a5`). Board renamed `embarke` -> `ultr4rick`: `boards/ultr4rick.h`,
  `PICO_BOARD=ultr4rick`, UF2s `dist/ultr4rick-v<ver>-{ultr4rick,ultr4rick-dsp}.uf2`.

## 0.1.12 — 2026-10-14
- **embarke port (RP2350B only):** new `boards/embarke.h` (RP2350B, 12 MHz
  crystal, W25Q128 16 MiB, no radio); CMake forces `PICO_BOARD=embarke`,
  RP2040/pico2 variants and the `MUX` option are gone (mux is on-board).
- **Pulser:** one PIO1 SM on GPIO22/23 = {A, B}: 00 all low, A negative,
  B positive, A+B damping. Replaces the 4-pin P+/P-/PDAMP/OE PMOD drive.
- **Shared SPI1 (GPIO26 SCK / GPIO27 DIN):** MCP4812 (CS_DAC GPIO11) and
  MAX14866 (nLE GPIO21, SET GPIO28, CLR GPIO29) now both use hardware spi1
  (`hw/spi_bus.c`); the MAX14866 PIO bit-bang is removed. A mux write shifts
  16 bits then pulses nLE once (the old leading nLE pulse would latch DAC bits).
- stdio build drives pulser A/B low before waiting for USB.
- I2C0 (GPIO24/25) and uSD (GPIO30/31, 40..43) pins defined, not yet driven.
- **Identity is now Ultr4rick:** USB strings `Ultr4rick` / `Ultr4rick RP2350B
  Signal Processor` / serial `U4RK0001` (VID:PID stays `cafe:4011`), frame magic
  `P0RK` -> `U4RK`, `version`/`status` report `ultr4rick`, CMake libs `*_u4rk`.

## 0.1.11 — 2026-09-13
- **Fix MUX/pulser PIO1 state-machine conflict (no-echo on MUX builds):**
  `max14866_init` hardcoded PIO1 `sm0`, but the shared pulser
  (`u4rk_acquisition_init`) claims PIO1 `sm0`/`sm1` first — so the mux init
  clobbered the pulser drive SM and no transmit pulse fired (only digital
  crosstalk, no echo). `max14866_init` now claims an unused SM. Confirmed on
  hardware: nomux builds already echoed; this restores echoes on MUX builds.

## 0.1.10 — 2026-09-13
- **Pulser fix (echo regression):** fire the bipolar pulse (both polarities
  back-to-back) **then** damp, matching the original stdio firmware (v0.1.0).
  The shared `pulser.pio` path (from P4b) had inserted the PDAMP damp phase
  *between* the negative and positive drive, splitting the bipolar excitation —
  which produced only a weak transmit and no echo. Reordered in
  `hw/acquisition.c` `queue_pulse` (no PIO change).

## 0.1.9 — 2026-09-13
- `build.sh` builds all 6 variants (rp2040 mux/nomux, rp2350 mux/nomux, rp2350
  mux/nomux +DSP) into `dist/`, sharing one CMSIS-DSP clone (`firmware/.deps`).
- CI caches `firmware/.deps` and builds/releases all 6 uf2.

## 0.1.8 — 2026-09-13
- Unified hardware drivers: the stdio and DSP builds now share one acquisition +
  pulser + spi1 DAC in `firmware/hw/`.
- The stdio build's `start acq` / `write dac` / `read` use the shared drivers
  (raw 8000).
- Retired the old PIO pulser (`adc/`) and PIO DAC (`max/dac.c`).

## 0.1.7 — 2026-09-13
- Two-length acquisition: `read_raw` = 8000-sample raw ADC (no FFT),
  `read_fft` = 4096-sample Hilbert envelope. Sample count is per-capture and
  self-described by the frame header. (DSP build.)

## 0.1.6 — 2026-09-13
- First runnable DSP build (`-DDSP`, RP2350): raw-TinyUSB command loop
  (`main_dsp.c`) with the onboard-DSP commands plus `version`, `reboot-dfu`, and
  (with `-DMUX`) `write/set/clear mux`. Non-DSP stdio builds unchanged.

## 0.1.5 — 2026-09-13
- Landed the onboard-DSP modules under `firmware/dsp/` as the `p0rk_dsp` static
  library (compiled only when `-DDSP=ON`). Not yet wired into the command loop.

## 0.1.4 — 2026-09-13
- Added the `-DDSP` build-option scaffolding (RP2350/`pico2`-guarded) that
  fetches CMSIS-DSP; firmware sources wired in later. Default builds unchanged.

## 0.1.3 — 2026-09-13
- Build against Pico SDK 2.3.0 (was 2.2.0), preparing for the onboard-DSP
  unification (needs 2.3.0 + CMSIS-DSP).

## 0.1.2 — 2026-09-13
- Added the `reboot-dfu` CLI command: reboots into the USB bootloader (BOOTSEL)
  so a new UF2 can be flashed without pressing BOOTSEL.

## 0.1.1 — 2026-09-13
- `version` command now also reports board/chip, MUX state, the git build hash
  (with `-dirty`) and the GitHub release URL.
- CI release notes list both the firmware and python library versions.

## 0.1.0 — 2026-09-13
- First versioned firmware: added the `version` CLI command (reads
  `version.yaml`); the MAX14866 MUX board is now optional at compile time via
  `-DMUX`.
