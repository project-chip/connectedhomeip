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
device, built using CHIP and the Silicon Labs gecko SDK. It can be controlled by
a Chip controller over Openthread network..

The EFR32 device can be commissioned over Bluetooth Low Energy where the device
and the Chip controller will exchange security information with the Rendez-vous
procedure. Thread Network credentials are then provided to the EFR32 device
which will then join the network.

The LCD on the Silabs WSTK shows a QR Code containing the needed commissioning
information for the BLE connection and starting the Rendez-vous procedure.

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

OR use the script

          cd ~/connectedhomeip
          $ export EFR32_SDK_ROOT=<path-to-silabs-sdk-v2.7>
          $ export EFR32_BOARD=BRD4161A
          ./scripts/examples/gn_efr32_example.sh examples/lighting-app/efr32/ out/debug/efr32_lighting_app

-   To delete generated executable, libraries and object files use:
    $ cd ~/connectedhomeip
          $ rm -rf out/debug/efr32_lighting_app

<a name="flashing"></a>

## Flashing the Application

-   On the command line:

          $ cd ~/connectedhomeip/examples/lighting-app/efr32
          $ python3 out/debug/chip-efr32-lighting-example.out.flash.py

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

Alternatively, SEGGER Ozone J-Link debugger can be used to view RTT logs too
after flashing the .out file.

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

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not, see the following guide
    [OpenThread Border Router](https://openthread.io/guides/border-router) for
    more information on how to setup a border router. Take note that the RCP
    code is available directly through
    [Simplicity Studio 5](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5)
    under File->New->Project Wizard->Examples->Thread : ot-rcp

-   User interface : **LCD** The LCD on Silabs WSTK shows a QR Code. This QR
    Code is be scanned by the CHIP Tool app For the Rendez-vous procedure over
    BLE

        * On devices that do not have or support the LCD Display like the BRD4166A Thunderboard Sense 2,
          a URL can be found in the RTT logs.

          <info  > [SVR] Copy/paste the below URL in a browser to see the QR Code:
          <info  > [SVR] https://dhrishi.github.io/connectedhomeip/qrcode.html?data=CH%3AI34NM%20-00%200C9SS0

    **LED 0** shows the overall state of the device and its connectivity. The
    following states are possible:

        -   _Short Flash On (50 ms on/950 ms off)_ ; The device is in the
            unprovisioned (unpaired) state and is waiting for a commissioning
            application to connect.

        -   _Rapid Even Flashing_ ; (100 ms on/100 ms off)_ &mdash; The device is in the
            unprovisioned state and a commissioning application is connected through
            Bluetooth LE.

        -   _Short Flash Off_ ; (950ms on/50ms off)_ &mdash; The device is fully
            provisioned, but does not yet have full Thread network or service
            connectivity.

        -   _Solid On_ ; The device is fully provisioned and has full Thread
            network and service connectivity.

    **LED 1** Simulates the Light The following states are possible:

        -   _Solid On_ ; Light is on
        -   _Off_ ; Light is off

    **Push Button 0** - Press and Release : If not commissioned, start thread
    with default configurations (DEBUG)


        -   Pressed and hold for 6 s: Initiates the factory reset of the device.
            Releasing the button within the 6-second window cancels the factory reset
            procedure. **LEDs** blink in unison when the factory reset procedure is
            initiated.

    **Push Button 1**
        Toggles the light state On/Off

-   Once the device is provisioned, it will join the Thread network is
    established, look for the RTT log

    ```
        [DL] Device Role: CHILD
        [DL] Partition Id:0x6A7491B7
        [DL] \_OnPlatformEvent default: event->Type = 32778
        [DL] OpenThread State Changed (Flags: 0x00000001)
        [DL] Thread Unicast Addresses:
        [DL]    2001:DB8::E1A2:87F1:7D5D:FECA/64 valid preferred
        [DL]    FDDE:AD00:BEEF::FF:FE00:2402/64 valid preferred rloc
        [DL]    FDDE:AD00:BEEF:0:383F:5E81:A05A:B168/64 valid preferred
        [DL]    FE80::D8F2:592E:C109:CF00/64 valid preferred
        [DL] LwIP Thread interface addresses updated
        [DL] FE80::D8F2:592E:C109:CF00 IPv6 link-local address, preferred)
        [DL] FDDE:AD00:BEEF:0:383F:5E81:A05A:B168 Thread mesh-local address, preferred)
        [DL] 2001:DB8::E1A2:87F1:7D5D:FECA IPv6 global unicast address, preferred)
    ```

    Keep The global unicast address; It is to be used to reach the Device with
    the chip-tool. The device will be promoted to Router shortly after [DL]
    Device Role: ROUTER

    (you can verify that the device is on the thread network with the command
    `router table` using a serial terminal (screen / minicom etc.) on the board
    running the lighting-app example. You can also get the address list with the
    command ipaddr again in the serial terminal )

-   Using chip-tool you can now control the light status with on/off command
    such as `chip-tool onoff on 1`

    \*\* Currently, chip-tool for Mac or Linux do not yet have the Thread
    provisioning feature
    `chip-tool bypass <Global ipv6 address of the node> 11097`

    You can provision the Chip device using Chip tool Android or iOS app or
    through CLI commands on your OT BR

### Notes

-   Depending on your network settings your router might not provide native ipv6
    addresses to your devices (Border router / PC). If this is the case, you
    need to add a static ipv6 addresses on both device and then an ipv6 route to
    the border router on your PC

          # On Border Router :
          $ sudo ip addr add dev <Network interface> 2002::2/64

          # On PC (Linux) :
          $ sudo ip addr add dev <Network interface> 2002::1/64

          # Add Ipv6 route on PC (Linux)
          $ sudo ip route add <Thread global ipv6 prefix>/64 via 2002::2
