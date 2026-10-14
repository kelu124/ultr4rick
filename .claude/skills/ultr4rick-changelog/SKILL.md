---
name: ultr4rick-changelog
description: Commit after each major step, record every change in .claude/CHANGELOG.md with a per-commit note in .claude/commits/, and keep Claude's to-do and done lists in .claude/TODO.md and .claude/DONE.md. Use whenever you change files or make a commit in the ultr4rick repo, when starting or finishing a task, and before reporting work as done. ultr4rick repo only.
---

# Change logging convention

Every change Claude makes in this repo gets logged. Versions are covered separately in `ultr4rick-versioning`.

## `.claude/TODO.md` and `.claude/DONE.md`

These are Claude's own task lists. They are separate from the root `TODO.md`, which is the maintainer's hardware list.

- **`.claude/TODO.md`**: open actions, newest first under `## Open`. Each item is a `- [ ]` line with the date it was
  added and, when relevant, where it came from (a user request, a finding during work). Add an item when work is
  proposed, deferred or left open, and remove it when it's done or dropped.
- **`.claude/DONE.md`**: finished actions, newest first under a date heading. Each item is a `- [x]` line with the
  commit hash (or `uncommitted`) and a link to its `commits/` note.
- When an item is finished, move it from TODO.md to DONE.md, in the same change as the work.

## `.claude/CHANGELOG.md`

Newest entries go at the top, under a date heading (`## YYYY-MM-DD`). Write one bullet per change:

```markdown
- `<short-hash>` <one-line summary> — [details](commits/<YYYY-MM-DD>-<short-hash>.md)
```

For work that isn't committed yet, use `uncommitted` in place of the hash and link to
`commits/<YYYY-MM-DD>-<slug>.md`. Once you commit, rename the file and update the bullet.
Past entries are history: never rewrite them to match later renames.

## `.claude/commits/<YYYY-MM-DD>-<short-hash>.md`

Create one file per commit:

```markdown
# <commit subject>

- **Commit:** <full hash> (branch <branch>)
- **Date:** YYYY-MM-DD
- **Files:** list of touched paths
- **Versions:** firmware A.B.C -> A.B.C', python A.B.C -> A.B.C' (or "unchanged")

## Why
<motivation / problem>

## What changed
<the substance: design decisions, register/pin/timing details, protocol impacts>

## Verification
<build result, flash, self-test / measurements with numbers, or "not verified on hardware">
```

## Commit at each major step

The maintainer wants the history to read as a sequence of understandable steps, so **commit after each major step**
without waiting to be asked. A major step is a self-contained change that builds and has been verified: a feature,
a fix, a refactor or rename, a tooling/skill change.

- One logical step per commit. Don't mix a rename with a feature, or firmware and unrelated Python work.
- The subject is `<area>: <what changed>` (areas: `firmware`, `python`, `hardware`, `repo`, `docs(claude)`). The
  body says why, and what was verified.
- Each commit gets its own `.claude/commits/` note and CHANGELOG bullet, and its own patch bump per touched component.
- Work on a branch off `main` (reuse the current feature branch). **Never commit to `main` directly and never push
  or merge without being asked.**
- Don't commit broken or unverified states. If a step can't be verified, say so in the commit body and the note.

## Workflow

1. Add the task to `.claude/TODO.md` if it isn't trivial.
2. Make the change, then build and verify it.
3. Bump patch versions for the components you touched (`ultr4rick-versioning`).
4. Commit the step (see above).
5. Get the hash (`git log -1 --format=%h`), write the commit note and the CHANGELOG bullet, and move the TODO item to
   DONE.md. Commit them as a follow-up `docs(claude): log <hash>` commit, or amend them into the step's commit
   before it's pushed.
