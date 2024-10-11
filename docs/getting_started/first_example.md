# An SDK example

The SDK provides a number of example devices and controllers that can be used to
familiarize yourself with the SDK and the Matter ecosystem.

## Example Devices

The example devices (occasionally referred to as "apps") are located in the
[examples](https://github.com/project-chip/connectedhomeip/tree/master/examples)
directory. The examples often implement one particular device type. Some have
implementations for various platforms.

<!--- Relative path link to "examples" above doesn't work. See #35855 --->

The linux platform examples are provided as examples, and are used in the CI.
These can be used for preliminary testing.

The all-clusters-app is used by the QA team for testing. This app implements
nearly all the available clusters and does not conform to a specific device
type. This app is not a good starting place for product development.

## Example Controllers

The SDK has two example controllers that can be used to interact with devices
for testing.

[chip-tool](../../examples/chip-tool/) is a C++ command line controller with an
interactive shell. More information on chip-tool can be found in the
[chip-tool guide](../guides/chip_tool_guide.md).

[chip-repl](../../src/controller/python/chip-repl.py) is a shell for the python
controller. The chip-repl is part of the python controller framework, often used
for testing. More information about the python controller can be found in the
[python testing](../testing/python.md) documentation.

## Building your first demo app (lighting)

The examples directory contains a set of apps using an example device
composition \.zap file. For more information about device composition and zap,
see [ZAP documentation](./zap.md).

This quick start guide will walk you through

-   Building an app (lighting app) for the host platform
-   Interacting with the app using chip\-tool \(controller\)

### Building the lighting app

-   Install prerequisites from
    [docs/guides/BUILDING\.md](../guides/BUILDING.md#prerequisites)
-   Run bootstrap or activate to install all the required tools etc.
    -   `. scripts/bootstrap.sh` \- run this first\, or if builds fail
    -   `. scripts/activate.sh` \- faster\, use if you’ve already bootstrapped
        and are just starting a new terminal

The build system we use is Ninja / GN. You can use a standard gn gen / ninja to
build as normal, or use the scripts to build specific variants. More information
about the build system can be found at [BUILDING.md](../guides/BUILDING.md). The
official quickstart guide for the build system is located ag
https://gn.googlesource.com/gn/+/master/docs/quick_start.md and a full reference
can be found at https://gn.googlesource.com/gn/+/main/docs/reference.md.

To build with the scripts, use scripts/build/build_examples\.py -
`scripts/build/build_examples.py targets` -
`scripts/build/build_examples.py --target <your target> build` - builds to
`out/<target_name>/`

Scripts can be used to build both the lighting app and chip tool

-   Lighting app \(device\)
    -   `./scripts/build/build_examples.py --target linux-x64-light-no-ble build`
    -   This will build an executable to
        `./out/linux-x64-light-no-ble/chip-lighting-app`

*   NOTE that the host name (linux-x64 here) may be different on different
    systems ex. darwin

-   chip-tool (controller)
    -   `./scripts/build/build_examples.py --target linux-x64-chip-tool build`
    -   This will build an executable to `./out/linux-x64-chip-tool/chip-tool`

### Building / Interacting with Matter Examples

The first thing you need to do is to commission the device. First start up the
lighting app in one terminal. By default it will start up with the default
discriminator (3840) and passcode (20202021) and save its non-volatile
information in a key-value-store file ("KVS") in /tmp/chip_kvs. You can change
this, and multiple other options on the command line. For a full description,
use the `--help` command.

Start the lighting app in one terminal using

`./out/linux-x64-light-no-ble/chip-lighting-app`

The lighting app will print out all its setup information. You can get the setup
codes, discriminator and passcode from the logs.

Open a new terminal to use chip tool. Commission the device using:

`./out/linux-x64-chip-tool/chip-tool pairing code 0x12344321 MT:-24J0AFN00KA0648G00`

NOTE: pairing is the old name for commissioning. 0x12344321 is the node ID you
want to assign to the node. 0x12344321 is the default for testing.
MT:-24J0AFN00KA0648G00 is the QR code for a device with the default
discriminator and passcode. If you have changed these, the code will be
different.

#### Basic device interactions - Sending a command

`./out/linux-x64-chip-tool/chip-tool onoff on 0x12344321 1`

where:

-   onoff is the cluster name
-   on is the command name
-   0x12344321 is the node ID you used for commissioning
-   1 is the endpoint

#### Basic device interactions - Reading an attribute

`./out/linux-x64-chip-tool/chip-tool onoff read on-off 0x12344321 1`

where:

-   onoff is the cluster name
-   read is the desired action
-   on-off is the attribute name
-   0x12344321 is the node ID you used for commissioning
-   1 is the endpoint
