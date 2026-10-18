# Ultr4rick documentation

Docs version **0.1.2** ([`version.yaml`](version.yaml), [changelog](CHANGELOG.md)). The pages were last checked
against **firmware 0.1.16** and **Python library 0.1.13**.

| Page | For |
|---|---|
| [1. Introduction](01-introduction.md) | What the board is and how ultrasound pulse-echo works. Start here if you are new. |
| [2. Setup and first acquisitions](02-setup-first-acquisitions.md) | Install the tools, flash the firmware, connect a transducer, take your first captures. |
| [3. Firmware](03-firmware.md) | How the RP2350B firmware is built: acquisition, pulser, mux, DSP, protocol, commands. |
| [4. Python library](04-python-library.md) | The `ultr4rick` package: the device driver, frame reader, NDT analysis, notebooks. |
| [5. Repository map](05-repository-map.md) | What lives in each folder of the repo. |

Other references kept next to the code:

- [`firmware/PINMAP.md`](../firmware/PINMAP.md): the source of truth for every GPIO.
- [`firmware/CHANGELOG.md`](../firmware/CHANGELOG.md): firmware history.
- [`python/Readme.md`](../python/Readme.md): Python package notes.

> **Safety:** the board generates high voltage for the transducer. Read the safety notes in the
> [introduction](01-introduction.md#safety) before you power a transducer.

---
This documentation is licensed under [CC BY-SA 4.0](../LICENSES/CC-BY-SA-4.0.txt). The firmware and Python code are
GPL-3.0-or-later and the hardware is TAPR OHL v1.0 (see [`LICENSE.md`](../LICENSE.md)).
