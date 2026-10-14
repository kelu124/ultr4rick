# Claude TODO

Claude's open actions for this repo (convention: `ultr4rick-changelog` skill). The maintainer's hardware list
is the root `TODO.md`. When an item is finished, move it to `DONE.md`.

## Open

- [ ] 2026-10-14 (in progress, user request): LP5562 RGB status LED on I2C0 with a different colour per pipeline
      stage, dimmed to 5%.
- [ ] 2026-10-14: Merge branch `embarke-ultr4rick` into `main` (or open a PR). Waiting on the maintainer.
- [ ] 2026-10-14: Check the pulser timing on a scope (GPIO22/23). Ringing ended at about 6.2 µs, but the damping phase
      should run until about 7.1 µs.
- [ ] 2026-10-14: Run `python/example_dsp.ipynb` section 14 with the pulser actually armed. So far it has only been
      dry-run.
- [ ] 2026-10-14: Write a protocol/command reference for the current firmware. The old `ULTR4RICK_PROTOCOL.md` was
      removed along with the 1.5 firmware.
- [ ] 2026-10-14: uSD support (SDIO on GPIO30/31, 40–43; needs PIO with GPIO base 16, not PIO0).
- [ ] 2026-10-12: DSP time. Worst case is 14.2 ms, against a 4.5 ms target. Try a newer GCC and check the CPU clock.
- [ ] 2026-10-12: Investigate the first-capture-after-idle transient at the start of the frame.
