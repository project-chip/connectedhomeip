![ARM Mbed-OS logo](https://raw.githubusercontent.com/ARMmbed/mbed-os/master/logo.png)

<h1> Matter Arm Mbed OS Pigweed Example Application </h1>

The Arm Mbed OS Pigweed Example demonstrates the usage of Pigweed module
functionalities in an application.

You can use this example as a reference for creating your own application.

The example is based on
[Matter](https://github.com/project-chip/connectedhomeip) and Arm Mbed OS, and
supports remote access and control of device over a serial port.

Pigweed functionalities are integrated into this application. The Remote
Procedure Call (RPC) server is created. It allows sending commands through the
serial port to the device. The following RPC protocols services are available:

-   **Echo** - receive message and send the same payload back

<hr>

-   [Overview](#overview)
-   [Run application](#run-application)
    -   [Environment setup](#environment-setup)
    -   [Building](#building)
    -   [Flashing](#flashing)
    -   [Debugging](#debugging)
    -   [Testing](#testing)
        -   [Serial port terminal](#serial-port-terminal)
        -   [RPC console](#rpc-console)
    -   [Supported devices](#supported-devices)
        -   [Notes](#notes)
-   [Device UI](#device-ui)
    -   [Notes](#notes-1)

<hr>

# Overview

Pigweed libraries are built and organized in a way that enables faster and more
reliable development. In the Matter project, the Pigweed module is planned to be
used to create system infrastructures, for example for performing on-device
tests, but considering its general functionalities, it can be useful also in
other cases.

# Run application

## Environment setup

Before building the example, check out the Matter repository and sync submodules
using the following command:

    $ git submodule update --init

Building the example application requires the use of **ARM Mbed-OS** sources and
the **arm-none-gnu-eabi** toolchain.

The Cypress OpenOCD package is required for flashing purpose. Install the
Cypress OpenOCD and set env var `OPENOCD_PATH` before calling the flashing
script.

```
cd ~
wget https://github.com/Infineon/openocd/releases/download/release-v4.3.0/openocd-4.3.0.1746-linux.tar.gz
tar xzvf openocd-4.3.0.1746-linux.tar.gz
export OPENOCD_PATH=$HOME/openocd
```

Some additional packages may be needed, depending on selected build target and
its requirements.

> **The VSCode devcontainer has these components pre-installed. Using the VSCode
> devcontainer is the recommended way to interact with Arm Mbed-OS port of the
> Matter Project.**
>
> **Please read this [README.md](../../..//docs/VSCODE_DEVELOPMENT.md) for more
> information about using VSCode in container.**

To initialize the development environment, download all registered sub-modules
and activate the environment:

```
$ source ./scripts/bootstrap.sh
$ source ./scripts/activate.sh
```

If packages are already installed then you just need to activate the development
environment:

```
$ source ./scripts/activate.sh
```

## Building

The Pigweed application can be built in the same way as any other Matter example
ported to the mbed-os platform.

-   **by using generic vscode task**:

```
Command Palette (F1) => Run Task... => Run Mbed Application => build => pigweed-app => (board name) => (build profile) => (build type)
```

-   **by calling explicitly building script:**

```
${MATTER_ROOT}/scripts/examples/mbed_example.sh -c=build -a=pigweed-app -b=<board name> -p=<build profile> -T=<build type>
```

Both approaches are limited to supported evaluation boards which are listed in
[Supported devices](#supported_devices) paragraph.

Mbed OS defines three building profiles: _develop, debug_ and _release_. For
more details please visit
[ARM Mbed OS build profiles](https://os.mbed.com/docs/mbed-os/latest/program-setup/build-profiles-and-rules.html).

There are also three types of built application: _simple, boot_ and _upgrade_:

-   **simple** - standalone application, mainly for developing and testing
    purpose (all building profiles are supported)
-   **boot** - signed application + bootloader, it supports booting process and
    can be use for firmware update (only _release_ building profiles is
    supported)
-   **update** - signed application, application image can be used for firmware
    update (only _release_ building profiles is supported)

When using the building script, it is possible expand the list of acceptable
targets; this may be useful for rapid testing of a new mbed-targets.

## Flashing

The Pigweed application can be flashed in the same way as any other Matter
example ported to mbed-os platform.

The [Open On-Chip Debugger](http://openocd.org/) is used to upload a binary
image and reset the device.

-   **by using VSCode task**:

```
Command Palette (F1) => Run Task... -> Run Mbed Application => flash => pigweed-app => (board name) => (build profile)
```

-   **by calling explicitly building script:**

```
${MATTER_ROOT}/scripts/examples/mbed_example.sh -c=flash -a=pigweed-app -b=<board name> -p=<build profile>
```

-   **by using VSCode launch task**:

```
Run and Debug (Ctrl+Shift+D) => Flash Mbed examples => Start Debugging (F5) => (board name) => pigweed-app => (build profile)
```

The last option uses the Open On-Chip Debugger to open and manage the gdb-server
session. Then gdb-client (arm-none-eabi-gdb) upload binary image and reset
device.

It is possible to connect to an external gdb-server session by using specific
**'Flash Mbed examples [remote]'** task.

## Debugging

Debugging can be performed in the same was as with any other Matter example
ported to mbed-os platform.

The [Open On-Chip Debugger](http://openocd.org/) is used to to open and manage
the gdb-server session. Then gdb-client (arm-none-eabi-gdb) connect the server
to upload binary image and control debugging.

```
Run and Debug (Ctrl+Shift+D) => Debug Mbed examples => Start Debugging (F5) => (board name) => pigweed-app => (build profile)
```

It is possible to connect to an external gdb-server session by using specific
**'Debug Mbed examples [remote]'** task.

## Testing

### Serial port terminal

The application traces are streaming to serial output. To start communication
open a terminal session and connect to the serial port of the device. You can
use **mbed-tools** for this purpose
([mbed-tools](https://github.com/ARMmbed/mbed-tools)):

    mbed-tools sterm -p /dev/ttyACM0 -b 115200 -e off

After device reset these lines should be visible:

    [INFO][CHIP]: [-]Mbed pigweed-app example application start
    ...
    [INFO][CHIP]: [-]Mbed pigweed-app example application run

The pigweed-app application launched correctly and you can follow traces in the
terminal.

### RPC console

The RPC console is an interactive Python shell console, where the different RPC
command can be invoked. It is a complete solution for interacting with hardware
devices using pw_rpc over a pw_hdlc transport. For more details about Pigweed
modules visit [Pigweed modules](https://pigweed.dev/module_guides.html).

**<h3> Building and installing </h3>**

To build and install the RPC console check the guide
[CHIP RPC console](../../common/pigweed/rpc_console/README.md).

**<h3> Run </h3>**

To start the RPC console run the following command and provide device connection
parameters as arguments:

-   --device/-d the serial port to use.
-   --baudrate/-b the baud rate to use.
-   --output/-o the file to which to write device output (HDLC channel 1),
    provide - or omit for stdout.
-   --socket-addr/-s alternatively use socket to connect to server, type default
    for localhost:33000, or manually input the server address:port.

Example:

    chip-console -d /dev/ttyUSB0 -b 115200 -o /tmp/pw_rpc.out

To send the echo message type the following command, where you define the
message content:

    In [1]: rpcs.pw.rpc.EchoService.Echo(msg="Hello device")

The response from the device should be:

    Out[1]: (Status.OK, pw.rpc.EchoMessage(msg="Hello device"))

For more details about RPC console and supported services visit
[CHIP RPC console](../../common/pigweed/rpc_console/README.md).

## Supported devices

The example supports building and running on the following mbed-enabled devices:

| Manufacturer                                          | Hardware platform                                                         | Build target          | Platform image                                                                                                                                                                 |       Status       | Platform components                                                                                                                                                                                                                                                                |
| ----------------------------------------------------- | ------------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :----------------: | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Cypress<br> Semiconductor](https://www.cypress.com/) | [CY8CPROTO-062-4343W](https://os.mbed.com/platforms/CY8CPROTO-062-4343W/) | `CY8CPROTO_062_4343W` | <details><summary>CY8CPROTO-062-4343W</summary><img src="https://os.mbed.com/media/cache/platforms/p6_wifi-bt_proto.png.250x250_q85.jpg" alt="CY8CPROTO-062-4343W"/></details> | :heavy_check_mark: | <details><summary>LEDs</summary><ul><li>Board has only one usable LED (LED4) which corresponds to USER LED from UI.</li></ul></details> <details><summary>Buttons</summary><ul><li>Unused</li></ul></details> <details><summary>Slider</summary><ul><li>Unused</li></ul></details> |

#### Notes

-   More details and guidelines about porting new hardware into the Matter
    project with Mbed OS can be found in
    [MbedNewTarget](../../../docs/guides/mbedos_add_new_target.md)
-   Some useful information about HW platform specific settings can be found in
    `pigweed-app/mbed/mbed_app.json`.  
    Information about this file syntax and its meaning in mbed-os project can be
    found here:
    [Mbed-Os configuration system](https://os.mbed.com/docs/mbed-os/latest/program-setup/advanced-configuration.html))

# Device UI

This section lists the User Interface elements that you can use to control and
monitor the state of the device. These correspond to PCB components on the
platform image.

**USER LED** shows the overall state of the device. The following states are
possible:

-   _Solid On_ &mdash; The application was flashed and run successfully.

### Notes

Some of the supported boards may not have sufficient number PCB components to
follow above description. In that case please refer to
[Supported devices](#Supported-devices) section and check board's 'Platform
components' column for additional information about the limitation.
