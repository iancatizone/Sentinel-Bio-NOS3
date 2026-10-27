# Sentinel-Bio-NOS3

This repository documents the NOS3 simulation baseline that SJSU uses for Sentinel-Bio+ simulation development. It covers how to reproduce the stock NOS3 reference mission, the exact software configuration of that baseline, evidence that it runs, the simulation architecture, and how NOS3 maps to Sentinel-Bio+ requirements.

Mission-specific executable simulation code is not part of this repository yet. Sprint 3 adds the source-level architecture note; Sprint 4 planning now targets the current Portenta X8/custom-EPS FlatSat, not the planned flight bus.

## Baseline at a glance

| Item | Value |
|---|---|
| NOS3 repository | [nasa/nos3](https://github.com/nasa/nos3) |
| NOS3 commit | `5a3bdee6be9a2c67fdf994ae6db56d5c60395302` |
| Stock reference flight software (not current FlatSat software) | cFS |
| Ground software | COSMOS (container `ballaerospace/cosmos:4.5.0`) |
| Scenario / spacecraft config | `STF1` / `spacecraft/sc-mission-config.xml` |
| Runtime | NOS3 Vagrant VM (Ubuntu 22.04.4) in VirtualBox 7.2; NOS3 processes run in Docker containers inside the VM |
| Baseline ID | `SBP-NOS3-BL-01` (see [configuration control](docs/baseline/config-control.md)) |

## Sprint 2 deliverable status

| Deliverable (execution plan §7.2) | Location | Status |
|---|---|---|
| Reproducible NOS3 build/run instructions | [docs/baseline/baseline-build-run.md](docs/baseline/baseline-build-run.md) | Draft complete |
| NOS3 baseline configuration record | [evidence/baseline-versions.txt](evidence/baseline-versions.txt), [docs/baseline/config-control.md](docs/baseline/config-control.md) | Docker/Compose versions filled; NOS3 image digest remains TO RECORD |
| Regression evidence | [evidence/test-records/](evidence/test-records/), [evidence/logs/](evidence/logs/), [evidence/screenshots/](evidence/screenshots/) | Checkout marked Pass; empty run log and screenshot-date clarification remain |
| Independent reproduction | [evidence/test-records/2026-10-15-independent-reproduction.md](evidence/test-records/2026-10-15-independent-reproduction.md) | Overall Pass with signatures; reconcile remaining incomplete fields |
| Initial digital-twin architecture diagram | [docs/architecture/native-architecture.md](docs/architecture/native-architecture.md) | Draft complete |
| Requirements-to-representation/verification mapping | [docs/requirements/](docs/requirements/) | Template ready; needs team requirement IDs |
| Updated representation matrix | [docs/requirements/representation-matrix.csv](docs/requirements/representation-matrix.csv) | NOS3 side filled; Sentinel-Bio+ hardware columns TBD |

## Sprint 3 (October 12–25)

| Deliverable (execution plan §7.3, NOS3 tasks) | Location | Status |
|---|---|---|
| SJSU NOS3 Architecture Note | [docs/sprint3/architecture-note.md](docs/sprint3/architecture-note.md) | Outline with starting file paths |

See the [Sprint 2/3 review](docs/sprint4/sprint2-3-review.md) for the remaining items. Existing test records have not been rewritten.

## Sprint 4: current FlatSat digital twin

The initial target is software-only replication of the current Portenta X8, breadboard EPS, and RockBLOCK 9603 interface. S-band, physical ADCS/payload emulators, orbital dynamics, and live hardware coupling are outside this initial scope.

| Planning artifact | Location | Status |
|---|---|---|
| Scope, priorities, document index | [Sprint 4 README](docs/sprint4/README.md) | Outline; no twin software implemented |
| As-built hardware and interface inventory | [FlatSat configuration](docs/sprint4/flatsat-as-built.md), [interface inventory](docs/sprint4/interface-inventory.csv) | User/manual inputs recorded; unresolved details labeled |
| Software provenance and source recovery | [FSW integration](docs/sprint4/fsw-integration.md) | Blocking: linked C scaffold differs from manual's Python/Docker services |
| Architecture and NOS3 file/VM workflow | [Twin architecture](docs/sprint4/twin-architecture.md), [workflow](docs/sprint4/nos3-workflow.md) | Proposed, not executed |
| Acceptance scenarios | [Validation plan](docs/sprint4/validation-plan.md), [scenario outline](tests/scenarios/README.md) | Not run |
| Questions and source/requirements decisions | [Open items](docs/sprint4/open-items.md) | Awaiting clarification |

## Repository layout

```
README.md                         Project overview and deliverable index (this file)
docs/
  baseline/
    baseline-build-run.md         Setup, build, run, checkout, and shutdown guide
    config-control.md             Baseline ID, change process, file conventions
  architecture/
    native-architecture.md        Stock NOS3 digital-twin architecture (Mermaid diagrams)
  requirements/
    README.md                     How to use the mapping and the verification-method codes
    verification-mapping.csv      Requirement -> NOS3 / emulator / FlatSat / analysis / flight
    representation-matrix.csv     Sentinel-Bio+ element -> how NOS3 represents it
  reference/
    nos3-glossary.md              Plain-language NOS3 terminology
  sprint3/
    architecture-note.md          SJSU NOS3 Architecture Note (Sprint 3)
  sprint4/                        Current-FlatSat twin planning, inventories, and open items
missions/
  sentinel-bio-flatsat/            Mission integration manifest outline (not a runnable profile)
components/                       Future device-model ownership conventions only
tests/
  scenarios/                      Acceptance scenario outline, not executable tests
evidence/
  baseline-versions.txt           Full software/environment record for the baseline
  test-records/                   One Markdown record per test session (TEMPLATE.md to start)
  logs/                           Raw terminal output (build.log, baseline-run.log)
  screenshots/                    Screenshots referenced by test records
.github/                          Issue and pull-request templates
```

## File formats

| Content | Format | Why |
|---|---|---|
| Procedures, notes, records | Markdown (`.md`) | Renders on GitHub, diffs cleanly |
| Diagrams | Mermaid code blocks inside Markdown | GitHub renders them; editable as text |
| Matrices and traceability tables | CSV (`.csv`) | GitHub renders them as tables; opens in Excel or Sheets |
| Raw tool output | Plain text (`.log`, `.txt`) | Unedited terminal output |
| Screenshots | PNG (`.png`) | Lossless; named `<what-it-shows>.png` |

See [docs/baseline/config-control.md](docs/baseline/config-control.md) for naming and change rules.
