# Sentinel Bio FlatSat EPS — 2.0.0

Complete cFS component source, INA219 simulator, COSMOS/YAMCS definitions,
installer, verification tools, and tests for the Sentinel Bio NOS3 FlatSat.
Start with [INSTALL.md](INSTALL.md). See [docs/VALIDATION.md](docs/VALIDATION.md)
for tested behavior and remaining integration checks.

## Model and interface

This package carries forward the previous three fixed-rail design. Values are
simulation assumptions, not measured FlatSat specifications. No battery, solar
array, charger, or remotely controllable switches are modeled.

| Rail | HWLIB handle / NOS bus | 7-bit address | Default load | Default output |
|---|---|---|---|---|
| 12 V | 1 / i2c_1 | 0x40 | 250 mA | 12000 mV / 3000 mW |
| 5 V | 2 / i2c_2 | 0x41 | 500 mA | 5000 mV / 2500 mW |
| 3.3 V | 3 / i2c_3 | 0x44 | 750 mA | 3300 mV / 2474 mW |

Defaults: 14 V bench supply, 0.1 ohm shunts, INA219 configuration 0x399F,
programmed calibration 4096. Simulator calibration starts at zero, as after a
sensor reset. Integer power-register quantization accounts for 2474 mW.

| Interface | Value |
|---|---|
| Component folder | sentinelbio_flatsat_eps |
| cFS library / entry / app name | sentinelbio_eps.so / EPS_AppMain / SB_EPS |
| Simulator name / library | sentinelbio_flatsat_eps_sim / libsentinelbio_flatsat_eps_sim.so |
| Ground command / HK request / HK telemetry MID | 0x19E0 / 0x19E1 / 0x09E0 |
| NOOP / reset counters / unsupported switch CC | 0 / 1 / 2 |

The short library filename fits the supplied OSAL filename limit. Ground formats
target the baseline little-endian cFS payload with an 8-byte command header and
16-byte telemetry header. The HK packet is 64 bytes. These MIDs must remain unique
in your mission.

## Startup and data validity

App initialization registers events, creates the command pipe, and subscribes to
commands before attempting I2C. Failed master initialization is retried on later
HK attempts. HK runs on explicit requests and after a 1000 ms idle receive timeout;
continuous unrelated commands can postpone the idle fallback, so retain the scheduler.

The driver checks and repairs sensor configuration/calibration, then waits until
a later sample before reporting valid data. A failed or overflowed sample is cleared.
`RAIL_VALID_MASK` bits 0/1/2 identify valid 12 V/5 V/3.3 V samples; 7 means all valid.
`I2C_READY_MASK` means masters opened, not that slaves responded. It is not a
live server-connectivity indicator. Counters are 8-bit and wrap at 255.
`DEVICE_COUNT` increments once when all three rails succeed; otherwise
`DEVICE_ERR_COUNT` increments once per HK. A first calibration attempt can therefore
increase the error counter normally. Switch CC 2 increments the command-error counter.

HWLIB calls remain synchronous. The supplied NOS3 simulation HWLIB ignores its
transaction timeout argument, so a blocked transport can delay this application.
Recovery from an entire NOS Engine server restart is not guaranteed by this component;
restart the mission if HWLIB retains unusable master handles.

## Contents and scope

- `fsw/cfs` and `fsw/shared`: app, packet definitions, driver, build definitions.
- `sim`: three INA219 slaves and configurable steady-state rail model.
- `gsw`: COSMOS target and XTCE commands/telemetry.
- `tools`: installer and source/generated/binary-presence verifier.
- `tests`: compiled driver/model and ground-format tests.

Supported integration target: single-spacecraft cFS mission using the recorded
NASA NOS3 baseline `5a3bdee6be9a2c67fdf994ae6db56d5c60395302`.
F-prime and multi-spacecraft operation are not validated. No prebuilt binaries are
included. The original NOS3 license is retained in LICENSE.
