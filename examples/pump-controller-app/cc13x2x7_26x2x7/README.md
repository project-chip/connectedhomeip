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
section will need to be done when migrating to new versions of the SDK.

-   An engineering SDK from TI is required. Please request access for it
    [here][ti_cc13x2_26x2_r7_chip_request].

    -   Follow the default installation instructions when executing the
        installer.

    -   The version of OpenThread used in this repository is newer than the one
        packaged with the TI SDK. Check the following section for a list of
        changes needed.

-   Download and install [SysConfig][sysconfig]
    ([sysconfig-1.5.0_1397][sysconfig-1.5.0_1397])

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

-   Run the bootstrap script to setup the build environment.

    ```
    $ cd ~/connectedhomeip
    $ source ./script/bootstrap.sh

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
    `/home/username/ti/simplelink_cc13x2_26x2_sdk_4_40_05_02_eng` and
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

In this example, the provisioning procedure (called Rendezvous) is done over
Bluetooth LE between a CHIP device (lock-app) and the CHIP controller
(CHIPTool), where the controller has the commissioner role.

To start the rendezvous, the controller must get the commissioning information
from the CHIP device.

This is done by scanning a QR code. A URL will be displayed on the lock-app's
log ([UART terminal](#viewing-logging-output)). It will look like the following:

```
SetupQRCode:  [MT:.81TM -00 0C9SS0]
Copy/paste the below URL in a browser to see the QR Code:
https://dhrishi.github.io/connectedhomeip/qrcode.html?data=CH%3A.81TM%20-00%200C9SS0
```

You can directly navigate to the webpage URL displayed (which has QR payload
pre-loaded). Alternatively, you can navigate to [the QR code
generator][qr_code_generator] and enter in the payload shown in `SetupQRCode`
(in this case `MT:.81TM -00 0C9SS0`).

### CHIP Remote Commands

Once the CHIP device is provisioned and operating on the network, CHIPTool can
be used to control the device. During the provisioning process, the CHIP device
would have sent one of its newly assigned IPv6 addresses to the CHIPTool.

In the app, you should see an On/Off cluster; this corresponds to the lock-app.
You can now control the lock-app CHIP device from the smartphone!

## TI Support

For technical support, please consider creating a post on TI's [E2E forum][e2e].
Additionally, we welcome any feedback.

[chip]: https://github.com/project-chip/connectedhomeip
[cc1352r1_launchxl]: https://www.ti.com/tool/LAUNCHXL-CC1352R1
[e2e]: https://e2e.ti.com/support/wireless-connectivity/zigbee-and-thread
[simplelink_sdk]: https://www.ti.com/tool/SIMPLELINK-CC13X2-26X2-SDK
[simplelink_sdk_4.30.00.54]:
    https://www.ti.com/tool/download/SIMPLELINK-CC13X2-26X2-SDK/4.30.00.54
[sysconfig]: https://www.ti.com/tool/SYSCONFIG
[sysconfig-1.5.0_1397]:
    http://software-dl.ti.com/ccs/esd/sysconfig/sysconfig-1.5.0_1397-setup.run
[ti_thread_dnd]:
    https://www.ti.com/wireless-connectivity/thread/design-development.html
[ti_cc13x2_26x2_r7_chip_request]: https://ti.com/chip_sdk
[ot_border_router_setup]:
    https://openthread.io/guides/border-router/beaglebone-black
[qr_code_generator]: https://dhrishi.github.io/connectedhomeip/qrcode.html
