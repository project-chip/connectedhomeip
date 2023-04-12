# Matter SiWx917 Window Covering Example

An example showing the use of CHIP on the Silicon Labs SiWx917 SoC device.

<hr>

-   [Matter SiWx917 Window Covering Example](#matter-siwx917-window-covering-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)
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

The SiWx917 window-covering example provides a baseline demonstration of a
Window Covering device, built using Matter, the Silicon Labs Gecko SDK, and the
Silicon Labs WiseMCU SDK. It can be controlled by a Chip controller over a Wi-Fi
network.

The SiWx917 device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
rendezvous procedure. The Wi-Fi Network credentials are provided to the SiWx917
device which will then join the Wi-Fi network.

If the LCD is enabled, the LCD on the Silabs WSTK shows a QR Code containing the
needed commissioning information for the BLE connection and starting the
rendezvous procedure. Once the device is commissioned, the displays shows a
representation of the window covering state.

The window-covering example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
Silicon Labs platform.

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

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_efr32_example.sh ./examples/window-app/silabs/SiWx917/ ./out/window-app BRD4325B

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

<a name="flashing"></a>

## Flashing the Application

-   Flashing requires the SiWx917 SoC device to be configured in the Ozone
    Debugger.
-   Once it's configured, it can be run with the Ozone Debugger by loading the
    .out file.
    -   > For detailed instructions, please refer to
        > [Running the Matter Demo on SiWx917 SoC](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/wifi/RUN_DEMO_SiWx917_SoC.md)
        > in the Silicon Labs Matter Github Repo

## Viewing Logging Output

The example application's logging output can be viewed in the Ozone Debugger.

## Running the Complete Example

    You can provision the [chip-tool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)
    and send ZCL commands to the window covering device. For
    instance, to set the window covering lift by percentage:

    -   > $SSID and $PSK are the SSID and passcode of your Wi-Fi Access Point.

    ```
    chip-tool pairing ble-wifi 1122 $SSID $PSK 20202021 3840

    chip-tool onoff on 1 1

    chip-tool windowcovering go-to-tilt-percentage 50 0 1 1
    ```

    To see the supported window covering cluster commands, use:

    ```
    chip-tool windowcovering
    ```

### Notes

-   Depending on your network settings your router might not provide native IPv6
    addresses to your devices (Router / PC). If this is the case, you need to
    add a static IPv6 addresses on both devices and then an IPv6 route to the
    border router on your PC

          # On PC (Linux) :
          $ sudo ip addr add dev <Network interface> 2002::1/64

          # Add IPv6 route on PC (Linux)
          $ sudo ip route add <Router global IPv6prefix>/64 via 2002::2

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options :

### Disabling logging

chip_progress_logging, chip_detail_logging, chip_automation_logging

    $ ./scripts/examples/gn_efr32_example.sh ./examples/window-app/silabs/SiWx917 ./out/window-app BRD4325B "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"

### Debug build / release build

is_debug

    $ ./scripts/examples/gn_efr32_example.sh ./examples/window-app/silabs/SiWx917 ./out/window-app BRD4325B "is_debug=false"

### Disabling LCD

show_qr_code

    $ ./scripts/examples/gn_efr32_example.sh ./examples/window-app/silabs/SiWx917 ./out/window-app BRD4325B "show_qr_code=false"

### KVS maximum entry count

kvs_max_entries

    Set the maximum Kvs entries that can be stored in NVM (Default 75)
    Thresholds: 30 <= kvs_max_entries <= 255

    $ ./scripts/examples/gn_efr32_example.sh ./examples/window-app/silabs/SiWx917 ./out/window-app BRD4325B kvs_max_entries=50
