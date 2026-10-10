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
- Guest operating system: [OS and version inside the VM]
- NOS3 commit: [Commit ID]

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

```[git clone https://github.com/nasa/nos3.git]```

3. Change directory to the repository: 

```[cd nos3]```


4. Clone the submodules: 

```[git submodule update --init --recursive]```



### 3.3 Add or create the VM in VirtualBox

1. In the nos3 directory in a command prompt, use: $$\color{red}\text{vagrant up}$$
This may take several minutes. When you get a return prompt, run: $$\color{red}\text{vagrant halt}$$

Close the command prompt

### 3.4 Start the VM

1. Open VirtualBox and start the NOS3 virtual machine

VM name should along the lines of "nos3_20250217_1790120441224_84756"  
2. Sign in to jstar user with passwords: jstar123!
3. In the VirtualBox toolbar under "Devices", click "Upgrade Guest Additions..."
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

Save the results in:
`evidence/baseline-versions.txt`

Expected result:
[Confirm that you are in the correct repository and the revision
matches the tested baseline]

## 5. Prepare and build NOS3

Starting location:
[NOS3 source folder inside the VM]

Preparation command, if required:

```bash
[Actual preparation command]
```

Build command:

```bash
[Actual build command]
```

Expected result:
[Describe the successful output you observed]

Build evidence:
[Link to saved build log]

Notes:
[First-run downloads, prompts, workarounds, or "None"]

## 6. Launch the baseline

Starting location:
[NOS3 source folder inside the VM]

Launch command or GUI procedure:

```bash
[Actual launch command, if using the terminal]
```

Expected result:
- [Application/window that opens]
- [Startup message or status]
- [Telemetry or simulation output that confirms it is running]

Launch evidence:
[Link to log and/or screenshot]

## 7. Basic baseline checkout

### Test 1: [Name of test]

Procedure:
1. [Action]
2. [Action]

Expected result:
[What should happen]

Observed result:
[What happened in your test]

Result:
[Pass / Fail]

Evidence:
[Link to log or screenshot]

### Test 2: [Name of test]

Procedure:
1. [Action]
2. [Action]

Expected result:
[What should happen]

Observed result:
[What happened in your test]

Result:
[Pass / Fail]

Evidence:
[Link to log or screenshot]

## 8. Stop NOS3 and shut down the VM

Stop command or procedure:

```bash
[Actual stop command, if applicable]
```

Expected result:
[How you confirmed NOS3 stopped]

VM shutdown procedure:
[How you shut down the guest OS/VM]

## 9. Troubleshooting and known limitations

| Symptom | Cause, if known | Fix or next check |
|---|---|---|
| [Problem encountered] | [Cause or "Unknown"] | [What worked] |

Known limitations:
- [Any incomplete checks or unresolved issues]

## 10. Independent reproduction

Tester:
[Second student's name]

Date:
[Date]

Starting environment:
[Their setup]

Result:
[Passed / Failed / Pending]

Deviations or assistance required:
[Details]

Full reproduction record:
[Link to ../evidence/independent-reproduction.md]
