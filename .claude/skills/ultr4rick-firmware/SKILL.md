---
name: ultr4rick-firmware
description: Work on the Ultr4rick RP2350B firmware in firmware/ — pin map, pulser A/B encoding, shared SPI1 DAC/MUX bus, stdio vs DSP variants, safety invariants and source layout. Use whenever touching anything under firmware/, or answering which GPIO does what on the board. ultr4rick repo only.
---

# Ultr4rick firmware (`firmware/`)

This is a fork of pic0rick 0.1.11, ported to the v2 board in `hardware/`, and it identifies as **Ultr4rick**.
The old Ultr4rick 1.5 firmware (`ultr4rick/` sources, `ULTR4RICK_PROTOCOL.md`, `tools/ultr4rick_capture.py`)
is only in git history (`4da14a5`).

- Target: **RP2350B only** (`boards/ultr4rick.h`, `PICO_BOARD=ultr4rick` forced in CMake, `rp2350-arm-s`).
  12 MHz crystal, W25Q128 16 MiB, no radio.
- Pin map (source of truth): **`firmware/PINMAP.md`**. Constants: `firmware/hw/u4rk.h`. If a pin changes,
  re-check it against the `hardware/` netlist:
  `kicad-cli sch export netlist --format kicadsexpr -o $SCRATCH/v.net hardware/Ultr4rick.kicad_sch`.
- Version: `firmware/version.yaml` + `firmware/CHANGELOG.md` (see `ultr4rick-versioning`).

## Pins (short version)

| Function | GPIO |
|---|---|
| ADC clock / D0..D9 | 0 / 1..10 |
| Pulser A / B | 22 / 23 |
| SPI1 SCK / DIN (DAC + MUX shared) | 26 / 27 |
| MCP4812 CS_DAC | 11 |
| MAX14866 nLE / SET / CLR | 21 / 28 / 29 |
| I2C0 SDA / SCL (LP5562, MCP4725) | 24 / 25 |
| uSD SCK / CMD / D0..D3 | 30 / 31 / 40..43 |

Pulser encoding is `{B,A}`: `00` all low, `01` (A) negative, `10` (B) positive, `11` damping.
There is one PIO1 state machine (`hw/pulser.pio`), with phases first polarity, second polarity, damping, then `00`.
The tick is 8 ns, and each phase needs at least 40 ns.

## Layout

| Path | Role |
|---|---|
| `main.c` | stdio REPL build (default): `start acq`, `read`, `write dac`, `write/set/clear mux`, `version`, `reboot-dfu` |
| `main_dsp.c` | DSP build (`-DDSP=ON`): raw TinyUSB, text + `U4RK` frames on one CDC; `help` lists commands |
| `hw/` | `acquisition.{c,pio}` (PIO0 ADC + DMA), `pulser.pio` (PIO1), `dac.c`, `spi_bus.c`, `u4rk.h` (pins/constants) |
| `max/` | MAX14866 driver on the shared SPI1 bus |
| `dsp/` | `dsp.c` (CMSIS rfft Hilbert + A-law), `pipeline.c` (core 1), `protocol.c` (64-byte header), USB transport/descriptors |
| `boards/ultr4rick.h` | SDK board definition |
| `build.sh` | builds both variants into `dist/` |

## Invariants
- The image runs from SRAM (`copy_to_ram`). Don't switch back to XIP: DSP time then depends on code layout in flash
  (measured 13.4–20.3 ms for the same code, against 6.55 ms from RAM).
- Pulser A/B must be `00` at boot. `u4rk_acquisition_init()` runs first in both `main.c` and `main_dsp.c`,
  before the USB wait. The pulser is disarmed on stop, DMA fault and disconnect.
- **Never arm the pulser on real hardware unless the user explicitly asks.**
- The DAC and MUX share SPI1. `hw/spi_bus.c` owns the bus init. A mux write shifts 16 bits **then** pulses
  nLE, and it must never pulse nLE first (that would latch leftover DAC bits).
- Wire identity: USB `cafe:4011` (DSP) / `2e8a:0009` (stdio), strings `Ultr4rick…`, serial `U4RK0001`, frame
  magic `U4RK`. The header layout must stay in sync with `python/ultr4rick/dsp.py`.
- I2C0 drives the LP5562 status LED (`hw/led.c`). The uSD pins are defined but **not driven** yet. PIO access to GPIO40–43 needs
  `pio_set_gpio_base(pio, 16)` on a PIO block other than PIO0 (ADC, base 0).

## MAX14866 mux wiring (from the `hardware/MUX.kicad_sch` netlist, U17)

- Even COMs (0, 2, … 14) join `MUX_IN` -> R77 (0 Ω) -> `ECHO_IN` (receiver, also SMA `AFE_IN` J2, U.FL RF4, pin J17).
- Odd COMs (1, 3, … 15) join `MUX_OUT` -> R78 (0 Ω) -> `TX` (pulser, also SMA `PULSER_OUT` J1, U.FL RF5, pin J18).
- NO(2k-2) and NO(2k-1) both go to element k's U.FL: k = 1..4 -> U18–U21, k = 5..8 -> U26–U29. So bit 2(k-1) = RX,
  bit 2(k-1)+1 = TX, assuming bit n drives SWn (MSB first). The maintainer's run in `e199deb` gave echoes on the
  elements expected, which is consistent with that order.
- DOUT goes only to TP3 (no read-back). `set mux` closes everything: TX shorted to `ECHO_IN`, all elements in parallel.
- v0 silkscreen error: U26–U29 read TR4, TR5, TR6, TR7 (should be TR5–TR8), so TR4 appears twice.
- Python: `write_mux(0..0xFFFF)`, `set_one_mux(k)`, `Ultr4rick.mux_element_mask(k, tx, rx)`.

Every firmware change also updates `docs/03-firmware.md` (`ultr4rick-docs`).

Build and flash: `ultr4rick-build-flash`. Test: `ultr4rick-board-test`.
