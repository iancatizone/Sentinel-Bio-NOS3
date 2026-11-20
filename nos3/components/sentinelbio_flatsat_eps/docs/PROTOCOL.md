# INA219 protocol used by `sentinelbio_flatsat_eps`

The cFS driver uses the TI INA219 register map. Every transaction is a 7-bit-address I²C transaction in which the first transmitted byte selects the register and the simulator returns two register bytes, most significant byte first.

| Register | Address | Access | Encoding |
| --- | ---: | --- | --- |
| Configuration | `0x00` | R/W | 16-bit register; default `0x399F` |
| Shunt voltage | `0x01` | R | signed two's complement; 10 µV/bit; reported as signed 32-bit telemetry |
| Bus voltage | `0x02` | R | bits 15:3 are the code; 4 mV/bit; bits 2:0 are status |
| Power | `0x03` | R | unsigned; 2 mW/bit with the default calibration |
| Current | `0x04` | R | signed two's complement; 0.1 mA/bit with the default calibration |
| Calibration | `0x05` | R/W | unsigned; reset `0`; FSW programs `4096` |

The default model uses a 0.1 Ω shunt and calibration `4096`. For a rail current `I` and voltage `V`:

```text
shunt_code  = round(I × 0.1 Ω / 10 µV)
bus_code    = round(V / 4 mV)
current_code = trunc(shunt_code × calibration / 4096)
power_code  = trunc(current_code × bus_code / 5000)
```

The cFS engineering-unit conversion is:

```text
bus_mV    = (bus_register >> 3) × 4
shunt_uV  = signed(shunt_register) × 10
current_mA = signed(current_register) × 0.1
power_mW  = power_register × 2
```

The simulator applies a simple regulator model: it reports the configured nominal output when the bench input is at least `nominal-voltage + dropout-voltage`; below that point it reports `max(0, input-voltage - dropout-voltage)`. This is intentionally a transparent approximation rather than a claim about an unspecified regulator part number.

## I²C topology

| INA219 | NOS Engine bus | Address | cFS handle |
| --- | --- | ---: | ---: |
| 12 V rail | `i2c_1` | `0x40` | `1` |
| 5 V rail | `i2c_2` | `0x41` | `2` |
| 3.3 V rail | `i2c_3` | `0x44` | `3` |

The driver first checks configuration and calibration, repairs mismatches, and returns
an invalid sample for that attempt. On a subsequent attempt it reads measurements,
requires CNVR set and OVF clear, and converts only complete successful samples.
Failed rails are zeroed; consult the validity bit before interpreting their fields.
DeviceErrorCount increments once per incomplete three-rail housekeeping cycle.

The simulator completes conversions immediately and does not clear CNVR on Power
reads. It approximates steady-state measurements, not all INA219 modes or timing.
The real driver relies on the sensor's conversion-ready status.

Register reference: [Texas Instruments INA219 datasheet, sections 8.5–8.6](https://www.ti.com/lit/ds/symlink/ina219.pdf).
