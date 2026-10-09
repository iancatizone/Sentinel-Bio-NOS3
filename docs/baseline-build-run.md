NOS3 Baseline Setup, Build, and Run Guide
=========================================
1. Purpose and scope

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
[Link Text](evidence/baseline-versions.md)

Installation instructions followed:
[Link to the actual NOS3 instructions you used]

## 3. First-time VM setup

Skip this section if the tested NOS3 VM is already installed and working.

### 3.1 Install Oracle VirtualBox

1. Download VirtualBox from: [Actual source]
2. Install: [Version used]
3. [Record any nondefault steps or settings you needed]

Expected result:
[How to confirm VirtualBox is installed and opens successfully]

### 3.2 Obtain the NOS3 virtual machine

Source:
[Actual download link or procedure used to obtain/create the VM]

VM filename or release:
[Record if applicable]

Steps:
1. [First step you actually followed]
2. [Next step]
3. [Continue as needed]

### 3.3 Add or create the VM in VirtualBox

1. [Describe the actual process you followed]
2. [Record any settings you changed]
3. [Explain how to start the VM]

VM settings used:
- Memory: [Amount]
- CPUs: [Number]
- Network configuration: [Setting, if relevant]
- Other changes: [Changes or "None"]

### 3.4 Start the VM and open a terminal

1. Start the NOS3 VM.
2. [Describe login using the documented VM account, if applicable]
3. Open a terminal inside the VM.

Expected result:
[Describe the desktop/terminal state you reached]

## 4. Locate and identify the NOS3 checkout

All commands below are run inside the VM unless stated otherwise.

Navigate to the NOS3 source folder:

```bash
[Your actual cd command]
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
