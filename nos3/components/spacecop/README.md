# SpaceCOP - Community Edition

## What is SpaceCOP?

SpaceCOP is an onboard intrusion detection system (IDS) for spacecraft funded by the
Department of Homeland Security (DHS) Science and Technology Directorate (S&T). It runs as a
[cFS](https://cfs.gsfc.nasa.gov/) flight application and continuously monitors the vehicle
for signs of compromise — watching system calls, file integrity, memory, and the commands
sent up from the ground — and flagging behavior that matches known threats.

Detections are expressed as rules mapped to [SPARTA](https://sparta.aerospace.org/) IOBs
(Indicators of Behavior), so operators can describe attacker techniques in a standard
vocabulary rather than hand-coding one-off checks. Threat intelligence can be delivered to
the spacecraft as [STIX](https://oasis-open.github.io/cti-documentation/)-formatted CTI
reports, and an optional machine-learning component (SpaceCOP ML) adds anomaly detection
trained on the vehicle's own telemetry. Alerts surface through the OpenC3/COSMOS ground
software so operators can respond in near real time.

In short: SpaceCOP gives a spacecraft the kind of host-based intrusion detection that's long
been standard on the ground, adapted to the constraints of flight software.

This repository contains the cFS SpaceCOP Community Edition Flight application.
This includes the flight software (FSW) and OpenC3/COSMOS ground software (GSW) definitions.

## To build Kernel Module

Go to `fsw/kernel` and run `make` to generate the `aerospace.ko` file.

Build the software like normal from the `nos3/` base directory.

Either copy the `fsw/kernel/aerospace.ko` file to the `/cf` directory, or enable it to load on boot:

```bash
cp fsw/kernel/aerospace.ko /lib/modules/$(uname -r)/kernel/drivers
echo aerospace > /etc/modules-load.d/aerospace_module.conf
```

## Things to do after building cFS

Copy the `whitelists/proc_whitelist.txt` to `/cf` directory if you wish to run either the procmon and/or memory monitor rules.

## For CTI processing

Create the directory `/cf/cti`.

SpaceCOP looks for 3 things within CTI. The following is an example that SpaceCOP can process:

```json
{
	"type": "bundle",
	"objects": [
		{
			"type": "indicator",
			"x_sparta_iob_id": "DISE-1",
		},
		{
			"type": "file",
			"name": "test.txt",
			"hashes": {
				"SHA-256": "5891b5b522d5df086d0ff0b110fbd9d21bb4fc7163af34d08286a2e846f6be03"
			}
		}
	]
}
```

While this is not everything provided in a SpaceCOP CTI report, these are the only important parts. SpaceCOP will only look at the items within the `"objects"` array. If it sees type `"indicator"`, it will grab the SPARTA ID and assign it to all objects below it. For type `"file"`, it will grab the `"name"` and the `"SHA-256"` hash. Then, it will see if there is a file with that hash and/or if there is the filename with the hash.

## To add a new Rule/SPARTA IOB

1) Add the rule to the `spacecop_call_table.c` file and increase the `num_rules` by 1. All rules live here and can have a variety of tasks:
   - `.enabled = 1` to run the rule. `.enabled = 0` if you wish to disable it
   - `iob_id[10]` is a string for the specific ID you wish to work with. This is only really required for the syscall IOBs.
   - `call_index` corresponds to the call_index for functions in the `spacecop_registry.c` file (explained later)
   - `RuleMode` is how to run the IOB. Currently, the options are `RULE_PERIODIC` (run every X number of ticks in the main loop), `RULE_TRIGGER` (run every X number of ticks in its own thread), `RULE_SYSCALL` (corresponds to a specific call within the `functions_syscall_kernel_module.c` file), and `RULE_ON_COMMAND` (reliant on a specific APID and Command Code from the software bus - not implemented yet)
   - `period_ticks` is how many "ticks" or iterations through the main loop the rule is checked
   - `cmd_any` is whether the rule is ran on all commands (not implemented yet)
   - `cmd_mid` is the APID of a specific command/telemetry to run the rule on (not implemented yet)
   - `cmd_code` is the Command Code to run the rule on. Dependent on the `cmd_mid` (not implemented yet)
   - `lhs` is the Left Hand Side of any operations taking place. This is of type `Operand` which internal values will be explained later
   - `op` is the Operator that will be performed on `lhs` and `rhs`. The current options here are `OP_NEQ` (Not Equal To), `OP_ABS_DIFF_GT` (Absolute Greater Than), `OP_EQ` (Equal To), `OP_NOT_IN_LIST` (lhs not in rhs list), and `OP_GT` (Greater Than)
   - `rhs` is the Right Hand Side of any operations taking place. This is of type `Operand` which internal values will be explained later
   - `tol` is the Tolerance that can be allowed. This is of type `Literal` which will be explained later
2) Create new `.h`/`.c` files to perform the task you are trying to do
3) Include your new `.h` at the top of `spacecop_registry.c` and write an `INVOKE` function to call on your new task. These functions MUST fit the style of `int32 (*GenericInvokeFn)(const ArgBlob* args, uint8_t* ret_buf, uint16_t ret_buf_len, uint16_t* ret_len)`. This is to allow them to be added to the `kSyms` array at the bottom of `spacecop_registry.c`.
4) Add your `INVOKE` function to the `kSyms` array. These values can include:
   - `name` is just a simple name for the function. Usually tried to correspond this to a `system:process` or something else related to the IOB STIX
   - `call_index` is the number the rule depends on in the table
   - `fn` is the name of your INVOKE function
   - `ret_type` is the type you are returning. This could be a `RT_U32` (`uint32_t`), `RT_F32` (`float`), `RT_I32` (`int32_t`), or `RT_STR64` (`string[64]`). This is not needed if you are not expecting a return result
   - `ret_size` is the size of the return. Usually `sizeof(<type>)`
   - `expected_args` is what kind of args to expect. This can be a combination of `ARG_F32`, `ARG_STR64`, `ARG_U32` or simply `ARG_NONE` if you don't want to pass anything
   - `need_iob` is a true(1)/false(0) call if you need to pass the `iob_id` from the rule

