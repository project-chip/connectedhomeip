# CHIP ESP32 Pigweed Example Application

The ESP32 example demonstrates the usage of Pigweed module functionalities in an
application.

The example is based on [CHIP](https://github.com/project-chip/connectedhomeip),
the [Pigweed](https://pigweed.googlesource.com/pigweed/pigweed) module, which is
a collection of libraries that provide different functionalities for embedded
systems, and the ESP32 platform.

You can use this example as a training ground for making experiments, testing
Pigweed module features and checking what actions are necessary to fully
integrate Pigweed in a CHIP project.

Pigweed functionalities are being gradually integrated into CHIP. Currently, the
following features are available:

-   **Echo RPC** - Creates a Remote Procedure Call server and allows sending
    commands through the serial port to the device, which makes echo and sends
    the received commands back.

---

-   [CHIP ESP32 Pigweed Example Application](#chip-esp32-pigweed-example-application)
    -   [Building the Example Application](#building-the-example-application)
    -   [Testing the Example Application](#testing-the-example-application)

---

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.3 tag](https://github.com/espressif/esp-idf/releases/v4.3)

          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout v4.3
          $ git submodule update --init
          $ ./install.sh

-   Install ninja-build

          $ sudo apt-get install ninja-build

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   Setting up the environment

To download and install packages.

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

        To choose from the different configuration options, run menuconfig.

          $ idf.py menuconfig

        This example uses UART0 for serial communication. You can change this through
        `PW RPC Example Configuration`. As a result, the console has been shifted to UART1
        You can change this through `Component config` -> `Common ESP-related` ->
        `UART for console output`

-   To build the demo application.

          $ idf.py build

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

          $ idf.py -p /dev/tty.SLAB_USBtoUART build flash

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

### Flashing app using script

-   Follow these steps to use `${app_name}.flash.py`.

    -   First set IDF target, run set-target with one of the commands.

            $ idf.py set-target esp32
            $ idf.py set-target esp32c3

    -   Execute below sequence of commands

```
        $ export ESPPORT=/dev/tty.SLAB_USBtoUART
        $ export ESPBAUD=${baud_value}
        $ idf.py build
        $ idf.py flashing_script
        $ python ${app_name}.flash.py
```

## Testing the Example Application

Run the following command to start an interactive Python shell, where the Echo
RPC commands can be invoked:

        python -m pw_hdlc.rpc_console --device /dev/tty.SLAB_USBtoUART -b 115200 $CHIP_ROOT/third_party/pigweed/repo/pw_rpc/pw_rpc_protos/echo.proto -o /tmp/pw_rpc.out

To send an Echo RPC message, type the following command, where the actual
message is the text in quotation marks after the `msg=` phrase:

        rpcs.pw.rpc.EchoService.Echo(msg="hi")
