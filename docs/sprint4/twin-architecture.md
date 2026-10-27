# Proposed FlatSat Twin Architecture

Status: design outline, not implemented. Recover the [actual service source](fsw-integration.md) before locking adapter APIs or build integration.

## Intended behavior

Run the actual FlatSat application logic against modeled peripherals, preserving the existing ground/CDH protocol where possible. Do not replace the apps with stock cFS applications and call that a replica of the current FlatSat.

```text
Existing dashboard / command client
             |
        Actual CDH logic
         |           |
    EPS service   RockBLOCK host-side code
         |           |
     I2C adapter   UART adapter
         |           |
     NOS Engine buses if selected and explicitly integrated
         |           |
    3 INA219 models  RockBLOCK AT/SBD model
         \           /
       Shared modeled power state
```

All software and hardware-facing boxes are proposed integration boundaries. NOS3 timing/bus wiring is not yet implemented; do not claim tick synchronization based on a diagram.

## Model responsibilities

- **Power model:** Track externally supplied rails, OBC/modem availability, configured loads, and reset/availability effects. Distinguish externally injected test power loss from a supported OBC command.
- **INA219 models:** Provide the register/configuration behavior needed by the recovered library, raw-value scaling, calibration, absent-device behavior, and independent bus/address identities.
- **RockBLOCK model:** Implement only the observed/required AT subset, echo/framing, buffers, session result fields, reset state, error cases, and bounded timing.
- **Network outcome fixture:** Supply deterministic SBD success/failure and incoming messages. This is not the real Iridium network.
- **Application services:** Preserve actual business logic; label health/CPU/clock values that belong to the simulation host rather than the X8.

OBC and model processes must not all disappear when simulating OBC power loss. Keep the test supervisor outside the modeled OBC boundary so it can observe shutdown/restart and restore external power.

## Component placement

The project-owned sources and configurations belong in this repository; the NOS3 integration checkout is a separate dependency, not a location for untracked replacements.

| Project-owned content | Proposed home | Later NOS3 integration |
|---|---|---|
| Scope, interface decisions, procedures | `docs/sprint4/` | Documentation only |
| Source pins, mission config source | `missions/sentinel-bio-flatsat/` | Explicitly selected/staged into the working configuration |
| INA219, power, modem model source | `components/<device>/sim/` | Simulator build and launch registration, after API choice |
| App-facing transport adapters | `components/<device>/adapters/` | Connect actual Linux app access to the chosen simulation bus |
| Device fixtures/unit tests | `components/<device>/tests/` | Model-level behavior, separate from end-to-end tests |
| Recovered team applications | Version-pinned external source or authorized maintained fork | Run alongside the models; do not silently relocate them into cFS app directories |
| End-to-end scenarios | `tests/scenarios/` | Test driver and assertions |
| Formal result records | `evidence/test-records/` | Exact source/configuration and limitations |

This tree is a project proposal. No executable directories or files have been created to imply these integration points already work.

## Time and fidelity

First establish whether the recovered apps use wall-clock sleeps, monotonic timers, or an injectable clock. NOS Engine model ticks alone do not make all Python timers deterministic.

Choose either a bounded real-time initial test mode or an explicitly coordinated/injected clock; document that decision before asserting synchronized behavior. 42 orbit/attitude truth is not needed for the initial EPS/UART tests.

## What simulated success will mean

A passing modem transcript test supports a host-software claim for the modeled cases. It does not prove correct power supply, pin directions, physical flow control, modem firmware, antenna, service account, RF link, or successful satellite delivery.

Independent expected transcripts should be derived from the protocol reference and later real measurements, not copied from the app's own assumptions. The RockBLOCK AT reference specifies stateful buffer/session behavior, rather than a universal `OK` response ([AT reference](https://docs.groundcontrol.com/iot/rockblock/user-manual/at-commands)).
