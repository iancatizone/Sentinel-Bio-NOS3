NOS3 Baseline Setup, Build, and Run Guide
=========================================
## 1. Purpose and scope

This guide describes how to set up and run the stock NOS3 baseline
used for Sentinel-Bio+ simulation development.

It covers:
- Setting up the NOS3 virtual machine in Oracle VirtualBox.
- Building and launching the stock baseline.
- Performing a basic checkout.
- Stopping the simulation.

It does not cover custom Sentinel-Bio+ simulation implementation.

## 2. Tested setup

- Test date: 10/9/2026
- Tester: Ian Catizone
- Host operating system: Windows 11
- VirtualBox version: 7.2
- Guest operating system: Ubuntu 22.04.4 LTS
- NOS3 commit: 5a3bdee6be9a2c67fdf994ae6db56d5c60395302

Full version record:
[Versions](../evidence/baseline-versions.txt)

Installation instructions followed:
[https://github.com/nasa/nos3](https://github.com/nasa/nos3)

## 3. First-time VM setup

Skip this section if the NOS3 VM is already installed and working.

### 3.1 Setup Oracle VirtualBox

1. Download VirtualBox version 7.16 or newer from: [VirtualBox](https://www.virtualbox.org/)
   
2. Download Vagrant AMD64 version 2.4.3 or newer from: [Vagrant](https://developer.hashicorp.com/vagrant)

3. Download git version 2.47 or newer from: [git](https://git-scm.com/)

### 3.2 Install the NOS3 repository

1. Open a command prompt

2. Clone the repository: 

```bash
git clone https://github.com/nasa/nos3.git
```

3. Change directory to the repository: 

```bash
cd C:\nos3
```


4. Clone the submodules: 

```bash
git submodule update --init --recursive
```



### 3.3 Add or create the VM in VirtualBox

1. In the nos3 directory in a command prompt, use
```bash
vagrant up
```
This may take several minutes. When you get a return prompt, run: 
```bash
vagrant halt
```

Close the command prompt

### 3.4 Start the VM

   1. Open VirtualBox and start the NOS3 virtual machine; the name should along the lines of "nos3_2025..."  
   2. Sign in to jstar user with the password: jstar123!

   3. In the VirtualBox toolbar click $$\color{red}\text{Devices -> Upgrade Guest Additions...}$$

   4. Reboot the VM

## 4. Locate and identify the NOS3 checkout

All steps below are run inside the VM unless stated otherwise.

Open a terminal
Run the following:
```bash
[cd ~/Desktop/github-nos3]
```

Record the baseline revision:

```bash
git rev-parse HEAD
git submodule status --recursive
git status --short
```

Compare the results with:
[evidence/baseline-versions.txt](/evidence/baseline-versions.txt)

## 5. Prepare and build NOS3

Starting inside a terminal in the directory ```~/Desktop/github-nos3```

Run preparation command, the NOS3 Igniter should open after:

```bash
make prep
``` 

When finished, run build command:

```bash
make
```

## 6. Launch the baseline

Launch command:

```bash
make launch
```

Expected result:
- COSMOS Legal agreement should open
- Several terminals should open rapidly
- 42 map, cam, and unit sphere should open
- NOS3 Launcher should open

## 7. Baseline checkout

### Test 1: Successful Startup

Procedure:
Launch NOS3 using the procedure in Section 6.

Expected result:
Telemtry and command windows open and the flight software updates time occasionally.
[evidence/screenshots/baseline-running.png](/evidence/screenshots/baseline-running.png)

### Test 2: Telemetry Updates

Procedure:
1. Click COSMOS on the NOS3 Launcher.
2. Open the COSMOS Command and Telemetry Server window.
3. Confirm that the housekeeping packets count increase with time.

Expected result:
Three COSMOS windows should open: Packet Viewer, Command and Telemetry Server, and Command Sender. Housekeeping packet count should be increasing.
[evidence/screenshots/telemetry-check.png](/evidence/screenshots/telemetry-check.png)

### Test 3: Sending a NOOP Command

Procedure:
1. Open COSMOS as done in test 2.
2. In Command Sender, set target to ```GENERIC_ADCS```
3. Set command to ```GENERIC_ADCS_NOOP_CC```
4. Before sending, in Packet Viewer, set target to the same, and ensure the packet selected is ```GENERIC_ADCS_HK_TLM```
5. Observe the bottom area of the Command and Telemetry Server.
6. Send the commmand, check that a new line appears on the Command and Telemetry Server

Expected result:
The new lines should read the following, with different date and times:

```2026/10/10 04:02:05.022  INFO: cmd("GENERIC_ADCS GENERIC_ADCS_NOOP_CC")```

```2026/10/10 04:02:05.025  INFO: Log File Opened : /home/jstar/Desktop/github-nos3/gsw/cosmos/outputs/logs/2026_10_10_04_02_05_cmd.bin```
[evidence/screenshots/command-check.png](/evidence/screenshots/command-check.png)\

## 8. Stop NOS3 and shut down the VM

1. Before stopping, close all COSMOS windows; do not run the stop command before this
2. Open a new terminal window and go into the NOS3 directory again, then run the stop command, do not click until all 42 and NOS3 Launcher windows have closed:

```bash
make stop
```
3. Graphics windows and NOS3 Launcher window should close, along with flight software terminals.

VM shutdown procedure:
1. Click the power icon in the upper right of the VM, then select $$\color{red}\text{Power Off/Log Out -> Power Off...}$$
2. The Oracle VirtualBox Manager should show the VM as "Powered Off"

## 9. Troubleshooting and known limitations

Issue 1: github-nos3 file is empty
Solution: In host computer's command prompt, inside nos3 directory, run:
```bash
vagrant reload --provision
```

Issue 2: System console freezes when booting or shutting down
Solution: Go to $$\color{red}\text{Machine -> Pause}$$, wait a few seconds, then unpause. If this does not work, press reset instead.

Known limitations:
None.

## 10. Independent reproduction

Tester:
Name

Date:
Date

Starting environment:
Setup

Result:
Passed / Failed / Pending

Deviations or assistance required:
[Details]

Full reproduction record:
[evidence/independent-reproduction.md](/evidence/independent-reproduction.md)
