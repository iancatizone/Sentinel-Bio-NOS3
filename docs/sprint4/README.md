# Sprint 4: Current FlatSat Digital Twin

Status: planning outlines only, October 27, 2026. No device models, launch scripts, adapters, or runnable mission profile have been implemented.

## Objective and boundary

Replicate the capabilities of the physical FlatSat as it exists now so software integration can proceed without lab access. The first use is developing and diagnosing RockBLOCK 9603 integration while preserving the working OBC/EPS behavior.

The user identified these first three behaviors:

- **Power availability:** Represent the prototype EPS supplying the Portenta X8 OBC and RockBLOCK. This is a modeled power dependency, not electrical validation.
- **EPS telemetry to OBC:** Reproduce the three INA219 measurement paths and the software's telemetry handling.
- **OBC commands to EPS:** Reproduce the actual supported operations after identifying the command API and hardware actuator, if any. Do not infer rail switching from telemetry access.

RockBLOCK AT/SBD integration is the first development use case built on these paths. Its suspected software problem is a hypothesis, not a verified diagnosis.

## In and out of scope

| In scope | Outside the initial implementation |
|---|---|
| Portenta X8 Linux application behavior using recovered team source | CPU/SoC, bootloader, or Yocto image emulation |
| Breadboard EPS: rail-state abstraction and three INA219 interfaces | Validating converter regulation, ripple, surge, wiring, or physical power margins |
| RockBLOCK host-side UART/AT/SBD state and error handling | Actual satellite/RF operation, service activation, or delivery guarantees |
| Existing TCP/JSON services and dashboard where supported by recovered source | Replacing the current apps with stock cFS/F′ logic |
| Software-only deterministic tests | Live FlatSat connections, S-band, or physical ADCS/payload emulators |

42 is not required for the initial bench-interface tests. Do not simulate six spacecraft or introduce flight-orbit assumptions merely because those appear in proposed mission requirements.

## Document index

| Document | Purpose |
|---|---|
| [Sprint 2/3 review](sprint2-3-review.md) | Remaining closeout work and stale-record reconciliation |
| [As-built configuration](flatsat-as-built.md) | Distinguish installed hardware from planned flight hardware |
| [Interface inventory](interface-inventory.csv) | Bus/device mappings, implementation status, and evidence gaps |
| [FSW integration](fsw-integration.md) | Recover the actual apps and define the simulation boundary |
| [Twin architecture](twin-architecture.md) | Proposed data flow, model fidelity, and component placement |
| [NOS3 workflow](nos3-workflow.md) | Explain source/configuration/build versus the VM |
| [Validation plan](validation-plan.md) | Acceptance gates and evidence, with no fabricated Pass results |
| [Open items](open-items.md) | Questions, document provenance, and requirement decisions |
| [Mission manifest outline](../../missions/sentinel-bio-flatsat/README.md) | Version/configuration manifest fields to fill later |

## Proposed sequence and gates

- **Recover the source:** Obtain the five services named in the operations manual and the current RockBLOCK experiment code; establish working revisions.
- **Trace the real interfaces:** Identify the INA219 API and EPS command behavior. Complete the relevant Sprint 3 trace for the NOS3 mechanism chosen for integration.
- **Approve the boundary:** Choose how Linux I²C/UART accesses are redirected while keeping the actual app logic under test.
- **Implement the minimum:** Build one telemetry/command path before expanding the modem model.
- **Exercise faults:** Test framing, timeouts, response parsing, disconnects, and modeled power states with independent expected transcripts.
- **Correlate later:** Compare with retained hardware logs and repeat a small lab checkout when access returns.

The official Sprint 4 task sheet, deadline, and presentation requirements have not been supplied. This sequence is a proposed work breakdown, not a claim about additional assigned deliverables.
