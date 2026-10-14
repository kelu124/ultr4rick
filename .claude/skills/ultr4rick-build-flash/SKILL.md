---
name: ultr4rick-build-flash
description: Build the Ultr4rick RP2350B firmware (Pico SDK + CMake/Ninja) and flash it to the board over USB without sudo. Use when compiling, flashing, checking binary size, or when picotool reports a permissions error. ultr4rick repo only.
---

# Build & flash Ultr4rick firmware

## One-shot

```bash
.claude/skills/ultr4rick-build-flash/flash.sh             # build + flash
.claude/skills/ultr4rick-build-flash/flash.sh --no-build  # flash existing build/adc-pulse.uf2
```

## Build by hand

```bash
cd firmware
export PICO_SDK_PATH=$HOME/.pico-sdk/sdk/2.3.0
cmake -S . -B build -G Ninja -DPICO_TOOLCHAIN_PATH=/usr   # first time only
ninja -C build
arm-none-eabi-size build/adc-pulse.elf
picotool info build/adc-pulse.uf2      # expect version 1.5, family rp2350-arm-s
```

- The output target is `adc-pulse` (historic name), not `ultr4rick`.
- CMakeLists pins toolchain `15_2_Rel1`, but `~/.pico-sdk/toolchain/` is empty on this
  machine, so the build uses the system `/usr/bin/arm-none-eabi-gcc` (10.3). It builds clean.
  A GCC 14.3 is also available at `~/.arduino15/packages/rp2040/tools/pqt-gcc/4.1.0-1aec55e`.
  Use it with `-DPICO_TOOLCHAIN_PATH=<that dir>` in a fresh build dir if you want to compare
  DSP timings.
- CMSIS-DSP v1.17.0 is fetched by FetchContent at configure time, so the first configure needs network access.
- `PICO_BOARD` is forced to `ultr4rick` (`firmware/boards/ultr4rick.h`). The `else()` RP2040
  branch in CMakeLists refers to `main.c`, `adc/`, `max/`, and none of those exist in this repo. It is dead.

## Flashing without sudo

There are no udev rules for picotool, so `picotool load` fails with "unable to connect…permissions".
Use the mass-storage path instead, which is what `flash.sh` automates:

1. Get the board into BOOTSEL (`2e8a:000f`). A 1200-baud touch on the CDC port only works when the board is running
   stock SDK `stdio_usb` firmware (`2e8a:0009`). **Ultr4rick firmware 1.5 does not implement it**, so a board
   already running Ultr4rick needs BOOTSEL held while plugging it in.
2. The `RP2350` drive auto-mounts at `/media/$USER/RP2350`. If it doesn't, run `udisksctl mount -b /dev/sdX1`.
3. `cp build/adc-pulse.uf2 /media/$USER/RP2350/`. The board reboots on its own.
4. It should re-enumerate as `cafe:4011 Ultr4rick RP2350B Signal Processor` on `/dev/ttyACM0`.

If the board is unresponsive, hold BOOTSEL while plugging it in, then run `flash.sh --no-build`.

After flashing, verify with the `ultr4rick-board-test` skill.
