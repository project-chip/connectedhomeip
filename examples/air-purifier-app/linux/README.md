# CHIP Linux Air Purifier Example

An example showing the use of CHIP on the Linux. The document will describe how
to build and run CHIP Air Purifier Example on A Linux System. This doc is tested
on **Ubuntu 20.04 LTS**.

The Air Purifier example demonstrates a fully functional Matter Air Purifier
which is a composed device with Endpoint 1 being the Air Purifier. Endpoint 2 is
an Air Quality Sensor, Endpoint 3 is a Relative Humidity Sensor, Endpoint 4 is a
Temperature Sensor and Endpoint 5 is a Thermostat.

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/platforms/nxp/nxp_imx8m_linux_examples.md) for
details.

<hr>

-   [CHIP Linux Air Purifier Example](#chip-linux-air-purifier-example)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/air-purifier-app/linux/
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/air-purifier-app/linux/
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
    3. Ubuntu 20.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth controller selector
            as described in
            [Linux BLE Settings](/platforms/linux/ble_settings.md).

    -   Run Linux Air Purifier Example App

              $ cd ~/connectedhomeip/examples/air-purifier-app/linux
              $ sudo out/debug/chip-air-purifier-app --ble-controller [bluetooth controller number]
              # In this example, the device we want to use is hci1
              $ sudo out/debug/chip-air-purifier-app --ble-controller 1

    -   Test the device using ChipTool on your laptop / workstation etc.
