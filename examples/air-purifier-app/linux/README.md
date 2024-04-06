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
[README document](../../../docs/guides/nxp_imx8m_linux_examples.md) for details.

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

-   `--ble-device <interface id>`

    Use specific bluetooth interface for BLE advertisement and connections.

    `interface id`: the number after `hci` when listing BLE interfaces by
    `hciconfig` command, for example, `--ble-device 1` means using `hci1`
    interface. Default: `0`.

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

        -   Plug USB Bluetooth dongle and find its bluetooth device number. The
            number after `hci` is the bluetooth device number, `1` in this
            example.

                  $ hciconfig
                  hci1:	Type: Primary  Bus: USB
                      BD Address: 00:1A:7D:AA:BB:CC  ACL MTU: 310:10  SCO MTU: 64:8
                      UP RUNNING PSCAN ISCAN
                      RX bytes:20942 acl:1023 sco:0 events:1140 errors:0
                      TX bytes:16559 acl:1011 sco:0 commands:121 errors:0

                  hci0:	Type: Primary  Bus: UART
                      BD Address: B8:27:EB:AA:BB:CC  ACL MTU: 1021:8  SCO MTU: 64:1
                      UP RUNNING PSCAN ISCAN
                      RX bytes:8609495 acl:14 sco:0 events:217484 errors:0
                      TX bytes:92185 acl:20 sco:0 commands:5259 errors:0

        -   Run Linux Air Purifier Example App

                  $ cd ~/connectedhomeip/examples/air-purifier-app/linux
                  $ sudo out/debug/chip-air-purifier-app --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/chip-air-purifier-app --ble-device 1

        -   Test the device using ChipTool on your laptop / workstation etc.
