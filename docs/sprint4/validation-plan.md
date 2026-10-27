# FlatSat Twin Validation Outline

Status: proposed tests, none executed. Record model fidelity separately from whether a scenario passes.

## Acceptance scenarios

| ID | Scenario | Required observation | Status / dependency |
|---|---|---|---|
| S4-PWR-01 | External bench supply/rail state powers modeled OBC and modem | Availability follows the documented power graph; supervisor remains able to observe/reset the test. | Planned; no electrical validation |
| S4-EPS-01 | EPS telemetry reaches actual OBC/CDH logic | Three distinct measurement paths with correct identity, scaling, units, validity, and schema; no hardware mounts. | Blocked on actual `eps.py` and calibration/configuration |
| S4-EPS-02 | OBC issues the real supported EPS operation | Exact input, hardware-facing transaction, response, and observable effect agree with the as-built contract. | Blocked on operation/actuator identification |
| S4-EPS-03 | One INA219 fails or is absent | Correct fault/validity behavior without mislabeling another rail as healthy. | Expected app behavior TBD |
| S4-IR-01 | Modem initialization and basic AT exchange | Real host code handles termination, echo, split reads, result framing, and timeouts against an independently specified model. | Blocked on current RockBLOCK code |
| S4-IR-02 | Outgoing message and successful simulated session | Buffer contents and length preserved; success follows session fields rather than any arbitrary `OK`. | Planned |
| S4-IR-03 | Failed/delayed session and retry | No false delivery claim; application follows approved timeout/backoff policy. | Policy/source needed |
| S4-IR-04 | Incoming command through modem | Injected MT message reaches the actual parser/router and affects only the intended supported operation. | Actual command path TBD |
| S4-IR-05 | Modeled power loss/reset | App and model clear/retain state as specified; recovery is observable without hidden automatic success. | Reset contract needed |

If current EPS commands only read telemetry or configure INA219 conversion, S4-EPS-02 must test that operation. Do not substitute fictional independent load switching to satisfy the user's requested path.

## Candidate mission traceability

The user says `Requirements.pdf` is proposed. The following are candidate relationships, not claims that simulation completes flight verification:

| Requirement | Possible contribution | What is not established |
|---|---|---|
| EPS-009 | Telemetry fields/update handling | Battery temperature/SoC exist in the current prototype; actual 1 Hz performance |
| EPS-010 / EPS-011 | Commanded switching only if a real actuator exists | Independent switches or real response timing |
| OBC-001 / SYS-018 | App acceptance-to-dispatch timing | Embedded worst-case latency or command acceptance definition |
| OBC-006 / COM-002 | Beacon generation and format | Actual satellite transmission |
| COM-003 | Simulated incoming-command routing | Live network reception |
| COM-004 / COM-005 | Retry and delivery-state handling | Real service behavior outside the modeled contract |

Use T/A/I/D for verification method as defined in the PDF, and a separate environment field such as software twin, FlatSat, or analysis. Do not equate the environment code `NOS3` with complete verification of a hardware requirement.

## Protocol and data oracle

Retain input bytes, response bytes, timestamps, expected outcomes, and actual assertions. The modem reference defines different states and lengths for AT commands, SBD writes, session results, and MT reads; do not implement a model that simply agrees with the application ([RockBLOCK AT reference](https://docs.groundcontrol.com/iot/rockblock/user-manual/at-commands)).

Test synthetic fixtures first, then compare retained hardware transcripts when available. Never label synthetic values as measured FlatSat telemetry.

## Evidence for a formal run

- **Configuration:** Project commit, actual app commit, NOS3/model dependencies, containers, and fixture revision.
- **Procedure:** Scenario and command sequence, expected values, actual values, and assertions.
- **Results:** Raw relevant logs/transcripts and explicit Pass/Fail/Blocked/Not run.
- **Limitations:** Mock boundary, clocks, unmodeled electrical effects, and requirements not covered.

Use the existing [test-record template](../../evidence/test-records/TEMPLATE.md), adding the scenario and actual app/model revisions. A signed baseline checkout is not evidence that the new twin works.

## Later lab comparison

Replay a small safe, approved command sequence on the real FlatSat and compare semantic outputs and timing with declared tolerances. Obtain explicit approval before live modem/network actions or hardware power manipulation; no account credentials or paid satellite traffic are needed for the software-only phase.
