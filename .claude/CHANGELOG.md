# Changelog

Changes made with Claude Code in this repo. Newest first. Each entry links to a detail note in
[`commits/`](commits/). The convention is in `.claude/skills/ultr4rick-changelog/SKILL.md`. Open and finished actions are in
[`TODO.md`](TODO.md) / [`DONE.md`](DONE.md).

## 2026-10-14

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