# Rule Help

## Operand Definitions

In the rules, the Operands (`lhs`/`rhs`) contain two different objects.

The first is a `SrcKind` which can be either `SRC_CALL_NEW` or `SRC_CALL_OLD`:

- `SRC_CALL_NEW` means it is the returned value of the function from `call_index`
- `SRC_CALL_OLD` is the saved value from a previous run
- `SRC_TOL` means the source is the tolerance set in the rule

The second is a `ValType` which tells what kind of type this object will be. The options are `VT_I32`, `VT_F32`, `VT_STR64`, `VT_STR32`, and `VT_U32`.

## Literal Definitions

In the rules, the `tol` is of type `Literal`. This type allows us to hardset literal values to compare items to. The objects that make up this struct are the `ValType` from the `Operand` struct and a union called `v`. In `v`, you can set a `int32_t i32`, `float f32`, or `uint32_t u32`.

## Example

The following is an example of a rule:

```c
{
    .enabled = 1,
    .iob_id = "GNTM-6",
    .mode = RULE_PERIODIC, //Run every tick
    .period_ticks = 3, //Limit one run every 3 seconds
    .call_index = 0, //look in spacecop_registry.c for index
    .lhs = { .kind = SRC_CALL_NEW, .type = VT_U32 },
    .op = OP_ABS_DIFF_GT,
    .rhs = { .kind = SRC_CALL_OLD, .type = VT_U32 },
    .tol = { .type = VT_U32, .v = { .u32 = 5 }},
}
```

Obviously, we have this rule enabled and we are calling the IOB `"GNTM-6"`. We have set the mode to `RULE_PERIODIC` and said to run every 3 ticks. Our `call_index` is 0 meaning it is the first value in our `kSyms` array in `spacecop_registry.c`. The `lhs` is of `SRC_CALL_NEW` and type `VT_U32`, meaning that we will store the results of `call_index=0` into the `lhs` Operand. Our `op` is `OP_ABS_DIFF_GT` meaning we are going to see if our `abs(lhs-rhs) >= tol`. `rhs` is of `SRC_CALL_OLD` and type `VT_U32` meaning we are storing the value from `call_index=0` and using the stored value every time. Finally, the `tol` is of type `VT_U32` and we have hardset a value of `.v.u32=5`.

This rule is following the "Unexpected Time Delta" IOB and calls on the time-controller (`call_index=0`) function. This function will return a `uint32` every time it is called. Once received, this value is subtracted from the old value and then compared to our tolerance (5).

## Command Monitoring Rules

In order to constantly monitor commands being sent to the SV, SPACECOP has a second table to define how commands are to be monitored. The following instructions explain how to add a new command monitoring rule to SPACECOP.

### Adding a STIX definition
First, command monitor entries are tracked based on STIX identifiers. This makes them easier to compare with IOBs and allows for multiple commands which perform effectively the same thing to be monitored together. The file `src/spacecop_stix_components.h` defines the constants for existing STIX components.
When creating a new identifier, two values must be created. The first is a `str` identifier, and the second is an `int` id:

```c
#define propulsion_burn_duration_id 0
#define propulsion_burn_duration_str "propulsion:burn_duration"
```

