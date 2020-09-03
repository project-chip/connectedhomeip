# CHIP EFR32 Lock Example

An example showing the use of CHIP on the Silicon Labs EFR32 MG12.

<hr>

-   [CHIP EFR32 Lock Example](#chip-efr32-lock-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
        -   [Note](#note)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)

<hr>

<a name="intro"></a>

## Introduction

The EFR32 lock example provides a baseline demonstration of a door lock device,
built using CHIP and the Silicon Labs gecko SDK. The example currently support
OpenThread. The BLE feature is still a work in progress.

The lock example is intended to serve both as a means to explore the workings of
CHIP as well as a template for creating real products based on the Silicon Labs
platform.

<a name="building"></a>

## Building

### Note

A consensus within the CHIP organization was reached to move from Make/Automake
to the GN/Ninja build system. As a result we are no longer supporting Make
inside the lock-app example. While the Makefile can stil be used to compile the
example, the output binary will be lacking key features (e.g. OpenThread).

-   Download the [sdk_support](https://github.com/SiliconLabs/sdk_support) from
    GitHub and export the path with :

            $ export EFR32_SDK_ROOT=<Path to cloned git repo>

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

           # Linux
           $ sudo apt-get install git make automake libtool ccache libwebkitgtk-1.0-0 ninja

           # Mac OS X
           $ brew install automake libtool ccache ninja

-   To build for an MG21 part make the following changes to the
    platform/CMSIS/Include/core_cm33.h file within the Silicon Labs SDK. Copy
    the following lines to the top of the core_cm33.h file.

```cpp
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
```

-   Supported hardware:

    MG12 boards:

    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
    -   BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm

    MG21 boards:

    -   BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

*   Build the example application:

    -   With Ninja

              $ export EFR32_SDK_ROOT=<path-to-silabs-sdk-v2.7>
              $ export EFR32_BOARD=BRD4161A
              <From CHIP root>
              $ ./scripts/examples/gn_efr32_example.sh examples/lock-app/efr32/ out/lock_app_debug

    -   With Make _deprecated_

             $ export EFR32_SDK_ROOT=<path-to-silabs-sdk-v2.7>
             $ make BOARD=BRD4161A

-   To delete generated executable, libraries and object files use:

    -   With Ninja

            $ rm -rf ./out/lock_app_debug

    -   With Make _deprecated_

             $ make BOARD=BRD4161A clean

<a name="flashing"></a>

## Flashing the Application

-   With Ninja

    -   From CHIP root,

              $ python out/lock_app_debug/BRD4161A/chip-efr32-lock-example.out.flash.py

-   With Make (_deprecated_)

    -   To rebuild the image and flash the example app:

            $ make BOARD=BRD4161A flash

    -   To rebuild the image and flash a specific device using its serial
        number:

            $ make BOARD=BRD4161A SERIALNO=440113717 flash

    -   To flash an existing image without rebuilding:

            $ make BOARD=BRD4161A flash-app

-   Or with the Ozone debugger, just load the .out file.

<a name="view-logging"></a>

## Viewing Logging Output

The example application is built to use the SEGGER Real Time Transfer (RTT)
facility for log output. RTT is a feature built-in to the J-Link Interface MCU
on the WSTK development board. It allows bi-directional communication with an
embedded application without the need for a dedicated UART.

Using the RTT facility requires downloading and installing the _SEGGER J-Link
Software and Documentation Pack_
([web site](https://www.segger.com/downloads/jlink#J-LinkSoftwareAndDocumentationPack)).
Alternatively the _SEGGER Ozone - J-Link Debugger_ can be used to view RTT logs.

-   Download the J-Link installer by navigating to the appropriate URL and
    agreeing to the license agreement.

-   [JLink_Linux_x86_64.deb](https://www.segger.com/downloads/jlink/JLink_Linux_x86_64.deb)
-   [JLink_MacOSX.pkg](https://www.segger.com/downloads/jlink/JLink_MacOSX.pkg)

*   Install the J-Link software

          $ cd ~/Downloads
          $ sudo dpkg -i JLink_Linux_V*_x86_64.deb

*   In Linux, grant the logged in user the ability to talk to the development
    hardware via the linux tty device (/dev/ttyACMx) by adding them to the
    dialout group.

          $ sudo usermod -a -G dialout ${USER}

Once the above is complete, log output can be viewed using the JLinkExe tool in
combination with JLinkRTTClient as follows:

-   Run the JLinkExe tool with arguments to autoconnect to the WSTK board:

    For MG12 use:

          $ JLinkExe -device EFR32MG12PXXXF1024 -if JTAG -speed 4000 -autoconnect 1

    For MG21 use:

          $ JLinkExe -device EFR32MG21AXXXF1024 -if SWD -speed 4000 -autoconnect 1

-   In a second terminal, run the JLinkRTTClient to view logs:

          $ JLinkRTTClient

<a name="running-complete-example"></a>

## Running the Complete Example

-   Once the example is flashed on the board, you should be able to establish a
    connection with an OpenThread border router. See
    [OpenThread Border Router](https://openthread.io/guides/border-router) for
    more information on how to setup a border router. Take note that the RCP
    code is available directly through
    [Simplicity Studio 5](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5)
    under File->New->Project Wizard->Examples->Thread : ot-rcp
-   Once said connectection is established (you can verify that with the command
    `router table` using a serial terminal (screen / minicom etc.) on the board
    running the lock-app example)
-   Using chip-tool you can now control the lock status with on/off command such
    as `chip-tool on <ipv6 address of the node> 11095 1`

### Notes

-   Depending on your network settings your router might not provide native ipv6
    addresses to your devices (Border router / PC). If this is the case, you
    need to add a static ipv6 addresses on both device and then a ipv6 route to
    the border router on your PC

          # On Border Router :
          $ sudo ip addr add dev <Network interface> 2002::2/64

          # On PC (Linux) :
          $ sudo ip addr add dev <Network interface> 2002::1/64

          # Add Ipv6 route on PC (Linux)
          $ sudo ip route add <Thread ipv6 prefix>/64 via 2002::2
