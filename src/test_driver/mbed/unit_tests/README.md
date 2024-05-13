![ARM Mbed-OS logo](https://raw.githubusercontent.com/ARMmbed/mbed-os/master/logo.png)

<h1> Matter Arm Mbed OS Unit Tests Application </h1>

The Arm Mbed OS Unit Tests application executes all supported unit tests on the
device.

The application is based on
[Matter](https://github.com/project-chip/connectedhomeip) and Arm Mbed OS, and
validates Matter solution components directly on the device.

<hr>

-   [Overview](#overview)
-   [Run application](#run-application)
    -   [Environment setup](#environment-setup)
    -   [Building](#building)
    -   [Flashing](#flashing)
    -   [Debugging](#debugging)
    -   [Testing](#testing)
        -   [Serial port terminal](#serial-port-terminal)
    -   [Supported devices](#supported-devices)
        -   [Notes](#notes)

<hr>

# Overview

The Matter unit tests are included in a monolithic library and allow to validate
most of the components used by Matter examples applications. The main goal of
this application is to run registered tests on the device and check the results.
The final result is the number of tests that failed.

# Run application

## Environment setup

Before building the application, check out the Matter repository and sync
submodules using the following command:

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
> **Please read this [README.md](../../../../docs/VSCODE_DEVELOPMENT.md) for
> more information about using VSCode in container.**

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

The Unit Test application can be built analogously to Matter examples ported to
the mbed-os platform. It uses a custom script therefore it is a separate defined
target.

-   **by using generic vscode task**:

```
Command Palette (F1) => Run Task... => Run Mbed Unit Tests => build => (board name) => (build profile)
```

-   **by calling explicitly building script:**

```
${MATTER_ROOT}/scripts/tests/mbed/mbed_unit_tests.sh -c=build -b=<board name> -p=<build profile>
```

Both approaches are limited to supported evaluation boards which are listed in
[Supported devices](#supported_devices) paragraph.

Mbed OS defines three building profiles: _develop, debug_ and _release_. For
more details please visit
[ARM Mbed OS build profiles](https://os.mbed.com/docs/mbed-os/latest/program-setup/build-profiles-and-rules.html).

When using the building script, it is possible expand the list of acceptable
targets; this may be useful for rapid testing of a new mbed-targets.

## Flashing

The Unit Test application can be flashed analogously to Matter examples ported
to the mbed-os platform. It uses a custom script therefore it is a separate
defined target.

The [Open On-Chip Debugger](http://openocd.org/) is used to upload a binary
image and reset the device.

-   **by using VSCode task**:

```
Command Palette (F1) => Run Task... -> Run Mbed Unit Tests => flash => (board name) => (build profile)
```

-   **by calling explicitly building script:**

```
${MATTER_ROOT}/scripts/tests/mbed/mbed_unit_tests.sh -c=flash -b=<board name> -p=<build profile>
```

-   **by using VSCode launch task**:

```
Run and Debug (Ctrl+Shift+D) => Flash Mbed unit tests => Start Debugging (F5) => (board name) => (build profile)
```

The last option uses the Open On-Chip Debugger to open and manage the gdb-server
session. Then gdb-client (arm-none-eabi-gdb) upload binary image and reset
device.

It is possible to connect to an external gdb-server session by using specific
**'Flash Mbed unit tests [remote]'** task.

## Debugging

Debugging can be performed analogously to Matter examples ported to the mbed-os
platform.

The [Open On-Chip Debugger](http://openocd.org/) is used to to open and manage
the gdb-server session. Then gdb-client (arm-none-eabi-gdb) connect the server
to upload binary image and control debugging.

```
Run and Debug (Ctrl+Shift+D) => Debug Mbed unit tests => Start Debugging (F5) => (board name) => (build profile)
```

It is possible to connect to an external gdb-server session by using specific
**'Debug Mbed unit tests [remote]'** task.

## Testing

### Serial port terminal

The test results are streaming to serial output. To start communication open a
terminal session and connect to the serial port of the device. You can use
**mbed-tools** for this purpose
([mbed-tools](https://github.com/ARMmbed/mbed-tools)):

    ```
    mbed-tools sterm -p /dev/ttyACM0 -b 115200 -e off
    ```

After device reset these lines should be visible:

    ```
    [INFO][CHIP]: [-]Mbed unit-tests application start
    ...
    [INFO][CHIP]: [-]Mbed unit-tests application run
    ```

The unit tests application launched correctly and registered tests are executed.

The final result is the number of tests that failed:

    ```
    [INFO][CHIP]: [-]CHIP test status: 0
    ```

## Supported devices

The application supports building and running on the following mbed-enabled
devices:

| Manufacturer                                          | Hardware platform                                                         | Build target          | Platform image                                                                                         |       Status       | Platform components                                                                                                                                                                                                                                                                |
| ----------------------------------------------------- | ------------------------------------------------------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------ | :----------------: | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [Cypress<br> Semiconductor](https://www.cypress.com/) | [CY8CPROTO-062-4343W](https://os.mbed.com/platforms/CY8CPROTO-062-4343W/) | `CY8CPROTO_062_4343W` | ![CY8CPROTO-062-4343W](https://os.mbed.com/media/cache/platforms/p6_wifi-bt_proto.png.250x250_q85.jpg) | :heavy_check_mark: | <details><summary>LEDs</summary><ul><li>Board has only one usable LED (LED4) which corresponds to USER LED from UI.</li></ul></details> <details><summary>Buttons</summary><ul><li>Unused</li></ul></details> <details><summary>Slider</summary><ul><li>Unused</li></ul></details> |

#### Notes

-   More details and guidelines about porting new hardware into the Matter
    project with Mbed OS can be found in
    [MbedNewTarget](../../../../docs/guides/mbedos_add_new_target.md)
-   Some useful information about HW platform specific settings can be found in
    `test_driver/mbed/mbed_app.json`.  
    Information about this file syntax and its meaning in mbed-os project can be
    found here:
    [Mbed-Os configuration system](https://os.mbed.com/docs/mbed-os/latest/program-setup/advanced-configuration.html))
