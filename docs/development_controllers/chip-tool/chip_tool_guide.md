# Working with the CHIP Tool

The CHIP Tool (`chip-tool`) is a Matter controller implementation that allows to
commission a Matter device into the network and to communicate with it using
Matter messages, which may encode Data Model actions, such as cluster commands.

The tool also provides other utilities specific to Matter, such as parsing of
the setup payload or performing discovery actions.

<hr>

## Table of Contents

-   [Installation](#installation)
-   [Building from source](#building-from-source)
-   [Running the CHIP Tool](#running-the-chip-tool)
-   [CHIP Tool modes](#chip-tool-modes)
    -   [Single-command mode (default)](#single-command-mode-default)
    -   [Interactive mode](#interactive-mode)
-   [Using CHIP Tool for Matter device testing](#using-chip-tool-for-matter-device-testing)
-   [Supported commands and options](#supported-commands-and-options)
    -   [List all supported clusters](#printing-all-supported-clusters)
    -   [List all supported command for a cluster](#getting-the-list-of-commands-supported-for-a-specific-cluster)
    -   [List all supported attributes for a cluster](#getting-the-list-of-attributes-supported-for-a-specific-cluster)
    -   [Command options](#getting-the-list-of-command-options)
-   [Testing and Interaction](#running-a-test-suite-against-a-paired-peer-device)
-   [Multi-admin scenario](#multi-admin-scenario)
-   [Subscribing to events or attributes](#subscribing-to-events-or-attributes)
-   [Using wildcards](#using-wildcards)
-   [Saving users and credentials for door lock device](#saving-users-and-credentials-on-door-lock-devices)

## Installation

On Linux distributions
[running snapd](https://snapcraft.io/docs/installing-snapd), such as Ubuntu, the
CHIP Tool can be installed using the
[chip-tool snap](https://snapcraft.io/chip-tool). To do this, run:

```
sudo snap install chip-tool
```

## Building from source

The source files of the CHIP Tool are available in the `examples/chip-tool`
directory.

The source can be compiled on Linux (amd64/aarch64) or macOS. If you want to run
it on Raspberry Pi, you must use a 64-bit OS.

> **Note:** To ensure compatibility, always build the CHIP Tool and the Matter
> device from the same revision of the `connectedhomeip` repository.

To build the CHIP Tool:

1. Install all required packages for Matter and prepare the source code and the
   build system. Read the [Building Matter](../../guides/BUILDING.md) guide for
   instructions.
2. Open a command prompt in the `connectedhomeip` directory.
3. Run the following command:

    ```
    ./scripts/examples/gn_build_example.sh examples/chip-tool BUILD_PATH
    ```

    In this command, `BUILD_PATH` specifies where the target binaries are to be
    placed.

## Running the CHIP Tool

If you installed the CHIP Tool as a snap, the command to run it would be:

```
$ chip-tool
```

If you compiled the CHIP Tool from source, it can be executed with the following
command from the `BUILD_PATH` directory:

```
$ ./chip-tool
```

As a result, the CHIP Tool starts in the default
[single-command mode](#single-command-mode-default) and prints all available
commands. These are called _clusters_ in this context, but not all listed
commands correspond to the _clusters_ in the Data Model (for example, pairing or
discover commands). Each listed command can however become the root of the new
more complex command by appending it with sub-commands. Examples of specific
commands and their use cases are described in the
[Supported commands and options](#supported-commands-and-options) section.

> **Note:** The CHIP Tool caches the configuration state in the
> `/tmp/chip_tool_config.ini` file. Deleting this and other `.ini` files in the
> `/tmp` directory can sometimes resolve issues related to stale configuration.
> To make the configuration persistent you can change the directory where CHIP
> Tool caches its configuration by using the command line option
> `--storage-directory`

<hr>

## CHIP Tool modes

The CHIP Tool can run in one of the following modes:

### Single-command mode (default)

In this mode, the CHIP Tool will exit with a timeout error if any single command
does not complete within a certain timeout period.

The timeout error will look similar to the following one:

```
[1650992689511] [32397:1415601] CHIP: [TOO] Run command failure: ../../../examples/chip-tool/commands/common/CHIPCommand.cpp:392: CHIP Error 0x00000032: Timeout
```

Moreover, when using the single-command mode, the CHIP Tool will establish a new
CASE session with every command sent.

#### Modifying timeout duration in single-command mode

This timeout can be modified for any command execution by supplying the optional
`--timeout` parameter, which takes a value in seconds, with the maximum being
65535 seconds.

**Example of command:**

```
$ ./chip-tool otasoftwareupdaterequestor subscribe-event state-transition 5 10 0x1234567890 0 --timeout 65535
```

### Interactive mode

In this mode, a command will terminate with an error if it does not complete
within the timeout period. However, the CHIP Tool will not be terminated and it
will not terminate processes that previous commands have started. Moreover, when
using the interactive mode, the CHIP Tool will establish a new CASE session only
when there is no session available yet. On the following commands, it will use
the existing session.

#### Starting the interactive mode

For commands such as event subscriptions that need to run for an extended period
of time, the CHIP Tool can be started in interactive mode first before running
the command.

To start the interactive mode, run the following command:

```
$ ./chip-tool interactive start
```

In this mode, you can subscribe to events or attributes. For detailed steps, see
[Subscribing to events or attributes](#subscribing-to-events-or-attributes).

<hr>

## Using CHIP Tool for Matter device testing

The following steps depend on the application clusters that you implemented on
the device.

The steps are using the
[Matter Lighting Application Example](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app)
with the Bluetooth LE commissioning method support. You can use other Matter
examples and still follow this procedure. If you use a different example, the
[Step 7](#step-7-control-application-data-model-clusters) may vary depending on
the clusters implemented in your application.

### Step 1: Prepare the Matter device

Build and program the device with the Matter device firmware by following the
[Matter Lighting Application Example](https://github.com/project-chip/connectedhomeip/tree/master/examples/lighting-app)
documentation.

### Step 2: Enable Bluetooth LE advertising on Matter device

Some examples are configured to advertise automatically on boot. Other examples
require physical trigger, for example pushing a button. Follow the documentation
of the Matter device example for the chosen platform to learn how Bluetooth LE
advertising is enabled for the given example.

### Step 3: Set up the IP network

To follow the next steps, the IP network must be up and running. For instance,
the Thread network can be established using
[OpenThread Border Router](https://openthread.io/codelabs/openthread-border-router#0).

### Step 4: Determine network pairing credentials

You must provide the CHIP Tool with network credentials that will be used in the
device commissioning procedure to configure the device with a network interface,
such as Thread or Wi-Fi.

The Matter specification does not define the preferred way of how the network
credentials are to be obtained by controller. In this guide, we are going to
provide steps for obtaining Thread and Wi-Fi network credentials.

#### Obtaining Thread network credentials

Fetch and store the current Active Operational Dataset from the Thread Border
Router. This step may vary depending on the Thread Border Router implementation.

If you are using
[OpenThread Border Router](https://openthread.io/codelabs/openthread-border-router#0)
(OTBR), retrieve this information using one of the following commands:

-   For OTBR running in Docker:

    ```
    sudo docker exec -it otbr sh -c "sudo ot-ctl dataset active -x"
    0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
    Done
    ```

-   For OTBR native installation:

    ```
    sudo ot-ctl dataset active -x
    0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8
    Done
    ```

For Thread, you might also use a different out-of-band method to fetch the
network credentials.

#### Obtaining Wi-Fi network credentials

You must get the following Wi-Fi network credentials to commission the Matter
device to the Wi-Fi network:

-   Wi-Fi SSID
-   Wi-Fi password

The steps required to determine the SSID and password may vary depending on the
setup. For instance, you might need to contact your local Wi-Fi network
administrator.

### Step 5: Determine Matter device's discriminator and setup PIN code

Matter uses the following values:

-   Discriminator - A 12-bit value used to discern between multiple
    commissionable device advertisements.
-   Setup PIN code - A 27-bit value used to authenticate the device.

You can find these values in the logging terminal of the device (for instance
UART) when the device boots up. For example:

```
I: 254 [DL]Device Configuration:
I: 257 [DL] Serial Number: TEST_SN
I: 260 [DL] Vendor Id: 65521 (0xFFF1)
I: 263 [DL] Product Id: 32768 (0x8000)
I: 267 [DL] Hardware Version: 1
I: 270 [DL] Setup Pin Code: 20202021
I: 273 [DL] Setup Discriminator: 3840 (0xF00)
I: 278 [DL] Manufacturing Date: (not set)
I: 281 [DL] Device Type: 65535 (0xFFFF)
```

In this printout, the discriminator is `3840 (0xF00)` and the setup PIN code is
`20202021`.

### Step 6: Commission Matter device into an existing IP network

Before communicating with the Matter device, first it must join an existing IP
network.

Matter devices can use different commissioning channels:

-   Devices that are not yet connected to the target IP network use Bluetooth LE
    as the commissioning channel.
-   Devices that have already joined an IP network only need to use the IP
    protocol for commissioning to the Matter network.

#### Commissioning over Bluetooth LE

In this case, your device can join the existing IP network over Bluetooth LE and
then be commissioned into a Matter network.

Different scenarios are available for Thread and Wi-Fi networks, as described in
the following subsections.

After connecting the device over Bluetooth LE, the controller prints the
following log:

```
Secure Session to Device Established
```

This log message means that the PASE (Password-Authenticated Session
Establishment) session using SPAKE2+ protocol has been established.

##### Commissioning into a Thread network over Bluetooth LE

To commission the device to the existing Thread network, use the following
command pattern:

```
$ ./chip-tool pairing ble-thread <node_id> hex:<operational_dataset> <pin_code> <discriminator>
```

In this command:

-   _<node_id\>_ is the user-defined ID of the node being commissioned.
-   _<operational_dataset\>_ is the Operational Dataset determined in the
    [step 4](#step-4-determine-network-pairing-credentials).
-   _<pin_code\>_ and _<discriminator\>_ are device-specific keys determined in
    the
    [step 5](#step-5-determine-matter-devices-discriminator-and-setup-pin-code).

##### Commissioning into a Wi-Fi network over Bluetooth LE

To commission the device to the existing Wi-Fi network, use the following
command pattern:

```
$ ./chip-tool pairing ble-wifi <node_id> <ssid> <password> <pin_code> <discriminator>
```

In this command:

-   _<node_id\>_ is the user-defined ID of the node being commissioned.
-   _<ssid\>_ and _<password\>_ are credentials determined in the step 3.
-   _<pin_code\>_ and _<discriminator\>_ are device-specific keys determined in
    the
    [step 5](#step-5-determine-matter-devices-discriminator-and-setup-pin-code).

If you prefer the hexadecimal format, use the `hex:` prefix. For example:

```
$ ./chip-tool pairing ble-wifi <node_id> hex:<ssid> hex:<password> <pin_code> <discriminator>
```

> **Note:** The _<node_id>_ can be provided as a hexadecimal value with the `0x`
> prefix.

#### Commissioning into a network over IP

This option is available when the Matter device is already present in an IP
network, but it has not been commissioned to a Matter network yet.

To commission the device, you can use either the setup PIN code or the setup PIN
code and the discriminator, both of which you obtained in the
[step 5](#step-5-determine-matter-devices-discriminator-and-setup-pin-code).
Alternatively, you can also use a QR code payload.

##### Commissioning with setup PIN code

To discover devices and try to pair with one of them using the provided setup
code, use the following command pattern:

```
$ ./chip-tool pairing onnetwork <node_id> <pin_code>
```

The command keeps trying devices until pairing with one of them succeeds or
until it runs out of pairing possibilities. In this command:

-   _<node_id\>_ is the user-defined ID of the node being commissioned.
-   _<pin_code\>_ is device specific _setup PIN code_ determined in the
    [step 5](#step-5-determine-matter-devices-discriminator-and-setup-pin-code)
    and used to discover the device.

##### Commissioning with long discriminator

To discover devices with a long discriminator and try to pair with one of them
using the provided setup code, use the following command pattern:

```
$ ./chip-tool pairing onnetwork-long <node_id> <pin_code> <discriminator>
```

The command keeps trying devices until pairing with one of them succeeds or
until it runs out of pairing possibilities. In this command:

-   _<node_id\>_ is the user-defined ID of the node being commissioned.
-   _<pin_code\>_ and _<discriminator\>_ are device specific keys determined in
    the
    [step 5](#step-5-determine-matter-devices-discriminator-and-setup-pin-code).

##### Commissioning with QR code payload or manual pairing code

Matter devices log the QR code payload and manual pairing code when they boot.

To discover devices based on the given QR code payload or manual pairing code
and try to pair with one of them, use the following command pattern:

```
$ ./chip-tool pairing code <node_id> <qrcode_payload-or-manual_code>
```

The command keeps trying devices until pairing with one of them succeeds or
until it runs out of pairing possibilities. In this command:

-   _<node_id\>_ is the user-defined ID of the node being commissioned.
-   _<qrcode_payload-or-manual_code\>_ is the QR code payload ID, for example
    `MT:Y.K9042C00KA0648G00`, or a manual pairing code like
    `749701123365521327694`.

#### Forgetting the already-commissioned device

In case commissioning needs to be retested, the following command removes the
device with the given node ID from the list of commissioned Matter devices:

```
$ ./chip-tool pairing unpair <node_id>
```

In this command, _<node_id\>_ is the user-defined ID of the node which is going
to be forgotten by the CHIP Tool.

### Step 7: Control application Data Model clusters

Having completed all previous steps, you have the Matter device successfully
commissioned to the network. You can now test the device by interacting with
Data Model clusters.

#### Example: Matter Lighting Application Example

In case of the Matter Lighting Application Example we referenced in step 1, the
application implements the On/Off and Level Control clusters. This means that
you can test it by toggling the bulb (using the `onoff` cluster commands) or by
manipulating its brightness (using the `levelcontrol` cluster commands):

-   Use the following command pattern to toggle the OnOff attribute state (e.g.
    visualized by the LED state):

    ```
    $ ./chip-tool onoff toggle <node_id> <endpoint_id>
    ```

    In this command:

    -   _<node_id\>_ is the user-defined ID of the commissioned node.
    -   _<endpoint_id\>_ is the ID of the endpoint with OnOff cluster
        implemented.

-   Use the following command pattern to change the value of the CurrentLevel
    attribute (e.g. visualized by the LED brightness):

    ```
    $ ./chip-tool levelcontrol move-to-level <level> <transition_time> <option_mask> <option_override> <node_id> <endpoint_id>
    ```

    In this command:

    -   _<level\>_ is the brightness level encoded between `0` and `254`, unless
        a custom range is configured in the cluster.
    -   _<transition_time\>_ is the transition time.
    -   _<option_mask\>_ is the option mask.
    -   _<option_override\>_ is the option override.
    -   _<node_id\>_ is the user-defined ID of the commissioned node.
    -   _<endpoint_id\>_ is the ID of the endpoint with LevelControl cluster
        implemented.

### Step 8: Read basic information from the Matter device

Every Matter device supports the Basic Information cluster, which maintains the
collection of attributes that a controller can obtain from a device. These
attributes can include the vendor name, the product name, or the software
version.

Use the CHIP Tool's `read` command on the `basicinformation` cluster to read
those values from the device:

```
$ ./chip-tool basicinformation read vendor-name <node_id> <endpoint_id>
$ ./chip-tool basicinformation read product-name <node_id> <endpoint_id>
$ ./chip-tool basicinformation read software-version <node_id> <endpoint_id>
```

In these commands:

-   _<node_id\>_ is the user-defined ID of the commissioned node.
-   _<endpoint_id\>_ is the ID of the endpoint with Basic Information cluster
    implemented.

You can also use the following command to list all available commands for the
Basic Information cluster:

```
$ ./chip-tool basicinformation
```

<hr>

## Supported commands and options

This section contains a general list of various CHIP Tool commands and options,
not limited to commissioning procedure and cluster interaction.

### Printing all supported clusters

To print all clusters supported by the CHIP Tool, run the following command:

```
$ ./chip-tool
```

**Example of output:**

```bash

[1647346057.900626][394605:394605] CHIP:TOO: Missing cluster name
Usage:
  ./chip-tool cluster_name command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Clusters:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * accesscontrol                                                                     |
  | * accountlogin                                                                      |
  | * administratorcommissioning                                                        |
  | * alarms                                                                            |
  | * any                                                                               |
  | * appliancecontrol                                                                  |
  | * applianceeventsandalert                                                           |
  | * applianceidentification                                                           |
  | * appliancestatistics                                                               |
  | * applicationbasic                                                                  |

```

<hr>

### Getting the list of commands supported for a specific cluster

To print the list of commands supported by a specific cluster, use the following
command pattern:

```
$ ./chip-tool <cluster_name>
```

In this command:

-   _<cluster_name\>_ is one of the available clusters (listed with
    `chip-tool`).

**Example of command:**

```
$ ./chip-tool onoff
```

**Example of output:**

```bash
[1647417645.182824][404411:404411] CHIP:TOO: Missing command name
Usage:
  ./chip-tool onoff command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * command-by-id                                                                     |
  | * off                                                                               |
  | * on                                                                                |
  | * toggle                                                                            |
  | * off-with-effect                                                                   |
  | * on-with-recall-global-scene                                                       |
  | * on-with-timed-off                                                                 |
  | * read-by-id                                                                        |
  | * read                                                                              |
  | * write-by-id                                                                       |
  | * write                                                                             |
  | * subscribe-by-id                                                                   |
  | * subscribe                                                                         |
  | * read-event-by-id                                                                  |
  | * subscribe-event-by-id                                                             |
  +-------------------------------------------------------------------------------------+
[1647417645.183836][404411:404411] CHIP:TOO: Run command failure: ../../examples/chip-tool/commands/common/Commands.cpp:84: Error 0x0000002F

```

<hr>

### Getting the list of attributes supported for a specific cluster

To get the list of attributes for a specific cluster, use the following command
pattern:

```
$ ./chip-tool <cluster_name> read
```

In this command:

-   _<cluster_name\>_ is one of the available clusters (listed with
    `chip-tool`).

**Example of command:**

```
$ ./chip-tool onoff read
```

**Example of output:**

```bash
[1647417857.913942][404444:404444] CHIP:TOO: Missing attribute name
Usage:
  ./chip-tool onoff read attribute-name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Attributes:                                                                         |
  +-------------------------------------------------------------------------------------+
  | * on-off                                                                            |
  | * global-scene-control                                                              |
  | * on-time                                                                           |
  | * off-wait-time                                                                     |
  | * start-up-on-off                                                                   |
  | * server-generated-command-list                                                     |
  | * client-generated-command-list                                                     |
  | * attribute-list                                                                    |
  | * feature-map                                                                       |
  | * cluster-revision                                                                  |
  +-------------------------------------------------------------------------------------+
[1647417857.914110][404444:404444] CHIP:TOO: Run command failure: ../../examples/chip-tool/commands/common/Commands.cpp:120: Error 0x0000002F
```

<hr>

### Getting the list of command options

To get the list of parameters for a specific command, use the following command
pattern:

```
$ ./chip-tool <cluster_name> <target_command>
```

In this command:

-   _<cluster_name\>_ is one of the available clusters (listed with
    `chip-tool`).
-   _<target_command\>_ is one of the target command names.

**Example of command:**

```
$ ./chip-tool onoff on
```

**Example of output:**

```bash
[1647417976.556313][404456:404456] CHIP:TOO: InitArgs: Wrong arguments number: 0 instead of 2
Usage:
  ./chip-tool onoff on node-id/group-id endpoint-id-ignored-for-group-commands [--paa-trust-store-path] [--commissioner-name] [--trace_file] [--trace_log] [--ble-adapter] [--timedInteractionTimeoutMs] [--suppressResponse]
[1647417976.556362][404456:404456] CHIP:TOO: Run command failure: ../../examples/chip-tool/commands/common/Commands.cpp:135: Error 0x0000002F

```

#### Selected command options

This section lists selected options that can be used to configure the input
command.

##### Choosing the Bluetooth adapter

To choose the Bluetooth adapter used by the CHIP Tool, use the following command
pattern:

```
--ble-adapter <id>
```

In this command:

-   _<id\>_ is the ID of HCI device.

**Example of usage:**

```
$ ./chip-tool pairing ble-thread 1 hex:0e080000000000010000000300001335060004001fffe002084fe76e9a8b5edaf50708fde46f999f0698e20510d47f5027a414ffeebaefa92285cc84fa030f4f70656e5468726561642d653439630102e49c0410b92f8c7fbb4f9f3e08492ee3915fbd2f0c0402a0fff8 20202021 3840 --ble-adapter 0
```

##### Using message tracing

Message tracing allows capturing CHIP Tool secure messages that can be used for
test automation. The tracing uses several types of flags that control where the
traces should go.

The following flags are available:

-   Trace file flag:

    ```
    --trace_file <filename>
    ```

    Here, _<filename\>_ is the name of the file where the trace data is stored.
    It can be appended to a command in the following way:

    ```
    $ ./chip-tool pairing <pairing_options> --trace_file <filename>
    ```

-   Trace log flag:

    ```
    --trace_log <onoff>
    ```

    Here, _<onoff\>_ is a `[0/1]` flag, which when set to `1` prints the trace
    data with automation logs to the console.

<hr>

##### Changing storage directory

By default, CHIP Tool stores its configuration into the `/tmp` directory. You
can change the storage directory by using the `--storage-directory` flag.

Usage:

```
--storage-directory <directory>
```

Here, _<directory\>_ is the path to the directory where the configuration is
stored.

**Example of usage:**

```
$ ./chip-tool pairing ble-wifi <node_id> <ssid> <password> <pin_code> <discriminator> --storage-directory <directory>
$ ./chip-tool temperaturemeasurement read measured-value <node_id> <endpoint_id> --storage-directory <directory>

```

<hr>

### Commissioner name and ID flags

All CHIP Tool commands can be used together with the following
commissioner-related flags:

-   `--commissioner-name`
-   `--commissioner-nodeid`

These flags let you control which fabric and node ID are used for the CHIP Tool
when interacting with devices. They are especially useful in scenarios where you
are working with more than one fabric, but you can also use them with a single
CHIP Tool node identity.

#### `--commissioner-name` flag

This flag lets you control which fabric is used by selecting a specific fabric
commissioner.

As per the CHIP Tool implementation, the commissioners are required to have the
following names: `alpha` for the first one, `beta` for the second one, `gamma`
for the third one, `4` for the fourth one, `5` for the fifth one, and so on.

If you don't use this flag, the CHIP Tool assumes that the command is meant for
the `alpha` commissioner and hence for the fabric associated with this
commissioner.

**Example of commands:**

```
$ ./chip-tool any subscribe-by-id '0x0028,0x0028,0x0101,0x0028,0x0028,0x0028' '5,6,0,1,2,4' 100 1000 1 '0,0,1,0,0,0' --keepSubscriptions true
$ ./chip-tool any subscribe-by-id '0x0028,0x0028,0x0101,0x0028,0x0028,0x0028' '5,6,0,1,2,4' 100 1000 2 '0,0,1,0,0,0' --keepSubscriptions true --commissioner-name beta
$ ./chip-tool any subscribe-by-id '0x0028,0x0028,0x0101,0x0028,0x0028,0x0028' '5,6,0,1,2,4' 100 1000 3 '0,0,1,0,0,0' --keepSubscriptions true --commissioner-name gamma
```

#### `--commissioner-nodeid` flag

This flag lets you select the node ID to use on the fabric specified with the
`--commissioner-name` flag.

If you don't use this flag, the CHIP Tool assumes that the command is sent with
the ID value that the CHIP Tool has in storage. If there's none, the CHIP Tool
sends the command with the default fallback node ID `112233`.

> **Note:** If the device has been already commissioned with a specific
> `--commissioner-nodeid`, you must always provide the `--commissioner-nodeid`
> flag with the CHIP Tool commands or update the Access Control List (ACL) on
> the device. Otherwise, the default fallback node ID `112233` is used and the
> communication will fail.

**Example of commands:**

```
$ ./chip-tool pairing code-thread 1 hex:000030000150208562618342348532605109bd31cda6908667addca8789211addac0102c4a9 34970112332 --commissioner-name alpha --commissioner-nodeid 999999
```

```
$ ./chip-tool basicinformation read vendor-id --commissioner-name alpha --commissioner-nodeid 999999
```

<hr>

### Running a test suite against a paired peer device

The CHIP Tool allows to run a set of tests, already compiled in the tool,
against a paired Matter device.

-   To get the list of available tests, run the following command:

    ```
    $ ./chip-tool tests
    ```

-   To execute a particular test against the paired device, use the following
    command pattern:

    ```
    $ ./chip-tool tests <test_name>
    ```

    In this command:

    -   _<test_name\>_ is the name of the particular test.

See the [Examples](#running-testclusters-test) section for an example of how to
run a test from the test suite.

<hr>

### Parsing the setup payload

The CHIP Tool offers a utility for parsing the Matter onboarding setup payload
and printing it in a readable form. For example, the payload can be printed on
the device console during boot.

To parse the setup code, use the `payload` command with the
`parse-setup-payload` sub-command, as in the following command pattern:

```
$ ./chip-tool payload parse-setup-payload <payload>
```

Here, _<payload\>_ is the ID of the payload to be parsed.

**Examples of commands:**

-   Setup QR code payload:

    ```
    $ ./chip-tool payload parse-setup-payload MT:6FCJ142C00KA0648G00
    ```

-   Manual pairing code:

    ```
    $ ./chip-tool payload parse-setup-payload 34970112332
    ```

<hr>

### Parsing additional data payload

To parse additional data payload, use the following command pattern:

```
$ ./chip-tool payload parse-additional-data-payload <payload>
```

In this command:

-   _<payload\>_ is the ID of the payload with additional data to be parsed.

<hr>

### Discover actions

The `discover` command can be used to resolve node ID and discover available
Matter devices.

Use the following command to print the available sub-commands of the `discover`
command:

```
$ ./chip-tool discover
```

#### Resolving node name

To resolve the DNS-SD name corresponding with the given Node ID and update the
address of the node in the device controller, use the following command pattern:

```
$ ./chip-tool discover resolve <node_id> <fabric_id>
```

In this command:

-   _<node_id\>_ is the ID of node to be resolved.
-   _<fabric_id\>_ is the ID of the Matter fabric to which the node belongs.

#### Discovering available Matter devices

To discover all Matter commissionables available in the operation area, run the
following command:

```
$ ./chip-tool discover commissionables
```

#### Discovering available Matter commissioners

To discover all Matter commissioners available in the operation area, run the
following command:

```
$ ./chip-tool discover commissioners
```

<hr>

### Pairing

The `pairing` command supports different methods for Matter device commissioning
procedure. The recommended methods are the following:

-   `code-thread` - For Thread commissioning.
-   `code-wifi` - For Wi-Fi commissioning.
-   `code` - For commissioning the device when it is already present in an IP
    network.

Alternatively, you can also use the following methods described in the
[Using CHIP Tool for Matter device testing](#using-chip-tool-for-matter-device-testing)
section:

-   `ble-thread` - For Thread commissioning; described under
    [Commissioning into a Thread network over Bluetooth LE](#commissioning-into-a-thread-network-over-bluetooth-le).
-   `ble-wifi` - For Wi-Fi commissioning; described under
    [Commissioning into a Wi-Fi network over Bluetooth LE](#commissioning-into-a-wi-fi-network-over-bluetooth-le)
-   `onnetwork` - For commissioning the device when it is already present in an
    IP network; described under
    [Commissioning with setup PIN code](#commissioning-with-setup-pin-code)

To list all `pairing` sub-commands and commissioning methods, run the following
command:

```
$ ./chip-tool pairing
```

**Example of commands:**

The following command commissions the Thread device with the node ID `1` to the
Matter fabric. The `hex:...` parameter is the operational dataset that contains
information about the Thread network to which the device is going to be
commissioned. The onboarding dataset payload `34970112332` (short manual pairing
code) is used to discover and commission the device.

```
$ ./chip-tool pairing code-thread 1 hex:000030000150208562618342348532605109bd31cda6908667addca8789211addac0102c4a9 34970112332
```

The following command commissions the Wi-Fi device with the node ID `1` to the
Matter fabric The SSID `wifi_test` and the password `admin123` is the required
information about the Wi-Fi network to which the device is going to be
commissioned. The onboarding dataset payload `34970112332` (short manual pairing
code) is used to discover and commission the device.

```
$ ./chip-tool pairing code-wifi 1 wifi_test admin123 34970112332
```

The following command commissions the device with the node ID `1` to the Matter
fabric. The onboarding dataset payload `MT:8IXS142C00KA0648G00` (QR code
payload) is used to discover and commission the device.

```
$ ./chip-tool pairing code 1 MT:8IXS142C00KA0648G00
```

#### Attestation-related flags

The `pairing` commissioning command can be run with several flags that allow you
to modify attestation-related settings:

-   `--paa-trust-store-path` - Use to provide the path to the directory that
    contains the information about Product Attestation Authority (PAA)
    certificates. The path can be absolute or relative to the current working
    directory. With this flag, the CHIP Tool looks for the PAA certificate that
    matches the PAI and the DAC certificates programmed on the device. Without
    this flag, the CHIP Tool uses the built-in test PAA certificate.

-   `--cd-trust-store-path` - Use to provide the path to the directory
    containing the key that is used to validate the device's Certification
    Declaration. The path can be absolute or relative to the current working
    directory. With this flag, the CHIP tool looks for additional public keys,
    in addition to the well-known built-in public keys (built-in public keys
    `src/credentials/attestation_verifier/DefaultDeviceAttestationVerifier.cpp`),
    to be used to validate Certification Declaration signatures.

*   `--only-allow-trusted-cd-keys` - Use to only allow the keys from
    `--cd-trust-store-path` and not the built-in test key. If the flag is not
    provided or it is provided with the value `false`, untrusted CD verifying
    keys are allowed. If it is provided with the value `true`
    (`--only-allow-trusted-cd-keys true`), test keys are disallowed and CD
    signed with the test key will not be accepted.

*   `--bypass-attestation-verifier` - Use to bypass the attestation verifier. If
    the flag is not provided or it is provided with the value `false`, the
    attestation verifier is not bypassed. If it is provided with the value
    `true` (`--bypass-attestation-verifier true`), the commissioning will
    continue in case of the attestation verification failure. The failure can be
    caused by errors in Certification Declaration, PAA or PAI certificates, or
    in the Device Attestation Certificate. This option can be helpful if you
    want to quickly commission a device with PAI and DAC certificates based on
    an unknown PAA and/or with a Certification Declaration signed by an unknown
    key.

<hr>

### Interacting with Data Model clusters

As mentioned in the
[Using CHIP Tool for Matter device testing](#using-chip-tool-for-matter-device-testing)
section, executing the `chip-tool` command with a particular cluster name lists
all operations supported for this cluster, as in the following command pattern:

```
$ ./chip-tool <cluster_name>
```

**Example of command:**

```
$ ./chip-tool binding
```

**Example of output:**

```bash
[1647502596.396184][411686:411686] CHIP:TOO: Missing command name
Usage:
  ./chip-tool binding command_name [param1 param2 ...]

  +-------------------------------------------------------------------------------------+
  | Commands:                                                                           |
  +-------------------------------------------------------------------------------------+
  | * command-by-id                                                                     |
  | * read-by-id                                                                        |
  | * read                                                                              |
  | * write-by-id                                                                       |
  | * write                                                                             |
  | * subscribe-by-id                                                                   |
  | * subscribe                                                                         |
  | * read-event-by-id                                                                  |
  | * subscribe-event-by-id                                                             |
  +-------------------------------------------------------------------------------------+
[1647502596.396299][411686:411686] CHIP:TOO: Run command failure: ../../examples/chip-tool/commands/common/Commands.cpp:84: Error 0x0000002F
```

According to this list, the `binding` cluster supports operations such as read
or write. Attributes from that cluster can also be subscribed by the controller,
which means that the CHIP Tool will receive notifications, for instance when the
attribute value is changed or a specific event happens.

#### Examples

This section lists examples of the CHIP Tool commands dedicated to specific use
cases.

##### Writing ACL to the `accesscontrol` cluster

The Access Control List concept allows to govern all Data Model interactions
(such as read attribute, write attribute, invoke command). For more information
about ACL, see
[Access Control Guide](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/access-control-guide.md).

To write ACL to the `accesscontrol` cluster, use the following command pattern:

```
$ ./chip-tool accesscontrol write acl <acl_data> <node_id> <endpoint_id>
```

In this command:

-   _<acl_data\>_ is the ACL data formatted as a JSON array.
-   _<node_id\>_ is the ID of the node that is going to receive ACL.
-   _<endpoint_id\>_ is the ID of the endpoint on which the `accesscontrol`
    cluster is implemented.

##### Adding a binding table to the `binding` cluster

Binding describes a relationship between the device that contains the binding
cluster and the end device. The proper ACL must be added to allow the end device
to receive commands from the bonded device. After the binding process, the
bonded device contains information about connected device, such as IPv6 address
and the route to the endpoint in the Matter network.

To add a binding table to the `binding` cluster, use the following command
pattern:

```
$ ./chip-tool binding write binding  <binding_data> <node_id> <endpoint_id>
```

In this command:

-   _<binding_data\>_ is the binding data formatted as a JSON array.
-   _<node_id\>_ is the ID of the node that is going to receive the binding.
-   _<endpoint_id\>_ is the ID of the endpoint on which the `binding` cluster is
    implemented.

##### Running `TestClusters` test

Complete the following steps to
[run one test from the test suite](#running-a-test-suite-against-a-paired-peer-device):

1. Clean the initialization of state using the following command:
    ```
    rm -fr /tmp/chip_*
    ```
1. In a shell window, start the DUT device:
    ```
    ./out/debug/standalone/chip-all-clusters-app
    ```
1. In a second shell window, pair the DUT with the CHIP Tool:
    ```
    ./out/debug/standalone/chip-tool pairing onnetwork 333221 20202021
    ```
1. Run the test with the following command:
    ```
    ./out/debug/standalone/chip-tool tests TestCluster --nodeId 333221
    ```

Read the [CHIP Test Suits](../../src/app/tests/suites/README.md) page for more
information about how the test suite is structured.

<hr>

### Multi-admin scenario

Multi-admin feature allows you to join Matter device to several Matter fabrics
and have several different Matter administrators administer it.

Complete the steps mentioned in the following sections.

#### Step 1: Commission to fabric

Commission the Matter device to first fabric following the
[Using CHIP Tool for Matter device testing](#using-chip-tool-for-matter-device-testing)
section.

#### Step 2: Open the commissioning window

Make sure the administrator from the first fabric opens the commissioning window
for a new administrator from another fabric.

Open the commissioning window on the paired Matter device by using the following
command pattern:

```
$ ./chip-tool pairing open-commissioning-window <node_id> <option> <window_timeout> <iteration> <discriminator>
```

In this command:

-   _<node_id\>_ is the ID of the node that should open commissioning window.
-   _<option\>_ is equal to 1 for Enhanced Commissioning Method and 0 for Basic
    Commissioning Method.
-   _<window_timeout\>_ is time in seconds, before the commissioning window
    closes.
-   _<iteration\>_ is number of PBKDF iterations to use to derive the PAKE
    verifier.
-   _<discriminator\>_ is device specific discriminator determined during
    commissioning.

> **Note:** The _<iteration\>_ and _<discriminator\>_ values are ignored if the
> _<option\>_ is set to 0.

**Example of command:**

```
$ ./chip-tool pairing open-commissioning-window 1 1 300 1000 2365
```

#### Step 3: Save the pairing code

Write down the manual pairing code or the QR code payload printed in the command
output, as it will be required by the second Matter admin to join Matter device
to its fabric.

**Example of output:**

```
[1663675289.149337][56387:56392] CHIP:DMG: Received Command Response Status for Endpoint=0 Cluster=0x0000_003C Command=0x0000_0000 Status=0x0
[1663675289.149356][56387:56392] CHIP:CTL: Successfully opened pairing window on the device
[1663675289.149409][56387:56392] CHIP:CTL: Manual pairing code: [36281602573]
[1663675289.149445][56387:56392] CHIP:CTL: SetupQRCode: [MT:4CT91AFN00YHEE7E300]
```

#### Step 4: Commission the Matter device to a new fabric

Complete the following steps:

1. Open another instance of the CHIP Tool.
1. In the new instance of the CHIP Tool, commission the Matter device to a new
   fabric by using the following command pattern:

    ```
    $ ./chip-tool pairing code <node_id> <payload> --commissioner-name <commissioner_name>
    ```

    In this command:

    - _<node_id\>_ is the user-defined ID of the node being commissioned. It
      doesn't need to be the same ID, as for the first fabric.
    - _<payload\>_ is the the QR code payload or a manual pairing code generated
      by the first commissioner instance when opened commissioning window
    - _<commissioner_name\>_ is the name of the second fabric. Valid values are
      "alpha", "beta", "gamma", and integers greater than or equal to 4. The
      default if not specified is "alpha".

    **Example of command:**

    ```
    $ ./chip-tool pairing code 1 36281602573 --commissioner-name beta
    ```

#### Step 5: Test reception of commands

After completing the above steps, the Matter device should be able to receive
and answer Matter commands sent in the second fabric. For example, you can use
the following command pattern to toggle the `OnOff` attribute state on a device
supporting `OnOff` cluster:

```
$ ./chip-tool onoff toggle <node_id> <endpoint_id> --commissioner-name <commissioner_name>
```

In this command:

-   _<node_id\>_ is the user-defined ID of the commissioned node.
-   _<endpoint_id\>_ is the ID of the endpoint with OnOff cluster implemented.
-   _<commissioner_name\>_ is the name of the second fabric. Valid values are
    "alpha", "beta", "gamma", and integers greater than or equal to 4. The
    default if not specified is "alpha".

**Example of command:**

```
$ ./chip-tool onoff toggle 1 1 --commissioner-name beta
```

<hr>

### Subscribing to events or attributes

Subscribing to an event or an attribute lets you mirror the state of the event
or the attribute as it changes in the Matter network. The list of events or
attributes you can subscribe to depends on the chosen cluster.

You can have more than one subscription at any given time and subscribe to more
than one attribute or event within one subscription (those attributes or events
can come from different clusters). However, you cannot subscribe to both
attributes and events as part of a single subscription. In other words, each
subscription must be dedicated exclusively to either attributes or events.

For more information about subscriptions, see the Matter specification at
chapter 8, section 5 (Subscribe Interaction).

> **Note:** The subscription behavior will be different if you set the
> subscription to be sent with the parameter `isUrgent` set to `True`. See the
> Matter specification for more information.

#### Subscribing to an attribute

The following procedure will use the `doorlock` cluster as an example. Complete
the following steps:

1. Start the CHIP Tool in [interactive mode](#interactive-mode) by running the
   following command:

    ```
    $ ./chiptool interactive start
    ```

    All of the commands that follow will be executed in the interactive mode
    (`>>>`).

1. Run the following command to display all the available attributes you can
   subscribe to for the given `<cluster-name>`:

    ```
    >>> <cluster-name> subscribe
    ```

    The list of all available attributes for the cluster will appears.

    > **Note:** Your accessory might not support all of these attributes. You
    > will get an error if the controller sends an unsupported attribute.

    For example, for the door lock cluster:

    ```bash
    >>> doorlock subscribe
    ```

    The following list will appear:

    ```
    +-------------------------------------------------------------------------------------+
    | Attributes:                                                                         |
    +-------------------------------------------------------------------------------------+
    | * lock-state                                                                        |
    | * lock-type                                                                         |
    | * actuator-enabled                                                                  |
    | * door-state                                                                        |
    | * door-open-events                                                                  |
    | * door-closed-events                                                                |
    | * open-period                                                                       |
    | * number-of-total-users-supported                                                   |
    | * number-of-pinusers-supported                                                      |
    | * number-of-rfidusers-supported                                                     |
    | * number-of-week-day-schedules-supported-per-user                                   |
    | * number-of-year-day-schedules-supported-per-user                                   |
    | * number-of-holiday-schedules-supported                                             |
    | * max-pincode-length                                                                |
    | * min-pincode-length                                                                |
    | * max-rfidcode-length                                                               |
    | * min-rfidcode-length                                                               |
    | * credential-rules-support                                                          |
    | * number-of-credentials-supported-per-user                                          |
    | * language                                                                          |
    | * ledsettings                                                                       |
    | * auto-relock-time                                                                  |
    | * sound-volume                                                                      |
    | * operating-mode                                                                    |
    | * supported-operating-modes                                                         |
    | * default-configuration-register                                                    |
    | * enable-local-programming                                                          |
    | * enable-one-touch-locking                                                          |
    | * enable-inside-status-led                                                          |
    | * enable-privacy-mode-button                                                        |
    | * local-programming-features                                                        |
    | * wrong-code-entry-limit                                                            |
    | * user-code-temporary-disable-time                                                  |
    | * send-pinover-the-air                                                              |
    | * require-pinfor-remote-operation                                                   |
    | * expiring-user-timeout                                                             |
    | * generated-command-list                                                            |
    | * accepted-command-list                                                             |
    | * event-list                                                                        |
    | * attribute-list                                                                    |
    | * feature-map                                                                       |
    | * cluster-revision                                                                  |
    +-------------------------------------------------------------------------------------+
    ```

1. Add the argument of your choice to the subscription command, using the
   following pattern:

    ```bash
    >>> <cluster-name> subscribe <argument> <min-interval> <max-interval> <node_id> <endpoint_id>
    ```

    In this command:

    - _<cluster-name\>_ is the name of the cluster.
    - _<argument\>_ is the name of the chosen argument.
    - _<min-interval\>_ specifies the minimum number of seconds that must elapse
      since the last report for the server to send a new report.
    - _<max-interval\>_ specifies the number of seconds that must elapse since
      the last report for the server to send a new report.
    - _<node-id\>_ is the user-defined ID of the commissioned node.
    - _<endpoint_id\>_ is the ID of the endpoint where the chosen cluster is
      implemented.

    For example:

    ```bash
    >>> doorlock subscribe lock-state 5 10 1 1
    ```

After this command is run, the CHIP Tool will check the state of the door lock
every time it changes (for example, as a result of a button press or an external
ecosystem action) and update it in its own records.

#### Subscribing to an event

The procedure for subscribing to an event is similar to
[subscribing to an attribute](#subscribing-to-an-attribute).

The following procedure will use the `doorlock` cluster as an example. Complete
the following steps:

1. Start the CHIP Tool in [interactive mode](#interactive-mode) by running the
   following command:

    ```
    $ ./chiptool interactive start
    ```

    All of the commands that follow will be executed in the interactive mode
    (`>>>`).

1. Run the following command to display all the available events you can
   subscribe to for the given `<cluster-name>`:

    ```
    >>> <cluster-name> subscribe-event
    ```

    The list of all available events for the cluster will appears.

    > **Note:** Your accessory might not support all of these events. You will
    > get an error if the controller sends an unsupported event.

    For example, for the door lock cluster:

    ```bash
    >>> doorlock subscribe-event
    ```

    The following list will appear:

    ```
    +-------------------------------------------------------------------------------------+
    | Events:                                                                             |
    +-------------------------------------------------------------------------------------+
    | * door-lock-alarm                                                                   |
    | * door-state-change                                                                 |
    | * lock-operation                                                                    |
    | * lock-operation-error                                                              |
    | * lock-user-change                                                                  |
    +-------------------------------------------------------------------------------------+
    ```

1. Add the event of your choice to the subscription command, using the following
   pattern:

    ```bash
    >>> <cluster-name> subscribe-event <event-name> <min-interval> <max-interval> <node_id> <endpoint_id>
    ```

    In this command:

    - _<cluster-name\>_ is the name of the cluster.
    - _<event-name\>_ is the name of the chosen event.
    - _<min-interval\>_ specifies the minimum number of seconds that must elapse
      since the last report for the server to send a new report.
    - _<max-interval\>_ specifies the number of seconds that must elapse since
      the last report for the server to send a new report.
    - _<node_id\>_ is the user-defined ID of the commissioned node.
    - _<endpoint_id\>_ is the ID of the endpoint where the chosen cluster is
      implemented.

    For example:

    ```bash
    >>> doorlock subscribe-event door-lock-alarm 5 10 1 1
    ```

After this command is run, the CHIP Tool will check the state of the door lock
every time it changes (for example, as a result of a button press or an external
ecosystem action) and update it in its own records.

#### Subscribing using attribute ID or event ID

You can also use the following commands instead of `subscribe` to subscribe
using the attribute ID or the event ID:

-   `subscribe-by-id`
-   `subscribe-event-by-id`

The steps are the same as for the `subscribe` or `subscribe-event` commands.

<hr>

### Using wildcards

The CHIP Tool supports command wildcards for parameter values for clusters,
attributes or events, or endpoints, or any combination of these. With the
wildcards, you can for example read all attributes for the cluster `0x101` on a
specific endpoint with a specific node ID on all devices in the Matter network.
This allows you to parse and gather cluster information faster and more
efficiently.

The following wildcards are available:

-   For all attributes: `0xFFFFFFFF`
-   For all clusters: `0xFFFFFFFF`
-   For all endpoints: `0xFFFF`

You can combine these wildcards within a single command. Wildcards can be used
in both [single-command](#single-command-mode-default) and
[interactive](#interactive-mode) modes.

You can use the following command pattern:

```
$ ./chip-tool <cluster_name> <command> <attribute_event_name> <node_id> <endpoint_id>
```

In this command:

-   _<cluster-name\>_ is the name of the cluster.
-   _<command\>_ is the name of the command supported by wildcards:

    ```
    +-------------------------------------------------------------------------------------+
    | Commands:                                                                           |
    +-------------------------------------------------------------------------------------+
    | * read                                                                              |
    | * read-by-id                                                                        |
    | * subscribe                                                                         |
    | * subscribe-by-id                                                                   |
    +-------------------------------------------------------------------------------------+
    ```

-   _<attribute_event_name\>_ is the name of the chosen attribute or event.
-   _<node_id\>_ is the user-defined ID of the commissioned node.
-   _<endpoint_id\>_ is the ID of the endpoint where the chosen cluster is
    implemented.

**Examples of commands:**

-   To read all attributes (wildcard `0xFFFFFFFF`) from the cluster `doorlock`
    for the node with ID `1` and on the endpoint `1`, run the following command:

    ```
    $ ./chip-tool doorlock read-by-id 0xFFFFFFFF 1 1
    ```

-   To read the `lock-state` attribute from the cluster `doorlock` for the node
    with ID `1` and on all endpoints (wildcard `0xFFFF`), run the following
    command:

    ```
    $ ./chip-tool doorlock read lock-state 1 0xFFFF
    ```

-   To read all attributes (wildcard `0xFFFFFFFF`) from the cluster `doorlock`
    for the node with ID `1` and on all endpoints (wildcard `0xFFFF`), run the
    following command:

    ```
    $ ./chip-tool doorlock read-by-id 0xFFFFFFFF 1 0xFFFF
    ```

#### Using wildcards with `any` command

Using the `any` command lets you use wildcards also for the cluster names. The
`any` command can be combined with the following commands:

```
+-------------------------------------------------------------------------------------+
| Commands:                                                                           |
+-------------------------------------------------------------------------------------+
| * command-by-id                                                                     |
| * read-by-id                                                                        |
| * write-by-id                                                                       |
| * subscribe-by-id                                                                   |
| * read-event-by-id                                                                  |
| * subscribe-event-by-id                                                             |
| * read-all                                                                          |
| * subscribe-all                                                                     |
+-------------------------------------------------------------------------------------+
```

As a result, you can use the following command pattern:

```
$ ./chip-tool any <command_name> [parameters of the <command_name>]
```

In this command:

-   _<command_name\>_ is one of the commands supported for the `any` command, as
    listed above.
-   _[parameters of the <command_name\>]_ are the parameters required by
    _<command_name\>_. You can check them by running the command without any
    parameters.

**Example of command pattern for `read-by-id`:**

```
$ ./chip-tool any read-by-id <cluster-ids> <attribute-ids> <destination-id> <endpoint-ids>
```

**Examples of commands:**

-   To read the `0x0` attribute (`lock state`) on the cluster `0x101`
    (`doorlock`) for the node with ID `1` and on the endpoint `1`, run the
    following command:

    ```
    $ ./chip-tool any read-by-id 0x101 0x0 1 1
    ```

-   To read all attributes (wildcard `0xFFFFFFFF`) from the cluster `0x101`
    (`doorlock`) for the node with ID `1` and on the endpoint `1`, run the
    following command:

    ```
    $ ./chip-tool any read-by-id 0x101 0xFFFFFFFF 1 1
    ```

-   To read all attributes (wildcard `0xFFFFFFFF`) on all clusters (wildcard
    `0xFFFFFFFF`) for the node with ID `1` and on the endpoint `1`, run the
    following command:

    ```
    ./chip-tool any read-by-id 0xFFFFFFFF 0xFFFFFFFF 1 1
    ```

-   To read all attributes (wildcard `0xFFFFFFFF`) on all clusters (wildcard
    `0xFFFFFFFF`) for the node with ID `1` and on all endpoints (wildcard
    `0xFFFF`), run the following command:

    ```
    ./chip-tool any read-by-id 0xFFFFFFFF 0xFFFFFFFF 1 0xFFFF
    ```

<hr>

## Saving users and credentials on door lock devices

Matter door lock devices can store pools of users and credentials that allow you
to configure different access scenarios. Each user and credential in the pool
has an index value. Additionally, each user has a list of Occupied Credentials.

By default, each door lock device comes with no user or credential defined, but
it reserves several slots in both pools that can be populated with new users and
credentials, up to the value specified in the the `NumberOfTotalUsersSupported`
attribute and the `NumberOfCredentialsSupportedPerUser` attribute, respectively.

All communication between users and credentials happens only using their
respective index values. No other information is shared between both pools.

The CHIP Tool lets you add users and credentials on door lock devices and
securely match their indexes to one another. This is an optional feature
available only when working with devices that implement the `doorlock` cluster.

> **Note:** Users and credentials can only be modified by whoever has the right
> permissions, as specified in the Access Control List.

To save credentials and users, you need to complete the following steps,
described in detail in the following sections:

1. Set up a user on the device.
1. Assign a credential for the newly created user.

### Step 1: Set up a user

To set up a user on a door lock device with the CHIP Tool, use the following
command pattern:

```
$ ./chip-tool doorlock set-user <operation-type> <user-index> <user-name> <user-unique-id> <user-status> <user-type> <credential-rule> <destination-id> <endpoint-id> --timedInteractionTimeoutMs <ms_value>
```

In this command:

-   _<operation-type\>_ is one of the available types of operation for the user:

    -   `Add` - This operation sets a new user in the slot at _<user-index\>_.
    -   `Clear` - This operation removes an existing user from the slot at
        _<user-index\>_.
    -   `Modify` - This operation modifies an existing user at the slot at
        _<user-index\>_.

-   _<user-index\>_ is the index value of the user, between `1` and the value of
    the `NumberOfTotalUsersSupported` attribute. Setting the user index to `0`
    will cause an error.
-   _<user-name\>_ is the name of the user, which can have maximum 10 bytes of
    size. Can be set to `null`.
-   _<user-unique-id\>_ is a 4-byte number that describes the unique user ID.
    Can be set to `null`.
-   _<user-status\>_ can be set to `null` or to one of the following values:

    -   `1` (`OccupiedEnabled`) - This status indicates that the given user slot
        is used and active.
    -   `3` (`OccupiedDisabled`) - This status indicates that the given user
        slot is used, but disabled. Unlike `0` and `1`, supporting this status
        is optional.

-   _<user-type\>_ is the type of the user, which can have one of the values
    specified in the Matter Application Clusters specification for the
    `doorlock` cluster (see section "5.2.9.16. `UserTypeEnum`"). Can be set to
    `null`.
-   _<credential-rule\>_ is the number of credentials that must be used to
    unlock the door lock. This parameter can be set to `null` or to one of the
    following values:

    -   `0` (Single) - One credential type is required to unlock.
    -   `1` (Dual) - Two credential types are required to unlock.
    -   `2` (Triple) - Three credential types are required to unlock.

-   _<destination-id\>_ is the node ID of the door lock device.
-   _<endpoint-id\>_ is the ID of the endpoint on the door lock device.
-   `--timedInteractionTimeoutMs` is the duration in milliseconds (_<ms_value>_)
    of the time window for receiving a request on the server side. This should
    allow enough time for receiving the request.

**Examples of command:**

The following command runs the `set-user` command that adds (`0`) a user at the
index `1`; the user has the name `AAA` and the unique ID `6452`. The user's
status is set to `OccupiedEnabled` (`1`), the user type is set to
`UnrestrictedUser` (`0`), the credential rule is set to single (`0`), the
targeted node ID of the destination door lock device is `1` and the targeted
`doorlock` cluster's endpoint ID on that device is `1`. The
`--timedInteractionTimeoutMs` has a custom value.

```
$ ./chip-tool doorlock set-user 0 1 AAA 6452 1 0 0 1 1 --timedInteractionTimeoutMs 1000
```

The following command mirrors the action of the command above, but it targets an
empty user name (`null`) and has `null` for the unique ID. The user status
defaults to `OccupiedEnabled`, the user type defaults to `UnrestrictedUser`, and
the credential rule defaults to single.

```
$ ./chip-tool doorlock set-user 0 1 null null null null null 1 1 --timedInteractionTimeoutMs 1000
```

For more use cases for this command, see the "5.2.7.34. Set User Command"
section in the Matter Application Clusters specification.

### Step 2: Assign a credential

Once you have a user created on the door lock device, use the following command
pattern to assign a credential to it:

```
$ ./chip-tool doorlock set-credential <operation-type> <{Credential}> <credential-data> <user-index> <user-status> <user-type> <destination-id> <endpoint-id> --timedInteractionTimeoutMs <ms_value>
```

In this command:

-   _<operation-type\>_ is one of the available types of operation for the
    credential:

    -   `Add` - This operation adds a new credential to a user at the slot at
        _<user-index\>_.
    -   `Clear` - This operation removes an existing credential from the user at
        the slot at _<user-index\>_.
    -   `Modify` - This operation modifies an existing credential for the user
        at the slot at _<user-index\>_.

-   _<{Credential}\>_ is a JSON object, with the following two fields:

    -   `"credentialType"` is the key field for the type of the credential. It
        can have one of the following values:

        -   `0` - Programming PIN
        -   `1` - PIN
        -   `2` - RFID
        -   `3` - Fingerprint
        -   `4` - Finger vein

    -   `"credentialIndex"` is the key field for the index of the credential. If
        `"credentialType"` is not "Programming PIN", `"credentialIndex"` must be
        between `1` and the value of the `NumberOfCredentialsSupportedPerUser`
        attribute (see the section 5.2.3.20 of the Matter Application Clusters
        specification for details). `0` is required for the Programming PIN. In
        other cases, setting the credential index to `0` will cause an error.

-   _<credential-data\>_ is an octet string parameter with the secret credential
    data. For example, the PIN code value (`12345` in the example below).
-   _<user-index\>_ is the index of the user that will be associated with the
    credential. Can be set to `null` to create a new user.
-   _<user-status\>_ is the status of the user that will be associated with the
    credential. See the description of this parameter in
    [Set up a user](#step-1-set-up-a-user). Can be set to `null`.
-   _<user-type\>_ is the type of the user, which can have one of the values
    specified in the Matter Application Clusters specification for the
    `doorlock` cluster (see section "5.2.9.16. `UserTypeEnum`"). Can be set to
    `null`.
-   _<destination-id\>_ is the node ID of the door lock device.
-   _<endpoint-id\>_ is the ID of the endpoint on the door lock device.
-   `--timedInteractionTimeoutMs` is the duration in milliseconds (_<ms_value>_)
    of the time window for receiving a request on the server side. This should
    allow enough time for receiving the request.

**Example of command:**

The following command runs the `set-credential` command that adds (`0`) a PIN
credential (type `1`) at the index `1`. The credential data is set to `12345`
(PIN code value). This credential is associated with the user at the index `1`.
The `null` parameters for the user status and the user type indicate that the
credentials are added to an existing user. The targeted node ID of the
destination door lock device is `1` and the targeted `doorlock` cluster's
endpoint ID on that device is `1`. The `--timedInteractionTimeoutMs` has a
custom value.

```
$ ./chip-tool doorlock set-credential 0 '{ "credentialType": 1, "credentialIndex": 1 }' "12345" 1 null null 1 1 --timedInteractionTimeoutMs 1000
```

For more use cases for this command, see the "5.2.7.40. Set Credential Command"
section in the Matter Application Clusters specification.

### Operations on users and credentials

After you set up users and credentials on your door lock device, you can use
several CHIP Tool commands to interact with them.

All commands reuse the parameters explained earlier in this section. The
following command patterns are available:

-   Reading the status of the user:

    ```
    $ ./chip-tool doorlock get-user <user-index> <destination-id> <endpoint-id>
    ```

    This command returns the status of the user at the specified _<user-index\>_
    at the specified _<destination-id\>_ and _<endpoint-id\>_.

-   Reading the status of the credential:

    ```
    $ ./chip-tool doorlock get-credential-status <{Credential}> <destination-id> <endpoint-id>
    ```

    This command returns the status of the credential of the specified
    _<{Credential}\>_ at the specified _<destination-id\>_ and _<endpoint-id\>_.

-   Cleaning the user:

    ```
    $ ./chip-tool doorlock clear-user <user-index> <destination-id> <endpoint-id> --timedInteractionTimeoutMs <ms_value>
    ```

    This command cleans the slot containing the specified _<user-index\>_ at the
    specified _<destination-id\>_ and _<endpoint-id\>_.

-   Cleaning the credential:

    ```
    $ ./chip-tool doorlock clear-credential <{Credential}> <destination-id> <endpoint-id> --timedInteractionTimeoutMs <ms_value>
    ```

    This command cleans the slot containing the specified _<{Credential}\>_ at
    the specified _<destination-id\>_ and _<endpoint-id\>_.

### Operations with user PIN code

If you set the _<credential-type\>_ to PIN when
[assigning credentials](#step-2-assign-a-credential), you can use the following
command patterns to verify if it works and invoke it to open or close the door
lock:

-   Verifying the PIN code:

    ```
    $ ./chip-tool doorlock read require-pinfor-remote-operation <destination-id> <endpoint-id>
    ```

    This command returns either `false` or `true`:

    -   `false` indicates that providing the PIN code is not required to close
        or open the door lock.
    -   `true` indicates that the PIN code is required to close or open the door
        lock.

-   Changing the requirement for the PIN code usage:

    ```
    $ ./chip-tool doorlock write require-pinfor-remote-operation true <destination-id> <endpoint-id>
    ```

    This command modifies the setting of `require-pinfor-remote-operation` to
    `true`. After you run it, you will have to use the PIN code to lock or
    unlock the door.

-   Closing the door lock with the PIN code:

    ```
    $ ./chip-tool doorlock lock-door <destination-id> <endpoint-id> --timedInteractionTimeoutMs <ms_value> --PinCode 12345
    ```

    In this command, you need to provide `--PinCode` corresponding to the PIN
    code you set with _<credential-data\>_ (for example `12345`).

-   Opening the door lock with the PIN code:

    ```
    $ ./chip-tool doorlock unlock-door <destination-id> <endpoint-id> --timedInteractionTimeoutMs <ms_value> --PinCode 12345
    ```

    In this command, you need to provide `--PinCode` corresponding to the PIN
    code you set with _<credential-data\>_ (for example `12345`).
