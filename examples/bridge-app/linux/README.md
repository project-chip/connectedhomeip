# CHIP Linux Bridge Example

An example showing the use of CHIP on the Linux. The document will describe how
to build and run CHIP Linux Bridge Example on Raspberry Pi. This doc is tested
on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu for
Raspberry Pi Desktop 20.10 (aarch64)**

<hr>

-   [CHIP Linux Bridge Example](#chip-linux-bridge-example)
    -   [Building](#building)
    -   [Running the Complete Example on Raspberry Pi 4](#running-complete-example)

<hr>

<a name="building"></a>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/bridge-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/bridge-app/linux
          $ rm -rf out/

<a name="running-complete-example"></a>

## Running the Complete Example on Raspberry Pi 4

> If you want to test ZCL, please disable Rendezvous
>
>     gn gen out/debug --args='bypass_rendezvous=true'
>     ninja -C out/debug
>
> Note that GN will set bypass_rendezvous for future builds, to enable
> rendezvous, re-generate using
>
>     gn gen out/debug --args='chip_bypass_rendezvous=false'

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

        -   Run Linux Bridge Example App

                  $ cd ~/connectedhomeip/examples/bridge-app/linux
                  $ sudo out/debug/chip-tool-server --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/chip-tool-server --ble-device 1

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.
