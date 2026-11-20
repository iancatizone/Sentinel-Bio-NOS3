# Test Record: Sentinel-Bio+ FlatSat EPS component checkout

| Field | Value |
|---|---|
| Record ID | TR-20261120-01 |
| Tester | Ian Catizone |
| Date | Recorded 2026-11-20 (tests run in the NOS3 VM after the 2026-11-18 component install) |
| Baseline ID | SBP-NOS3-BL-02 (candidate) |
| NOS3 commit | `5a3bdee6be9a2c67fdf994ae6db56d5c60395302` + `sentinelbio_flatsat_eps` 2.0.0 integration |
| Representation exercised | NOS3 / software only |
| Environment | NOS3 Vagrant VM (Ubuntu 22.04.4) in VirtualBox 7.2; NOS3 in Docker containers |
| Configuration used | `cfg/nos3-mission.xml` default (cFS, COSMOS); `sc-mission-config.xml` with `eps/enable = true`; `sentinelbio_flatsat_eps_sim` as the EPS simulator |
| Procedure followed | `nos3/components/sentinelbio_flatsat_eps/INSTALL.md` §4 (launch and full-path checks). BL-02 build/run guide not yet written |

## Results

| Test | Expected result | Observed result | Pass / Fail | Evidence |
|---|---|---|---|---|
| 1. EPS is the configured EPS | `SB_EPS` app and `sentinelbio_flatsat_eps_sim` start in place of the generic EPS | Custom EPS app and simulator run as the mission EPS | Pass | To attach (FSW syslog, EPS Sim tab) |
| 2. Periodic HK | `SENTINELBIO_FLATSAT_EPS_HK_TLM` received at a fixed rate | HK published every 1 second | Pass | To attach (Packet Viewer screenshot with receive count/time) |
| 3. Commanded HK | `SENTINELBIO_FLATSAT_EPS_REQ_HK` produces an HK packet | Command works; HK packet returned | Pass | To attach |
| 4. NOOP | FSW NOOP receipt event; `CMD_COUNT` increments | EPS app receives NOOP commands | Pass | To attach (event message, `CMD_COUNT` before/after) |
| 5. Rail voltages | 12000 / 5000 / 3300 mV, constant | Defined regulator voltages of 12 V, 5 V, and 3.3 V reported | Pass | To attach |
| 6. Stationary current | Rail currents constant at configured load with no pulse | Current stationary | Pass | To attach |
| 7. Load pulse | Pulsed rail's current changes in HK; voltage stays fixed | Current increase seen in HK packets when a rail was pulsed | Pass | To attach (HK before/during pulse; simulator pulse command) |

## Deviations from the procedure

- The load-pulse capability used in the VM is not in the committed simulator source or the component README (open item EPS-OI-01 in [the EPS component note](../../docs/sprint4/eps-component.md)).

## Known limitations

- Software-only result. No comparison yet with FlatSat EPS hardware or an Arduino emulator.
- Rail voltages and loads are simulation assumptions, not measured FlatSat or flight EPS values.
- Per [configuration control](../../docs/baseline/config-control.md) §4, each Pass needs an attached log or screenshot before sign-off.

## Sign-off

| Role | Name | Date |
|---|---|---|
| Tester | Ian Catizone | |
| Reviewer | | |
