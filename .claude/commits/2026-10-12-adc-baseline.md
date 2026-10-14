# ADC baseline measurement

- **Commit:** uncommitted (branch main)
- **Date:** 2026-10-12
- **Files:** `.claude/skills/ultr4rick-board-test/SKILL.md`

## Why
The user asked what the average value of an acquisition is.

## What changed
Documentation only. The measurement and the transient note were added to the board-test skill.

## Verification
I took 3 raw captures with `ultr4rick_capture.py --mode raw`, firmware 1.5, pulser disarmed and nothing on the input:

| capture | mean | std | min–max |
| --- | ---: | ---: | --- |
| 1 | 524.2 | 56.1 | 515–1020 |
| 2 | 517.7 | 0.94 | 515–520 |
| 3 | 517.9 | 0.86 | 512–523 |

Capture 1, samples 0–54, read 1012–1020. Without them its mean is 517.7. The cause hasn't been investigated
(ADC/AFE settling after idle?). The counts haven't been converted to volts because the ADC reference isn't documented yet.
