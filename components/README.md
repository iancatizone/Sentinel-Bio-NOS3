# FlatSat Device-Model Source Conventions

Status: organization outline only. No simulators, drivers, adapters, or build registrations are implemented here.

## Proposed component boundaries

After recovering actual app interfaces, consider separate model sources for the prototype power state, INA219 devices, and RockBLOCK 9603 protocol. Three INA219 instances may share one implementation with distinct configuration, not three duplicated codebases.

An eventual component may contain:

```text
<device>/
  sim/         Device behavior and NOS3 model integration, if selected
  adapters/    Actual application transport to simulated I2C/UART
  tests/       Model unit tests and independent protocol fixtures
  README.md    Interface, limits, build integration, and source references
```

This project layout is not an automatic NOS3 build convention. Native NOS3 models require explicit configuration, build, registration, and launch integration, and application adapters must match the recovered code.

Do not add a generic EPS model and label it a replica of the breadboard without matching its measured interfaces. Do not port the application into cFS/F′ solely because the stock NOS3 baseline uses one of those frameworks.

See [architecture](../docs/sprint4/twin-architecture.md) and [software integration](../docs/sprint4/fsw-integration.md) for the decisions required before code.
