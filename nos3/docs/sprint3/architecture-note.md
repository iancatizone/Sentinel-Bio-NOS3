# SJSU NOS3 Architecture Note

Sprint 3 (October 12–25, 2026). Baseline `SBP-NOS3-BL-01`, NOS3 commit `5a3bdee6be9a2c67fdf994ae6db56d5c60395302`.

Status: OUTLINE. Each section lists the questions from the execution plan (§7.3 NOS3 tasks) and verified starting files. Replace the "Starting points" with findings, source line references, and diagrams.

Source-reference format: `repo/path/file.c:LINE` at the baseline commit. Component paths are inside their submodule (for example, `components/generic_mag/...`).

## 1. Representative component trace

Suggested component: generic magnetometer (`components/generic_mag`, submodule `nasa-itc/generic_magnetometer` @ `7d6833f`). It is small, uses a 42 data provider, and is directly relevant to Sentinel-Bio+ detumble.

Starting points:

| Layer | File |
|---|---|
| cFS app (commands, HK, request loop) | `components/generic_mag/fsw/cfs/src/generic_mag_app.c` |
| Device driver (bus protocol, shared by cFS/F′/standalone) | `components/generic_mag/fsw/shared/generic_mag_device.c` |
| Message IDs / platform config | `components/generic_mag/fsw/cfs/platform_inc/generic_mag_msgids.h`, `generic_mag_platform_cfg.h` |
| F′ version | `components/generic_mag/fsw/fprime/mag_src/Generic_mag.cpp` |
| Standalone checkout app | `components/generic_mag/fsw/standalone/generic_mag_checkout.c` |
| Simulator hardware model | `components/generic_mag/sim/src/generic_mag_hardware_model.cpp` |
| 42 data provider | `components/generic_mag/sim/src/generic_mag_42_data_provider.cpp` |
| Ground definitions | `components/generic_mag/gsw/generic_mag.xtce` |
| Sim config block | `cfg/sims/nos3-simulator.xml` (`generic-mag-sim`: `spi_2`, CS 2, 42 port 4234) |
| App startup entry | `cfg/nos3_defs/cpu1_cfe_es_startup.scr` (`MAG_AppMain`) |

To do: sequence diagram from COSMOS command → app → device → hwlib → NOS Engine → sim → 42, and back to HK telemetry.

## 2. Where 42 truth data enters

Starting points: `GENERIC_MAG_42_PROVIDER` in `nos3-simulator.xml`; `generic_mag_42_data_provider.cpp`; 42 IPC setup in `cfg/InOut/Inp_IPC.txt`; `sims/truth_42_sim` (truth to COSMOS on port 5111).

To do: identify the function that parses the 42 message and the point where the hardware model reads it.

## 3. NOS Engine timing, messaging, registration, transport

Starting points: `cfg/sims/nos_engine_server_config.json` (URIs `tcp://nos-engine-server:12000/12001`, `uart` plugin); `sims/nos_time_driver`; `sims/sim_common`; `fsw/apps/hwlib/sim/`; `osal_nos-linux_impl` / `psp-nos-linux-impl` builds.

To do: for each of timing, messaging, registration, and transport, give the class or function name and the file.

## 4. Simulated vs. physical interface selection

Starting points: `fsw/apps/hwlib/` contains `fsw/` and `sim/` trees plus `arch_build.cmake`; the baseline build links `apps/hwlib/sim` (`noslink`).

To do: find where the build chooses `sim` vs. the Linux/hardware implementation, and how a FlatSat target would select real devices.

## 5. Mission configuration and build integration points

Starting points: `cfg/nos3-mission.xml`, `cfg/spacecraft/sc-mission-config.xml`, `scripts/cfg/config.sh`, `scripts/cfg/configure.py`, `cfg/nos3_defs/targets.cmake`, `cfg/nos3_defs/cpu1_cfe_es_startup.scr`, `sims/CMakeLists.txt`, `scripts/fsw/fsw_cfs_launch.sh`, `.gitmodules`.

To do: a table listing every file you touch to add one component.

## 6. Recommended minimal Sentinel-Bio+ extension path

To do: recommend one new synchronized component (candidate: UART payload simulator modeled on `components/sample`, which uses `usart_16`). Cover where the code lives, which config and build files change, and how it hooks into the time ticks and data flow.

## 7. Presentation assignment

| Student | Assigned piece | Section |
|---|---|---|
| Ian Catizone | TBD | |
| | | |
