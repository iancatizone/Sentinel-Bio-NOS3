# Test Record: Independent Reproduction

Purpose: show that a second student can reproduce the stock NOS3 run from the released instructions (Sprint 2 acceptance criterion).

| Field | Value |
|---|---|
| Record ID | TR-20261015-01 |
| Tester | Sarah Boughton |
| Date | 10/15/2026 |
| Baseline ID | SBP-NOS3-BL-01 |
| NOS3 commit | TO RECORD (`git rev-parse HEAD` inside the VM) |
| Environment | Windows 11 version 25H2 host; VirtualBox VM `nos3_20251222` |
| Configuration used | `cfg/nos3-mission.xml` unmodified (confirm) |
| Procedure followed | [baseline-build-run.md](../../docs/baseline/baseline-build-run.md) as of 10/15/2026 |

## Results

| Test | Expected result | Observed result | Pass / Fail | Evidence |
|---|---|---|---|---|
| Build | Build finishes without errors | Pass | | |
| 1. Successful startup | 42, NOS3 Launcher, and FSW terminals open | The screenshot shows the 42 Map/Cam/Unit Sphere windows, the NOS3 Launcher, the NOS Time Driver at about 1.0x speed, and FSW events with components enabled | TO CONFIRM | [independent-reproduction-1.png](../screenshots/independent-reproduction-1.png) |
| 2. Telemetry updates | HK packet counts increase | Pass | | |
| 3. NOOP command | NOOP logged in Command and Telemetry Server | Pass | | |

## Deviations or assistance required

1. `make launch` did not open the 42 windows on the first attempt. The screenshot shows 42 running, so record what fixed it.
2. The NOS3 install directory was not specified in the guide. The guide was corrected (section 3.2, `cd C:\`).
3. Git had to be added to the Windows PATH. The guide was corrected (section 3.1).

## Result

Pass.

## Sign-off

| Role | Name | Date |
|---|---|---|
| Tester | Sarah Boughton | 10/20/2026 |
| Reviewer | Ian Catizone | 1-/27/2026 |
