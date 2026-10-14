---
name: embarke-firmware
description: Work on the embarke firmware (embarke/ — the RP2350B-only Ultr4rick-v2 port forked from pic0rick 0.1.11): pin map, pulser A/B encoding, shared SPI1 DAC/MUX bus, build variants, flashing and DFU reboot. Use whenever touching, building or flashing anything under embarke/, or answering which GPIO does what on that board. ultr4rick repo only.
---

# embarke firmware

`embarke/` is a **separate firmware tree** from `firmware/`. It is the pic0rick 0.1.11 code base,
ported to the Ultr4rick v2 board (`hardware_v2/`), and it presents itself as **Ultr4rick**. Don't
assume `firmware/` conventions (version 1.5, `firmware/boards/ultr4rick.h`) apply here.

- Target: **RP2350B only** (`boards/embarke.h`, `PICO_BOARD=embarke`, `rp2350-arm-s`). It has no radio, and the
  RP2040/pico2 variants were removed.
- Pin map (source of truth): **`embarke/PINMAP.md`**. Constants: `embarke/hw/u4rk.h`.
- Version: `embarke/version.yaml` (patch bump on every change) + `embarke/CHANGELOG.md`, in addition to
  the repo-wide `.claude/CHANGELOG.md` (see `ultr4rick-changelog`).

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

## Invariants
- Pulser A/B must be `00` at boot. `u4rk_acquisition_init()` runs first in both `main.c` and
  `main_dsp.c`, before the USB wait. The pulser is disarmed on stop, DMA fault and disconnect.
- **Never arm the pulser on real hardware unless the user explicitly asks.**
- The DAC and MUX share SPI1. `hw/spi_bus.c` owns the bus init. A mux write shifts 16 bits **then**
  pulses nLE, and it must never pulse nLE first (that would latch leftover DAC bits).
- Wire identity is Ultr4rick: USB `cafe:4011`, strings `Ultr4rick…`, serial `U4RK0001`, frame magic `U4RK`
  (the 64-byte header layout is identical to `firmware/`).
- I2C0 and the uSD pins are defined but **not driven** yet. PIO access to GPIO40–43 needs
  `pio_set_gpio_base(pio, 16)` on a PIO block other than PIO0 (ADC, base 0).

## Build

```bash
cd embarke && export PICO_SDK_PATH=$HOME/.pico-sdk/sdk/2.3.0
./build.sh -DPICO_TOOLCHAIN_PATH=/usr     # both variants -> dist/embarke-v<ver>-{embarke,embarke-dsp}.uf2
# or one variant:
cmake -S . -B build -G Ninja -DPICO_TOOLCHAIN_PATH=/usr [-DDSP=ON] && ninja -C build
picotool info -a build/adc-pulse.uf2        # expect pico_board embarke, family rp2350-arm-s
picotool info -p build-dsp/adc-pulse.uf2    # pin list from bi_decl (DSP build declares all pins)
```

- `build/` holds the stdio REPL (`main.c`, SDK `stdio_usb`, `2e8a:0009`). Its commands are `start acq`, `read`, `write dac`,
  `write/set/clear mux`, `version` and `reboot-dfu`.
- `build-dsp/` holds the raw TinyUSB build (`main_dsp.c`, CMSIS-DSP via FetchContent into `.deps/`, `cafe:4011`).
  `help` lists its commands.
- Build outputs (`build*/`, `dist/`, `.deps/`, `*.uf2`) are gitignored.

## Flash / DFU
- Both builds accept **`reboot-dfu`**, which calls `reset_usb_boot(0,0)` and brings up the `RP2350` mass-storage drive.
- The stdio build also honours the SDK **1200-baud touch**. The DSP build does not, so send `reboot-dfu`
  or hold BOOTSEL.
- Then copy the UF2 to `/media/$USER/RP2350/` (see `ultr4rick-build-flash` for the no-sudo mount flow).
