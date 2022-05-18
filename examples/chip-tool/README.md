# Matter Client Example

An example application that uses Matter to send messages to a Matter server.

---

-   [Building the Example Application](#building-the-example-application)
-   [Using the Client to Request an Echo](#using-the-client-to-request-an-echo)

---

## Building the Example Application

See [the build guide](../../docs/guides/BUILDING.md#prerequisites) for general
background on build prerequisites.

Building the example application is quite straightforward. It can either be done
as part of an overall "build everything" build:

```
./gn_build.sh
```

which puts the binary at `out/debug/standalone/chip-tool` or directly via:

```
scripts/examples/gn_build_example.sh examples/chip-tool SOME-PATH/
```

which puts the binary at `SOME-PATH/chip-tool`.

### Using message tracing

Message tracing allows capture of the secure messages which can be used for test
automation.

There are additional flags to chip-tool to control where the traces should go:

-   --trace_file <file> Outputs trace data to the specified file.
-   --trace_log <0/1> Outputs trace data to the console with automation logs if
    set to 1

For example:

```
out/with_trace/chip-tool pairing <pairing_args> --trace_file trace.log
```

## Using the Client to commission a device

In order to send commands to a device, it must be commissioned with the client.
chip-tool currently only supports commissioning and remembering one device at a
time. The configuration state is stored in `/tmp/chip_tool_config.ini`; deleting
this and other `.ini` files in `/tmp` can sometimes resolve issues due to stale
configuration.

#### Commission a device

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

#### Commission a device over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

The command below uses the default values hard-coded into the debug versions of
the ESP32 all-clusters-app to commission it onto a Wi-Fi network:

    ```
    $ chip-tool pairing ble-wifi ${NODE_ID_TO_ASSIGN} ${SSID} ${PASSWORD} 20202021 3840
    ```

where:

-   \${NODE_ID_TO_ASSIGN} (which must be a decimal number or a `0x`-prefixed hex
    number) is the node id to assign to the node being commissioned.
-   \${SSID} is the Wi-Fi SSID either as a string, or in the form `hex:XXXXXXXX`
    where the bytes of the SSID are encoded as two-digit hex numbers.
-   \${PASSWORD} is the Wi-Fi password, again either as a string or as hex data

For example:

    ```
    $ chip-tool pairing ble-wifi 0x11 xyz secret 20202021 3840
    ```

or equivalently:

    ```
    $ chip-tool pairing ble-wifi 17 hex:787980 hex:736563726574 20202021 3840
    ```

#### Pair a device over IP

The command below will discover devices and try to pair with the first one it
discovers using the provided setup code.

    ```
    $ chip-tool pairing onnetwork ${NODE_ID_TO_ASSIGN} 20202021
    ```

The command below will discover devices with long discriminator 3840 and try to
pair with the first one it discovers using the provided setup code.

    ```
    $ chip-tool pairing onnetwork-long ${NODE_ID_TO_ASSIGN} 20202021 3840
    ```

The command below will discover devices based on the given QR code (which
devices log when they start up) and try to pair with the first one it discovers.

    ```
    $ chip-tool pairing code ${NODE_ID_TO_ASSIGN} MT:#######
    ```

In all these cases, the device will be assigned node id `${NODE_ID_TO_ASSIGN}`
(which must be a decimal number or a 0x-prefixed hex number).

#### Trust Store

Trust store will be automatically created using the default Test Attestation
PAA. To use a different set of PAAs, pass the path using the optional parameter
--paa-trust-store-path while running the built executable. Trusted PAAs are
available at credentials/development/paa-root-certs/.

The command below will select a set of trusted PAAs to be used during
Attestation Verification. It will also discover devices with long discriminator
3840 and try to pair with the first one it discovers using the provided setup
code.

    ```
    $ chip-tool pairing onnetwork-long ${NODE_ID_TO_ASSIGN} 20202021 3840 --paa-trust-store-path path/to/PAAs
    ```

### Forget the currently-commissioned device

    ```
    $ chip-tool pairing unpair
    ```

## Using the Client to Send Matter Commands

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

    ```
    $ chip-tool onoff on 1
    ```

The client will send a single command packet and then exit.

## Configuring the server side for Group Commands

1. Commission and pair device with nodeId 1234

2. Add Group to device

    ```
    $ chip-tool groups add-group 0x4141 Light 1234 1
    ```

3. Add group Keyset to device

    ```
    $ chip-tool groupkeymanagement key-set-write '{"groupKeySetID": 42,
    "groupKeySecurityPolicy": 0, "epochKey0":
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime0": 2220000,"epochKey1":
    "d1d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime1": 2220001,"epochKey2":
    "d2d1d2d3d4d5d6d7d8d9dadbdcdddedf", "epochStartTime2": 2220002, }' 1234 0
    ```

4. Bind Key to group
    ```
    $ chip-tool groupkeymanagement write group-key-map
    '[{"groupId": 16705, "groupKeySetID": 42, "fabricIndex": 0}]' 1234 0
    ```

## Configuring the client for Group Commands

Prior to sending a Group command, both the end device and the Client (Chip-tool)
must be configured appropriately.

To configure the client please use the groupsettings option

    ```
    $ chip-tool groupsettings
    ```

A group with a valid encryption key needs to be set. The groupid and the
encryption key must match the one configured on the end device.

To add a group

    ```
    $ chip-tool groupsettings add-group TestName 0x1010
    ```

To add a keyset

    ```
    $ chip-tool groupsettings add-keyset 0xAAAA 0 0x000000000021dfe0 hex:d0d1d2d3d4d5d6d7d8d9dadbdcdddedf
    ```

Take note that the epoch key must be in hex form with the 'hex:' prefix

Finally to bind the keyset to the group

    ```
    $ chip-tool groupsettings bind-keyset 0x1010 0xAAAA
    ```

## Using the Client to Send Group (Multicast) Matter Commands

To use the Client to send Matter commands, run the built executable and pass it
the target cluster name, the target command name, the Group Id in Node Id form
(`0xffffffffffffXXXX`) and an unused endpoint Id. Take note that Only commands
and attributes write can be send with Group Id.

E.G. sending to group Id 0x0025

    ```
    $ chip-tool onoff on 0xffffffffffff0025 1
    ```

The client will send a single multicast command packet and then exit.

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    ```
    $ chip-tool
    ```

Example output:

```bash
Usage:
  ./chip-tool cluster_name command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Clusters:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * barriercontrol                                                                    |
  | * basic                                                                             |
  | * colorcontrol                                                                      |
  | * doorlock                                                                          |
  | * groups                                                                            |
  | * iaszone                                                                           |
  | * identify                                                                          |
  | * levelcontrol                                                                      |
  | * onoff                                                                             |
  | * pairing                                                                           |
  | * payload                                                                           |
  | * scenes                                                                            |
  | * temperaturemeasurement                                                            |
  +-------------------------------------------------------------------------------------+
```

### How to get the list of supported commands for a specific cluster

To get the list of commands for a specific cluster, run the built executable
with the target cluster name.

    ```
    $ chip-tool onoff
    ```

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

    ```
    $ chip-tool onoff read
    ```

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

    ```
    $ chip-tool onoff on
    ```

### Run a test suite against a paired peer device

    ```
    $ chip-tool tests Test_TC_OO_1_1
    ```

## Using the Client for Setup Payload

### How to parse a setup code

To parse a setup code, run the built executable with the `payload` cluster name
and the `parse-setup-payload` command

    ```
    $ chip-tool payload parse-setup-payload code
    ```

#### QR Code

    ```
    $ chip-tool payload parse-setup-payload "MT:#####"
    ```

#### QR Code with optional Vendor Info

    ```
    $ chip-tool payload parse-setup-payload "MT:#####"
    ```

#### Manual Setup Code

    ```
    $ chip-tool payload parse-setup-payload "#####"
    ```

# Using the Client for Additional Data Payload

To parse an additional data payload, run the built executable with the `payload`
cluster name and the `parse-additional-data-payload` command

    ```
    $ chip-tool payload parse-additional-data-payload "#####"
    ```

# Command Reference

## Command List

-   [barriercontrol](#barriercontrol)
-   [basic](#basic)
-   [colorcontrol](#colorcontrol)
-   [doorlock](#doorlock)
-   [groups](#groups)
-   [iaszone](#iaszone)
-   [identify](#identify)
-   [levelcontrol](#levelcontrol)
-   [onoff](#onoff)
-   [pairing](#pairing)
-   [payload](#payload)
-   [scenes](#scenes)
-   [temperaturemeasurement](#temperaturemeasurement)

## Command Details

### barriercontrol

```bash
Usage:
  ./chip-tool barriercontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * barrier-control-go-to-percent                                                     |
  | * barrier-control-stop                                                              |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

### basic

```bash
Usage:
  ./chip-tool basic command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * reset-to-factory-defaults                                                         |
  | * ping                                                                              |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

### colorcontrol

```bash
Usage:
  ./chip-tool colorcontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * move-color                                                                        |
  | * move-color-temperature                                                            |
  | * move-hue                                                                          |
  | * move-saturation                                                                   |
  | * move-to-color                                                                     |
  | * move-to-color-temperature                                                         |
  | * move-to-hue                                                                       |
  | * move-to-hue-and-saturation                                                        |
  | * move-to-saturation                                                                |
  | * step-color                                                                        |
  | * step-color-temperature                                                            |
  | * step-hue                                                                          |
  | * step-saturation                                                                   |
  | * stop-move-step                                                                    |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

### doorlock

```bash
Usage:
  ./chip-tool doorlock command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * clear-all-pins                                                                    |
  | * clear-all-rfids                                                                   |
  | * clear-holiday-schedule                                                            |
  | * clear-pin                                                                         |
  | * clear-rfid                                                                        |
  | * clear-weekday-schedule                                                            |
  | * clear-yearday-schedule                                                            |
  | * get-holiday-schedule                                                              |
  | * get-pin                                                                           |
  | * get-rfid                                                                          |
  | * get-user-type                                                                     |
  | * get-weekday-schedule                                                              |
  | * get-yearday-schedule                                                              |
  | * lock-door                                                                         |
  | * set-holiday-schedule                                                              |
  | * set-pin                                                                           |
  | * set-rfid                                                                          |
  | * set-user-type                                                                     |
  | * set-weekday-schedule                                                              |
  | * set-yearday-schedule                                                              |
  | * unlock-door                                                                       |
  | * unlock-with-timeout                                                               |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

### groups

```bash
Usage:
  ./chip-tool groups command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-group                                                                         |
  | * add-group-if-identifying                                                          |
  | * get-group-membership                                                              |
  | * remove-all-groups                                                                 |
  | * remove-group                                                                      |
  | * view-group                                                                        |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

### iaszone

```bash
Usage:
  ./chip-tool iaszone command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * discover                                                                          |
  | * read                                                                              |
  | * write                                                                             |
  +-------------------------------------------------------------------------------------+
```

### identify

```bash
Usage:
  ./chip-tool identify command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * identify                                                                          |
  | * identify-query                                                                    |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

### levelcontrol

```bash
Usage:
  ./chip-tool levelcontrol command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * move                                                                              |
  | * move-to-level                                                                     |
  | * move-to-level-with-on-off                                                         |
  | * move-with-on-off                                                                  |
  | * step                                                                              |
  | * step-with-on-off                                                                  |
  | * stop                                                                              |
  | * stop-with-on-off                                                                  |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

### onoff

```bash
Usage:
  ./chip-tool onoff command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * off                                                                               |
  | * on                                                                                |
  | * toggle                                                                            |
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

### onoff off [endpoint-id]

Send the OFF command to the ONOFF cluster on the given endpoint.

### onoff on [endpoint-id]

Send the ON command to the ONOFF cluster on the given endpoint.

### onoff toggle [endpoint-id]

Send the TOGGLE command to the ONOFF cluster on the given endpoint.

### onoff discover [endpoint-id]

Send the DISCOVER command to the ONOFF cluster on the given endpoint.

### pairing

```bash
Usage:
  ./chip-tool pairing command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * unpair                                                                            |
  | * ble                                                                               |
  | * softap                                                                            |
  +-------------------------------------------------------------------------------------+
```

### payload

```bash
Usage:
  ./chip-tool payload command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * parse-setup-payload                                                               |
  | * parse-additional-data-payload                                                     |
  +-------------------------------------------------------------------------------------+
```

### scenes

```bash
Usage:
  ./chip-tool scenes command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * add-scene                                                                         |
  | * get-scene-membership                                                              |
  | * recall-scene                                                                      |
  | * remove-all-scenes                                                                 |
  | * remove-scene                                                                      |
  | * store-scene                                                                       |
  | * view-scene                                                                        |
  | * discover                                                                          |
  | * read                                                                              |
  +-------------------------------------------------------------------------------------+
```

### temperaturemeasurement

```bash
Usage:
  ./chip-tool temperaturemeasurement command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * discover                                                                          |
  | * read                                                                              |
  | * report                                                                            |
  +-------------------------------------------------------------------------------------+
```

To learn more about the tool, how to build it, use its commands and advanced
features, read the following guide:

-   [Working with the CHIP Tool](https://github.com/project-chip/connectedhomeip/tree/master/docs/guides/chip_tool_guide.md)
