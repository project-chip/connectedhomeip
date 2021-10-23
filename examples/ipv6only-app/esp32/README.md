# CHIP ESP32 IPV6 Only Example Application

This application implements ESP32 wifi control to support IPV6 tests.

Once connected the application acts as a UDP echo server and will echo udp
messages it receives, this can be used to test for disconnect events.

-   [CHIP ESP32 IPV6 Only Example Application](#chip-esp32-ipv6-only-example-application)
    -   [Building the Example Application](#building-the-example-application)
        -   [To build the application, follow these steps:](#to-build-the-application-follow-these-steps)
    -   [Testing the Example Application](#testing-the-example-application)

---

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout release/v4.1 branch

          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout release/v4.3
          $ git submodule update --init
          $ ./install.sh

-   Install ninja-build

          $ sudo apt-get install ninja-build

### To build the application, follow these steps:

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   Setting up the environment

        $ cd ${HOME}/tools/esp-idf
        $ ./install.sh
        $ . ./export.sh
        $ cd {path-to-connectedhomeip}

    To download and install packages.

        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh

    If packages are already installed then simply activate them.

        $ source ./scripts/activate.sh

-   Configuration Options

        To choose from the different configuration options, run menuconfig

          $ idf.py menuconfig

        This example uses UART0 for serial communication. You can change this through
        `PW RPC Example Configuration`. As a result, the console has been shifted to UART1
        You can change this through `Component config` -> `Common ESP-related` ->
        `UART for console output`

-   Build the demo application.

          $ idf.py build

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

          $ idf.py flash -p /dev/tty.SLAB_USBtoUART

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

## Testing the Example Application

Build or install the [rpc console](../../common/pigweed/rpc_console/README.md)

Start the console:

    $ python -m chip_rpc.console --device /dev/ttyUSB0 -b 115200

An example flow of performing a scan, connecting, and getting the IPv6 address:

    scan = rpcs.chip.rpc.Wifi.StartScan(pw_rpc_timeout_s=5)
    ap = next(filter(lambda a: b"SSID\000" in a.ssid, next(scan.responses()).aps))

    connect = protos.chip.rpc.ConnectionData(ssid=ap.ssid,security_type=ap.security_type,secret=b"PASSWORD")
    rpcs.chip.rpc.Wifi.Connect(connect, pw_rpc_timeout_s=10)

    rpcs.chip.rpc.Wifi.GetIP6Address()
