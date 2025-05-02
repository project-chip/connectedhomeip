# CHIP Linux Closure Example App

An example showing the use of Matter on the Linux. The document will describe
how to build and run Matter Linux Closure Example on Raspberry Pi. This doc
is tested on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu
for Raspberry Pi Desktop 20.10 (aarch64)**

<hr>

- [CHIP Linux Closure Example App](#chip-linux-closure-example-app)
  - [Building](#building)
  - [Commandline arguments](#commandline-arguments)
  - [Running the Complete Example on Raspberry Pi 4](#running-the-complete-example-on-raspberry-pi-4)

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
TBD