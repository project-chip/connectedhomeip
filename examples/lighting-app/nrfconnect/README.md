# CHIP nRF52840 Lighting Example Application

An example application showing the use
[CHIP](https://github.com/project-chip/connectedhomeip) on the Nordic nRF52840.

<hr>

-   [CHIP nRF52840 Lighting Example Application](#chip-nrf52840-lighting-example-application)
    -   [Introduction](#introduction)
    -   [Device UI](#device-ui)
    -   [Building](#building)
        -   [Using Docker container](#using-docker-container)
        -   [Using Native shell](#using-native-shell)
        -   [Supported nRF Connect SDK versions](#supported-nrf-connect-sdk-versions)
    -   [Configuring the example](#configuring-the-example)
    -   [Flashing and debugging](#flashing-and-debugging)
    -   [Accessing the command line](#accessing-the-command-line)

<hr>

<a name="intro"></a>

## Introduction

![nrf52840 DK](../../platform/nrf528xx/doc/images/nrf52840-dk.jpg)

The nRF52840 lighting example application provides a working demonstration of a
connected lighting device, built using CHIP, and the Nordic nRF Connect SDK. The
example supports remote access and control of a lighting over a low-power,
802.15.4 Thread network. It is capable of being paired into an existing CHIP
network along with other CHIP-enabled devices. The example targets the
[Nordic nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
development kit, but is readily adaptable to other nRF52840-based hardware.

The lighting example is intended to serve both as a means to explore the
workings of CHIP, as well as a template for creating real products based on the
Nordic platform.

The example makes use of the CMake build system to generate the ninja build
script. The build system takes care of invoking the CHIP library build with all
necessary flags exported from the Zephyr environment.

<a name="device-ui"></a>

## Device UI

The example application provides a simple UI that depicts the state of the
device and offers basic user control. This UI is implemented via the
general-purpose LEDs and buttons built in to the nRF52840 DK dev board.

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

*   _Solid On_ &mdash; The device is fully provisioned and has full network and
    service connectivity.

**Button #1** can be used to initiate a OTA software update as well as to reset
the device to a default state.

Pressing and holding Button #1 for 6 seconds initiates a factory reset. After an
initial period of 3 seconds, all four LED will flash in unison to signal the
pending reset. Holding the button past 6 seconds will cause the device to reset
its persistent configuration and initiate a reboot. The reset action can be
cancelled by releasing the button at any point before the 6 second limit.

**LED #2** shows the state of the lighting.

**Button #2** can be used to change the state of the lighting. This can be used
to mimick a user manually switching the lighting. The button behaves as a
toggle, swapping the state every time it is pressed.

**Button #3** can be used to start Thread networking using default configuration
which was selected to match OpenThread Border Router default settings and
network credentials.

**Button #4** can be used to start BLE advertisement, which is disabled by
default.

The remaining two LEDs (#3 and #4) are unused.

**NFC** can be used to scan shared tag and get information about device necessary
to perform rendezvous and network provisioning operation.

Tag can be read by bringing NFC poller e.g. smartphone supporting NFC close to the nRF52840's NFC antenna.
Moreover in the reaction on sensing field from the smartphone CHIP device will start BLE
advertisement, what is an alternative way to trigger this from manually pressing Button 4.

<a name="building"></a>

## Building

### Using Docker container

> **Important**:
>
> Due to
> [certain limitations of Docker for MacOS](https://docs.docker.com/docker-for-mac/faqs/#can-i-pass-through-a-usb-device-to-a-container)
> it is impossible to use the Docker container to communicate with a USB device
> such as nRF 52840 DK. Therefore, MacOS users are advised to follow the
> [Using Native shell](#using-native-shell) instruction.

The easiest way to get started with the example is to use nRF Connect SDK Docker
image for CHIP applications. Run the following commands to start a Docker
container:

        $ mkdir ~/nrfconnect
        $ mkdir ~/connectedhomeip
        $ docker pull nordicsemi/nrfconnect-chip
        $ docker run --rm -it -v ~/nrfconnect:/var/ncs -v ~/connectedhomeip:/var/chip -v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule "c 189:* rmw" nordicsemi/nrfconnect-chip

> **Note**:
>
> -   `~/nrfconnect` can be replaced with an absolute path to nRF Connect SDK
>     source directory in case you have it already installed.
> -   Likewise, `~/connectedhomeip` can be replaced with an absolute path to
>     CHIP source directory.
> -   `-v /dev/bus/usb:/dev/bus/usb --device-cgroup-rule 'c 189:* rmw`
>     parameters can be omitted if you're not planning to flash the example onto
>     hardware. The parameters give the container access to USB devices
>     connected to your computer such as the nRF52840 DK.
> -   `--rm` flag can be omitted if you don't want the container to be
>     auto-removed when you exit the container shell session.

If you use the container for the first time and you don't have nRF Connect SDK
and CHIP sources downloaded yet, run `setup` command in the container to pull
the sources into directories mounted as `/var/ncs` and `/var/chip`,
respectively:

        $ setup --ncs 83764f
        /var/ncs repository is empty. Do you wish to check out nRF Connect SDK sources [83764f]? [Y/N] y
        ...
        /var/chip repository is empty. Do you wish to check out Project CHIP sources [master]? [Y/N] y
        ...

Now you may build the example by running the commands below in the Docker
container:

        $ cd /var/chip/examples/lighting-app/nrfconnect
        $ west build -b nrf52840dk_nrf52840

If the build succeeds, the binary will be available under
`/var/chip/examples/lighting-app/nrfconnect/build/zephyr/zephyr.hex`. Note that
other operations described in this document like flashing or debugging can also
be done in the container.

### Using native shell

Before building the example,
[download the nRF Connect SDK and install all requirements](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html).
Please read the
[Supported nRF Connect SDK versions](#supported-nrf-connect-sdk-versions)
section to learn which version to use to avoid unexpected compatibility issues.

If you don't want to use SEGGER Embedded Studio, you may skip the part about
installing and configuring it.

Download and install the
[nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools).

Download and install [GN meta-build system](https://gn.googlesource.com/gn/).

Make sure that you source the following file:

        $ source <ncs-dir>/zephyr/zephyr-env.sh

> **Note:**
>
> Ensure that `$ZEPHYR_BASE`, `$GNUARMEMB_TOOLCHAIN_PATH`, and
> `$ZEPHYR_TOOLCHAIN_VARIANT` environment variables are set in your current
> terminal before building. `$GNUARMEMB_TOOLCHAIN_PATH` and
> `$ZEPHYR_TOOLCHAIN_VARIANT` must be set manually.

After your environment is set up, you are ready to build the example. The
recommended tool for building and flashing the device is
[west](https://docs.zephyrproject.org/latest/guides/west/).

The following commands will build the `lighting-app` example:

        $ cd ~/connectedhomeip/examples/lighting-app/nrfconnect

        # If this is a first time build or if `build` directory was deleted
        $ west build -b nrf52840dk_nrf52840

        # Any subsequent build
        $ west build

After a successful build, the binary will be available under
`<example-dir>/build/zephyr/zephyr.hex`

### Supported nRF Connect SDK versions

It is recommended to use the nRF Connect version which is being verified as a
part of CHIP Continuous Integration testing, which happens to be `83764f` at the
moment. You may verify that the revision is used in
[chip-build-nrf-platform](https://github.com/project-chip/connectedhomeip/blob/master/integrations/docker/images/chip-build-nrf-platform/Dockerfile)
Docker image in case of doubt.

Please refer to
[this section](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#updating-the-repositories)
in the user guide to learn how to update nRF Connect SDK repository.

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

The example application is designed to run on the
[Nordic nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
development kit.

To flash the application to the device, use the `west` tool:

        $ cd <example-dir>
        $ west flash

If you have multiple nRF52840 DK boards connected, `west` will prompt you to
pick the correct one.

To debug the application on target:

        $ cd <example-dir>
        $ west debug

<a name="accessing-the-command-line"></a>

## Accessing the command line

The application includes a command line interface with support for logs and the
OpenThread commands.

To access it, use any serial terminal program you like, for example `minicom` or
`GNU screen`.

The UART interface is configured for `115200` baud rate.

All OpenThread commands must be prefixed with `ot`, for example
`ot thread start`.
