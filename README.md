# Sentinel-Bio-NOS3

This repository holds the NOS3 simulation that SJSU uses for Sentinel-Bio+ FlatSat development, along with its documentation and evidence. It covers how to reproduce the stock NOS3 reference mission, the exact software configuration of each baseline, evidence that it runs, the simulation architecture, and how NOS3 maps to Sentinel-Bio+ requirements.

The first Sentinel-Bio+ specific component is in place: the custom **`sentinelbio_flatsat_eps`** component is the configured EPS. It models fixed 12 V, 5 V, and 3.3 V regulator rails with stationary current, supports simulator load pulses on any rail, and publishes housekeeping telemetry every 1 second. See [docs/sprint4/eps-component.md](docs/sprint4/eps-component.md).

The `nos3/` folder is the working NOS3 code. Documentation lives in `docs/` and `evidence/` only.

## Baseline at a glance

| Item | Value |
|---|---|
| NOS3 repository | [nasa/nos3](https://github.com/nasa/nos3) |
| NOS3 commit | `5a3bdee6be9a2c67fdf994ae6db56d5c60395302` |
| Flight software | cFS |
| Ground software | COSMOS (container `ballaerospace/cosmos:4.5.0`) |
| Scenario / spacecraft config | `STF1` / `spacecraft/sc-mission-config.xml` |
| Runtime | NOS3 Vagrant VM (Ubuntu 22.04.4) in VirtualBox 7.2; NOS3 processes run in Docker containers inside the VM |
| Baseline ID | `SBP-NOS3-BL-01` stock reference; `SBP-NOS3-BL-02` (candidate) adds the Sentinel-Bio+ EPS (see [configuration control](docs/baseline/config-control.md)) |
| Configured EPS | `sentinelbio_flatsat_eps` 2.0.0 (`SB_EPS` app, `sentinelbio_flatsat_eps_sim`) |
| EPS rails | 12 V / 5 V / 3.3 V fixed; stationary current; load pulses available in the simulator |
| EPS HK | `SENTINELBIO_FLATSAT_EPS_HK_TLM` (MID `0x09E0`) every 1 s |

## Sprint 2 deliverable status

| Deliverable (execution plan §7.2) | Location | Status |
|---|---|---|
| Reproducible NOS3 build/run instructions | [docs/baseline/baseline-build-run.md](docs/baseline/baseline-build-run.md) | Draft complete |
| NOS3 baseline configuration record | [evidence/baseline-versions.txt](evidence/baseline-versions.txt), [docs/baseline/config-control.md](docs/baseline/config-control.md) | Docker image entries need confirming in the VM |
| Regression evidence | [evidence/test-records/](evidence/test-records/), [evidence/logs/](evidence/logs/), [evidence/screenshots/](evidence/screenshots/) | Run log is empty; startup and telemetry screenshots need re-capture |
| Independent reproduction | [evidence/test-records/2026-10-15-independent-reproduction.md](evidence/test-records/2026-10-15-independent-reproduction.md) | Pending tester sign-off |
| Initial digital-twin architecture diagram | [docs/architecture/native-architecture.md](docs/architecture/native-architecture.md) | Draft complete |
| Requirements-to-representation/verification mapping | [docs/requirements/](docs/requirements/) | Template ready; needs team requirement IDs |
| Updated representation matrix | [docs/requirements/representation-matrix.csv](docs/requirements/representation-matrix.csv) | NOS3 side filled; Sentinel-Bio+ hardware columns TBD |

## Sprint 3 (October 12–25)

| Deliverable (execution plan §7.3, NOS3 tasks) | Location | Status |
|---|---|---|
| SJSU NOS3 Architecture Note | [docs/sprint3/architecture-note.md](docs/sprint3/architecture-note.md) | Outline with starting file paths |

## Sprint 4 (October 26 – November 8)

| Deliverable (execution plan §7.4) | Location | Status |
|---|---|---|
| Updated OBC/EPS and external-interface definitions | [docs/interfaces/eps-interface-definition.md](docs/interfaces/eps-interface-definition.md) | EPS draft complete; payload/external interface not started |
| First Sentinel Bio+ NOS3 component/service source code and configuration | [nos3/components/sentinelbio_flatsat_eps/](nos3/components/sentinelbio_flatsat_eps/), [docs/sprint4/eps-component.md](docs/sprint4/eps-component.md) | Done. Custom EPS is the configured EPS. Pulse-capable simulator source still to commit |
| Model publishes timestamped, version-identified telemetry | [docs/interfaces/eps-interface-definition.md](docs/interfaces/eps-interface-definition.md) §3 | Done. HK every 1 s with cFS time stamp; version 2.0.0 in startup events |
| Arduino emulator source and wiring definition (if used) | n/a | Not used |
| Common telemetry/interface definition | [docs/interfaces/eps-interface-definition.md](docs/interfaces/eps-interface-definition.md) | Draft complete |
| Automated or scripted test and raw results | [evidence/test-records/2026-11-20-sentinelbio-eps-checkout.md](evidence/test-records/2026-11-20-sentinelbio-eps-checkout.md) | Manual checks passed (HK at 1 s, `REQ_HK`, NOOP, load pulse). Screenshots/logs and a scripted test still needed |
| NOS3 vs. emulator/FlatSat comparison (one controlled case) | — | Open |

## Sprint 5 (November 9–22)

| Deliverable (execution plan §7.5) | Location | Status |
|---|---|---|
| Updated communications ICDs and dictionaries | — | Not started |
| NOS3 communications model/configuration or controlled emulator definition | — | Not started (stock `generic_radio` only) |
| End-to-end communications test procedure | — | Not started |
| FlatSat/emulator and NOS3 test records | — | Not started |
| Discrepancy log | — | Not started |

## Build and run

The stock BL-01 procedure is in [docs/baseline/baseline-build-run.md](docs/baseline/baseline-build-run.md). Build/run instructions for the BL-02 configuration with the Sentinel-Bio+ EPS are pending (open item EPS-OI-07).

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
  sprint4/
    eps-component.md              Custom EPS component: model, data path, results, open items
  interfaces/
    eps-interface-definition.md   EPS command/telemetry contract (MIDs, packet layout, states)
evidence/
  baseline-versions.txt           Full software/environment record for the baseline
  test-records/                   One Markdown record per test session (TEMPLATE.md to start)
  logs/                           Raw terminal output (build.log, baseline-run.log)
  screenshots/                    Screenshots referenced by test records
.github/                          Issue and pull-request templates
nos3/                             Working NOS3 code, including components/sentinelbio_flatsat_eps (do not edit for docs changes)
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
