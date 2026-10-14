---
name: ultr4rick-changelog
description: Record changes in .claude/CHANGELOG.md and a per-commit detail note in .claude/commits/. Use whenever you change files or make a commit in the ultr4rick repo, and before reporting work as done. ultr4rick repo only.
---

# Change logging convention

Every change Claude makes in this repo gets logged in two places.

## `.claude/CHANGELOG.md`

Newest entries go at the top, under a date heading (`## YYYY-MM-DD`). Write one bullet per change:

```markdown
- `<short-hash>` <one-line summary> — [details](commits/<YYYY-MM-DD>-<short-hash>.md)
```

For work that isn't committed yet, use `uncommitted` in place of the hash and link to
`commits/<YYYY-MM-DD>-<slug>.md`. Once you commit, rename the file and update the bullet.

## `.claude/commits/<YYYY-MM-DD>-<short-hash>.md`

Create one file per commit:

```markdown
# <commit subject>

- **Commit:** <full hash> (branch <branch>)
- **Date:** YYYY-MM-DD
- **Files:** list of touched paths

## Why
<motivation / problem>

## What changed
<the substance: design decisions, register/pin/timing details, protocol impacts>

## Verification
<build result, flash, self-test / measurements with numbers, or "not verified on hardware">
```

## Workflow

1. Make the change, then build and verify it.
2. Commit. Only do this when the user asks, and don't commit straight to `main` without asking.
3. Get the hash (`git log -1 --format=%h`) and write the commit note and CHANGELOG bullet.
   Commit them together with the change, or as a follow-up `docs(claude): log <hash>` commit.
