# CHIP Client Example

An example application that uses CHIP to send messages to a CHIP server.

---

-   [CHIP Client Example](#chip-client-example)
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

## Using the Client to commission a device

In order to send commands to a device, it must be commissioned with the client.
chip-tool currently only supports commissioning and remembering one device at a
time. The configuration state is stored in `/tmp/chip_tool_config.ini`; deleting
this and other `.ini` files in `/tmp` can sometimes resolve issues due to stale
configuration.

#### Commission a device

To initiate a client commissioning request to a device, run the built executable
and choose the pairing mode.

##### Commission a device configured to bypass Rendezvous

The command below commissions a device with the provided IP address and port of
the server to talk to.

    $ chip-tool pairing bypass 192.168.0.30 5540

#### Commission a device over BLE

Run the built executable and pass it the discriminator and pairing code of the
remote device, as well as the network credentials to use.

The command below uses the default values hard-coded into the debug versions of
the ESP32 all-clusters-app to commission it onto a Wi-Fi network:

    $ chip-tool pairing ble-wifi ssid password 0 20202021 3840

where:

-   ssid is the Wi-Fi SSID either as a string, or in the form hex:XXXXXXXX where
    the bytes of the SSID are encoded as two-digit hex numbers.
-   paswword is the Wi-Fi password, again either as a string or as hex data
-   The 0 is the fabric id, until more complete support for multiple fabrics is
    implemented in our commissioning process.

For example:

    $ chip-tool pairing ble-wifi xyz secret 0 20202021 3840

or equivalently:

    $ chip-tool pairing ble-wifi hex:787980 hex:736563726574 0 20202021 3840

#### Pair a device over IP

The command below will pair to a localhost device (`::1`) running the
`all-clusters-app` on the default port (`5540`) over IPv6:

    $ chip-tool pairing onnetwork 0 20202021 3840 ::1 5540

### Forget the currently-commissioned device

    $ chip-tool pairing unpair

## Using the Client to Send CHIP Commands

To use the Client to send a CHIP commands, run the built executable and pass it
the target cluster name, the target command name as well as an endpoint id.

The endpoint id must be between 1 and 240.

    $ chip-tool onoff on 1

The client will send a single command packet and then exit.

### How to get the list of supported clusters

To get the list of supported clusters, run the built executable without any
arguments.

    $ chip-tool

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

    $ chip-tool onoff

### How to get the list of supported attributes for a specific cluster

To the the list of attributes for a specific cluster, run the built executable
with the target cluster name and the `read` command name.

    $ chip-tool onoff read

### How to get the list of parameters for a command

To get the list of parameters for a specific command, run the built executable
with the target cluster name and the target command name

    $ chip-tool onoff on

### Run a test suite against a paired peer device

    $ chip-tool tests Test_TC_OO_1_1

## Using the Client for Setup Payload

### How to parse a setup code

To parse a setup code, run the built executable with the `payload` cluster name
and the `parse-setup-payload` command

    $ chip-tool payload parse-setup-payload code

#### QR Code

    $ chip-tool payload parse-setup-payload "MT:#####"

#### QR Code with optional Vendor Info

    $ chip-tool payload parse-setup-payload "MT:#####"

#### Manual Setup Code

    $ chip-tool payload parse-setup-payload "#####"

# Using the Client for Additional Data Payload

To parse an additional data payload, run the built executable with the `payload`
cluster name and the `parse-additional-data-payload` command

    $ chip-tool payload parse-additional-data-payload "#####"

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
  | * bypass                                                                            |
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
