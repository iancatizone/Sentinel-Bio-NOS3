# Sentinel-Bio FlatSat Mission Manifest Outline

Status: manifest placeholder, not a runnable NOS3 mission. Creating this directory does not change `cfg/nos3-mission.xml`, install a component, or launch services.

## Configuration identity

| Field | Current value |
|---|---|
| Target | Current Portenta X8/custom breadboard EPS/RockBLOCK bench |
| Mode | Software-only; no live hardware or satellite account |
| Twin baseline ID | Not assigned; no validated twin build exists |
| NOS3 study/reference baseline | `SBP-NOS3-BL-01`, upstream revision `5a3bdee6be9a2c67fdf994ae6db56d5c60395302` |
| Actual FlatSat app revision | Missing; see [source recovery](../../docs/sprint4/fsw-integration.md) |
| Supplied C scaffold reference | `06db4b7f4726e7a7a128a2573e95e981626d4dad`, not proven deployed |
| Model/adapters/configuration revisions | Not implemented |
| Container pins | To capture when the actual services are recovered and built |
| Time mode | To decide after inspecting app timers |
| NOS3 integration mode | Tracked branch/fork or controlled overlay, not yet chosen |

## Future configuration sources

Reserve `cfg/` here for approved mission-owned source configurations only when the explicit NOS3 integration procedure is implemented. Do not add dummy XML or edit generated NOS3 `cfg/build/` copies to make the folder appear runnable.

Required contents of a future manifest:

- **Dependencies:** NOS3, actual application source, models, adapters, and container revisions.
- **Interfaces:** Bus identities, addresses, port allocations, protocol revisions, and no host hardware mounts.
- **Models:** Declared fidelity, power-state assumptions, calibration, and synthetic/measured fixture provenance.
- **Launch/build:** Tested working directories, configuration selection, model/service startup, and shutdown.
- **Acceptance:** Scenario IDs, expected observations, and formal evidence references.

Use the [workflow outline](../../docs/sprint4/nos3-workflow.md) before adding executable instructions. Never store credentials, modem account information, or private board configuration in this manifest.
