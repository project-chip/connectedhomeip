# Matter SiWx917 Lighting Example

An example showing the use of CHIP on the Silicon Labs SiWx917.

<hr>

- [Matter SiWx917 Lighting Example](#matter-siwx917-lighting-example)
  - [Introduction](#introduction)
  - [Building](#building)
  - [Flashing the Application](#flashing-the-application)
  - [Viewing Logging Output](#viewing-logging-output)
  - [Running the Complete Example](#running-the-complete-example)
    - [Notes](#notes)
  - [Memory settings](#memory-settings)
  - [Group Communication (Multicast)](#group-communication-multicast)
  - [Building options](#building-options)
    - [Disabling logging](#disabling-logging)
    - [Debug build / release build](#debug-build--release-build)
    - [Disabling LCD](#disabling-lcd)
    - [KVS maximum entry count](#kvs-maximum-entry-count)

<hr>

> **NOTE:** Silicon Laboratories now maintains a public matter GitHub repo with
> frequent releases thoroughly tested and validated. Developers looking to
> develop matter products with silabs hardware are encouraged to use our latest
> release with added tools and documentation.
> [Silabs Matter Github](https://github.com/SiliconLabs/matter/releases)

<a name="intro"></a>

## Introduction

The SiWx917 lighting example provides a baseline demonstration of a Light
control device, built using Matter, the Silicon Labs Gecko SDK, and the Silicon
Labs WiseMCU SDK. It can be controlled by a Chip controller over a Wi-Fi
network.

The SiWx917 device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
rendezvous procedure. Wi-Fi Network credentials are then provided to the SiWx917
device which will then join the Wi-Fi network.

If the LCD is enabled, the LCD on the Silabs WSTK shows a QR Code containing the
needed commissioning information for the BLE connection and starting the
rendezvous procedure.

The lighting example is intended to serve both as a means to explore the
workings of Matter as well as a template for creating real products based on the
Silicon Labs platform.

<a name="building"></a>

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is in your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM GCC tool chain:
    [GNU Arm Embedded Toolchain (arm-none-eabi)](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/downloads)

-   Install some additional tools (likely already present for CHIP developers):

    -   Linux: `sudo apt-get install git ninja-build`

    -   Mac OS X: `brew install ninja`

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/general/HARDWARE_REQUIREMENTS.md)
        > in the Silicon Labs Matter Github Repo

*   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_efr32_example.sh silabs_examples/Direct_Internet_lighting-app/SiWx917/ out/lighting-app BRD4325B

-   To delete generated executable, libraries and object files use:

          $ cd ~/matter
          $ rm -rf ./out/

    OR use GN/Ninja directly

          $ cd ~/matter/examples/lighting-app/efr32
          $ git submodule update --init
          $ source third_party/matter/scripts/activate.sh
          $ export EFR32_BOARD=BRD4161A
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/matter/examples/lighting-app/efr32
          $ rm -rf out/

*   Build the example as Sleepy End Device (SED)

          $ ./scripts/examples/gn_efr32_example.sh ./examples/lighting-app/efr32/ ./out/lighting-app_SED BRD4161A --sed

    or use gn as previously mentioned but adding the following arguments:

          $ gn gen out/debug '--args=silabs_board="BRD4161A" enable_sleepy_device=true chip_openthread_ftd=false'

*   Build the example with pigweed RPC

          $ ./scripts/examples/gn_efr32_example.sh silabs_examples/Direct_Internet_lighting-app/efr32/ out/lighting_app_rpc BRD4161A 'import("//with_pw_rpc.gni")'

    or use GN/Ninja Directly

          $ cd ~/matter/examples/lighting-app/efr32
          $ git submodule update --init
          $ source third_party/matter/scripts/activate.sh
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

          $ cd ~/matter/examples/lighting-app/efr32
          $ python3 out/debug/chip-efr32-lighting-example.flash.py

-   Or with the Ozone debugger, just load the .out file.

<a name="view-logging"></a>

## Viewing Logging Output

The example application's logging output can be viewed in the Ozone Debugger.

<a name="running-complete-example"></a>

## Running the Complete Example

-   You can provision and control the Chip device using the chip-tool standalone

    [chip-tool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

    Here is an example with the chip-tool:

    -   > $SSID and $PSK are the SSID and passcode of your Wi-Fi Access Point.

    ```
    chip-tool pairing ble-wifi 1122 $SSID $PSK 20202021 3840

    chip-tool onoff on 1 1
    ```

### Notes

-   Depending on your network settings your router might not provide native IPv6
    addresses to your devices (Router / PC). If this is the case, you need to
    add a static IPv6 addresses on both devices and then an IPv6 route to your
    router on your PC

    -   On PC(Linux): `sudo ip addr add dev <Network interface> 2002::1/64`

    -   Add IPv6 route on PC(Linux)
        `sudo ip route add <Router global IPv6 prefix>/64 via 2002::2`

## Memory settings

While most of the RAM usage in CHIP is static, allowing easier debugging and
optimization with symbols analysis, we still need some HEAP for the crypto and
Wi-Fi stack. Size of the HEAP can be modified by changing the value of the
`configTOTAL_HEAP_SIZE` define inside of the FreeRTOSConfig.h file of this
example. Please take note that a HEAP size smaller than 13k can and will cause a
Mbedtls failure during the BLE rendez-vous or CASE session

To track memory usage you can set `enable_heap_monitoring = true` either in the
BUILD.gn file or pass it as a build argument to gn. This will print on the RTT
console the RAM usage of each individual task and the number of Memory
allocation and Free. While this is not extensive monitoring you're welcome to
modify `examples/platform/silabs/SiWx917/MemMonitoring.cpp` to add your own
memory tracking code inside the `trackAlloc` and `trackFree` function

## Group Communication (Multicast)

With this lighting example you can also use group communication to send Lighting
commands to multiples devices at once. Please refer to the
[chip-tool documentation](../../../chip-tool/README.md) _Configuring the server
side for Group Commands_ and _Using the Client to Send Group (Multicast) Matter
Commands_

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options to be
passed to the build scripts.

### Disabling logging

`chip_progress_logging, chip_detail_logging, chip_automation_logging`

    $ ./scripts/examples/gn_efr32_example.sh ./silabs_examples/Direct_Internet_lighting-app/SiWx917 ./out/lighting-app BRD4325B "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"

### Debug build / release build

`is_debug`

    $ ./scripts/examples/gn_efr32_example.sh ./silabs_examples/Direct_Internet_lighting-app/SiWx917 ./out/lighting-app BRD4325B "is_debug=false"

### Disabling LCD

`show_qr_code`

    $ ./scripts/examples/gn_efr32_example.sh ./silabs_examples/Direct_Internet_lighting-app/
    /SiWx917 ./out/lighting-app BRD4325B "show_qr_code=false"

### KVS maximum entry count

`kvs_max_entries`

    Set the maximum Kvs entries that can be stored in NVM (Default 75)
    Thresholds: 30 <= kvs_max_entries <= 255

    $ ./scripts/examples/gn_efr32_example.sh ./silabs_examples/Direct_Internet_lighting-app/
    /SiWx917 ./out/lighting-app BRD4325B kvs_max_entries=50
