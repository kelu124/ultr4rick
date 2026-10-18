# Claude TODO

Claude's open actions for this repo (convention: `ultr4rick-changelog` skill). The maintainer's hardware list
is the root `TODO.md`. When an item is finished, move it to `DONE.md`.

## Open

- [ ] 2026-10-18: Ask the maintainer how to name the hardware in the docs: the photos folder says "v0", the silkscreen
      and README say "V2.0"/"v2".
- [ ] 2026-10-18: `python/Readme.md` lacks `set_one_mux`, `mux_element_mask`, `versions()` and the license (needs a Python
      patch bump, so on a branch). `docs/04-python-library.md` is up to date.
- [ ] 2026-10-18: Add SPDX headers (`GPL-3.0-or-later`) to the firmware and Python sources (firmware + Python bumps).
- [ ] 2026-10-18: `CONTRIBUTING.md` (maintainer's, committed in 6bd438b) is pic0rick text and says CC BY-SA 3.0; align with
      `LICENSE.md` (4.0) once the maintainer decides.
- [ ] 2026-10-18: `firmware/CHANGELOG.md` points to `docs/claude/README.md`, which doesn't exist; fix with the next
      firmware change.
- [ ] 2026-10-18: Run `example_dsp.ipynb` section 13 (mux elements 2 and 4) on the board (`f69f2ac`).
- [ ] 2026-10-18: Check on the board that `Ultr4rick()` prints and stores the firmware version (`0de84ad`); it was not
      enumerated when this was written.
- [ ] 2026-10-18: Run `set_one_mux(1..8)` / `write_mux()` on the board (it was not enumerated) and confirm that
      bit n drives SWn: with HV off, `write_mux(0x0001)` should give continuity U18 -> J17 only.
- [ ] 2026-10-14: Confirm visually that the LED colours (white/green/yellow/blue/magenta/red) and the W LED
      (LED2) on port open/close behave as intended.
- [ ] 2026-10-14: Check the pulser timing on a scope (GPIO22/23). Ringing ended at about 6.2 µs, but the damping phase
      should run until about 7.1 µs.
- [ ] 2026-10-14: Run `python/example_dsp.ipynb` section 15 with the pulser actually armed. So far it has only been
      dry-run.
- [ ] 2026-10-14: Write a protocol/command reference for the current firmware. The old `ULTR4RICK_PROTOCOL.md` was
      removed along with the 1.5 firmware.
- [ ] 2026-10-14: uSD support (SDIO on GPIO30/31, 40–43; needs PIO with GPIO base 16, not PIO0).
- [ ] 2026-10-12: DSP time. Since running from SRAM it is 6.55 ms (7.3 ms worst with A-law), still over the 4.5 ms
      target. Next options: a newer GCC, the CPU clock, or a 2048-point transform.
- [ ] 2026-10-14: 50 Hz envelope streaming is USB-bound (820 KB/s): ~25% drops. Consider float16/int16 envelope
      payloads, or recommend A-law for high rates.
- [ ] 2026-10-12: Investigate the first-capture-after-idle transient at the start of the frame.
