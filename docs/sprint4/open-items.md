# Sprint 4 Inputs, Decisions, and Open Items

Status: October 27 intake. This is a planning record; unresolved inputs have not been converted into executable defaults.

## Confirmed by the user

- **Target:** Current physical FlatSat capabilities, software-only for now; no planned live hardware coupling.
- **Hardware:** Linux Portenta X8/breakout, custom breadboard EPS, and RockBLOCK 9603; OBC/EPS communication works and modem integration remains unresolved.
- **First paths:** EPS power availability for OBC/modem, EPS telemetry to OBC, and OBC commands to EPS.
- **Not implemented:** S-band and Arduino ADCS/payload emulators.
- **Environment:** NOS3 installation remains stock.
- **Requirements:** Proposed, not approved. System-level sections 1.1/1.2 take precedence over conflicting orbit-subsystem entries; deorbit target is within 25 years.

## Priority questions

| ID | Question / needed material | Why needed | State |
|---|---|---|---|
| Q-SRC | Where are `health/`, `payload-sim/`, `eps/`, `cdh/`, and `groundstation/` with Python code, Dockerfiles, and launch scripts? Is there another repo/branch/archive? | The provided C repo does not reproduce the manual's five-service stack. | Blocking app-equivalent implementation |
| Q-EPS | What exact OBC-to-EPS command works today, what response occurs, and what hardware changes? Supply the controlling code/pin if load switching is claimed. | INA219 measurement/configuration is not rail switching. | Blocking EPS command acceptance |
| Q-IR | What is the failing RockBLOCK invocation and raw transcript? Is failure at AT response, buffering, SBD session, or app routing? | A suspected software cause cannot yet be diagnosed. | Blocking a specific regression test |
| Q-DOC | Does current hardware match the manual's historical AT success, or has wiring/software changed? Confirm module revision and pin numbering. | Manual UART/CTS statements conflict with the device guide. | Requires reconciliation before lab reuse |
| Q-CAL | What INA219 library, shunts, calibration, sample rates, raw readings, and parsed schema are used? | Determines model and telemetry equivalence. | Needed before register-level implementation |
| Q-INT | Must the first demonstration execute native NOS3 models and NOS Engine buses, or is standalone app recovery an accepted intermediate milestone? | Avoid delivering a generic Python mock while claiming NOS3 integration. | Confirm with assignment/advisor |
| Q-SCHED | Supply official Sprint 4 task text/deadline and any Sprint 3 work stored elsewhere. | Align acceptance/presentation and avoid duplicating completed work. | Requested |
| Q-RIGHTS | Which recovered software/material may be committed publicly, and under what license? | Keep private source, credentials, accounts, and board identifiers out of this repo. | Confirm before importing |

These questions do not prevent useful document outlines. They do prevent claiming a completed, validated replica or a diagnosed modem fix.

## Supplied document register

Original attachments remain outside the repo; this PR does not republish them. Names below identify user-supplied inputs, not missing relative links.

| Input | Role and limitation |
|---|---|
| `Requirements.pdf` | Proposed requirements; retain original text and record reconciliation decisions separately. |
| `Screenshot-2026-10-27-155152.png` | EPS prototype wiring; missing actuator/control details and uncertain modem signal directions. |
| `Screenshot-2026-10-27-155235.png` | Broader flight power architecture; not an as-built component list. |
| `Copy-of-FlatSat-Operations-Manual.pdf` | Historical Python/Docker service descriptions, Linux device mapping, procedures; reconcile with current board state. |
| ICD-200 through ICD-206, Revision A | Communications target interfaces, including S-band and Iridium; not proof of implemented behavior. |
| ICD-400 through ICD-405, Revision A | Proposed software/driver, payload, telemetry/command, and data-product contracts; not identical by default to the manual's TCP JSON/plain-text service protocol. |
| Public software repository | C scaffold at `06db4b7f4726e7a7a128a2573e95e981626d4dad`; source gap detailed in [FSW integration](fsw-integration.md). |

The public C repo itself describes real UART mapping and scheduling as future work ([architecture document](https://raw.githubusercontent.com/aalamillo014/sentinel_bio_software_sjsu/06db4b7f4726e7a7a128a2573e95e981626d4dad/docs/Software_Architecture.md)). Do not infer deployed cFS from its naming.

## Requirements reconciliation

| Topic | Recorded direction | Remaining work |
|---|---|---|
| Orbit groups | User confirms sections 1.1/1.2: 650 km at 97°, 550 km at 63°, 450 km at 40°, two spacecraft per group. | Propose corrections to ORB-001/002 in the requirement owner's source; do not configure old 500 km/90°/60° entries. |
| Mission life | User confirms system-level requirements take precedence, including SYS-003's 5 years following commissioning. | Align ORB-004's 24-month allocation; do not claim it establishes a 5-year capability. |
| Disposal | User specifies deorbit within 25 years. | Define the interval's reference epoch and approved compliance authority before claiming compliance. |
| Approval | Requirements are proposed. | Keep proposed mapping separate from signed-off verification. |
| Initial twin | Bench EPS/UART behavior does not require orbital parameters. | Defer orbit/environment implementation until it is part of the intended tests. |

No requirement PDF or original ICD has been edited, and no numerical interface uncertainty has been silently resolved.

## Source discrepancies to retain

- **Deployment:** Manual's five Python containers versus the provided C scaffold.
- **Physical control:** Claimed EPS command path versus an identified measurement-only sensor interface and no confirmed switching actuator.
- **Protocols:** Manual's TCP/JSON and line commands versus ICD-402/403's proposed binary headers/fields/checksums.
- **Power figures:** Prototype, flight architecture, manual, and ICD supply/current descriptions are not interchangeable validated load parameters.
- **Wiring:** Manual's CTS-to-ground and UART labels versus the developer guide's signal directions ([RockBLOCK connector guide](https://cdn.sparkfun.com/assets/6/d/4/c/a/RockBLOCK-9603-Developers-Guide_1.pdf)).

Record these openly; do not build a model that conceals them.
