---
name: ultr4rick-docs
description: Keep the Ultr4rick user documentation in docs/ (introduction, setup and first acquisitions, firmware, Python library, repository map) in step with the code, and version it (docs/version.yaml, docs/CHANGELOG.md). Use whenever a change touches firmware/, python/, hardware/ or the repo layout, when writing or editing a docs page, and before committing. ultr4rick repo only.
---

# Ultr4rick documentation (`docs/`)

`docs/` is the user documentation of the board. It is licensed CC BY-SA 4.0 (`LICENSE.md`).

| Page | Covers |
|---|---|
| `README.md` | index, docs version, the firmware / Python versions the pages describe |
| `01-introduction.md` | for newcomers: pulse-echo, the blocks on the board, the 8 elements, safety |
| `02-setup-first-acquisitions.md` | install, build, flash, connect, self-test, idle / mux / pulsed captures, streaming |
| `03-firmware.md` | builds, boot, acquisition, pulser, SPI/DAC/mux (element table), DSP stages, frame header, command reference, LED, pins |
| `04-python-library.md` | `Ultr4rick` methods per build, mux helpers, `dsp`, `ndt_acquisition`, examples |
| `05-repository-map.md` | every folder and the main files, licenses |
| `version.yaml`, `CHANGELOG.md` | docs version and history |
| `images/` | photos, one folder per hardware version; every image is described in `images/README.md` (`ultr4rick-docs-images`) |

## Rule: every code change is reflected in the docs

In the **same commit** as a code change, update the pages it affects:

| If you change | Update |
|---|---|
| a firmware command, reply, or `status`/`help` text (`main.c`, `main_dsp.c`) | `03-firmware.md` command reference; `02-…` if a first-steps command changes |
| the frame header (`dsp/protocol.*`, `python/ultr4rick/dsp.py`) | `03-firmware.md` binary frames, `04-…` `dsp` table |
| pins, pulser, acquisition, SPI, mux, LED, DSP stages, timings, sample counts | `03-firmware.md` (and `01-…` if the block diagram or safety changes) |
| a public `Ultr4rick` / `dsp` / `ndt_acquisition` function | `04-python-library.md` tables; `02-…` if the first-steps code uses it |
| notebooks or example scripts (sections, names) | `04-…` examples table, `02-…` "Go further" |
| build, flash or install steps, requirements | `02-setup-first-acquisitions.md`, `03-…` building |
| files or folders added, moved or removed | `05-repository-map.md` |
| hardware that the user sees (connectors, mux wiring, parts) | `01-…`, `03-…` |

Check the examples on the pages still run as written (the code blocks use the real API). If a behaviour was not
verified on hardware, don't describe it as measured.

## Versioning

Same scheme as firmware and Python (`ultr4rick-versioning`):

- `docs/version.yaml`: `version` (A.B.C), `changes` (one double-quoted line), and `firmware` / `python`: the
  component versions the pages were last checked against.
- **Claude bumps only the patch**, once per commit that changes a docs page. Major and minor are the maintainer's.
- Every bump adds a `## A.B.C — YYYY-MM-DD` entry at the top of `docs/CHANGELOG.md`.
- When a commit bumps the firmware or Python version, also update `firmware:` / `python:` in `docs/version.yaml`, the
  version line at the top of `docs/README.md`, and the "This page describes …" line in `03-…` / `04-…`.
- A commit that only changes `docs/` (and `.claude/`, root Markdown) bumps the docs patch only. It may be committed and
  pushed straight to `main` (`ultr4rick-changelog`).

## Style

- Plain, direct English for a reader who may be new to ultrasound. Explain a term the first time it appears.
- Link between pages and to the source of truth (`firmware/PINMAP.md`, the code) rather than duplicating long tables.
- Diagrams in Mermaid code blocks (GitHub renders them).
