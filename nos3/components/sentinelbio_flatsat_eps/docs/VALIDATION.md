# Validation record — package 2.0.0

Checked on November 17, 2026. This is component-level validation, not a claim of
successful end-to-end operation in the user's VM.

## Passed

| Check | Evidence and boundary |
|---|---|
| FSW compilation | Both C source files checked against cFE/HWLIB and generated headers in the supplied repository snapshot, GCC C99, Wall/Wextra/Werror/Wformat=2. Old absolute include paths remapped in temporary headers. An empty shim replaced the unrelated missing libsocketcan header; EPS uses no CAN API. Syntax checking only, no cFE link/runtime. |
| Actual device driver execution | Calibration zero repaired; subsequent valid readings; signed current/shunt conversion; failed transaction clears all sample fields; overflow rejects data; sensor-reset calibration repaired; closed bus rejected. Injected I2C transport. |
| Actual C++ model execution | Compiled with GCC C++11 warnings as errors and Boost 1.74. Confirmed default zero calibration/current, programmed measurements on all three rails, reset, register-pointer transactions, malformed-length rejection. NOS Engine/base class/logger shims used; real transport/ABI not tested. |
| Ground packet layout | Compiled C packet decoded against both COSMOS and XTCE definitions; 64-byte packet, signed fields, IDs, command sizes/codes agree. |
| XTCE validation | Passed the XTCE 1.2 schema in the supplied Yamcs source; common CCSDS header checked. No live Yamcs/COSMOS session. |
| Installer | Missing-prerequisite failure leaves target unchanged; dry-run; install; backup manifest; repeat install idempotent; unrelated component files preserved. Tested against extracted baseline source. |
| Real upstream configuration | Patched NASA NOS3 baseline 5a3bdee6be9a2c67fdf994ae6db56d5c60395302; ran configure.py and all three COSMOS configuration scripts using supplied COSMOS configuration files. Generated active EPS startup, simulator, scheduler/TO, and launch selections passed verification. This ran configuration scripts directly, not Docker make/build targets. |
| Protocol and Python syntax | Five protocol checks and all Python files passed. |

## Still required in the VM

Full `make fsw`, `make sim`, `make gsw`, real shared-library load, NOS Engine
transactions, live command receipt, telemetry routing, late-start behavior under
the real transport, and complete mission launch. Follow INSTALL.md acceptance
checks and retain logs. A successful presence verifier alone cannot establish that
a binary was rebuilt or loaded.

The earlier I2C-startup explanation was a code-supported possible failure path;
there was no runtime log proving it caused the original missing telemetry. This
release removes that fatal initialization dependency without treating the old
diagnosis as proven.

## Reproduce component checks

From this package directory, with Python 3 and GCC/G++ installed:

```bash
python3 tests/test_protocol.py
python3 tests/test_device.py
python3 tests/test_ground.py
python3 tests/test_sim.py
```

The simulator test needs Boost headers (`--boost-include PATH` supported).
`test_ground.py --nos3-root PATH` additionally needs lxml and the baseline Yamcs
schema/common CCSDS files. `test_fsw_compile.py PATH` needs the configured EPS
CMake flags and cFE/HWLIB headers from the original snapshot; it remaps that
snapshot's absolute paths without modifying it.

`test_install.py PATH` copies the supplied baseline into a temporary fixture and
tests installation there. It does not install into PATH.
