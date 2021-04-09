# CHIP ESP32 Persistent Storage Example

An example testing and demonstrating the key value storage API.

<hr>

-   [CHIP ESP32 Persistent Storage Example](#chip-esp32-persistent-storage-example)
    -   [Introduction](#introduction)
    -   [ESP32](#esp32)
        -   [Building](#building)
        -   [Flashing the Application](#flashing-the-application)
        -   [Viewing Logging Output](#viewing-logging-output)

<hr>

<a name="introduction"></a>

## Introduction

This example serves to both test the key value storage implementation and API as
it is brought-up on different platforms, as well as provide an example for how
to use the API.

In the future this example can be moved into a unit test when available on all
platforms.

<a name="ESP32"></a>

## ESP32

The ESP32 platform KVS is not yet fully implemented. In particular offset and
partial reads are not yet supported.

<a name="building"></a>

### Building

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout release/v4.2 branch

          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout release/v4.2
          $ git submodule update --init
          $ export IDF_PATH=${HOME}/tools/esp-idf
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
        $ source ./scripts/bootstrap.sh
        $ source ./scripts/activate.sh
        $ cd {path-to-connectedhomeip-examples}

If packages are already installed then simply activate it.

        $ cd ${HOME}/tools/esp-idf
        $ ./install.sh
        $ . ./export.sh
        $ cd {path-to-connectedhomeip}
        $ source ./scripts/activate.sh
        $ cd {path-to-connectedhomeip-examples}

-   Configuration Options

        To choose from the different configuration options, run menuconfig

          $ idf.py menuconfig

-   To build the demo application.

          $ idf.py build

<a name="flashing"></a>

### Flashing the Application

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

          $ idf.py flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

<a name="view-logging"></a>

### Viewing Logging Output

The output of the test will be streamed out of the UART and can be observed with
whichever tool is typically used for monitoring this (eg, minicom on linux).
