# Current FlatSat Configuration

Status: input-based inventory, not a fresh lab inspection. The user's October 27 account defines the current target; older manual/diagram details remain provisional until reconciled.

## Installed and planned elements

| Element | Current knowledge | Modeling treatment |
|---|---|---|
| OBC | Portenta X8 on Portenta breakout board, running Linux; OBC/EPS communication works per user. | Reuse application logic; do not emulate the X8 processor. |
| EPS | Custom breadboard prototype with 3.3 V, 5 V, and 12 V converters. | Rail availability/load abstractions, with explicitly synthetic initial values. |
| Measurement | Three INA219 modules; manual says one per rail, default address 0x40 on separate buses. | Independent bus/address/register identities and calibration parameters. |
| Iridium | RockBLOCK 9603 owned and wired; integration remains unsuccessful per current user. Manual reports prior AT-level communication, not integrated SBD. | Distinguish historical manual claim from current unresolved behavior. |
| S-band | Not implemented on the FlatSat. | Excluded; no modeled primary-link dependency required. |
| ADCS/payload emulators | Arduino emulator is an eventual goal, not implemented. | Excluded except existing software payload-sim dependencies if recovered apps require them. |
| Battery/arrays | The flight power architecture image includes them, but the current bench description identifies a supply-powered converter prototype. | Do not model battery/solar hardware as installed without confirmation. |

## Prototype versus flight design

The supplied prototype wiring image is the relevant starting point for this twin. The other image includes future arrays, battery, reaction wheels, payload loads, and S-band; do not treat that flight-design view as the as-built bench inventory.

The prototype image labels the supply 12–16 V with a 1 A limit, while the manual gives a different bench power-up current-limit procedure. These are source discrepancies, not approved operating instructions in this document.

## Provisional Linux device mapping

Transcribed from the supplied operations manual, not rediscovered in the lab:

| Rail/interface | Breakout label | Linux endpoint | Address/settings |
|---|---|---|---|
| 3.3 V INA219 | I2C0 | `/dev/i2c-2` | 0x40 |
| 5 V INA219 | I2C1 | `/dev/i2c-1` | 0x40 |
| 12 V INA219 | I2C2 | `/dev/i2c-3` | 0x40 |
| RockBLOCK UART | UART2 | `/dev/ttymxc2` | 19200 baud, 8N1 per manual |

The manual warns that bus 1 also has kernel-managed devices. The twin should not reproduce broad permission changes or probe real host buses to make tests pass.

The manual also lists calibration value 4096, current LSB 100 µA/bit, and an assumed 0.1 Ω shunt. Treat these as historical configuration inputs to confirm against code and installed modules, not freshly measured values.

## Power and command boundary

INA219 is a current/power monitor with an I²C/SMBus interface; calibration and conversion configuration are not external rail-switch control ([TI INA219 description](https://www.ti.com/product/INA219)).

To fill before implementation:

- [ ] Identify the exact OBC-to-EPS command and expected response.
- [ ] Identify any GPIO, converter EN wire, MOSFET, relay, or EPS controller that changes load power.
- [ ] Determine whether the claimed command only requests telemetry or changes sensor configuration.
- [ ] Confirm whether OBC and modem power are independently switchable.
- [ ] Record shunt values, calibration settings, sensor-library conventions, and sample logs.

Until answered, do not claim that `EPS`, `PAYLOAD_OFF`, INA219 power-down mode, or a modem sleep command physically turns an EPS rail off.

## RockBLOCK wiring caution

The manual's CTS direction and UART pin descriptions conflict with the RockBLOCK connector guide. The guide identifies pin 1 RXD and pin 2 CTS as outputs from RockBLOCK, and pin 6 TXD and pin 3 RTS as inputs ([developer guide](https://cdn.sparkfun.com/assets/6/d/4/c/a/RockBLOCK-9603-Developers-Guide_1.pdf)).

Treat this as an unresolved document/harness issue. Do not use the manual's CTS-to-ground instruction or wire colors as validated wiring guidance; confirm the exact board revision and pin directions before future hardware work. The twin can investigate software behavior but cannot exclude an electrical fault.
