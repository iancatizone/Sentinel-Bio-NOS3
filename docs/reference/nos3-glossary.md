# NOS3 Common Terminology

Plain-language definitions for terms used in this repository. Primary references: [NASA NOS3 repository](https://github.com/nasa/nos3) and [NOS3 documentation](https://nos3.readthedocs.io/).

| Term                         | Plain-language definition                                                                                                                                                                                                          |
| ---------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| NOS3                         | A spacecraft development and test environment that connects flight software, simulated hardware, ground software, and dynamics/environment models. It is a collection of cooperating tools, not just one satellite simulator. |
| Flight software (FSW)        | Software that performs spacecraft functions: processing commands, collecting data, controlling devices, and reporting status. In NOS3, it can run against simulated hardware. |
| cFS                          | Core Flight System: a reusable flight-software architecture with applications and common services. It is a flight-software framework, not the entire NOS3 environment. |
| Flight-software application  | A software unit within the flight-software system that performs a particular function and can handle commands and produce telemetry. |
| Ground software              | The tools used outside the spacecraft to send commands and receive/display telemetry. COSMOS is the ground-software tool used in the documented NOS3 architecture. |
| Component simulator          | A program that imitates a hardware device’s interfaces and behavior so flight software can interact with it without the physical device being present. |
| NOS Engine                   | Middleware that provides simulated communication buses and supports coordinated simulation time. It connects participating software; it is not itself the spacecraft dynamics model. |
| Hardware abstraction / HWLIB | The layer through which flight software accesses hardware interfaces. In NOS3, those accesses can be connected to simulated devices instead of physical ones. |
| 42                           | The dynamics/environment simulator used by NOS3 to calculate spacecraft motion and environmental information needed by relevant component simulations. |
| COSMOS | Ground-software tool (Ball Aerospace COSMOS 4.5.0 in this baseline) used to send commands and view telemetry. Its windows include the Command and Telemetry Server, Command Sender, and Packet Viewer. |
| Docker container | An isolated runtime environment. In NOS3, each process (FSW, each simulator, 42, COSMOS) runs in its own container inside the VM. The window title "(on fortytwo)" names the container's host. |
| Vagrant / VirtualBox | Host-side tools that create and run the Ubuntu NOS3 virtual machine from the `Vagrantfile` in the NOS3 repository. |
| NOS3 Igniter | GUI opened by `make prep` for choosing the mission configuration before building. |
| Mission configuration | XML files in `cfg/` (`nos3-mission.xml`, `spacecraft/sc-*-config.xml`, `sims/nos3-simulator.xml`) that select the FSW, GSW, scenario, and enabled components. |
| Scenario (STF1) | The 42 orbit and environment setup used by the baseline. STF1 is the default LEO scenario, named after the Simulation-to-Flight 1 CubeSat. |
| Data provider | The part of a component simulator that supplies its truth values, usually read from 42 over a TCP socket. |
| Time driver | `nos_time_driver`. It sends simulation-time ticks through NOS Engine so FSW and simulators advance together. |
| NOOP command | A no-operation command. The app only increments its command counter and logs an event, which proves the command path works end to end. |
| Housekeeping (HK) telemetry | Periodic status packets an app sends (for example, `GENERIC_ADCS_HK_TLM`), including command and error counters. |
| F′ (F Prime) | NASA JPL's component-based flight-software framework. NOS3 supports it as an alternative to cFS, and Sentinel-Bio+ plans to fly it. |
