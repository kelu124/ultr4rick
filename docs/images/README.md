# Images

Pictures used by the documentation. Every image in this folder is described here (rule in the
`ultr4rick-docs-images` skill). Each photo comes as a full-size file and a `_small` copy that the pages display.

## `v0/`: first assembled boards (hardware v0)

The boards in these photos are silkscreened **"Ultr4rick V2.0 25/08/2026"**, the KiCad design in `hardware/`.

### `v0_front.jpg`, `v0_front_small.jpg`

Front of a bare v0 board, on squared paper. The silkscreen marks the blocks: **HV** (SEPIC supply, top left),
**PULSER** (top right, with the `PULSER_OUT` SMA pads on the right edge), **MCU** (RP2350B in the centre, with the
SW1/SW2 buttons and the SWCLK/SWDIO pads), **AFE** (bottom right, with the `AFE_IN` SMA pads). The USB-C connector and
the uSD slot are on the left edge, the H11 2×9 header at the bottom right. Used in
[Introduction](../01-introduction.md).

### `v0_back.jpg`, `v0_back_small.jpg`

Back of the same board. The MAX14866 mux (U17) sits between the 8 U.FL element connectors: U18–U21 at the bottom,
silkscreened TR1–TR4, and U26–U29 at the top. RF4 (J17, receive side) and RF5 (J18, transmit side) are the U.FL
connectors of the mux common lines, and TP3 is the mux DOUT test point. The table printed over the bottom-left header
names the H11 pins (CS_DAC, CLK, DIN, nLE, SET, CLR, …). **Silkscreen error:** U26–U29 are labelled TR4, TR5, TR6,
TR7. They are elements 5–8, so TR4 appears twice (see the root `TODO.md`). Used in
[Introduction](../01-introduction.md) and [Firmware](../03-firmware.md).

### `v0_working.jpg`, `v0_working_small.jpg`

A v0 board running, powered over USB-C through an inline USB power meter that reads about 5.18 V, 0.186 A, 0.96 W.
A transducer cable is connected through SMA cables and a tee to both `PULSER_OUT` (J1) and `AFE_IN` (J2), so one
piezo transmits and receives without going through the mux. Two indicator LEDs are lit: red near the MCU and
green (LED2) near the AFE. Used in [Setup and first acquisitions](../02-setup-first-acquisitions.md).
