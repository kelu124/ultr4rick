---
name: ultr4rick-versioning
description: Track and bump the major.minor.patch versions of the Ultr4rick firmware (firmware/version.yaml) and Python library (python/ultr4rick/version.yaml). Claude may bump only the patch, when committing; major and minor belong to the maintainer. Use before every commit, when a version is asked about, or when a change touches the wire protocol between firmware and Python. ultr4rick repo only.
---

# Versioning

There are two independently versioned components, both `A.B.C` = major.minor.patch:

| Component | Version file | Also update | Where it shows |
|---|---|---|---|
| Firmware (`firmware/`) | `firmware/version.yaml` (`version`, `changes`) | `firmware/CHANGELOG.md` (new `## A.B.C — date` entry) | `version` command, `status firmware=`, `picotool info` |
| Python (`python/`) | `python/ultr4rick/version.yaml` (`version`, `changes`) | — | `ultr4rick.__version__`, `ultr4rick.__changes__` |

## Rules
- **Major (A) and minor (B) are the maintainer's.** Never change them, even for big or breaking changes. If you
  think one is due, say so and let the user decide.
- **Patch (C): Claude bumps it when committing**, once per commit, for each component whose files that commit
  changes. A firmware-only commit bumps only the firmware, and so on. Docs-only commits (`.claude/`, README) bump nothing.
- `changes:` is one double-quoted line saying what changed since the previous version. It is embedded in the
  firmware binary and shown by the Python library.
- The firmware's CMake reads `version.yaml` at configure time, so rebuild after bumping and confirm with
  `picotool info -a dist/*.uf2` (`version:`).

## Protocol coupling
The firmware frame header (`firmware/dsp/protocol.c`, `U4RK` magic, 64 bytes) and `python/ultr4rick/dsp.py`
(`HEADER`, `MAGIC`, `PROTOCOL_VERSION`) must change together. Bump both patches in the same commit, and mention
the coupling in both `changes:` lines.

## Current versions
Read them from the files rather than trusting this skill:

```bash
grep -h '^version:' firmware/version.yaml python/ultr4rick/version.yaml
```

Record every bump in the commit note's **Versions:** line (`ultr4rick-changelog`).
