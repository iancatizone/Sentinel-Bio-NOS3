# Requirements-to-Representation and Verification Mapping

This folder holds the NOS3 (simulation) contribution to the Sprint 2 mapping. The mapping shows which Sentinel-Bio+ requirements can be tested in NOS3 and which need an Arduino emulator, the FlatSat, analysis, or future flight hardware.

| File | Purpose |
|---|---|
| [verification-mapping.csv](verification-mapping.csv) | One row per requirement: how it will be verified and what NOS3 element supports it |
| [representation-matrix.csv](representation-matrix.csv) | One row per spacecraft element: how the stock NOS3 baseline represents it and what is missing |

GitHub renders both CSV files as tables, and both open directly in Excel or Google Sheets.

## Status

The rows in `verification-mapping.csv` marked `[Candidate]` with IDs `TBD-...` are placeholder requirement areas. They come from the Sentinel-Bio+ interfaces known so far: ADCS detumble and sun pointing, I2C EPS and thermal telemetry, UART payload and Iridium links, and S-band. When the team's baselined system requirements are released:

1. Replace each `TBD-...` ID and summary with the real requirement ID and text.
2. Fill `icd_ref` with the ICD section.
3. Add rows for any requirement not yet listed, and delete candidate rows that do not match a real requirement.

## Verification-method codes

| Code | Meaning |
|---|---|
| `NOS3` | Verified in the NOS3 software simulation (FSW against simulated components) |
| `EMU` | Needs an Arduino (or similar) hardware emulator of the device interface |
| `FS` | Needs the physical FlatSat |
| `AN` | Verified by analysis (for example, MATLAB, power or thermal budgets) |
| `FLT` | Only verifiable with future flight hardware, environmental test, or on orbit |

## Column definitions

`verification-mapping.csv`

| Column | Meaning |
|---|---|
| `req_id` | Requirement ID from the team baseline |
| `requirement_summary` | Short paraphrase of the requirement |
| `subsystem` | Owning subsystem |
| `icd_ref` | ICD document and section, if an interface is involved |
| `critical` | `Y` if the team marks it critical |
| `nos3_coverage` | `Y` = fully testable in stock NOS3, `Partial` = logic/flow testable but generic models, `N` = not represented |
| `nos3_element` | NOS3 app, simulator, or config file that supports the test |
| `primary_method` / `secondary_methods` | Codes above; separate multiple codes with `;` |
| `rationale` | Why that method was chosen |
| `status` | `Open`, `Planned`, `In work`, `Verified` |
| `evidence` | Path to the test record, log, or screenshot |

`representation-matrix.csv`

| Column | Meaning |
|---|---|
| `sentinel_element` | Spacecraft element |
| `sentinel_hardware` | Selected flight hardware (from the EnduroSat-based architecture) |
| `flight_interface` | Physical interface on the flight vehicle |
| `nos3_baseline_representation` | What stands in for it in `SBP-NOS3-BL-01` |
| `nos3_bus` | NOS Engine bus used by the simulator (from `cfg/sims/nos3-simulator.xml`) |
| `data_source` | Where the simulator gets its values (42 port, commands, none) |
| `fidelity` | `Generic`, `Sentinel-specific`, `Not represented` |
| `arduino_emulator` / `flatsat` | Whether the element can be represented there |
| `gap_and_next_step` | What it takes to make the representation Sentinel-Bio+-specific |

The NOS3 columns (`nos3_*`, `data_source`) are filled from the baseline configuration files. The Sentinel-Bio+ hardware columns are `TBD` until the flight architecture block diagram is approved.
