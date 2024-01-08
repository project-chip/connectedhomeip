# Matter `CC32XXSF` Air Purifier Example Application

An example application showing the use of [Matter][matter] on the Texas
Instruments CC32XX family of Wireless MCUs.

---

-   [Matter `CC32XXSF` Air Purifier Example Application](#matter-cc32xxsf-air-purifier-example-application)
    -   [Introduction](#introduction)
    -   [Device UI](#device-ui)
    -   [Building](#building)
        -   [Preparation](#preparation)
        -   [Compilation](#compilation)
    -   [Adding DAC Certificates](#adding-dac-certificates)
    -   [Programming](#programming)
        -   [Code Composer Studio](#code-composer-studio)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Example](#running-the-example)
        -   [Provisioning](#provisioning)
            -   [Bluetooth LE Provisioning](#bluetooth-le-provisioning)

---

## Introduction

The CC32XX air purifier example application provides a working demonstration of
a connected air purifier device. This uses the open-source CHIP implementation
and the Texas Instruments SimpleLink™ Wi-Fi® CC32xx software development kit.

By default this example targets the [CC3235SF_LAUNCHXL][cc3235sf_launchxl]
LaunchPad, but the example application is enabled to build on the whole `CC32XX`
family of MCUs.

The air purifier example is intended to serve both as a means to explore the
workings of CHIP, as well as a template for creating real products based on the
Texas Instruments devices.

## Device UI

The left button (`BTN-1`) is used to enable provisioning (provisioning is
enabled as "oneshot" by default). The right button (`BTN-2`) long press is used
to reset the device.

## Building

### Preparation

Some initial setup is necessary for preparing the build environment. This
section will need to be done when migrating to new versions of the SDK. This
guide assumes that the environment is linux based, and recommends Ubuntu 20.04.

-   Download and install [SysConfig][sysconfig] ([recommended
    version][sysconfig_recommended]). This can be done simply with the following
    commands.

    ```
    $ cd ~
    $ wget https://software-dl.ti.com/ccs/esd/sysconfig/sysconfig-1.13.0_2553-setup.run
    $ chmod +x sysconfig-1.13.0_2553-setup.run
    $ ./sysconfig-1.13.0_2553-setup.run
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

-   Run the build to produce a default executable. By default on Linux the
    Sysconfig is located in a `ti` folder in the user's home directory, and you
    must provide the absolute path for it. For example
    `/home/username/ti/sysconfig_1.13.0`. On Windows the default directory is
    `C:\ti`. Take note of this install path, as it will be used in the next
    step.

    ```
    $ cd ~/connectedhomeip/examples/air-purifier-app/cc32xx
    $ gn gen out/debug --args="ti_sysconfig_root=\"$HOME/ti/sysconfig_1.13.0\""
    $ ninja -C out/debug
    ```

## Adding DAC Certificates

To add custom DAC Certificates, the `CC32XXDeviceAttestationCreds.cpp` file in
`examples/platform/cc32xx` can be modified. The private key, public key, DAC
cert and PAI cert arrays all need to be replaced.

## Programming

Loading the built image onto a LaunchPad is supported through Code Composer
Studio (CCS). Code Composer Studio can be used to load the image and debug the
source code. UniFlash programming (bin) image is not generated currently.

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

### Provisioning

The first step to bring the Matter device onto the network is to provision it.
The example accomplishes this through the proprietary SimpleLink provisioning
method (AP or Smart Config) using the SimpleLink Starter Pro mobile app. Once
the device is connected to the local AP, commissioning can be triggered using
"OnNetwork" configuration.

#### Bluetooth LE Provisioning

BLE provisioning is not supported currently.
