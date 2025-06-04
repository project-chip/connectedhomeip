# CHIP Linux Closure Example App

An example showing the use of Matter on the Linux. The document will describe
how to build and run Matter Linux Closure Example on Raspberry Pi. This doc is
tested on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu
for Raspberry Pi Desktop 20.10 (aarch64)**

<hr>

-   [CHIP Linux Closure Example App](#chip-linux-closure-example-app)
    -   [Building](#building)
    -   [Commandline arguments](#commandline-arguments)
    -   [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)

<hr>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/closure-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/closure-app/linux
          $ rm -rf out/

-   Build the example with pigweed RPC

          $ cd ~/connectedhomeip/examples/closure-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
          $ ninja -C out/debug

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

>     gn gen out/debug
>     ninja -C out/debug

-   Prerequisites

    1. A Raspberry Pi 4 board
    2. A USB Bluetooth Dongle, Ubuntu desktop will send Bluetooth advertisement,
       which will block Matter from connecting via BLE. On Ubuntu server, you
       need to install `pi-bluetooth` via APT.
    3. Ubuntu 20.04 or newer image for ARM64 platform.

-   Building

    Follow [Building](#building) section of this document.

-   Running

    -   [Optional] Plug USB Bluetooth dongle

        -   Plug USB Bluetooth dongle and find its bluetooth controller selector
            as described in
            [Linux BLE Settings](/platforms/linux/ble_settings.md).

    -   Run Linux Closure Example App

              $ cd ~/connectedhomeip/examples/closure-app/linux
              $ sudo out/debug/closure-app --ble-controller [bluetooth controller number]
              # In this example, the device we want to use is hci1
              $ sudo out/debug/closure-app --ble-controller 1

    -   Test the device using ChipDeviceController on your laptop / workstation
        etc.
