NOS3 and the overall test environment

Flight software

NOS Engine

Component simulators

42

Ground software

Command, telemetry, and simulation-data flow between them


| Architecture item                 | Actual item in my baseline        | Evidence                         | Uncertainty |
| --------------------------------- | --------------------------------- | -------------------------------- | ----------- |
| Ground software                   | COSMOS                      | [Window, configuration, or log]  | Classic COSMOS or C3    |
| Flight-software framework         | cFS                         | [Startup/configuration evidence] | [If any]    |
| Commanded application             | [Target/application]        | [NOOP test/configuration]        | [If any]    |
| Component simulators              | Stock enabled components    | [Configuration/startup logs]     | Confirm whhich components are enabled  |
| Hardware abstraction / NOS Engine | [How it is used]            | [Relevant configuration/source]  | [If any]    |
| Dynamics/environment              | 42                          | [Configuration/log]              | None   |
| Execution environment             | Ubuntu Linux VM, Docker?  | [/evidence/baseline-versions.txt]        | [If any]    |
