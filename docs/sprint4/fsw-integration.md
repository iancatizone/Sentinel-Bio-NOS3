# FlatSat Software Recovery and Integration Outline

Status: actual running source not yet identified. Do not call the supplied C applications a recovered image of the current FlatSat.

## Two different software descriptions

| Input | What it describes | Treatment |
|---|---|---|
| Operations manual, May 7, 2026 | Five Python-based Docker services, TCP JSON telemetry, CDH text commands, web dashboard. | Working-system description to reconcile against actual source. |
| Linked GitHub repository, reviewed at `06db4b7f4726e7a7a128a2573e95e981626d4dad` | C application and interface scaffolding; no five-service Python/Docker deployment found in the reviewed main tree. | Reference material, not proven deployed software. |
| CDR phrase supplied by user | Modular, NASA-style, inspired by cFS. | Design inspiration, not proof of cFE/cFS or F′ runtime integration. |

The C repo lists hardware UART mapping, task scheduling, checksums, and tests as future work ([software architecture](https://raw.githubusercontent.com/aalamillo014/sentinel_bio_software_sjsu/06db4b7f4726e7a7a128a2573e95e981626d4dad/docs/Software_Architecture.md)). Its UART driver prints writes and returns the fixed simulated string `CMD:SET_BEACON:1800` rather than accessing a serial device ([UART implementation](https://raw.githubusercontent.com/aalamillo014/sentinel_bio_software_sjsu/06db4b7f4726e7a7a128a2573e95e981626d4dad/interfaces/uart_driver.c)).

Therefore, bugs in that scaffold are not established causes of the physical FlatSat's RockBLOCK problem. The user's stock NOS3 cFS baseline is a third, separate software system.

## Service inventory from the manual

| Service/image family | Manual source folder | Interface | Missing input |
|---|---|---|---|
| `flatsat-health` | `health/` | TCP 9000 telemetry | `health.py`, Dockerfile, dependencies |
| `flatsat-payload-sim` | `payload-sim/` | TCP 9001 telemetry, 9011 commands | `payload_sim.py`, Dockerfile, dependencies |
| `flatsat-eps` | `eps/` | TCP 9002 telemetry | `eps.py`, I²C library/configuration, Dockerfile |
| `flatsat-cdh` | `cdh/` | TCP 9090 aggregate, 9100 commands | `cdh.py`, autonomy/command code, Dockerfile |
| `flatsat-groundstation` | `groundstation/` | HTTP 8080, subscribes to CDH | `groundstation.py`, any templates/static assets, dependencies |
| RockBLOCK experiment | Not established | `/dev/ttymxc2` in manual | Current script/service, invocation, and failing transcript |

These filenames and ports are manual-transcribed identifiers, not a verified running deployment in this environment. Do not publish private credentials, board IDs, account information, or internal network configuration from a recovered archive.

## Recovery checklist

- [ ] Find the actual source archive/repo from the outgoing team or retained board backup.
- [ ] Pin the source revision and document differences from the running Docker images.
- [ ] Obtain launch commands, Dockerfiles, dependency versions, runtime architecture, and permitted redistribution terms.
- [ ] Identify the exact EPS driver/library and current RockBLOCK caller.
- [ ] Preserve raw boot/telemetry/AT transcripts and the invocation that produced the failure.
- [ ] Identify whether the modem issue is no AT response, framing/parsing, SBD buffer write, session status, or CDH routing.

Also request the manual's `flatsat_obc_test_log.md`, `flatsat_obc_test_log_day2.md`, and `flatsat_obc_test_log_day5.md`, plus any retained AT transcripts. No board access is required to receive an existing source backup; if only binary images remain, document that limitation before claiming source-equivalent behavior.

## Proposed adaptation boundary

Prefer the real CDH logic, telemetry schema, and dashboard, with narrow injectable hardware transports. Inspect the actual Python library before choosing between a transport-level adapter and an application-level mock; a register/byte path provides stronger driver coverage than simply generating expected JSON.

For UART, a pseudo-terminal may support a real serial parser, but it is not a complete physical UART and does not validate baud wiring or electrical flow control. For I²C, a high-level library substitute does not validate kernel ioctls or board device mapping.

If NOS Engine is used, explicitly bridge the selected app transport to its bus API and implement/register the device models. An ordinary Linux `/dev/i2c-*` or `/dev/tty*` access does not become a NOS Engine transaction automatically.

## Outputs to fill

| Artifact | Required content |
|---|---|
| Source lock | Actual service repository/revision plus dependency and container pins |
| App reuse map | Unchanged app logic, minimal portability changes, and test-double boundaries |
| Command/telemetry contract | Exact actual schemas, framing, acknowledgments, and errors |
| Reproducibility instructions | Tested build and launch procedure on the chosen simulation host |
