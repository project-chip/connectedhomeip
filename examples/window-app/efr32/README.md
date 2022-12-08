# Matter EFR32 Window Covering Example

An example showing the use of CHIP on the Silicon Labs EFR32 MG12 and MG24.

<hr>

-   [Matter EFR32 Window Covering Example](#matter-efr32-window-covering-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)
    -   [OTA Software Update](#ota-software-update)
    -   [Building options](#building-options)
        -   [Disabling logging](#disabling-logging)
        -   [Debug build / release build](#debug-build--release-build)
        -   [Disabling LCD](#disabling-lcd)
        -   [KVS maximum entry count](#kvs-maximum-entry-count)

<hr>

> **NOTE:** Silicon Laboratories now maintains a public matter GitHub repo with
> frequent releases thoroughly tested and validated. Developers looking to
> develop matter products with silabs hardware are encouraged to use our latest
> release with added tools and documentation.
> [Silabs Matter Github](https://github.com/SiliconLabs/matter/releases)

<a name="intro"></a>

## Introduction

The EFR32 window-covering example provides a baseline demonstration of a Window
Covering device, built using Matter and the Silicon Labs gecko SDK. It can be
controlled by a Chip controller over an Openthread or Wifi network.

The EFR32 device can be commissioned over Bluetooth Low Energy where the device
and the Chip controller will exchange security information with the Rendez-vous
procedure. In the case of Thread, the Thread Network credentials are provided to
the EFR32 device which will then join the Thread network.

If the LCD is enabled, the LCD on the Silabs WSTK shows a QR Code containing the
needed commissioning information for the BLE connection and starting the
Rendez-vous procedure. Once the device is commissioned, the displays shows a
representation of the window covering state.

The window-covering example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
Silicon Labs platform.

<a name="building"></a>

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

           # Linux
           $ sudo apt-get install git ninja-build

           # Mac OS X
           $ brew install ninja

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/general/HARDWARE_REQUIREMENTS.md)
        > in the Silicon Labs Matter Github Repo

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

    -   BRD2601B / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD2703A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_efr32_example.sh ./examples/window-app/efr32/ ./out/window-app BRD4161A

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

    OR use GN/Ninja directly

          $ cd ~/connectedhomeip/examples/window-app/efr32
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export EFR32_BOARD=BRD4161A
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/window-app/efr32
          $ rm -rf out/

*   Build the example as Sleepy End Device (SED)

          $ ./scripts/examples/gn_efr32_example.sh ./examples/window-app/efr32/ ./out/window-app_SED BRD4161A --sed

    or use gn as previously mentioned but adding the following arguments:

          $ gn gen out/debug '--args=silabs_board="BRD4161A" enable_sleepy_device=true chip_openthread_ftd=false'

*   Build the example with pigweed RCP

          $ ./scripts/examples/gn_efr32_example.sh examples/window-app/efr32/ out/window_app_rpc BRD4161A 'import("//with_pw_rpc.gni")'

    or use GN/Ninja Directly

          $ cd ~/connectedhomeip/examples/window-app/efr32
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export EFR32_BOARD=BRD4161A
          $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
          $ ninja -C out/debug

    [Running Pigweed RPC console](#running-pigweed-rpc-console)

For more build options, help is provided when running the build script without
arguments

         ./scripts/examples/gn_efr32_example.sh

<a name="flashing"></a>

## Flashing the Application

-   On the command line:

          $ cd ~/connectedhomeip/examples/window-app/efr32
          $ python3 out/debug/chip-efr32-window-example.flash.py

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
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

    Take note that the RCP code is available directly through
    [Simplicity Studio 5](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5)
    under File->New->Project Wizard->Examples->Thread : ot-rcp

-   User interface : **LCD** The LCD on Silabs WSTK shows a QR Code. This QR
    Code is be scanned by the CHIP Tool app For the Rendez-vous procedure over
    BLE

        * On devices that do not have or support the LCD Display like the BRD4166A Thunderboard Sense 2,
          a URL can be found in the RTT logs.

          <info  > [SVR] Copy/paste the below URL in a browser to see the QR Code:
          <info  > [SVR] https://project-chip.github.io/connectedhomeip/qrcode.html?data=CH%3AI34NM%20-00%200C9SS0

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

    **LED 1** Shows the state of the window covering

        -   _Solid On_ ; The window cover if fully open
        -   _Off_ ; The window cover if fully closed
        -   _Blinking slowly_ ; The window cover is half-open, either by tilt, or lift
        -   _Blinking quickly_ ; The window cover is being automatically open or closed

    **Push Button 0** Increase either tilt or lift, and factory reset

        -   Pressed and release: The lift/tilt increases by 10%

        -   Pressed and hold for 6 s: Initiates the factory reset of the device.
            Releasing the button within the 6-second window cancels the factory reset
            procedure. **LEDs** blink in unison when the factory reset procedure is
            initiated.

    **Push Button 1** Decreases either tilt or lift, or switch the cover type

        -   Pressed and release: The lift/tilt decreases by 10%

        -   Press and hold for 3 s: Cycle between window covering type (Rollershade, Drapery, Tilt Blind - Lift and Tilt).

    **Push Button0 and Button1** Switch between lift and tilt

        - Pressing and release both buttons at the same time: switches between lift and tilt modes. Most window covering types support either lift only, or tilt only, but type 0x08 support both (default)

        - Pressing and hold both buttons at the same time: Cycles between window covering 1, and window covering 2.

*   Once the device is provisioned, it will join the Thread network is
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

    (you can verify that the device is on the thread network with the command
    `router table` using a serial terminal (screen / minicom etc.) on the board
    running the window-app example. You can also get the address list with the
    command ipaddr again in the serial terminal )

    You can provision the Chip device using Chip tool Android or iOS app or
    through CLI commands on your OT BR

    The
    [CHIPTool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
    can now be used to send ZCL commands to the window covering device. For
    instance, to set the window covering lift by percentage:

    ```
    chip-tool pairing ble-thread 1 hex:<operationalDataset> 20202021 3840

    chip-tool onoff on 1 1

    chip-tool windowcovering go-to-tilt-percentage 50 0 1 1
    ```

    To see the supported window covering cluster commands, use:

    ```
    chip-tool windowcovering
    ```

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

<a name="running-pigweed-rpc-console"></a>

## OTA Software Update

For the description of Software Update process with EFR32 example applications
see
[EFR32 OTA Software Update](../../../docs/guides/silabs_efr32_software_update.md)

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options :

### Disabling logging

chip_progress_logging, chip_detail_logging, chip_automation_logging

    $ ./scripts/examples/gn_efr32_example.sh ./examples/lighting-app/efr32 ./out/lighting-app BRD4164A "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"

### Debug build / release build

is_debug

    $ ./scripts/examples/gn_efr32_example.sh ./examples/lighting-app/efr32 ./out/lighting-app BRD4164A "is_debug=false"

### Disabling LCD

show_qr_code

    $ ./scripts/examples/gn_efr32_example.sh ./examples/lighting-app/efr32 ./out/lighting-app BRD4164A "show_qr_code=false"

### KVS maximum entry count

kvs_max_entries

    Set the maximum Kvs entries that can be stored in NVM (Default 75)
    Thresholds: 30 <= kvs_max_entries <= 255

    $ ./scripts/examples/gn_efr32_example.sh ./examples/lighting-app/efr32 ./out/lighting-app BRD4164A kvs_max_entries=50
