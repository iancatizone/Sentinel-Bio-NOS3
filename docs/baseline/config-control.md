# NOS3 Baseline Configuration Control

This document defines what "the baseline" means for this repository, how it is identified, and how changes to it are made and recorded.

## 1. Baseline identification

| Baseline ID | Description | NOS3 commit | Mission config | Status |
|---|---|---|---|---|
| `SBP-NOS3-BL-01` | Stock NOS3 reference mission, unmodified | `5a3bdee6be9a2c67fdf994ae6db56d5c60395302` | `cfg/nos3-mission.xml` default (cFS, COSMOS, `STF1`, `sc-mission-config.xml`) | Active |

The full record of submodule revisions, tool versions, and container images is in [evidence/baseline-versions.txt](../../evidence/baseline-versions.txt). A baseline is defined by all of these together, not only the top-level NOS3 commit.

## 2. What is under control

| Item | Where it is recorded | How to check it |
|---|---|---|
| NOS3 top-level commit | `baseline-versions.txt` §2 | `git rev-parse HEAD` |
| Submodule revisions | `baseline-versions.txt` §3 | `git submodule status --recursive` |
| Local changes | `baseline-versions.txt` §6 | `git status --short` (must be empty for `BL-01`) |
| Mission/spacecraft config | `baseline-versions.txt` §2 | `cfg/nos3-mission.xml`, `cfg/spacecraft/*.xml` |
| Host tools (VirtualBox, Vagrant, Git) | `baseline-versions.txt` §4 | `VBoxManage --version`, `vagrant --version`, `git --version` |
| Container images | `baseline-versions.txt` §5 | `docker images --digests` |

## 3. Changing the baseline

1. Open a GitHub issue describing the change and why it is needed (for example, a newer NOS3 commit or a non-default mission config).
2. Make the change on a branch. Update `baseline-versions.txt` and the build/run guide.
3. Re-run the checkout in [baseline-build-run.md](baseline-build-run.md) §7 and add a new record under `evidence/test-records/`.
4. If the NOS3 commit, any submodule, the mission config, or a container image changes, assign a new baseline ID (`SBP-NOS3-BL-02`, ...) and add a row to the table above. Documentation-only fixes do not need a new ID.
5. Merge through a pull request reviewed by a second team member.

## 4. Repository conventions

### File naming

- Use lowercase with hyphens: `baseline-build-run.md`, not `Baseline Build Run.md`.
- Test records: `evidence/test-records/YYYY-MM-DD-<short-title>.md`.
- Logs: `evidence/logs/YYYY-MM-DD-<what>.log`. Logs without a date (`build.log`, `baseline-run.log`) belong to `BL-01`.
- Screenshots: `evidence/screenshots/<what-it-shows>.png`. Add `-1`, `-2`, and so on for a series.

### File formats

| Content | Format |
|---|---|
| Procedures, notes, records | Markdown (`.md`) |
| Diagrams | Mermaid code blocks in Markdown; export to PNG only for slides |
| Matrices and traceability | CSV (`.csv`), one row per item, UTF-8, comma-separated, header row first |
| Raw terminal output | `.log` / `.txt`, unedited except for a short header (tester, date, commit, command) |
| Images | PNG |

### Links

Use relative links between files (for example, `../../evidence/baseline-versions.txt`) so they work in branches, forks, and local clones.

### Evidence rules

- Do not edit a signed-off test record. Add a new one.
- Every Pass result must point to a log or screenshot.
- Screenshots should show the system clock or a timestamped log line so they can be matched to a run.
