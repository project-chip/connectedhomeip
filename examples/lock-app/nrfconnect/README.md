# CHIP nRF Connect nRF52840 Lock Example Application

An example application showing the use
[CHIP](https://github.com/project-chip/connectedhomeip) on the Nordic nRF52840.

<hr>

-   [CHIP nRF52840 Lock Example Application](#chip-nrf52840-lock-example-application)
    -   [Introduction](#introduction)
    -   [Device UI](#device-ui)
    -   [Building](#building)
    -   [Configuring the example](#configuring-the-example)
    -   [Flashing and debugging](#flashing-and-debugging)
    -   [Accessing the command line](#accessing-the-command-line)
    -   [Viewing Logging Output](#viewing-logging-output)

<hr>

<a name="intro"></a>

## Introduction

![nrf52840 DK](doc/images/nrf52840-dk.jpg)

The nRF52840 lock example application provides a working demonstration of a
connected door lock device, built using CHIP, and the Nordic nRF Connect. The
example supports remote access and control of a simulated door lock over a
low-power, 802.15.4 Thread network. It is capable of being paired into an
existing CHIP network along with other CHIP-enabled devices. The example targets
the
[Nordic nRF52840 DK](https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK)
development kit, but is readily adaptable to other nRF52840-based hardware.

The lock example is intended to serve both as a means to explore the workings of
CHIP, as well as a template for creating real products based on the Nordic
platform.

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

-   _Short Flash Off (950ms on/50ms off)_ &mdash; The device is full
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

**LED #2** shows the state of the simulated lock bolt. When the LED is lit the
bolt is extended (i.e. door locked); when not lit, the bolt is retracted (door
unlocked). The LED will flash whenever the simulated bolt is in motion from one
position to another.

**Button #2** can be used to change the state of the simulated bolt. This can be
used to mimick a user manually operating the lock. The button behaves as a
toggle, swapping the state every time it is pressed.

The remaining two LEDs and buttons (#3 and #4) are unused.

<a name="building"></a>

## Building

Before building the example,
[download the nRF Connect SDK and install all requirements](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html).

If you don't want to use SEGGER Embedded Studio, you may skip the part about
installing and configuring it.

Download and install the
[nRF Command Line Tools](https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools).

Make sure that you source the following file:

        $ source <ncs-dir>/zephyr/zephyr-env.sh

> **Note:** Ensure that `$ZEPHYR_BASE`, `$GNUARMEMB_TOOLCHAIN_PATH`, and
> `$ZEPHYR_TOOLCHAIN_VARIANT` environment variables are set in your current
> terminal before building. `$GNUARMEMB_TOOLCHAIN_PATH` and
> `$ZEPHYR_TOOLCHAIN_VARIANT` must be set manually.

After your environment is set up, you are ready to build the example. The
recommended tool for building and flashing the device is
[west](https://docs.zephyrproject.org/latest/guides/west/).

The following commands will build the `lock-app` example:

        $ cd ~/connectedhomeip
        $ ./bootstrap
        $ cd ~/connectedhomeip/examples/lock-app/nrfconnect

        # If this is a first time build or if `build` directory was deleted
        $ west build -b nrf52840dk_nrf52840

        # Any subsequent build
        $ west build

After a successful build, the binary will be available under
`<example-dir>/build/zephyr/zephyr.hex`

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

<a name="view-logging"></a>

## Accessing the command line

The application includes a command line interface with support for logs and the
OpenThread commands.

To access it, use any serial terminal program you like, for example `minicom` or
`GNU screen`.

The UART interface is configured for `115200` baud rate.

All OpenThread commands must be prefixed with `ot`, for example
`ot thread start`.

## Viewing Logging Output

The example application is built to use the SEGGER Real Time Transfer (RTT)
facility for log output. RTT is a feature built-in to the J-Link Interface MCU
on the development kit board. It allows bi-directional communication with an
embedded application without the need for a dedicated UART.

Using the RTT facility requires downloading and installing the _SEGGER J-Link
Software and Documentation Pack_
([web site](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack)).

-   Download the J-Link installer by navigating to the appropriate URL and
    agreeing to the license agreement.

<p style="margin-left: 40px">Linux: <a href="https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb">JLink_Linux_x86_64.deb</a></p>
<p style="margin-left: 40px">MacOS: <a href="https://www.segger.com/downloads/jlink/JLink\_MacOSX.pkg">JLink_MacOSX.pkg</a></p>

-   Install the J-Link software

          $ cd ~/Downloads
          $ sudo dpkg -i JLink_Linux_V*_x86_64.deb

*   In Linux, grant the logged in user the ability to talk to the development
    hardware via the linux tty device (/dev/ttyACMx) by adding them to the
    dialout group.

          $ sudo usermod -a -G dialout ${USER}

Once the above is complete, log output can be viewed using the JLinkExe tool in
combination with JLinkRTTClient as follows:

-   Run the JLinkExe tool with arguments to autoconnect to the nRF82480 DK
    board:

          $ JLinkExe -device NRF52840_XXAA -if SWD -speed 4000 -autoconnect 1

-   In a second terminal, run the JLinkRTTClient:

          $ JLinkRTTClient

Logging output will appear in the second terminal.
