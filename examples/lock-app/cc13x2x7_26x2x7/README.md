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
        -   [Code Composer Studio](#code-composer-studio)
        -   [UniFlash](#uniflash)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Example](#running-the-example)
        -   [Provisioning](#provisioning)
            -   [Bluetooth LE Advertising](#bluetooth-le-advertising)
            -   [Bluetooth LE Rendezvous](#bluetooth-le-rendezvous)
        -   [CHIP Remote Commands](#chip-remote-commands)
    -   [TI Support](#ti-support)

---

## Introduction

![CC1352R1_LAUNCHXL](doc/images/cc1352r1_launchxl.jpg)

The CC13X2_26X2 lock example application provides a working demonstration of a
connected door lock device. This uses the open-source CHIP implementation and
the Texas Instruments SimpleLink™ CC13x2 and CC26x2 software development kit.

This example is enabled to build for CC2652R7 devices. This upcoming devices are
currently not yet in full production. For more information on device
availability or early access to an engineering build of our CHIP-enabled SDK,
please reach out [here][ti_cc13x2_26x2_r7_chip_request].

The lock example is intended to serve both as a means to explore the workings of
CHIP, as well as a template for creating real products based on the Texas
Instruments devices.

## Device UI

This example application has a simple User Interface to depict the state of the
door lock and to control the state. The user LEDs on the LaunchPad are set on
when the lock is locked, and are set off when unlocked. The LEDs will flash when
in the transition state between locked and unlocked.

Short presses (less than 1000ms) of the user buttons are used for requesting
lock and unlock of the door lock. The left button (`BTN-1`) is used to request
locking. The right button (`BTN-2`) is used to request unlocking.

Long presses (greater than 1000ms) of the user buttons are used for controlling
BLE advertisements. The left button (`BTN-1`) is used to disable advertisements
if they are enabled. The Right button (`BTN-2`) is used to enable
advertisements.

## Building

### Preparation

Some initial setup is necessary for preparing the build environment. This
section will need to be done when migrating to new versions of the SDK. This
guide assumes that the environment is linux based, and recommends Ubuntu 20.04.

-   An engineering SDK from TI is required. Please request access for it
    [here][ti_cc13x2_26x2_r7_chip_request].

    -   Follow the default installation instructions when executing the
        installer.

    -   The version of OpenThread used in this repository is newer than the one
        packaged with the TI SDK. Check the following section for a list of
        changes needed.

-   Download and install [SysConfig][sysconfig] ([recommended
    version][sysconfig_recommended])

    -   This may have already been installed with your SimpleLink SDK install.

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

        -   Check python3 version:

        ```
        $ python3 --version
        Python 3.8.0
        ```

        -   If it is not Python 3.8:

        ```
        $ sudo update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.8 1
        ```

            -   This may affect your environment in other ways if there was a
                specific dependency on the prior python3 version (e.g. apt).
                After completing the build process for this example, you can
                revert the python3 version, for instance:

                ```
                $ sudo update-alternatives --config python3
                There are 2 choices for the alternative python3 (providing /usr/bin/python3).

                  Selection    Path                Priority    Status
                -------------------------------------------------------------
                  0            /usr/bin/python3.8   1          auto mode
                  1            /usr/bin/python3.6   1          manual mode
                * 2            /usr/bin/python3.8   1          manual mode

                Press <enter> to keep the current choice[*], or type selection number: 1
                update-alternatives: using /usr/bin/python3.6 to provide /usr/bin/python3 (python3) in manual mode
                ```

-   Run the bootstrap script to setup the build environment.

    ```
    $ cd ~/connectedhomeip
    $ source ./scripts/bootstrap.sh

    ```

### Compilation

It is necessary to activate the environment in every new shell. Then run GN and
Ninja to build the executable.

-   Activate the build environment with the repository activate script.

    ```
    $ cd ~/connectedhomeip
    $ source ./scripts/activate.sh

    ```

-   Run the build to produce a default executable. By default on Linux both the
    TI SimpleLink SDK and Sysconfig are located in a `ti` folder in the user's
    home directory, and you must provide the absolute path to them. For example
    `/home/username/ti/simplelink_cc13x2_26x2_sdk_4_40_05_02_eng` and
    `/home/username/ti/sysconfig_1.7.0`. On Windows the default directory is
    `C:\ti`. Take note of this install path, as it will be used in the next
    step.

    ```
    $ cd ~/connectedhomeip/examples/lock-app/cc13x2x7_26x2x7
    $ export TI_SIMPLELINK_SDK_ROOT=$HOME/ti/simplelink_cc13x2_26x2_sdk_4_40_05_02_eng
    $ export TI_SYSCONFIG_ROOT=$HOME/ti/sysconfig_1.7.0
    $ gn gen out/debug --args="ti_simplelink_sdk_root=\"${TI_SIMPLELINK_SDK_ROOT}\" ti_sysconfig_root=\"${TI_SYSCONFIG_ROOT}\""
    $ ninja -C out/debug

    ```

## Programming

Loading the built image onto a LaunchPad is supported through two methods;
Uniflash and Code Composer Studio (CCS). UniFlash can be used to load the image.
Code Composer Studio can be used to load the image and debug the source code.

### Code Composer Studio

Programming with CCS will allow for a full debug environment within the IDE.
This is accomplished by creating a target connection to the XDS110 debugger and
starting a project-less debug session. The CCS IDE will attempt to find the
source files on the local machine based on the debug information embedded within
the ELF. CCS may prompt you to find the source code if the image was built on
another machine or the source code is located in a different location than is
recorded within the ELF.

Download and install [Code Composer Studio][ccs].

First open CCS and create a new workspace.

Create a target connection (sometimes called the CCXML) for your target SoC and
debugger as described in the [Manual Method][ccs_manual_method] section of the
CCS User's Guide.

Next initiate a project-less debug session as described in the [Manual
Launch][ccs_manual_launch] section of the CCS User's Guide.

CCS should switch to the debug view described in the [After
Launch][ccs_after_launch] section of the User's Guide. The SoC core will likely
be disconnected and symbols will not be loaded. Connect to the core as described
in the [Debug View][ccs_debug_view] section of the User's Guide. Once the core
is connected, use the `Load` button on the toolbar to load the ELF image.

Note that the default configuration of the CCXML uses 2-wire cJTAG instead of
the full 4-wire JTAG connection to match the default jumper configuration of the
LaunchPad.

### UniFlash

Uniflash is Texas Instrument's uniform programming tool for embedded processors.
This will allow you to erase, flash, and inspect the SoC without setting up a
debugging environment.

Download and install [UniFlash][uniflash].

First open UniFlash. Debug probes connected to the computer will usually be
displayed under the Detected Devices due to the automatic device detection
feature. If your device does not show up in this view it my be disconnected, or
you may have to create a New Configuration. If you already have a CCXML for your
SoC and debug connection you can use that in the section at the bottom. Once
your device is selected, click the `Start` button within the section to launch
the session.

Select the ELF image to load on the device with the `Browse` button. This file
is placed in the `out/debug` folder by this guide and ends with the `*.out` file
extension.

Finally click the `Load Image` button to load the executable image onto the
device. You should be able to see the log output over the XDS110 User UART.

Note that programming the device through JTAG sets the Halt-in-Boot flag and may
cause issues when performing a software reset. This flag can be reset by
power-cycling the LaunchPad.

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

## Running the Example

Once a device has been flashed with this example, it can now join and operate in
an existing Thread network. The following sections assume that a Thread network
is already active, and has at least one [OpenThread Border
Router][ot_border_router_setup].

### Provisioning

The first step to bring the CHIP device onto the network is to provision it. Our
example accomplishes this with Bluetooth Low Energy (BLE) and the
[CHIPTool](../../../src/android/CHIPTool/README.md) mobile app.

#### Bluetooth LE Advertising

To provision this example onto a Thread network, the device must be discoverable
over Bluetooth LE. BLE advertising is started by long pressing the right button
(greater than 1000ms), labeled `BTN-2` on the silkscreen. Once the device is
fully provisioned, BLE advertising will stop.

#### Bluetooth LE Rendezvous

To commission and control this application within a CHIP-enabled Thread network,
consult the [CHIPTool README](../../../src/android/CHIPTool/README.md) for
information on the Android smartphone application. Reference the Device
Configuration information printed in the Logging Output of this application.

## TI Support

For technical support, please consider creating a post on TI's [E2E forum][e2e].
Additionally, we welcome any feedback.

[chip]: https://github.com/project-chip/connectedhomeip
[ccs]: https://www.ti.com/tool/CCSTUDIO
[ccs_after_launch]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#after-launch
[ccs_debug_view]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#debug-view
[ccs_manual_launch]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#manual-launch
[ccs_manual_method]:
    https://software-dl.ti.com/ccs/esd/documents/users_guide/ccs_debug-main.html?configuration#manual-method
[cc1352r1_launchxl]: https://www.ti.com/tool/LAUNCHXL-CC1352R1
[e2e]: https://e2e.ti.com/support/wireless-connectivity/zigbee-and-thread
[simplelink_sdk]: https://www.ti.com/tool/SIMPLELINK-CC13X2-26X2-SDK
[simplelink_sdk_4.30.00.54]:
    https://www.ti.com/tool/download/SIMPLELINK-CC13X2-26X2-SDK/4.30.00.54
[sysconfig]: https://www.ti.com/tool/SYSCONFIG
[sysconfig_recommended]:
    https://software-dl.ti.com/ccs/esd/sysconfig/sysconfig-1.7.0_1746-setup.run
[ti_thread_dnd]:
    https://www.ti.com/wireless-connectivity/thread/design-development.html
[ti_cc13x2_26x2_r7_chip_request]: https://ti.com/chip_sdk
[ot_border_router_setup]: https://openthread.io/guides/border-router/build
[uniflash]: https://www.ti.com/tool/download/UNIFLASH
