# Changelog — Ultr4rick documentation

All notable changes to `docs/`. Versions follow the `A.B.C` scheme in [`version.yaml`](version.yaml)
(major.minor.patch). The patch is bumped on every change to a docs page. Newest first.

## 0.1.3 — 2026-10-18
- Checked against Python 0.1.14. The Python page says the `.py` files carry SPDX GPL-3.0-or-later headers.

## 0.1.2 — 2026-10-18
- Checked against firmware 0.1.16. The firmware page says the sources carry `SPDX-License-Identifier:
  GPL-3.0-or-later` headers.

## 0.1.1 — 2026-10-18
- Photos of the v0 board in [`images/v0/`](images/v0/), each described in [`images/README.md`](images/README.md):
  front (introduction), back with the element connectors (introduction, firmware), running with a transducer
  (setup).
- Setup and introduction: the `PULSER_OUT` (J1) / `AFE_IN` (J2) SMA connection with a tee, as the simplest setup.
- The v0 silkscreen labels U26–U29 TR4–TR7 instead of TR5–TR8; the element table in the firmware page has a
  silkscreen column.

## 0.1.0 — 2026-10-18
- First docs set, checked against firmware 0.1.15 and Python 0.1.13:
  [introduction](01-introduction.md), [setup and first acquisitions](02-setup-first-acquisitions.md),
  [firmware](03-firmware.md), [Python library](04-python-library.md), [repository map](05-repository-map.md).
- Licensed CC BY-SA 4.0 (see `LICENSE.md`).
