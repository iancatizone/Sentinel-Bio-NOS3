# Test Record: Baseline Build and Checkout

| Field | Value |
|---|---|
| Record ID | TR-20261009-01 |
| Tester | Ian Catizone |
| Date | 10/9/2026 (build), 10/10/2026 (NOOP command check) |
| Baseline ID | SBP-NOS3-BL-01 |
| NOS3 commit | `5a3bdee6be9a2c67fdf994ae6db56d5c60395302` |
| Environment | Windows 11 host, VirtualBox 7.2, Ubuntu 22.04.4 LTS NOS3 VM |
| Configuration used | `cfg/nos3-mission.xml` unmodified: cFS, COSMOS, `STF1`, `spacecraft/sc-mission-config.xml` |
| Procedure followed | [baseline-build-run.md](../../docs/baseline/baseline-build-run.md) sections 4–8 |

## Results

| Test | Expected result | Observed result | Pass / Fail | Evidence |
|---|---|---|---|---|
| Build (`make`) | Build finishes without errors | `make exit status: 0`; clock-skew warnings only | Pass | [build.log](../logs/build.log) |
| 1. Successful startup | 42, NOS3 Launcher, and FSW terminals open; FSW time updates | 42 Map/Cam/Unit Sphere, NOS3 Launcher, and FSW `MGR: TimeTics` output visible | PASS | [baseline-running.png](../screenshots/baseline-running.png) (dated Sep 23 — see note) |
| 2. Telemetry updates | HK packet counts increase | `CFDP_ENGINE_HK` = 509 and `CFE_ES_HKPACKET` = 84 in the Tlm Packets tab | PASS | [telemetry-check.png](../screenshots/telemetry-check.png) (dated Sep 23 — see note) |
| 3. NOOP command | `cmd("GENERIC_ADCS GENERIC_ADCS_NOOP_CC")` logged; `GENERIC_ADCS_HK_TLM` updates | Command logged at 2026/10/10 04:02:05.022; `CMD_ERR_COUNT` = 0 | Pass | [command-check.png](../screenshots/command-check.png) |

Note: the startup and telemetry screenshots show a Sep 23 system date, which is earlier than this baseline record (10/9/2026). Re-capture both screenshots on the pinned commit, or confirm they were taken on the same checkout, before marking Tests 1 and 2 as passed.

## Deviations from the procedure

- None recorded.

## Known limitations

- `evidence/logs/baseline-run.log` is empty. Capture a run log as described in the guide's "Capturing run evidence" section.
- A single snapshot cannot show that packet counts are increasing. Take two screenshots a few seconds apart, or note both counts in this record.

## Sign-off

| Role | Name | Date |
|---|---|---|
| Tester | Ian Catizone | 10/27/2026 |
| Reviewer | | |
