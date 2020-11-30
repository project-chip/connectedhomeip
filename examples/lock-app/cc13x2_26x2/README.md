# CHIP CC1352 CC2652 Lock Example Application

An example application showing the use [CHIP][chip] on the Texas Instruments
CC13X2_26X2 family of Wireless MCUs.

---

-   [CHIP CC1352 CC2652 Lock Example Application](#chip-cc1352-cc2652-lock-example-application)
    -   [Introduction](#introduction)
        -   [Device UI](#device-ui)
    -   [Building](#building)
        -   [Preparation](#preparation)
        -   [Compilation](#compilation)
    -   [Programming](#programming)
        -   [UniFlash](#uniflash)
        -   [Code Composer Studio](#code-composer-studio)
    -   [Viewing Logging Output](#viewing-logging-output)

---

## Introduction

![CC1352R1_LAUNCHXL](doc/images/cc1352r1_launchxl.jpg)

The CC13X2_26X2 lock example application provides a working demonstration of a
connected door lock device. This uses the open-source CHIP implementation and
the Texas Instruments SimpleLink™ CC13x2 and CC26x2 software development kit.

This example is enabled to build on all members of the `CC13X2_26X2` family of
MCUs [recommended for Thread][ti_thread_dnd]. By default this example targets
the [CC1352R1_LAUNCHXL][cc1352r1_launchxl] LaunchPad.

The lock example is intended to serve both as a means to explore the workings of
CHIP, as well as a template for creating real products based on the Texas
Instruments devices.

## Device UI

This example application has a simple User Interface to depict the state of the
door lock and to control the state. The user LEDs on the LaunchPad are set on
when the lock is locked, and are set off when unlocked. The LEDs will flash when
in the transition state between locked and unlocked. The user buttons are used
for requesting lock and unlock of the door lock. The left button (`BTN-1`) is
used to request locking. The right button (`BTN-2`) us used to request
unlocking.

## Building

### Preparation

Some initial setup is necessary for preparing the build environment. This
section will need to be done when migrating to new versions of the SDK.

-   Download and install the [SimpleLink™ CC13x2 and CC26x2 software development
    kit (SDK)][simplelink_sdk] ([4.30.00.54][simplelink_sdk_4.30.00.54])

    -   Follow the default installation instructions when executing the
        installer.

-   Download and install [SysConfig][sysconfig]
    ([sysconfig-1.5.0_1397][sysconfig-1.5.0_1397])

    -   This may have already been installed with your SimpleLink SDK install

-   Download and install [FreeRTOS][freertos]
    ([FreeRTOSv10.3.1.zip][freertosv10.3.1])

    -   Unzip to a useful installation location.
    -   The GN build will look at `~/third_party/FreeRTOSv10.3.1` by default.

-   If you have installed different versions, the build defaults will need to be
    changed to reflect this in
    `${chip_root}/examples/build_overrides/ti_simplelink_sdk.gni`.

-   Install Python 3.8 for the GN build system:

    ```
    # Linux
    $ sudo apt-get install python3.8 python3.8-distutils python3.8-dev python3.8-venv
    # Distutils listed due to a package manager error on Ubuntu 18.04

    ```

    -   You will have to ensure that the default version of Python 3 is Python
        3.8.

-   run the bootstrap script to setup the build environment.

    ```
    $ cd ~/connectedhomeip
    h

    ```

### Compilation

It is necessary to activate the environment in every new shell. Then run GN and
Ninja to build the executable.

-   Activate the build environment with the repository activate script.

    ```
    $ cd ~/connectedhomeip
    h

    ```

-   Run the build to produce a default executable.

    ```
    $ cd ~/connectedhomeip/examples/lock-app/cc13x2_26x2
    $ gn gen out/debug
    $ ninja -C out/debug

    ```

## Programming

Loading the built image onto a LaunchPad is supported through two methods;
Uniflash and Code Composer Studio (CCS). UniFlash can be used to load the image.
Code Composer Studio can be used to load the image and debug the source code.

### UniFlash

[Programming UniFlash](doc/programming-uniflash.md)

### Code Composer Studio

[Programming and Debugging with CCS](doc/programming-ccs.md)

## Viewing Logging Output

By default the log output will be sent to the Application/User UART. Open a
terminal emulator to that port to see the output with the following options:

| Parameter    | Value    |
| ------------ | -------- |
| Speed (baud) | `115200` |
| Data bits    | `8`      |
| Stop bits    | `1`      |
| Parity       | `None`   |
| Flow control | `None`   |

[chip]: https://github.com/project-chip/connectedhomeip
[cc1352r1_launchxl]: https://www.ti.com/tool/LAUNCHXL-CC1352R1
[simplelink_sdk]: https://www.ti.com/tool/SIMPLELINK-CC13X2-26X2-SDK
[simplelink_sdk_4.30.00.54]:
    https://www.ti.com/tool/download/SIMPLELINK-CC13X2-26X2-SDK/4.30.00.54
[sysconfig]: https://www.ti.com/tool/SYSCONFIG
[sysconfig-1.5.0_1397]:
    http://software-dl.ti.com/ccs/esd/sysconfig/sysconfig-1.5.0_1397-setup.run
[freertos]: https://www.freertos.org/index.html
[freertosv10.3.1]:
    https://github.com/FreeRTOS/FreeRTOS/releases/download/V10.3.1/FreeRTOSv10.3.1.zip
[ti_thread_dnd]:
    https://www.ti.com/wireless-connectivity/thread/design-development.html