Additionally, you must increment the `STIX_COMPONENT_COUNT` variable at the top of the file.

### Adding a Command Monitor Rule
Once you have your STIX identifier configured, you can add a new command monitoring entry. The definition for the entry is in `src/spacecop_mission_specific.h`. For CFS that entry looks like:

```c
struct {
    CommandMonitorAction_t Action;
    const StixComponentId_t StixId;
} StixIdAction_t;

typedef struct {
    CFE_SB_MsgId_t ActionOnMid; // Mission ID of the component you want to monitor
    CFE_MSG_FcnCode_t ActionOnCommandCode; // Command code which indicates the start of the action
    CFE_SB_MsgId_t DeactivateOnMid; // Mission ID of the component you want to monitor (can be different from ActionOnMid if needed)
    CFE_MSG_FcnCode_t DeactivateOnCommandCode; // Command code which indicates the end of the action
    StixIdAction_t StixInfo;
} CommandMonitorEntry_t;
```

`DeactivateOnMid`/`DeactivateOnCommandCode` are both optional if command rules only need to trigger when a command is seen. For example, if you're trying to count the duration your propulsion was enabled you could use this rule:

```c
{
    .ActionOnMid={GENERIC_THRUSTER_CMD_MID},
    .ActionOnCommandCode=GENERIC_THRUSTER_ENABLE_CC,
    .DeactivateOnMid={GENERIC_THRUSTER_CMD_MID},
    .DeactivateOnCommandCode=GENERIC_THRUSTER_DISABLE_CC,
    .StixInfo={
            .Action=COUNT_DURATION,
            .StixId=propulsion_burn_duration_id
    }
}
```
This would tell SPACECOP to enable the `COUNT_DURATION` action when `GENERIC_THRUSTER` receives the `ENABLE_CC` command, and disable the `COUNT_DURATION` when `GENERIC_THRUSTER` receives the `DISABLE_CC` command.

If you only wanted to count the number of times `GENERIC_THRUSTER` receives the `ENABLE_CC` command you could leave the Deactivate fields blank.

These entries are added in `table/spacecop_command_monitor_table.c`.

#### Updating Imports
If you are importing new `MID`/`CC` for your rule, make sure `src/spacecop_external_imports.h` has the proper import files. If not, add them to `src/spacecop_external_imports.h` and ensure the proper directories are included in `CMakeLists.txt`.

### Adding a Registry Rule
Finally, you can configure a monitor rule in the standard spacecop registry as defined above to monitor values and thresholds for your STIX components. An example of this using the monitor entry above can be found in `src/spacecop_registry.c`. The function `INVOKE_PropulsionBurnDuration` monitors the rule. Specifically, `SPACECOP_RemoveCountDurationIfInactive(<stix_component_id>)` is used to get the current duration of that STIX ID. The `kSyms` call index 6 shows a sample `SymEntry` for the rule. Finally, `table/spacecop_call_table.c` Rule 12 shows a sample entry in the rule table for configuring the threshold.

## Adding a New Action
Current actions can be found in `src/spacecop_mission_specific.h`. If a new action is needed, add an entry to the `CommandMonitorAction_t` enum. Functions which support that action should be placed in `src/functions_command_monitors.c`.

Finally, add the new action, and any activate/deactivate code to the switch statement in `src/spacecop_table_runtime.c`. As an example, here is the count duration code:

```c
while((MonitorEntry = SPACECOP_ConvertCommandToStixId(&ResultIndicator, &offset)) != NULL ){
        if(ResultIndicator == ACTIVATE){
            #ifdef SPACECOP_MONITOR_DEBUG
            printf("SPACECOP: Found activate command for %d\n", MonitorEntry -> StixInfo.StixId);
            #endif
            SPACECOP_StartCountDuration(MonitorEntry -> StixInfo.StixId);
        }
        else if (ResultIndicator == DEACTIVATE){
            #ifdef SPACECOP_MONITOR_DEBUG
            printf("SPACECOP: Found deactivate command for %d\n", MonitorEntry -> StixInfo.StixId);
            #endif
            SPACECOP_SetEntryInactive(MonitorEntry -> StixInfo.StixId);
        }
        else{
            printf("SPACECOP: Somehow found ERROR command for %d\n", MonitorEntry -> StixInfo.StixId);
        }
}
```

## SpaceCOP ML
SpaceCOP ML (SCML) is a standalone executable that runs on the spacecraft and listens on port `9111` for command and telemetry hex values to monitor. SpaceCOP listens to SCML on port `9112` to receive anomaly alerts detected by the machine learning models trained on previous values. All training is done on the ground and new models are uploaded to the spacecraft for inference. See `spacecop_ml/README.md` for more information.
