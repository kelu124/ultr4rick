---
name: ultr4rick-docs-images
description: Rules for pictures in docs/images/ of the Ultr4rick repo — every image must be described with a short paragraph in docs/images/README.md, and how to name, size and reference photos from the docs pages. Use whenever an image is added, renamed, replaced or removed under docs/images/, or when a docs page shows a picture. ultr4rick repo only.
---

# Images in `docs/images/`

## Rule: every image is described in `docs/images/README.md`

When an image is added under `docs/images/` (by Claude or by the maintainer), add a short paragraph for it to
`docs/images/README.md` **in the same commit**:

- the file name(s), as a heading or a bold lead;
- what it shows, in one to three sentences: the board side or setup, the hardware version, and what to notice
  (connectors, labels, instruments, readings visible in the picture);
- which docs pages use it.

Describe only what is visible in the picture or confirmed in `hardware/`. If a label in the photo disagrees with
the schematic, say so. When an image is renamed or removed, update or remove its paragraph and every page
that links to it.

## Conventions

- One folder per hardware version: `docs/images/<hw-version>/` (e.g. `v0/`).
- Names: `<hw-version>_<subject>.jpg` for the full-size photo and `<hw-version>_<subject>_small.jpg` for a copy
  about 800 px wide (well under 300 KB).
- Pages show the `_small` copy and link it to the full-size file:
  `[![alt text](images/v0/v0_front_small.jpg)](images/v0/v0_front.jpg)`.
- Before committing a photo, check it carries no location metadata (`grep -c -a GPS <file>` should print 0),
  and strip it if it does.
- Images in `docs/` are documentation: CC BY-SA 4.0 (`LICENSE.md`).
- Adding or changing images is a docs change: bump the docs patch and add a `docs/CHANGELOG.md` entry
  (`ultr4rick-docs`, `ultr4rick-versioning`).
