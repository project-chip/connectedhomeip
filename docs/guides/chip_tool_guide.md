# Working with the CHIP Tool

The CHIP Tool (`chip-tool`) is a Matter controller implementation that allows to
commission a Matter device into the network and to communicate with it using
Matter messages, which may encode Data Model actions, such as cluster commands.

The tool also provides other utilities specific to Matter, such as parsing of
the setup payload or performing discovery actions.

<hr>

## Source files

You can find source files of the CHIP Tool in the `examples/chip-tool`
directory.

> **Note:** The CHIP Tool caches the configuration state in the
> `/tmp/chip_tool_config.ini` file. Deleting this and other `.ini` files in the
> `/tmp` directory can sometimes resolve issues related to stale configuration.

<hr>

## Building and running the CHIP Tool

Before you can use the CHIP Tool, you must compile it from source on Linux
(amd64/aarch64) or macOS. If you want to run it on Raspberry Pi, it must use a
64-bit OS.

> **Note:** To ensure compatibility, always build the CHIP Tool and the Matter
> device from the same revision of the `connectedhomeip` repository.

### Building the CHIP Tool

To build and run the CHIP Tool:

1. Install all required packages for Matter and prepare the source code and the
   build system. Read the [Building Matter](BUILDING.md) guide for instructions.
2. Open a command prompt in the `connectedhomeip` directory.
3. Run the following command:

    ```
    ./scripts/examples/gn_build_example.sh examples/chip-tool BUILD_PATH
    ```

    In this command, `BUILD_PATH` specifies where the target binaries are to be
    placed.

### Running the CHIP Tool

To check if the CHIP Tool runs correctly, execute the following command from the
`BUILD_PATH` directory:

```
$ ./chip-tool
```

As a result, the CHIP Tool prints all available commands. These are called
_clusters_ in this context, but not all listed commands correspond to the
_clusters_ in the Data Model (for example, pairing or discover commands). Each
listed command can however become the root of the new more complex command by
appending it with sub-commands. Examples of specific commands and their use
cases are described in the
[Supported commands and options](#supported-commands-and-options) section.

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

Every Matter device supports the Basic cluster, which maintains the collection
of attributes that a controller can obtain from a device. These attributes can
include the vendor name, the product name, or the software version.

Use the CHIP Tool's `read` command on the `basic` cluster to read those values
from the device:

```
$ ./chip-tool basic read vendor-name <node_id> <endpoint_id>
$ ./chip-tool basic read product-name <node_id> <endpoint_id>
$ ./chip-tool basic read software-version <node_id> <endpoint_id>
```

In these commands:

-   _<node_id\>_ is the user-defined ID of the commissioned node.
-   _<endpoint_id\>_ is the ID of the endpoint with Basic cluster implemented.

You can also use the following command to list all available commands for Basic
cluster:

```
$ ./chip-tool basic
```

<hr>

## Supported commands and options

This section contains a general list of various CHIP Tool commands and options,
not limited to commissioning procedure and cluster interaction.

### Interactive mode versus single-command mode

The CHIP Tool can run in one of the following modes:

-   Single-command mode (default) - In this mode, the CHIP Tool will exit with a
    timeout error if any single command does not complete within a certain
    timeout period.

    The timeout error will look similar to the following one:

    ```
    [1650992689511] [32397:1415601] CHIP: [TOO] Run command failure: ../../../examples/chip-tool/commands/common/CHIPCommand.cpp:392: CHIP Error 0x00000032: Timeout
    ```

    Moreover, when using the single-command mode, the CHIP Tool will establish a
    new CASE session with every command sent.

-   Interactive mode - In this mode, a command will terminate with an error if
    it does not complete within the timeout period. However, the CHIP Tool will
    not be terminated and it will not terminate processes that previous commands
    have started. Moreover, when using the interactive mode, the CHIP Tool will
    establish a new CASE session only when there is no session available yet. On
    the following commands, it will use the existing session.

#### Modifying timeout duration in single-command mode

This timeout can be modified for any command execution by supplying the optional
`--timeout` parameter, which takes a value in seconds, with the maximum being
65535 seconds.

**Example of command:**

```
$ ./chip-tool otasoftwareupdaterequestor subscribe-event state-transition 5 10 0x1234567890 0 --timeout 65535
```

#### Starting the interactive mode

For commands such as event subscriptions that need to run for an extended period
of time, the CHIP Tool can be started in interactive mode first before running
the command.

**Example of command:**

```
$ ./chip-tool interactive start
otasoftwareupdaterequestor subscribe-event state-transition 5 10 ${NODE_ID} 0
```

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

### Parsing additional data payload

To parse additional data payload, use the following command pattern:

```
$ ./chip-tool parse-additional-data-payload <payload>
```

In this command:

-   _<payload\>_ is the ID of the payload with additional data to be parsed.

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

### Pairing

The `pairing` command supports different means regarding Matter device
commissioning procedure.

Thread and Wi-Fi commissioning use cases are described in the
[Using the CHIP Tool for Matter device testing](#using-chip-tool-for-matter-device-testing)
section.

To list all `pairing` sub-commands, run the following command:

```
$ ./chip-tool pairing
```

### Interacting with Data Model clusters

As mentioned in the
[Using the CHIP Tool for Matter device testing](#using-chip-tool-for-matter-device-testing)
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

The Access Control List (ACL) concept allows to govern all Data Model
interactions (such as read attribute, write attribute, invoke command). For more
information about ACL, see
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
    $ ./chip-tool pairing code <payload> <node_id> --commissioner-name <commissioner_name>
    ```

    In this command:

    - _<payload\>_ is the the QR code payload or a manual pairing code generated
      by the first commissioner instance when opened commissioning window
    - _<node_id\>_ is the user-defined ID of the node being commissioned. It
      doesn't need to be the same ID, as for the first fabric.
    - _<commissioner_name\>_ is the name of the second fabric. Valid values are
      "alpha", "beta", "gamma", and integers greater than or equal to 4. The
      default if not specified is "alpha".

    **Example of command:**

    ```
    $ ./chip-tool pairing code 36281602573 1 --commissioner-name beta
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
