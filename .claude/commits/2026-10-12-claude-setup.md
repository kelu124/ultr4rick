# Claude Code setup + first firmware build/flash

- **Commit:** uncommitted (branch main)
- **Date:** 2026-10-12
- **Files:** `CLAUDE.md`, `.claude/CHANGELOG.md`, `.claude/commits/`, `.claude/skills/ultr4rick-build-flash/{SKILL.md,flash.sh}`,
  `.claude/skills/ultr4rick-board-test/SKILL.md`, `.claude/skills/ultr4rick-changelog/SKILL.md`

## Why

To give future Claude sessions project context and repeatable build, flash and test procedures, and to keep
a log of changes.

## What changed

- Added CLAUDE.md, covering the dual-core firmware architecture, safety invariants and the known timing issue.
- Added the skills. `flash.sh` flashes without sudo: a 1200-baud touch puts the board in BOOTSEL, then the UF2 is copied
  to the `RP2350` drive. picotool has no udev permissions on this machine.
- No firmware source changes.

## Verification

- Build: `cmake -G Ninja -DPICO_TOOLCHAIN_PATH=/usr` with system arm-none-eabi-gcc 10.3.1 and Pico SDK 2.3.0.
  The build was clean with no warnings. text=108592 B, bss=123096 B. `picotool info` reports adc-pulse 1.5, rp2350-arm-s.
- Flash: the board was running another SDK app (`2e8a:0009`). After the 1200-baud touch it showed up as BOOTSEL (`2e8a:000f`). Copying the UF2
  brought it back as `cafe:4011 Ultr4rick RP2350B Signal Processor`.
- `status`: `board=ultr4rick package=RP2350B firmware=1.5 pulser=disarmed`.
- `ultr4rick_capture.py --selftest`: 21/21 frames, no CRC errors or gaps, and all 7 cases passed:

  | case | nrms | peak Δ | A-law Δ |
  | --- | ---: | ---: | ---: |
  | zero | 0 | 0 | 0 |
  | dc | 0 | 0 | 0 |
  | sinusoid | 8.4e-8 | 0 | 0 |
  | am | 5.2e-8 | 0 | 1 |
  | two-bursts | 3.3e-8 | 0 | 1 |
  | impulse | 1.5e-9 | 0 | 1 |
  | clipping | 4.7e-8 | 0 | 0 |

- DSP timing after the self-test was `stages_us=558/6601/163/6327/1072/441` (preprocess/fwd FFT/mask/inv FFT/mag/A-law)
  and `worst_us=15591`. **That exceeds the 14.29 ms period at the 70 Hz A-law limit.** The 60 s stream test has not been run yet.
  The protocol doc quotes 12.98 ms on a Pico 2 W, so the gap may be partly down to the older GCC 10.3 used here.
