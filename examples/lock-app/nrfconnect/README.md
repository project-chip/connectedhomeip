# CHIP nRF Connect Lock Example Application

The lock example provides demonstration of application running on the connected
door lock device. It uses
[CHIP](https://github.com/project-chip/connectedhomeip) and the nRF Connect
platform. The example supports remote access and control of a simulated door
lock over a low-power, 802.15.4 Thread network. It is capable of being paired
into an existing CHIP network along with other CHIP-enabled devices. Besides of
the remote access, application also allows controlling lock and device states
using buttons and visualizes those states on the LEDs.

<hr>

-   [Overview](#overview)
-   [Requirements](#requirements)
-   [Device UI](#device-ui)
-   [Setting up environment](#setting-up-environment)
    -   [Using Docker container](#using-docker-container)
    -   [Using Native shell](#using-native-shell)
-   [Building](#building)
        -   [Building minimal binary](#building-minimal-binary)
-   [Flashing and debugging](#flashing-and-debugging)
-   [Testing the example](#testing-the-example)

<hr>

<a name="overview"></a>

## Overview

This example application is running on the nRF Connect platform, which is based
on the
[nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/index.html)
and [Zephyr RTOS](https://zephyrproject.org/). Visit CHIP's
[nRF Connect Platform Overview](TODO:...) to read more information about
platform structure and dependencies.

CHIP device running lock application is controlled by the CHIP controller device
over Thread protocol. It is assumed that CHIP device by default has Thread
disabled, so it should be paired with CHIP controller and get configuration from
it. There are few actions necessary to make before full communication is
possible and they were described below. There is also a test mode, which allows
to start Thread with default settings, by pressing button manually, but it does
not fully guarantee that device will be able to communicate with CHIP controller
and other devices.

### BLE Advertising

After powering up device for the first time, it should start advertising over
BLE in order to inform other devices about its presence. For security reasons
different devices may start advertising automatically or on User demand and in
this example case, starting is triggered by pressing button.

### BLE Rendezvous

In CHIP there is a commissioning procedure called Rendezvous, which is done over
BLE between CHIP device and CHIP controller, performing the role of the
commissioner. To start it, controller has to previously get onboarding
information from the CHIP device. Data are encoded as a QR code payload and
typically presented on a device's display or like in this example case, shared
using NFC tag.

### Thread Provisioning

Successfully finishing Rendezvous procedure allows to perform Provisioning
operation, whose goal is to send Thread network credentials from CHIP controller
to the CHIP device. As a result, device is able to join the Thread network and
communicate with other Thread devices belonging to this network.

<hr>

<a name="requirements"></a>

## Requirements

In order to make sure that demonstrated application will be working properly, it
should be ran with the nRF Connect SDK 1.4 version.

The example supports building and running on the following devices:

| Board name                                                                                | Board platform build name |
| ----------------------------------------------------------------------------------------- | ------------------------- |
| [nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK) | nrf52840dk_nrf52840       |

<hr>

<a name="device-ui"></a>

## Device UI

This section lists device's elements making User Interface in the example
application. All described items should be possible to find on the board picture
presented below:

![nrf52840 DK](../../platform/nrfconnect/doc/images/nrf52840-dk.png)

The following elements of the development kit are used by this application to
allow User controlling and monitoring device's state.

**LED #1** shows the overall state of the device and its connectivity. Four
states are depicted:

-   _Short Flash On (50ms on/950ms off)_ &mdash; The device is in an
    unprovisioned (unpaired) state and is waiting for a commissioning
    application to connect.

*   _Rapid Even Flashing (100ms on/100ms off)_ &mdash; The device is in an
    unprovisioned state and a commissioning application is connected via BLE.

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is fully
    provisioned, but does not yet have full network (Thread) or service
    connectivity.

*   _Solid On_ &mdash; The device is fully provisioned and has full Thread
    network and service connectivity.

**LED #2** shows the state of the simulated lock bolt. When the LED is lit the
bolt is extended (i.e. door locked); when not lit, the bolt is retracted (door
unlocked). The LED will flash whenever the simulated bolt is in motion from one
position to another.

**Button #1** can be used to initiate a OTA software update as well as to reset
the device to a default state.

**Button #2** can be used to manually change the state of the lock. Pushing the
button is changing lock state to the opposite.

**Button #3** can be used to manually start Thread networking in the test mode,
using default configuration.

**Button #4** can be used to start BLE advertising.

**Segger J-Link USB port** can be used to get logs from the device or
communicate with it using [command line interface](TODO:).

<hr>

## Setting up environment

First, checkout CHIP repository and sync submodules using the following command:

        $ git submodule update --init

The example requires nRF Connect SDK v1.4.0. You can either install it along with
related tools directly on your system or use a Docker image which comes with the
tools pre-installed.

Note that if you're a macOS user, you won't be able to use the Docker container
to flash the application onto a Nordic board due to
[certain limitations of Docker for macOS](https://docs.docker.com/docker-for-mac/faqs/#can-i-pass-through-a-usb-device-to-a-container),
so it's recommended that you skip to the [Using native shell](#using-native-shell)
instruction.

### Using Docker container

If you don't have nRF Connect SDK installed yet, create a directory where it should be placed:

        $ mkdir ~/nrfconnect

Download the latest `nordicsemi/nrfconnect-chip` Docker image:

        $ docker pull nordicsemi/nrfconnect-chip

The next command will start a Docker container using the image acquired in the previous step. Please read the below explanation for all specified arguments before proceeding.

        $ docker run --rm -it -e RUNAS=$(id -u) -v ~/nrfconnect:/var/ncs -v ~/connectedhomeip:/var/chip \
            -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" nordicsemi/nrfconnect-chip

> **Note**:
>
> -   `~/connectedhomeip` should be replaced with an absolute path to CHIP
>     source directory.
> -   `~/nrfconnect` can be replaced with an absolute path to nRF Connect SDK
>     directory in case you have it already installed.
> -   `-v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule 'c 189:* rmw'`
>     parameters can be omitted if you're not planning to flash the example onto
>     hardware. The parameters give the container access to USB devices
>     connected to your computer such as Nordic development kits.
> -   `-e RUNAS=$(id -u)` is needed to start the container session as the
>     current user instead of root.

To checkout or update nRF Connect SDK to the recommended `1.4.0` version, run:

        $ setup --ncs v1.4.0
        /var/ncs repository is empty. Do you wish to check out nRF Connect SDK sources [v1.4.0]? [Y/N] y
        ...
        /var/chip repository is initialized, skipping...

Proceed with the [Building](#building) instruction.

### Using native shell

Follow the [guide](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html#)
to install nRF Connect SDK v1.4.0. Since further in the text we use command-line tools to build
the example, installing SEGGER Embedded Studio is not required.

In case you have an older version of the SDK installed, use the following commands to update it to the recommended version. Replace `<nrfconnect-dir>` with a path to the nRF Connect SDK installation directory.

        $ cd <nrfconnect-dir>/nrf
        $ git fetch origin
        $ git checkout v1.4.0
        $ west update

Download and install the
[nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools).

Download and install [GN meta-build system](https://gn.googlesource.com/gn/).

Initialize environment variables referred to by CHIP and nRF Connect SDK build scripts. Replace `<nrfconnect-dir>` with a path to the nRF Connect SDK installation directory. Also replace `<toolchain-dir>` with a path to GNU Arm Embedded Toolchain.

        $ source <nrfconnect-dir>/zephyr/zephyr-env.sh
        $ export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
        $ export GNUARMEMB_TOOLCHAIN_PATH=<toolchain-dir>

## Building

Navigate to the example's directory:

        $ cd examples/lock-app/nrfconnect

Run the following command to build the example. Replace `<board-name>` with name of the Nordic board you own, for example `nrf52840dk_nrf52840`.

        $ west build -b <board-name>

You only need to specify the board name on the first build, then it's enough to run:

        $ west build

To remove all build artifacts:

        $ rm -r build

To build the example with release configuration which disables diagnostic features like logs and command-line interface:

        $ west build -b <board-name> -- -DOVERLAY_CONFIG=third_party/connectedhomeip/config/nrfconnect/release.conf

### Building minimal binary

In order to build the example with no diagnostic features like UART console or
application logs, which should result in significantly smaller binary, run the
following commands:

        # Delete the build directory to make sure that no settings are cached
        $ rm -rf build/

        # Build the example using release config overlay
        $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG=third_party/connectedhomeip/config/nrfconnect/release.conf

<a name="configuring"></a>

## Configuring the example

The Zephyr ecosystem is higly configurable and allows the user to modify many
aspects of the application. The configuration system is based on `Kconfig` and
the settings can be modified using the `menuconfig` utility.

To open the configuration menu, do the following:

        $ cd <example-dir>
        # First time build
        $ west build -b nrf52840dk_nrf52840 -t menuconfig

        # Any subsequent build
        $ west build -t menuconfig

        # Running menuconfig with ninja
        $ cd <example-dir>/build
        $ ninja menuconfig

Changes done with `menuconfig` will be lost, if the `build` directory is
deleted. To make them persistent, save the configuration options in `prj.conf`
file.

<a name="flashing"></a>

## Flashing and debugging

To flash the application onto the device, run the command below in the example's directory:

        $ west flash

If you have multiple Nordic devices connected, `west` will prompt you to
pick the correct one.

To start the application with a debugger attached to your board, run:

        $ west debug

## Testing the example

Check the [CLI tutorial](../../../docs/guides/nrfconnect_examples_cli.md) to
learn how to use command-line interface of the application.

TODO: mention Rendezvous tutorial here
