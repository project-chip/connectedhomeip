# CHIP EFR32 Lighting Example

An example showing the use of CHIP on the Silicon Labs EFR32 MG12.

<hr>

-   [CHIP EFR32 Lighting Example](#chip-efr32-lighting-example)
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

The EFR32 lighting example provides a baseline demonstration of a Light control
device, built using CHIP and the Silicon Labs gecko SDK. The example currently
support OpenThread. The BLE feature is still a work in progress.

The lighting example is intended to serve both as a means to explore the
workings of CHIP as well as a template for creating real products based on the
Silicon Labs platform.

<a name="building"></a>

## Building

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
           $ sudo apt-get install git libwebkitgtk-1.0-0 ninja-build

           # Mac OS X
           $ brew install ninja

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

          $ cd ~/connectedhomeip/examples/lighting-app/efr32
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export EFR32_SDK_ROOT=<path-to-silabs-sdk-v2.7>
          $ export EFR32_BOARD=BRD4161A
          $ gn gen out/debug --args="efr32_sdk_root=\"${EFR32_SDK_ROOT}\" efr32_board=\"${EFR32_BOARD}\""
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/lighting-app/efr32
          $ rm -rf out/

<a name="flashing"></a>

## Flashing the Application

-   On the command line:

          $ cd ~/connectedhomeip/examples/lock-app/efr32
          $ python3 out/debug/chip-efr32-lighting-example.flash.py

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
    running the lighting-app example)
-   Using chip-tool you can now control the light status with on/off command
    such as `chip-tool on <ipv6 address of the node> 11095 1`

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
