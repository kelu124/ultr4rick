# Claude TODO

Claude's open actions for this repo (convention: `ultr4rick-changelog` skill). The maintainer's hardware list
is the root `TODO.md`. When an item is finished, move it to `DONE.md`.

## Open

- [ ] 2026-10-14: Confirm visually that the LED colours (white/green/yellow/blue/magenta/red) and the W LED
      (LED2) on port open/close behave as intended.
- [ ] 2026-10-14: Check the pulser timing on a scope (GPIO22/23). Ringing ended at about 6.2 µs, but the damping phase
      should run until about 7.1 µs.
- [ ] 2026-10-14: Run `python/example_dsp.ipynb` section 14 with the pulser actually armed. So far it has only been
      dry-run.
- [ ] 2026-10-14: Write a protocol/command reference for the current firmware. The old `ULTR4RICK_PROTOCOL.md` was
      removed along with the 1.5 firmware.
- [ ] 2026-10-14: uSD support (SDIO on GPIO30/31, 40–43; needs PIO with GPIO base 16, not PIO0).
- [ ] 2026-10-12: DSP time. Since running from SRAM it is 6.55 ms (7.3 ms worst with A-law), still over the 4.5 ms
      target. Next options: a newer GCC, the CPU clock, or a 2048-point transform.
- [ ] 2026-10-14: 50 Hz envelope streaming is USB-bound (820 KB/s): ~25% drops. Consider float16/int16 envelope
      payloads, or recommend A-law for high rates.
- [ ] 2026-10-12: Investigate the first-capture-after-idle transient at the start of the frame.
