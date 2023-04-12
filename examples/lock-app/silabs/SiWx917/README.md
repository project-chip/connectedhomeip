# Matter SiWx917 Lock Example

An example showing the use of CHIP on the Silicon Labs SiWx917 SoC device.

<hr>

-   [Matter SiWx917 Lock Example](#matter-siwx917-lock-example)
    -   [Introduction](#introduction)
    -   [Building](#building)
    -   [Flashing the Application](#flashing-the-application)
    -   [Viewing Logging Output](#viewing-logging-output)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Notes](#notes)
    -   [Memory settings](#memory-settings)
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

The SiWx917 Lock example provides a baseline demonstration of a door lock
control device, built using Matter, the Silicon Labs Gecko SDK, and the Silicon
Labs WiseMCU SDK. It can be controlled by a Chip controller over a Wi-Fi
network.

The SiWx917 device can be commissioned over Bluetooth Low Energy where the
device and the Chip controller will exchange security information with the
Rendez-vous procedure. Wi-Fi Network SSID and passcode are then provided to the
SiWx917 device which will then join the Wi-Fi network.

If the LCD is enabled, the LCD on the Silabs WSTK shows a QR Code containing the
needed commissioning information for the BLE connection and starting the
rendezvous procedure.

The lock example is intended to serve both as a means to explore the workings of
Matter as well as a template for creating real products based on the Silicon
Labs platform.

## Building

-   Download the
    [Simplicity Commander](https://www.silabs.com/mcu/programming-options)
    command line tool, and ensure that `commander` is your shell search path.
    (For Mac OS X, `commander` is located inside
    `Commander.app/Contents/MacOS/`.)

-   Download and install a suitable ARM gcc tool chain:
    [GNU Arm Embedded Toolchain 9-2019-q4-major](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)

-   Install some additional tools(likely already present for CHIP developers):

Linux

    sudo apt-get install git ninja-build

Mac OS X

    brew install ninja

-   Supported hardware:

    -   > For the latest supported hardware please refer to the
        > [Hardware Requirements](https://github.com/SiliconLabs/matter/blob/latest/docs/silabs/general/HARDWARE_REQUIREMENTS.md)
        > in the Silicon Labs Matter Github Repo

*   Build the example application:

          ```
          cd ~/connectedhomeip
          ./scripts/examples/gn_efr32_example.sh ./examples/lock-app/silabs/SiWx917/ ./out/lock_app BRD4325B
          ```

-   To delete generated executable, libraries and object files use:

          ```
          $ cd ~/connectedhomeip
          $ rm -rf ./out/
          ```

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

-   You can provision and control the Chip device using the chip-tool standalone

    [chip-tool](https://github.com/project-chip/connectedhomeip/blob/master/examples/chip-tool/README.md)

Here are some chip-tool examples:

    Pairing with chip-tool:
    ```
    chip-tool pairing ble-wifi 1122 $SSID $PSK 20202021 3840
    ```
    -   > $SSID and $PSK are the SSID and passcode of your Wi-Fi Access Point.

    Set a user:
    ```
    ./out/chip-tool doorlock set-user OperationType UserIndex UserName UserUniqueId UserStatus UserType CredentialRule node-id/group-id
    ./out/chip-tool doorlock set-user 0 1 "mike" 5 1 0 0 1 1 --timedInteractionTimeoutMs 1000
    ```

    Set a credential:
    ```
    ./out/chip-tool doorlock set-credential OperationType Credential CredentialData UserIndex UserStatus UserType node-id/group-id
    ./out/chip-tool doorlock set-credential 0 '{ "credentialType": 1, "credentialIndex": 1 }' "123456" 1 null null 1 1 --timedInteractionTimeoutMs 1000
    ```

    Changing a credential:
    ```
    ./out/chip-tool doorlock set-credential OperationType Credential CredentialData UserIndex UserStatus UserType node-id/group-id
    ./out/chip-tool doorlock set-credential 2 '{ "credentialType": 1, "credentialIndex": 1 }' "123457" 1 null null 1 1 --timedInteractionTimeoutMs 1000
    ```

    Get a user:
    ```
    ./out/chip-tool doorlock get-user UserIndex node-id/group-id
    ./out/chip-tool doorlock get-user 1 1 1
    ```

    Unlock door:
    ```
    ./out/chip-tool doorlock unlock-door node-id/group-id
    ./out/chip-tool doorlock unlock-door 1 1
    ```

    Lock door:
    ```
    ./out/chip-tool doorlock lock-door node-id/group-id
    ./out/chip-tool doorlock lock-door 1 1
    ```

### Notes

-   Depending on your network settings your router might not provide native IPv6
    addresses to your devices (Router / PC). If this is the case, you need to
    add a static IPv6 addresses on both devices and then an IPv6 route to the
    router on your PC

#On PC(Linux): \$ sudo ip addr add dev <Network interface> 2002::1/64

#Add IPv6 route on PC(Linux) \$ sudo ip route add <Router global IPv6 prefix>/64
via 2002::2

## Memory settings

While most of the RAM usage in CHIP is static, allowing easier debugging and
optimization with symbols analysis, we still need some HEAP for the crypto and
Wi-Fi stack. Size of the HEAP can be modified by changing the value of the
`configTOTAL_HEAP_SIZE` define inside of the FreeRTOSConfig.h file of this
example. Please take note that a HEAP size smaller than 13k can and will cause
an Mbedtls failure during the BLE rendezvous or CASE session

To track memory usage you can set `enable_heap_monitoring = true` either in the
BUILD.gn file or pass it as a build argument to gn. This will print on the RTT
console the RAM usage of each individual task and the number of Memory
allocation and Free. While this is not extensive monitoring you're welcome to
modify `examples/platform/silabs/SiWx917/MemMonitoring.cpp` to add your own
memory tracking code inside the `trackAlloc` and `trackFree` function

## Building options

All of Silabs's examples within the Matter repo have all the features enabled by
default, as to provide the best end user experience. However some of those
features can easily be toggled on or off. Here is a short list of options :

### Disabling logging

`chip_progress_logging, chip_detail_logging, chip_automation_logging`

    ```
    $ ./scripts/examples/gn_efr32_example.sh ./examples/lock-app/silabs/SiWx917 ./out/lock-app BRD4325B "chip_detail_logging=false chip_automation_logging=false chip_progress_logging=false"
    ```

### Debug build / release build

`is_debug`

    ```
    $ ./scripts/examples/gn_efr32_example.sh ./examples/lock-app/silabs/SiWx917 ./out/lock-app BRD4325B "is_debug=false"
    ```

### Disabling LCD

`show_qr_code`

    ```
    $ ./scripts/examples/gn_efr32_example.sh ./examples/lock-app/silabs/SiWx917 ./out/lock-app BRD4325B "show_qr_code=false"
    ```

### KVS maximum entry count

`kvs_max_entries`

    ```
    Set the maximum Kvs entries that can be stored in NVM (Default 75)
    Thresholds: 30 <= kvs_max_entries <= 255

    $ ./scripts/examples/gn_efr32_example.sh ./examples/lock-app/silabs/SiWx917 ./out/lock-app BRD4325B kvs_max_entries=50
    ```
