![ARM Mbed-OS logo](https://raw.githubusercontent.com/ARMmbed/mbed-os/master/logo.png)

# Matter Arm Mbed OS Lock Example Application

The Arm Mbed OS OTA Requestor Example demonstrates how to remotely trigger
update image downloading and apply it if needed. Full functionality of this
examples can be obtained with the addition of a Mbed bootloader that allows
launching the right application image form memory. The example takes advantage
of the IO available on board:

-   A buttons press confirm or reject firmware update steps.
-   A LED shows the application state.

You can use this example as a reference for creating your own application.

The example is based on
[Matter](https://github.com/project-chip/connectedhomeip) and Arm Mbed OS, and
supports remote access and control of lock over a WiFi network.

The example behaves as a Matter accessory, in other words a device that can be
paired into an existing Matter network and can be controlled by this network.

<hr>

-   [Overview](#overview)
    -   [Bluetooth Low Energy advertising](#bluetooth-low-energy-advertising)
    -   [Bluetooth Low Energy rendezvous](#bluetooth-low-energy-rendezvous)
    -   [WiFi provisioning](#wifi-provisioning)
-   [Run application](#run-application)
    -   [Environment setup](#environment-setup)
    -   [Building](#building)
    -   [Flashing](#flashing)
    -   [Debugging](#debugging)
    -   [Testing](#testing)
        -   [Serial port terminal](#serial-port-terminal)
        -   [CHIP Tools](#chip-tools)
            -   [Notes](#notes)
    -   [Supported devices](#supported-devices)
        -   [Notes](#notes-1)
    -   [Device UI](#device-ui)

<hr>

## Overview

The Matter device that runs the lock application is controlled by the Matter
controller device over WiFi. By default, the Matter device is disconnected , and
it should be paired with Matter controller and get configuration from it.
Actions required before establishing full communication are described below.

### Bluetooth Low Energy advertising

To commission the device onto a Matter network, the device must be discoverable
over BLE. The BLE advertising starts automatically after device boot-up.

### Bluetooth Low Energy rendezvous

In Matter, the commissioning procedure (called rendezvous) is done over BLE
between a Matter device and the Matter controller, where the controller has the
commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the Matter device. The data payload is encoded within a QR code, printed to
the UART console.

### WiFi provisioning

The last part of the rendezvous procedure, provisioning involves sending the
network credentials from the Matter controller to the Matter device. As a
result, device is able to join the network and communicate with other devices in
the network.

## Run application

### Environment setup

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

### Building

The OTA Requestor application can be built in the same way as any other Matter
example ported to the mbed-os platform.

-   **by using generic vscode task**:

```
Command Palette (F1) => Run Task... => Run Mbed Application => build => ota-requestor-app => (board name) => (build profile) => (build type)
```

-   **by calling explicitly building script:**

```
${MATTER_ROOT}/scripts/examples/mbed_example.sh -c=build -a=ota-requestor-app -b=<board name> -p=<build profile> -T=<build type>
```

Both approaches are limited to supported evaluation boards which are listed in
[Supported devices](#supported-devices) paragraph.

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

### Flashing

The Lock application can be flashed in the same way as any other Matter example
ported to mbed-os platform.

The [Open On-Chip Debugger](http://openocd.org/) is used to upload a binary
image and reset the device.

-   **by using VSCode task**:

```
Command Palette (F1) => Run Task... -> Run Mbed Application => flash => ota-requestor-app => (board name) => (build profile)
```

-   **by calling explicitly building script:**

```
${MATTER_ROOT}/scripts/examples/mbed_example.sh -c=flash -a=ota-requestor-app -b=<board name> -p=<build profile>
```

-   **by using VSCode launch task**:

```
Run and Debug (Ctrl+Shift+D) => Flash Mbed examples => Start Debugging (F5) => (board name) => ota-requestor-app => (build profile)
```

The last option uses the Open On-Chip Debugger to open and manage the gdb-server
session. Then gdb-client (arm-none-eabi-gdb) upload binary image and reset
device.

It is possible to connect to an external gdb-server session by using specific
**'Flash Mbed examples [remote]'** task.

### Debugging

Debugging can be performed in the same was as with any other Matter example
ported to mbed-os platform.

The [Open On-Chip Debugger](http://openocd.org/) is used to to open and manage
the gdb-server session. Then gdb-client (arm-none-eabi-gdb) connect the server
to upload binary image and control debugging.

```
Run and Debug (Ctrl+Shift+D) => Debug Mbed examples => Start Debugging (F5) => (board name) => ota-requestor-app => (build profile)
```

It is possible to connect to an external gdb-server session by using specific
**'Debug Mbed examples [remote]'** task.

### Testing

The provider application is required to transfer image file to OTA requestor.
Mbed example is compatible with Linux version of OTA provider example. Read the
[OTAProvider](../../ota-provider-app/linux/README.md) to see how to build and
run the OTA provider.

#### Serial port terminal

The application traces are streaming to serial output. To start communication
open a terminal session and connect to the serial port of the device. You can
use **mbed-tools** for this purpose
([mbed-tools](https://github.com/ARMmbed/mbed-tools)):

    mbed-tools sterm -p /dev/ttyACM0 -b 115200 -e off

After device reset these lines should be visible:

    [INFO][CHIP]: [-]Mbed ota-requestor-app example application start
    ...
    [INFO][chip]: [-]Mbed ota-requestor-app example application run

The ota-requestor-app application launched correctly and you can follow traces
in the terminal.

#### CHIP Tools

Read the [MbedCommissioning](../../../docs/guides/mbedos_commissioning.md) to
see how to use different CHIP tools to commission and control the application
within a WiFi network.

After commissioning is successful, announce OTA provider's presence using
`OtaSoftwareUpdateRequestor` cluster with `AnnounceOTAProvider` command. On
receiving this command OTA requestor will query for OTA image:

    chip-device-ctrl > zcl OtaSoftwareUpdateRequestor AnnounceOTAProvider 1234 0 0 providerNodeId=1235 vendorId=9020 announcementReason=0

The OTA requestor should communicate with provider, download update image and
apply it.

##### Notes

-   You have to provision the OTA Provider in the same Matter network. Use the
    `connect -ip` command of Python Device Controller:

        chip-device-ctrl > connect -ip 127.0.0.1 20202021 1235

-   POSIX CLI CHIPTool can be also used for testing this example. Use the
    correct `chip-tool` arguments to perform above-mentioned steps.

### Supported devices

The example supports building and running on the following mbed-enabled devices:

| Manufacturer                                          | Hardware platform                                                         | Build target          | Platform image                                                                                                                                                                 | Status | Platform components                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| ----------------------------------------------------- | ------------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :----: | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Cypress<br> Semiconductor](https://www.cypress.com/) | [CY8CPROTO-062-4343W](https://os.mbed.com/platforms/CY8CPROTO-062-4343W/) | `CY8CPROTO_062_4343W` | <details><summary>CY8CPROTO-062-4343W</summary><img src="https://os.mbed.com/media/cache/platforms/p6_wifi-bt_proto.png.250x250_q85.jpg" alt="CY8CPROTO-062-4343W"/></details> |   ✔    | <details><summary>LEDs</summary><ul><li>Board has only one usable LED (LED4) which corresponds to USER LED from UI.</li><li>Lock state LED should be an external component connected to PB9_6 pin (active high).</li></ul></details> <details><summary>Buttons</summary><ul><li>SW2 push-button is not used in this example due to its interaction with WIFI module interrupt line.</li><li>Button 0 corresponds to BTN0 capacitive button.</li><li>Button 1 corresponds to BTN1 capacitive button.</li></ul></details> <details><summary>Slider</summary><ul><li>Unused</ul></details> |

##### Notes

-   More details and guidelines about porting new hardware into the Matter
    project with Mbed OS can be found in
    [MbedNewTarget](../../../docs/guides/mbedos_add_new_target.md)
-   Some useful information about HW platform specific settings can be found in
    `ota-requestor-app/mbed/mbed_app.json`. Information about this file syntax
    and its meaning in mbed-os project can be found here:
    [Mbed-Os configuration system](https://os.mbed.com/docs/mbed-os/latest/program-setup/advanced-configuration.html))

### Device UI

This section lists the User Interface elements that you can use to control and
monitor the state of the device. These correspond to PCB components on the
platform image.

**USER LED** shows the overall state of the device and its connectivity. The
following states are possible:

-   _Short Flash On (50 ms on/950 ms off)_ &mdash; The device is in the
    unprovisioned (unpaired) state and is waiting for a commissioning
    application to connect.

-   _Rapid Even Flashing (100 ms on/100 ms off)_ &mdash; The device is in the
    unprovisioned state and a commissioning application is connected through
    Bluetooth LE.

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is fully
    provisioned, but does not yet have full network or service connectivity.

-   _Solid On_ &mdash; The device is fully provisioned and has full network and
    service connectivity.

**Button 0** can be used for the following purposes:

-   _Pressed for 6 s_ &mdash; Initiates the factory reset of the device.
    Releasing the button within the 6-second window cancels the factory reset
    procedure. **LEDs 1-4** blink in unison when the factory reset procedure is
    initiated.

-   _Pressed for less than 3 s_ &mdash; Trigger confirm user response.

**Button 1** can be used for the following purposes:

-   _Pressed for 6 s_ &mdash; Initiates the commissioning reset of the device.
    The fabric IDs are deleted and BLE advertising start. Releasing the button
    within the 6-second window cancels the commissioning reset procedure. **LEDs
    1-4** blink in unison when the commissioning reset procedure is initiated.

-   _Pressed for less than 3 s_ &mdash; Trigger reject user response.

**Button 1** &mdash; Pressing the button once delete all fabric IDs and start
BLE advertising.

Some of the supported boards may not have sufficient number PCB components to
follow above description. In that case please refer to
[Supported devices](#supported-devices) section and check board's 'Platform
components' column for additional information about the limitation.
