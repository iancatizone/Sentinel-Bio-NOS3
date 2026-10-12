NOS3 and the overall test environment

Flight software

NOS Engine

Component simulators

42

Ground software

Command, telemetry, and simulation-data flow between them


| Architecture item                 | Item in baseline        | Evidence                         | Uncertainty |
| --------------------------------- | --------------------------------- | -------------------------------- | ----------- |
| Ground software                   | COSMOS                      | [Window, configuration, or log]  | Classic COSMOS or C3    |
| Flight-software framework         | cFS                         | [Startup/configuration evidence] | [If any]    |
| Commanded application             | [Target/application]        | [NOOP test/configuration]        | [If any]    |
| Component simulators              | Stock enabled components    | [Configuration/startup logs]     | Confirm whhich components are enabled  |
| Hardware abstraction / NOS Engine | HWLIB connects flight-software hardware ops to buses, NOS3 Engine does bus communication and simulation time           | [Relevant configuration/source]  | None  |
| Dynamics/environment              | 42                          | [Configuration/log]              | None   |
| Execution environment             | Ubuntu Linux VM, Docker?  | [evidence/baseline-versions.txt](/evidence/baseline-versions.txt) | [If any]    |
