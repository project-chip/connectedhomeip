#CHIP EFR32 Persistent Storage Example

An example testing and demonstrating the key value storage API.

<hr>

-   [CHIP EFR32 Persistent Storage Example](#chip-efr32-persistent-storage-example)
    -   [Introduction](#introduction)
    -   [EFR32](#efr32)
        -   [Building](#building)
        -   [Flashing the Application](#flashing-the-application)
        -   [Viewing Logging Output](#viewing-logging-output)

<hr>

<a name="intro"></a>

## Introduction

This example serves to both test the key value storage implementation and API as
it is brought-up on different platforms, as well as provide an example for how
to use the API.

In the future this example can be moved into a unit test when available on all
platforms.

<a name="EFR32"></a>

## EFR32

The EFR32 platform KVS is fully implemented

<a name="building"></a>

### Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

#Linux \$ sudo apt-get install git libwebkitgtk-1.0-0 ninja-build

#Mac OS X \$ brew install ninja

-   Supported hardware:

    MG12 boards:

    -   BRD4161A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm,
        868MHz@19dBm
    -   BRD4164A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@19dBm
    -   BRD4166A / SLTB004A / Thunderboard Sense 2 / 2.4GHz@10dBm
    -   BRD4170A / SLWSTK6000B / Multiband Wireless Starter Kit / 2.4GHz@19dBm,
        915MHz@19dBm
    -   BRD4304A / SLWSTK6000B / MGM12P Module / 2.4GHz@19dBm

    MG21 boards: Currently not supported due to RAM limitation.

    -   BRD4180A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

    MG24 boards :

    -   BRD4162A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4163A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm,
        868MHz@19dBm
    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_efr32_example.sh ./examples/persistent-storage/efr32/ ./out/persistent-storage BRD4161A

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/persistent-storage

OR use GN/Ninja directly

          $ cd ~/connectedhomeip/examples/persistent-storage/efr32
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export EFR32_BOARD=BRD4161A
          $ gn gen out/debug --args="efr32_sdk_root=\"${EFR32_SDK_ROOT}\" efr32_board=\"${EFR32_BOARD}\""
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/persistent-storage/efr32
          $ rm -rf out/

<a name="flashing"></a>

### Flashing the Application

-   On the command line:

          $ cd ~/connectedhomeip/examples/persistent-storage/efr32
          $ python3 out/debug/chip-efr32-persistent_storage-example.flash.py

-   Or with the Ozone debugger, just load the .out file.

<a name="view-logging"></a>

### Viewing Logging Output

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

## Memory settings

While most of the RAM usage in CHIP is static, allowing easier debugging and
optimization with symbols analysis, we still need some HEAP for the crypto and
OpenThread. Size of the HEAP can be modified by changing the value of the
`SL_STACK_SIZE` define inside of the BUILD.gn file of this example. Please take
note that a HEAP size smaller than 5k can and will cause a Mbedtls failure
during the BLE rendez-vous.
