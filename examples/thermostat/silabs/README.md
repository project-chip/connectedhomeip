# Matter EFR32 Thermostat Example

An example showing the use of CHIP on the Silicon Labs EFR32 MG24.

<hr>

-   [Matter EFR32 Thermostat Example](#matter-efr32-thermostat-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
        -   [Linux](#linux)
        -   [Mac OS X](#mac-os-x)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)
            -   [On Border Router:](#on-border-router)
            -   [On PC(Linux):](#on-pclinux)
    -   [Running RPC console](#running-rpc-console)
    -   [Memory settings](#memory-settings)
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

## Introduction

The EFR32 Thermostat example provides a baseline demonstration of a thermostat
device, built using Matter and the Silicon Labs gecko SDK. It can be controlled
by a Chip controller over an Openthread or Wifi network.

The EFR32 device can be commissioned over Bluetooth Low Energy where the device
and the Chip controller will exchange security information with the Rendez-vous
procedure. If using Thread, Thread Network credentials are then provided to the
EFR32 device which will then join the Thread network.

If the LCD is enabled, the LCD on the Silabs WSTK shows a QR Code containing the
needed commissioning information for the BLE connection and starting the
Rendez-vous procedure.

The thermostat example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
Silicon Labs platform.

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain (For most Host, the
    bootstrap already installs the toolchain):
    [GNU Arm Embedded Toolchain 12.2 Rel1](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

-   Install some additional tools(likely already present for CHIP developers):

#### Linux

    $ sudo apt-get install git ninja-build

#### Mac OS X

    $ brew install ninja

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/general/HARDWARE_REQUIREMENTS.md)
        > in the Silicon Labs Matter Github Repo

    MG24 boards :

    -   BRD2601B / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD2703A / SLWSTK6000B / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4186C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@10dBm
    -   BRD4187A / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm
    -   BRD4187C / SLWSTK6006A / Wireless Starter Kit / 2.4GHz@20dBm

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_silabs_example.sh ./examples/thermostat/silabs/ ./out/thermostat-app BRD4187C

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

    OR use GN/Ninja directly

          $ cd ~/connectedhomeip/examples/thermostat/silabs
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export SILABS_BOARD=BRD4187C
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/thermostat/silabs
          $ rm -rf out/

*   Build the example with Matter shell

          ./scripts/examples/gn_silabs_example.sh examples/thermostat/silabs/ out/thermostat-app BRD4187C chip_build_libshell=true

*   Build the example as Intermittently Connected Device (ICD)

          $ ./scripts/examples/gn_silabs_example.sh ./examples/thermostat/silabs/ ./out/thermostat-app_ICD BRD4187C --icd

    or use gn as previously mentioned but adding the following arguments:

          $ gn gen out/debug '--args=SILABS_BOARD="BRD4187C" enable_sleepy_device=true chip_openthread_ftd=false chip_build_libshell=true'

*   Build the example with pigweed RCP

          $ ./scripts/examples/gn_silabs_example.sh examples/thermostat/silabs/ out/thermostat-app_rpc BRD4187C 'import("//with_pw_rpc.gni")'

    or use GN/Ninja Directly

          $ cd ~/connectedhomeip/examples/thermostat/silabs
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export SILABS_BOARD=BRD4187C
          $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
          $ ninja -C out/debug

For more build options, help is provided when running the build script without
arguments

    ./scripts/examples/gn_silabs_example.sh

## Flashing the Application

-   On the command line:

          $ cd ~/connectedhomeip/examples/thermostat/silabs
          $ python3 out/debug/matter-silabs-thermostat-switch-example.flash.py

-   Or with the Ozone debugger, just load the .out file.

All EFR32 boards require a bootloader, see Silicon Labs documentation for more
info. Pre-built bootloader binaries are available in the Assets section of the
Releases page on
[Silabs Matter Github](https://github.com/SiliconLabs/matter/releases) .

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

    For MG24 use:

          ```
          $ JLinkExe -device EFR32MG24AXXXF1536 -if SWD -speed 4000 -autoconnect 1
          ```

-   In a second terminal, run the JLinkRTTClient to view logs:

          $ JLinkRTTClient

## Running the Complete Example

-   It is assumed here that you already have an OpenThread border router
    configured and running. If not see the following guide
    [Openthread_border_router](https://github.com/project-chip/connectedhomeip/blob/master/docs/platforms/openthread/openthread_border_router_pi.md)
    for more information on how to setup a border router on a raspberryPi.

    Take note that the RCP code is available directly through
    [Simplicity Studio 5](https://www.silabs.com/products/development-tools/software/simplicity-studio/simplicity-studio-5)
    under File->New->Project Wizard->Examples->Thread : ot-rcp

-   For this example to work, it is necessary to have a second efr32 device
    running the
    [thermostat app example](https://github.com/project-chip/connectedhomeip/blob/master/examples/thermostat/silabs/README.md)
    commissioned on the same openthread network

-   User interface : **LCD** The LCD on Silabs WSTK shows a QR Code. This QR
    Code is be scanned by the CHIP Tool app For the Rendez-vous procedure over
    BLE

        * On devices that do not have or support the LCD Display like the BRD4166A Thunderboard Sense 2,
          a URL can be found in the RTT logs.

          <info  > [SVR] Copy/paste the below URL in a browser to see the QR Code:
          <info  > [SVR] https://project-chip.github.io/connectedhomeip/qrcode.html?data=CH%3AI34NM%20-00%200C9SS0

    **LED 0** shows the overall state of the device and its connectivity. The
    following states are possible:

        -   Short Flash On (50 ms on/950 ms off): The device is in the
            unprovisioned (unpaired) state and is waiting for a commissioning
            application to connect.

        -   Rapid Even Flashing (100 ms on/100 ms off): The device is in the
            unprovisioned state and a commissioning application is connected through
            Bluetooth LE.

        -   Short Flash Off (950ms on/50ms off): The device is fully
            provisioned, but does not yet have full Thread network or service
            connectivity.

        -   Solid On: The device is fully provisioned and has full Thread
            network and service connectivity.

    **Push Button 0**

        -   _Press and Release_ : Start, or restart, BLE advertisement in fast mode. It will advertise in this mode
            for 30 seconds. The device will then switch to a slower interval advertisement.
            After 15 minutes, the advertisement stops.
            Additionally, it will cycle through the QR code, application status screen and device status screen, respectively.

        -   _Pressed and hold for 6 s_ : Initiates the factory reset of the device.
            Releasing the button within the 6-second window cancels the factory reset
            procedure. **LEDs** blink in unison when the factory reset procedure is
            initiated.

*   You can provision and control the Chip device using the python controller,
    [CHIPTool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
    standalone, Android or iOS app

    Here is an example with the CHIPTool:

    ```
    chip-tool pairing ble-thread 1 hex:<operationalDataset> 20202021 3840
    ```

### Notes

-   Depending on your network settings your router might not provide native ipv6
    addresses to your devices (Border router / PC). If this is the case, you
    need to add a static ipv6 addresses on both device and then an ipv6 route to
    the border router on your PC

#### On Border Router:

`$ sudo ip addr add dev <Network interface> 2002::2/64`

#### On PC(Linux):

`$ sudo ip addr add dev <Network interface> 2002::1/64`

#Add Ipv6 route on PC(Linux) \$ sudo ip route add <Thread global ipv6 prefix>/64
via 2002::2

## Running RPC console

-   As part of building the example with RPCs enabled the chip_rpc python
    interactive console is installed into your venv. The python wheel files are
    also created in the output folder: out/debug/chip_rpc_console_wheels. To
    install the wheel files without rebuilding:

    `pip3 install out/debug/chip_rpc_console_wheels/*.whl`

-   To use the chip-rpc console after it has been installed run:

    `chip-console --device /dev/tty.<SERIALDEVICE> -b 115200 -o /<YourFolder>/pw_log.out`

-   Then you can simulate a button press or release using the following command
    where : idx = 0 or 1 for Button PB0 or PB1 action = 0 for PRESSED, 1 for
    RELEASE Test toggling the LED with

    `rpcs.chip.rpc.Button.Event(idx=1, pushed=True)`

## Memory settings

While most of the RAM usage in CHIP is static, allowing easier debugging and
optimization with symbols analysis, we still need some HEAP for the crypto and
OpenThread. Size of the HEAP can be modified by changing the value of the
`configTOTAL_HEAP_SIZE` define inside of the FreeRTOSConfig.h file of this
example. Please take note that a HEAP size smaller than 13k can and will cause a
Mbedtls failure during the BLE rendez-vous or CASE session

To track memory usage you can set `enable_heap_monitoring = true` either in the
BUILD.gn file or pass it as a build argument to gn. This will print on the RTT
console the RAM usage of each individual task and the number of Memory
allocation and Free. While this is not extensive monitoring you're welcome to
modify `examples/platform/efr32/MemMonitoring.cpp` to add your own memory
tracking code inside the `trackAlloc` and `trackFree` function

## OTA Software Update

For the description of Software Update process with EFR32 example applications
see
[EFR32 OTA Software Update](../../../docs/platforms/silabs/silabs_efr32_software_update.md)

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options :

### Disabling logging

chip_progress_logging, chip_detail_logging, chip_automation_logging

    $ ./scripts/examples/gn_silabs_example.sh ./examples/thermostat/silabs ./out/thermostat-app BRD4164A "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"

### Debug build / release build

is_debug

    $ ./scripts/examples/gn_silabs_example.sh ./examples/thermostat/silabs ./out/thermostat-app BRD4164A "is_debug=false"

### Disabling LCD

show_qr_code

    $ ./scripts/examples/gn_silabs_example.sh ./examples/thermostat/silabs ./out/thermostat-app BRD4164A "show_qr_code=false"

### KVS maximum entry count

kvs_max_entries

    Set the maximum Kvs entries that can be stored in NVM (Default 75)
    Thresholds: 30 <= kvs_max_entries <= 255

    $ ./scripts/examples/gn_silabs_example.sh ./examples/thermostat/silabs ./out/thermostat-app BRD4164A kvs_max_entries=50
