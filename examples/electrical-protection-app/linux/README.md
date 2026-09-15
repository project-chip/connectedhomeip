# CHIP Linux Electrical Protection Example

An example showing the use of CHIP on Linux for the Matter 1.7 electrical
protection device types. The app presents an Electrical Distribution Enclosure
and demonstrates the new Electrical Distribution cluster. The Electrical Circuit
Breaker endpoint and the remaining electrical clusters are added by follow-up
changes.

The document will describe how to build and run the CHIP Linux Electrical
Protection Example on Raspberry Pi. It targets a currently supported **Ubuntu
for Raspberry Pi Server LTS (aarch64)** release, 24.04 LTS or newer.

<hr>

-   [CHIP Linux Electrical Protection Example](#chip-linux-electrical-protection-example)
    -   [Device composition](#device-composition)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)

<hr>

## Device composition

-   **Endpoint 0**: Root Node.
-   **Endpoint 1**: Electrical Distribution Enclosure (0x0517), hosting
    Descriptor and Electrical Distribution.

Electrical Distribution is provisional in Matter 1.7.

The device type also defines Power Topology as mandatory, and the Electrical
Circuit Breaker endpoint it can contain is not present yet. Both arrive with the
follow-up changes that add the breaker endpoint and its clusters.

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Check out the repository and its submodules:

          $ cd ~
          $ git clone git@github.com:project-chip/connectedhomeip.git
          $ cd connectedhomeip
          $ ./scripts/checkout_submodules.py --platform linux --recursive

    Alternatively you can check out all submodules and resync with:

          $ git submodule sync --recursive; git submodule update --init --recursive

-   Activate at the top level, then build the example application:

          $ cd ~/connectedhomeip
          $ source ./scripts/activate.sh

          $ cd ~/connectedhomeip/examples/electrical-protection-app/linux
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/electrical-protection-app/linux
          $ rm -rf out/

## Commandline arguments

-   `--wifi`

    Enables WiFi management feature. Required for WiFi commissioning.

-   `--thread`

    Enables Thread management feature, requires ot-br-posix dbus daemon running.
    Required for Thread commissioning.

-   `--ble-controller <selector>`

    Use the specific Bluetooth controller for BLE advertisement and connections.
    For details on controller selection refer to
    [Linux BLE Settings](/platforms/linux/ble_settings.md).

## Running the Complete Example on Raspberry Pi 4

> If you want to test Echo protocol, please enable Echo handler
>
>     gn gen out/debug --args='chip_app_use_echo=true'
>     ninja -C out/debug

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block CHIP from connecting via BLE. On Ubuntu server, you need
       to install `pi-bluetooth` via APT.
    3. Ubuntu 24.04 LTS or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth controller selector
            as described in
            [Linux BLE Settings](/platforms/linux/ble_settings.md).

    -   Run Linux Electrical Protection Example App

              $ cd ~/connectedhomeip/examples/electrical-protection-app/linux
              $ sudo out/debug/chip-electrical-protection-app --ble-controller [bluetooth device number]
              # In this example, the device we want to use is hci1
              $ sudo out/debug/chip-electrical-protection-app --ble-controller 1

    -   Test the device using ChipDeviceController on your laptop / workstation
        etc.
