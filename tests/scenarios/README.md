# FlatSat Acceptance Scenario Outlines

Status: no executable tests or Pass results. The authoritative scenario list is the [validation plan](../../docs/sprint4/validation-plan.md).

## Future scenario format

Each implemented scenario should specify initial power/device state, input bytes or ground command, expected device exchanges, expected app output, timing basis, assertions, and teardown. Mark fixtures synthetic or hardware-recorded and pin their revisions.

Prioritize:

- **Power availability:** OBC and modem availability follow the modeled prototype graph.
- **EPS telemetry:** Each INA219 remains identifiable and values reach the actual CDH app.
- **EPS commands:** Exercise only the confirmed as-built operation.
- **Iridium integration:** Initialization, outgoing buffers/session results, incoming command routing, and fault/retry handling.

Keep model tests distinct from application-integration tests and future hardware correlation. A hand-written client that passes against a model does not prove the missing real FlatSat application works.

## Safety and reproducibility

Tests must not discover or mount real host I²C/UART devices, connect to live modem accounts, or send paid satellite messages. Use deterministic local fixtures and bounded waits; retain raw transcripts for formal regression/acceptance runs.
