# Lock Application for Linux

Application that showcases abilities of the Door Lock Cluster.

## Building

The application could be build in the same manner as `all-clusters-app`:

```
? scripts/examples/gn_build_example.sh examples/lock-app/linux out/lock-app chip_config_network_layer_ble=false
```

# Named Pipe Interface

This application accepts commands over named pipe. Pipe is located in
`/tmp/chip_lock_app_fifo-<PID>`.

## Command Format

Commands interface accepts commands formatted as a JSON object. Structure of the
object:

| Key      | Type   | Value Description                                                                               | Optional |
| -------- | ------ | ----------------------------------------------------------------------------------------------- | -------- |
| `Cmd`    | String | Contains name of the command to execute                                                         | No       |
| `Params` | Object | Contains parameters for the command. Could be omitted if the command does not accept parameters | Yes      |

For example:

-   `{ "Cmd": "SetDoorState", "Params": { "EndpointId": 1, "DoorState": 2 } }` -
    command to set the Door Jammed door state.
-   `{ "Cmd": "SendDoorLockAlarm", "Params": { "AlarmCode": 0 } }` - command to
    send the Door Lock Alarm event with Lock Jammed alarm code.

## Available commands

### Set Door State

-   Name: `SetDoorState`
-   Parameters:
    -   `EndpointId` (Uint, Optional): ID of the endpoint where command should
        be executed. Could be omitted, in that case the default endpoint with ID
        1 is targeted.
    -   `DoorState` (DoorState enum): new door state.
-   Usage:
    ```bash
     echo '{"Cmd": "SetDoorState", "Params": { "EndpointId": 1, "DoorState": 1 } }' > /tmp/chip_lock_app_fifo-<PID>
    ```
    This command will set the door state to "Closed" resulting in Door State
    Change event.

### Send Door Lock Alarm

-   Name: `SetDoorState`
-   Parameters:
    -   `EndpointId` (Uint, Optional): ID of the endpoint where command should
        be executed. Could be omitted, in that case the default endpoint with ID
        1 is targeted
    -   `AlarmCode` (AlarmCode enum): code of the Lock Door Alarm to send.
-   Usage:
    ```bash
     echo '{"Cmd": "SendDoorLockAlarm", "Params": { "EndpointId": 1, "AlarmCode": 0 } }' > /tmp/chip_lock_app_fifo-<PID>
    ```
    This command will send the Door Lock Alarm event with "Lock Jammed" alarm
    code.
