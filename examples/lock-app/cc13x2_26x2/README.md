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

    -   The version of OpenThread used in this repository is newer than the one
        packaged with the TI SDK. Check the following section for a list of
        changes needed.

-   Download and install [SysConfig][sysconfig]
    ([sysconfig-1.5.0_1397][sysconfig-1.5.0_1397])

    -   This may have already been installed with your SimpleLink SDK install

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
    $ source ./script/bootstrap.sh

    ```

#### Changes to the TI SDK

The OpenThread library will set the short address assigned to the device as soon
as it receives the Child ID response. This may happen while the radio driver is
still in transmit mode. This is easilly fixed by removing state check in the
else condition in
`${ti_simplelink_sdk_root}/examples/rtos/${ti_simplelink_board}/thread/cli_mtd/platform/radio.c`
on linke 1791.

```
-     else if (sState != platformRadio_phyState_Transmit)
+     else
      {
          sReceiveCmd.localShortAddr = aAddress;
      }
```

### Compilation

It is necessary to activate the environment in every new shell. Then run GN and
Ninja to build the executable.

-   Activate the build environment with the repository activate script.

    ```
    $ cd ~/connectedhomeip
    $ source ./script/activate.sh

    ```

-   Run the build to produce a default executable. By default on Linux both the
    TI SimpleLink SDK and Sysconfig are located in a `ti` folder in the user's
    home directory, and you must provide the absolute path to them. For example
    `/home/username/ti/simplelink_cc13x2_26x2_sdk_4_30_00_54` and
    `/home/username/ti/sysconfig_1.6.0`. On Windows the default directory is
    `C:\ti`

    ```
    $ cd ~/connectedhomeip/examples/lock-app/cc13x2_26x2
    $ export TI_SIMPLELINK_SDK_ROOT=<path-to-simplelink-sdk>
    $ export TI_SYSCONFIG_ROOT=<path-to-sysconfig-sdk>
    $ gn gen out/debug --args="ti_simplelink_sdk_root=\"${TI_SIMPLELINK_SDK_ROOT}\" ti_sysconfig_root=\"${TI_SYSCONFIG_ROOT}\""
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
[ti_thread_dnd]:
    https://www.ti.com/wireless-connectivity/thread/design-development.html
