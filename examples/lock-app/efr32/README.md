# CHIP EFR32 Lock Example

An example showing the use of CHIP on the Silicon Labs EFR32 MG12.

<hr>

-   [CHIP EFR32 Lock Example](#chip-efr32-lock-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Initializing the EFR32 module](#initializing-the-efr32-module)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)

<hr>

<a name="intro"></a>

## Introduction

The EFR32 lock example provides a baseline demonstration of a door lock device,
built using CHIP and the Silicon Labs gecko SDK. The example is currently very
basic and only supports responding to button presses to indicate the state of
the lock and view logging from the device. It doesn't currently support any over
the air connectivity via, BLE or OT.

Eventually, the lock example is intended to serve both as a means to explore the
workings of CHIP as well as a template for creating real products based on the
Silicon Labs platform.

A top-level Makefile orchestrates the entire build process, including building
CHIP, FreeRTOS, and select files from the Silicon Labs SDK. The resultant image
file can be flashed directly onto the Silicon Labs WSTK kit hardware.

<a name="building"></a>

## Building

-   Download and install the
    [Silicon Labs Simplicity Studio and SDK for Thread and Zigbee version v2.7](https://www.silabs.com/products/development-tools/software/simplicity-studio)

Install SimplicityStudio or extract the SimplicityStudio archive to where you
want to install Simplicity Studio and follow the instructions in README.txt
found within the extracted archive to complete installation.

On OSX, be sure to rename `/Applications/Simplicity Studio.app/` to something
without a "space". For example, rename it to
`/Applications/Simplicity_Studio.app/`

In Simplicity Studio from the Launcher perspective click on the "Update
Software" button. The Package Manager window will Open. Ensure that the
following SDKs are installed (as of January 2020).

-   Bluetooth 2.13.0.0
-   Flex 2.7.0.0

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

           # Linux
           $ sudo apt-get install git make automake libtool ccache libwebkitgtk-1.0-0

           # Mac OS X
           $ brew install automake libtool ccache

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

    -   With Make

             $ export EFR32_SDK_ROOT=<path-to-silabs-sdk-v2.7>
             $ make BOARD=BRD4161A

    -   With Ninja

              $ export EFR32_SDK_ROOT=<path-to-silabs-sdk-v2.7>
              $ export EFR32_BOARD=BRD4161A
              <From CHIP root>
              $ ./scripts/examples/gn_efr32_example.sh examples/lock-app/efr32/ out/lock_app_debug

-   To delete generated executable, libraries and object files use:

    -   With Make

             $ make BOARD=BRD4161A clean

    -   With Ninja

              $ rm -rf ./out/lock_app_debug

<a name="initializing"></a>

## Initializing the EFR32 module

The example application is designed to run on the Silicon Labs SDK development
kit. Prior to installing the application, the device's flash memory should be
erased.

-   Connect the host machine to the J-Link Interface MCU USB connector on the
    EFR32 WSTK.

-   Use the Makefile to erase the flash:

          $ make BOARD=BRD4161A erase

-   To erase a specific device using its serial number:

          $ make BOARD=BRD4161A SERIALNO=440113717 erase

<a name="flashing"></a>

## Flashing the Application

-   With Make

    -   To rebuild the image and flash the example app:

            $ make BOARD=BRD4161A flash

    -   To rebuild the image and flash a specific device using its serial
        number:

            $ make BOARD=BRD4161A SERIALNO=440113717 flash

    -   To flash an existing image without rebuilding:

            $ make BOARD=BRD4161A flash-app

*   With Ninja
    -   To Flash directly the board with the .s37 binary please follows
        instruction
        [here](https://www.silabs.com/community/mcu/32-bit/knowledge-base.entry.html/2014/10/22/using_jlink_commande-YYdy).
        **However** do **NOT** erase the flash since it will erase the
        bootloader and the example is not standalone as for now.
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
