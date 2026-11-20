# Installation and acceptance checks

Run these commands inside your NOS3 Linux VM. Extract this archive into a separate
folder first. Do not copy it over your existing component before running the installer:
the installer backs up files before replacing them.

## 1. Select the actual NOS3 root

The root must contain `Makefile`, `scripts/`, `cfg/nos3_defs/`,
`components/ComponentSettings.cmake`, and `sims/CMakeLists.txt`.
The uploaded GitHub `nos3/` snapshot lacked several of these source files. Generated
`cfg/build/` files alone are not a reproducible checkout. Use your complete working
checkout, or restore a separate copy of the recorded baseline with its submodules.
Do not run `make clean` on the incomplete snapshot: it would remove generated artifacts
without the source needed to recreate them.

Example for your earlier VM path (change it if your actual checkout differs):

```bash
NOS3="$HOME/Desktop/github-nos3"
EPS_PACKAGE="$HOME/Downloads/sentinelbio_flatsat_eps"
python3 "$EPS_PACKAGE/tools/install_into_nos3.py" "$NOS3" --dry-run
python3 "$EPS_PACKAGE/tools/install_into_nos3.py" "$NOS3"
python3 "$EPS_PACKAGE/tools/verify_install.py" "$NOS3"
```

Stop running NOS3 before installation. The installer plans integration in a temporary
copy before writing. Changed files are backed up under
`sentinelbio_eps_backups/<timestamp>/files/`, with a `manifest.json` listing newly
created files and replacements. It preserves unrelated files and does not rename or
delete the generic EPS directory. Repeating installation with identical inputs makes
no changes. If a prerequisite or patch anchor is missing, resolve that error first.

If all you need immediately is to restore the empty component folder in an incomplete
checkout, use this explicit mode:

```bash
python3 "$EPS_PACKAGE/tools/install_into_nos3.py" "$NOS3" --component-only
```

This copies the source with backups. It does **not** integrate, configure, or build NOS3.

## 2. Check mission selection and regenerate

Ensure `cfg/nos3-mission.xml` selects cFS and the intended ground system. In the
spacecraft XML selected by `sc-1-cfg`, set `components/eps/enable` to `true`.
Populate missing upstream submodules in a proper Git checkout before building:

```bash
cd "$NOS3"
git submodule update --init --recursive
```

If this populated COSMOS after installation, run the installer again so it patches
its source target declarations. Run `make prep` only if this checkout's NOS3 Docker
and 42 environment has not already been prepared. Then regenerate explicitly:

```bash
cd "$NOS3"
make config
python3 components/sentinelbio_flatsat_eps/tools/verify_install.py "$NOS3" --generated
```

A failed generated check means stop and resolve configuration; do not launch stale
binaries. The verifier checks source wiring and active generated startup/simulator
selection. Upstream simulator XML uses some nonstandard element names elsewhere;
the verifier parses the EPS block separately.

## 3. Build and verify

```bash
cd "$NOS3"
make fsw
make sim
make gsw
python3 components/sentinelbio_flatsat_eps/tools/verify_install.py "$NOS3" --built
```

Run commands separately and stop on any failure. `--built` checks configuration and
library presence, not freshness or a successful live run. Require successful build
exit statuses. If CMake reports an old checkout path, save logs, then remove stale
build outputs using the baseline's clean targets and regenerate **after** cleaning:

```bash
make clean-fsw
make clean-sim
make config
make fsw
make sim
make gsw
```

Expected libraries:
`fsw/build/exe/cpu1/cf/sentinelbio_eps.so` and
`sims/build/lib/libsentinelbio_flatsat_eps_sim.so`.

The installer updates cFS target/startup, scheduler/TO tables, simulator templates,
launch/configuration scripts, COSMOS mappings, and YAMCS references. It disables
standard LC battery watchpoints 27 and 28: the new packet has no battery field.
Stored switch commands remain compatible in packet shape but are rejected as
unsupported. Review any mission-specific actionpoints/RTS relying on those behaviors.

## 4. Launch and prove the full path

```bash
cd "$NOS3"
make launch
```

In the FSW terminal/syslog, look for:

```text
SB_EPS: EPS_AppMain entered; package 2.0.0
SENTINELBIO_FLATSAT_EPS App Initialized. Version 2.0.0; I2C connects on HK
SB_EPS: initialized; CMD=0x19E0 REQ_HK=0x19E1 TLM=0x09E0
```

The EPS simulator should log INA219 registration on `i2c_1` at `0x40`, `i2c_2`
at `0x41`, and `i2c_3` at `0x44`. Its launch argument must be
`sentinelbio_flatsat_eps_sim`.

In COSMOS Command Sender choose target `SENTINELBIO_FLATSAT_EPS` and send
`SENTINELBIO_FLATSAT_EPS_NOOP_CC`. Confirm the **FSW receipt event** and an increase
in `CMD_COUNT`; a Command Sender log alone only proves transmission.
Send `SENTINELBIO_FLATSAT_EPS_REQ_HK`, then view
`SENTINELBIO_FLATSAT_EPS_HK_TLM` in Packet Viewer. After calibration settles:

- Packet reception count/timestamps must advance.
- `RAIL_VALID_MASK = 7`, `I2C_READY_MASK = 7`.
- Rail voltages/currents/powers match the README defaults.
- `DEVICE_COUNT` advances. The error counter stops increasing once all rails are valid.

A packet with validity zero is distinguishable from no packet received. Pink/NA
alone is insufficient to diagnose the device: first check packet reception, then
validity. If there is no startup entry log, inspect the active startup script,
module loader errors, and `EPS_AppMain` export. If NOOP arrives but no HK reaches
the ground, inspect TO subscriptions and the ground connection. If HK arrives with
invalid rails, inspect simulator startup and bus/address matching. Missing Docker
containers, 42 windows, or the ground system require NOS3 environment diagnosis;
this component cannot repair those services.

## 5. Change assumptions and preserve source

Edit loads/supply/shunts in the package's `sim/cfg/nos3-eps-simulator.xml`, rerun the
installer from that package, then `make config`, `make sim`, and restart. Changing
I2C addresses requires matching FSW settings and simulator settings. Changing shunt
resistance requires recalculating FSW calibration/current/power scaling and rebuilding
FSW as well. Keep simulator startup calibration at zero; FSW programs it.

The steady-state model does not model ADC timing, conversion-ready clearing,
all INA219 operating modes, regulator dynamics, or battery/solar behavior.
Do not treat it as a measured electrical model.

Commit the restored component and integration source changes in your project so
another checkout can recreate them. Keep the downloaded archive as a separate copy.
