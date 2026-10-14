---
name: ultr4rick-build-flash
description: Build the Ultr4rick RP2350B firmware (firmware/, Pico SDK + CMake/Ninja, stdio and DSP variants) and flash it to the board over USB without sudo. Use when compiling, flashing, checking binary size or pin info, or when picotool reports a permissions error. ultr4rick repo only.
---

# Build & flash Ultr4rick firmware

## One-shot

```bash
.claude/skills/ultr4rick-build-flash/flash.sh                 # build both, flash the DSP variant
.claude/skills/ultr4rick-build-flash/flash.sh --stdio         # build both, flash the stdio variant
.claude/skills/ultr4rick-build-flash/flash.sh --no-build [uf2]
```

## Build by hand

```bash
cd firmware && export PICO_SDK_PATH=$HOME/.pico-sdk/sdk/2.3.0
./build.sh -DPICO_TOOLCHAIN_PATH=/usr        # build/ (stdio) + build-dsp/ (DSP) -> dist/ultr4rick-v<ver>-*.uf2
# or one variant:
cmake -S . -B build-dsp -G Ninja -DPICO_TOOLCHAIN_PATH=/usr -DDSP=ON && ninja -C build-dsp
picotool info -a dist/ultr4rick-v<ver>-ultr4rick-dsp.uf2   # pico_board ultr4rick, rp2350-arm-s, boot2_w25q080
picotool info -p build-dsp/adc-pulse.uf2                    # pin list (bi_decl)
arm-none-eabi-size build-dsp/adc-pulse.elf
```

- The output target is `adc-pulse` (historic name). `build.sh` passes its arguments to CMake.
- `~/.pico-sdk/toolchain/` is empty on this machine, so use `-DPICO_TOOLCHAIN_PATH=/usr`, which selects the system
  `arm-none-eabi-gcc` 10.3. Both variants build with no warnings.
- The DSP build fetches CMSIS-DSP v1.17.0 into `firmware/.deps/` (FetchContent; the first run needs network).
  If the tree moves, delete `.deps/*-subbuild .deps/*-build` (they cache absolute paths) and keep `*-src`.
- `build*/`, `dist/`, `.deps/` and `*.uf2` are gitignored.

## Flashing without sudo

There are no udev rules for picotool, so `picotool load` fails with a permissions error. Use mass storage instead,
which is what `flash.sh` automates:

1. Get the board into BOOTSEL (`2e8a:000f`): send **`reboot-dfu`** on the CDC port (both variants support it).
   The stdio variant also honours the 1200-baud touch. Otherwise hold BOOTSEL while plugging it in.
2. The `RP2350` drive auto-mounts at `/media/$USER/RP2350`. If it doesn't, run `udisksctl mount -b /dev/sdX1`.
3. `cp <uf2> /media/$USER/RP2350/`. The board reboots on its own.
4. The DSP build re-enumerates as `cafe:4011 Ultr4rick RP2350B Signal Processor`; the stdio build as `2e8a:0009`.

If `reboot-dfu` gets no answer, check `fuser -v /dev/ttyACM*`: another process (e.g. a serial monitor) may own the
port. Then verify with `ultr4rick-board-test`.
