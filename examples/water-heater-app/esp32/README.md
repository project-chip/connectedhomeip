# Matter ESP32 Water Heater Example

This example demonstrates the Matter Water Heater application on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Water Heater Example](#water-heater-example)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning and Control](#commissioning-and-control)

---

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain for ESP32 modules or
the riscv-esp32-elf toolchain for ESP32C3 modules.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout the appropriate release

            $ mkdir ${HOME}/tools
            $ cd ${HOME}/tools
            $ git clone https://github.com/espressif/esp-idf.git
            $ cd esp-idf
            $ git checkout v5.1.2
            $ git submodule update --init
            $ ./install.sh

-   Install ninja-build

            $ sudo apt-get install ninja-build

Currently building in VSCode using ESP-IDF Visual Studio Code Extension v1.6.0
is not supported, please build the application using idf.py script.

### To build the application, follow these steps:

-   If you are using a devcontainer, activate the Matter environment by running
    the following command:

            $ source scripts/activate.sh

-   If you are not using a devcontainer, activate the ESP-IDF environment by
    running the following command:

            $ cd ${HOME}
            $ mkdir -p ${HOME}/tools
            $ cd ${HOME}/tools
            $ git clone https://github.com/espressif/esp-idf.git
            $ cd esp-idf
            $ git checkout v5.1.2
            $ git submodule update --init
            $ export IDF_PATH=${HOME}/tools/esp-idf
            $ ./install.sh
            $ . ./export.sh

-   In the root of the example directory, sync submodules

            $ cd examples/water-heater-app/esp32
            $ git submodule update --init

-   Build the example:

            $ idf.py set-target {TARGET}
            $ idf.py build

    where {TARGET} can be `esp32`, `esp32c3`, `esp32c6`, `esp32h2`, etc.

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC, use `/dev/ttyUSB0` by default.

            $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

            $ idf.py -p /dev/tty.SLAB_USBtoUART monitor

## Commissioning and Control

Refer to the main [Water Heater README](../README.md) for general information
about the Water Heater application.

For commissioning and controlling the device, refer to the
[chip-tool documentation](../../chip-tool/README.md) or the examples in the main
[Water Heater Linux README](../linux/README.md#interaction-using-the-chip-tool-and-testeventtriggers).
