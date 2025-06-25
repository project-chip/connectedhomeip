# Matter SiWx917 Closure Example

An example showing the use of CHIP on the Silicon Labs SiWx917

<hr>

-   [Matter SiWx917 Closure Example](#matter-siwx917-closure-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)
    -   [Group Communication (Multicast)](#group-communication-multicast)
    -   [Building options](#building-options)
        -   [Disabling logging](#disabling-logging)
        -   [Debug build / release build](#debug-build--release-build)
        -   [Disabling QR CODE](#disabling-qr-code)
        -   [KVS maximum entry count](#kvs-maximum-entry-count)

<hr>

> **NOTE:** Silicon Laboratories now maintains a public matter GitHub repo with
> frequent releases thoroughly tested and validated. Developers looking to
> develop matter products with silabs hardware are encouraged to use our latest
> release with added tools and documentation.
> [Silabs Matter Github](https://github.com/SiliconLabs/matter/releases)

## Introduction

The SiWx917 Closure example provides a baseline demonstration of a closure base
device type, built using Matter and the Silicon Labs gecko SDK. It can be
controlled by a Chip controller over Wifi network.

The SiWx917 device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
Rendez-vous procedure.

The Closure examples are intended to serve both as a means to explore the
workings of Matter Closure as well as a template for creating real products
based on the Silicon Labs platform.

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools (likely already present for CHIP developers):

    -   Linux: `sudo apt-get install git ninja-build`

    -   Mac OS X: `brew install ninja`

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/general/HARDWARE_REQUIREMENTS.md)
        > in the Silicon Labs Matter Github Repo

    917SoC boards :

    -   BRD4338A

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs/ ./out/closure-app BRD4338A

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

    OR use GN/Ninja directly

          $ cd ~/connectedhomeip/examples/closure-app/silabs
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export SILABS_BOARD=BRD4338A

    To build the Closure example

          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/closure-app/silabs
          $ rm -rf out/

For more build options, help is provided when running the build script without
arguments

         ./scripts/examples/gn_silabs_example.sh

## Flashing the Application

-   SiWx917 SoC device support is available in the latest Simplicity Commander.
    The SiWx917 SOC board will support .rps as the only file to flash.

-   Or with the Ozone debugger, just load the .out file.

All SiWx917 boards require a connectivity firmware, see Silicon Labs
documentation for more info. Pre-built bootloader binaries are available in the
Assets section of the Releases page on
[Silabs Matter Github](https://github.com/SiliconLabs/matter/releases) .

## Running the Complete Example

-   To run a Matter over Wi-Fi application, you must first create a Matter
    network using the chip-tool, and then control the Matter device from the
    chip-tool.

**Creating the Matter Network**

     This procedure uses the chip-tool installed on the Matter Hub. The commissioning procedure does the following:
     - Chip-tool scans BLE and locates the Silicon Labs device that uses the specified discriminator.
     - Establishes operational certificates.
     - Sends the Wi-Fi SSID and Passkey.
     - The Silicon Labs device will join the Wi-Fi network and get an IP address. It then starts providing mDNS records on IPv4 and IPv6.
     - Future communications (tests) will then happen over Wi-Fi.

-   You can provision and control the Chip device using the python controller,
    Chip tool standalone, Android or iOS app

    [CHIPTool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

    Here is an example with the chip-tool:

          $ chip-tool pairing ble-wifi 1 <SSID> <Password> 20202021 3840

*   User interface :

    **Push Button 0**

        -   _Press and Release_ : Start, or restart, BLE advertisement in fast mode. It will advertise in this mode
            for 30 seconds. The device will then switch to a slower interval advertisement.
            After 15 minutes, the advertisement stops.
            Additionally, it will cycle through the QR code, application status screen and device status screen, respectively.

        -   _Pressed and hold for 6 s_ : Initiates the factory reset of the device.
            Releasing the button within the 6-second window cancels the factory reset
            procedure. **LEDs** blink in unison when the factory reset procedure is
            initiated.

## Group Communication (Multicast)

With this Closure example you can also use group communication to send Closure
commands to multiples devices at once. Please refer to the
[chip-tool documentation](../../chip-tool/README.md) _Configuring the server
side for Group Commands_ and _Using the Client to Send Group (Multicast) Matter
Commands_

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options to be
passed to the build scripts.

### Disabling logging

`chip_progress_logging, chip_detail_logging, chip_automation_logging`

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"

### Debug build / release build

`is_debug`

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A "is_debug=false"

### Disabling QR CODE

show_qr_code

    $ ./scripts/examples/gn_silabs_example.sh ./examples/window-app/silabs ./out/window-app BRD4164A "show_qr_code=false"

### KVS maximum entry count

`kvs_max_entries`

    Set the maximum Kvs entries that can be stored in NVM (Default 255)
    Thresholds: 30 <= kvs_max_entries <= 511

    $ ./scripts/examples/gn_silabs_example.sh ./examples/closure-app/silabs ./out/closure-app BRD4338A kvs_max_entries=355
