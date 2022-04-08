# CHIP Linux Lighting Example

An example showing the use of CHIP on the Linux. The document will describe how
to build and run CHIP Linux Lighting Example on Raspberry Pi. This doc is tested
on **Ubuntu for Raspberry Pi Server 20.04 LTS (aarch64)** and **Ubuntu for
Raspberry Pi Desktop 20.10 (aarch64)**

To cross-compile this example on x64 host and run on **NXP i.MX 8M Mini**
**EVK**, see the associated
[README document](../../../docs/guides/nxp_imx8m_linux_examples.md) for details.

<hr>

-   [CHIP Linux Lighting Example](#chip-linux-lighting-example)
    -   [Building](#building)
    -   [Commandline Arguments](#command-line-args)
    -   [Running the Complete Example on Raspberry Pi 4](#running-complete-example)
    -   [Running RPC console](#running-rpc-console)
    -   [Device Tracing](#device-tracing)

<hr>

<a name="building"></a>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/lighting-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/lighting-app/linux
          $ rm -rf out/

-   Build the example with pigweed RPC

          $ cd ~/connectedhomeip/examples/lighting-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug --args='import("//with_pw_rpc.gni")'
          $ ninja -C out/debug

<a name="command-line-args"></a>

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

<a name="running-complete-example"></a>

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

        -   Run Linux Lighting Example App

                  $ cd ~/connectedhomeip/examples/lighting-app/linux
                  $ sudo out/debug/chip-lighting-app --ble-device [bluetooth device number]
                  # In this example, the device we want to use is hci1
                  $ sudo out/debug/chip-lighting-app --ble-device 1

        -   Test the device using ChipDeviceController on your laptop /
            workstation etc.

## Running RPC Console

-   As part of building the example with RPCs enabled the chip_rpc python
    interactive console is installed into your venv. The python wheel files are
    also created in the output folder: out/debug/chip_rpc_console_wheels. To
    install the wheel files without rebuilding:
    `pip3 install out/debug/chip_rpc_console_wheels/*.whl`

-   To use the chip-rpc console after it has been installed run:
    `chip-console -s localhost:33000 -o /<YourFolder>/pw_log.out`

-   Then you can Get and Set the light using the RPCs:
    `rpcs.chip.rpc.Lighting.Get()`

    `rpcs.chip.rpc.Lighting.Set(on=True, level=128, color=protos.chip.rpc.LightingColor(hue=5, saturation=5))`

## Device Tracing

Device tracing is available to analyze the device performance. To turn on
tracing, build with RPC enabled. See [Building with RPC enabled](#building).

Obtain tracing json file.

```
    $ ./{PIGWEED_REPO}/pw_trace_tokenized/py/pw_trace_tokenized/get_trace.py -s localhost:33000 \
     -o {OUTPUT_FILE} -t {ELF_FILE} {PIGWEED_REPO}/pw_trace_tokenized/pw_trace_protos/trace_rpc.proto
```
