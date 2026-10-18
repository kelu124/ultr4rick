# Changelog

Changes made with Claude Code in this repo. Newest first. Each entry links to a detail note in
[`commits/`](commits/). The convention is in `.claude/skills/ultr4rick-changelog/SKILL.md`. Open and finished actions are in
[`TODO.md`](TODO.md) / [`DONE.md`](DONE.md).

## 2026-10-18

- `39a6aa4` Session know-how recorded in CLAUDE.md, ultr4rick-firmware and ultr4rick-board-test (mux wiring, LED, maintainer workflow)
- `deb6dab` README: board photos and two acquisition examples — [details](commits/2026-10-18-deb6dab.md)
- `ab9ad86` Removed the stale `.last-verified` file — [details](commits/2026-10-18-ab9ad86.md)
- `57363af` v0 photos in the docs, `ultr4rick-docs-images` skill, SMA connection, v0 silkscreen error; docs 0.1.1 —
  [details](commits/2026-10-18-57363af.md)
- `6bd438b` Maintainer edits committed: copyright holder, TODO item, CONTRIBUTING.md — [details](commits/2026-10-18-6bd438b.md)
- `8e6cc43` User documentation in `docs/` (intro, setup and first acquisitions, firmware, Python, repo map), docs 0.1.0,
  new `ultr4rick-docs` skill — [details](commits/2026-10-18-8e6cc43.md)
- `a857ab0` Licenses: hardware TAPR OHL 1.0, firmware and Python GPL-3.0-or-later, docs CC BY-SA 4.0 —
  [details](commits/2026-10-18-a857ab0.md)
- `f69f2ac` `example_dsp.ipynb` section 13: TX/RX on mux element 2, then element 4. Python 0.1.13. —
  [details](commits/2026-10-18-f69f2ac.md)
- `7104665` Rule: commits that bump no version may go straight to `main` and be pushed — [details](commits/2026-10-18-7104665.md)
- `0de84ad` `Ultr4rick()` reads the firmware version at connection and keeps `firmware_version` / `library_version`
  (strings), plus `versions()`. Python 0.1.12. — [details](commits/2026-10-18-0de84ad.md)
- `ebd298c` Python mux helpers: `write_mux()` takes any 0..0xFFFF pattern, `set_one_mux(k)` connects element k
  (1..8) to TX+RX, `mux_element_mask()`. Python 0.1.11. — [details](commits/2026-10-18-ebd298c.md)

## 2026-10-15

- `b16e8b2` Move Claude's private memory into CLAUDE.md and skills — [details](commits/2026-10-15-b16e8b2.md)

## 2026-10-14

- `c797503` Run the firmware from SRAM (`copy_to_ram`). DSP time no longer depends on flash layout: envelope
  13.4–20.3 ms -> 6.55 ms, worst with A-law 7.3 ms. Firmware 0.1.15. — [details](commits/2026-10-14-c797503.md)
- `4cff792` LP5562 status LED on I2C0. The RGB LED shows the pipeline stage at 5% (white/green/yellow/blue/magenta/red),
  and W (LED2) is on while a host has the ttyACM open. `status` gains `led=`. `flash.sh` automount race fixed.
  Firmware 0.1.14. — [details](commits/2026-10-14-4cff792.md)
- `0e1d65a` Repo cleanup: `hardware_v2/` replaces `hardware/` and `embarke/` replaces `firmware/` (old trees in
  history at `4da14a5`). Board renamed `embarke` -> `ultr4rick`, `build.sh` argument passing fixed, `flash.sh` now uses
  `reboot-dfu`. README, CLAUDE.md and skills rewritten for the new layout. New `ultr4rick-versioning` skill and
  `.claude/TODO.md`/`DONE.md`. Firmware 0.1.13 and Python 0.1.10. — [details](commits/2026-10-14-0e1d65a.md)
- `a63e0f7` Rename the `python/pic0rick` package to `python/ultr4rick` (0.1.9) for the embarke firmware: `Ultr4rick`
  class (`Pic0rick` alias), `U4RK` frame magic, USB-ID port detection, plus DFU reboot, streaming, self-test and
  `dsp scale` helpers. Flashed `embarke-dsp` 0.1.12; DSP self-test 7/7 passes, raw/envelope/A-law captures and a 20-frame stream work, 0 drops. Added the root `requirements.txt`, the notebooks' `python/data/` folder and an idle-vs-pulsed figure cell. — [details](commits/2026-10-14-a63e0f7.md)
- `90185e6` Port `embarke/` (pic0rick 0.1.11 fork) to the RP2350B Ultr4rick v2 board as firmware 0.1.12: new
  `embarke` board, pulser on GPIO22/23 A/B, MCP4812 + MAX14866 on a shared SPI1 bus, Ultr4rick USB/frame identity,
  old RP2040/pico2 builds removed. Added `embarke/PINMAP.md`, the `embarke-firmware` skill, and a TODO for the
  MCP4725 SDA/SCL swap. — [details](commits/2026-10-14-90185e6.md)

## 2026-10-12

- `uncommitted` Measured the ADC baseline (mean 517.8 counts) and found a near-full-scale transient at the start of the first capture after idle. Recorded in the board-test skill. — [details](commits/2026-10-12-adc-baseline.md)
- `uncommitted` Add CLAUDE.md, the `.claude/` skills (build-flash, board-test, changelog) and this changelog. Built
  firmware 1.5, flashed it to the RP2350B, and the DSP self-test passed. Measured worst-case DSP time is 15.6 ms. — [details](commits/2026-10-12-claude-setup.md)
