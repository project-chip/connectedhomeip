# CHIP WiFi Echo Server Example

A prototype appplication that uses CHIP to setup WiFi on the ESP32 and runs an
Echo Server on a configured port. This example will evolve as more complex
messaging is supported in CHIP.

---

-   [CHIP WiFi Echo Server Example](#chip-wifi-echo-server-example)
-   [Supported Devices](#supported-devices)
-   [Building the Example Application](#building-the-example-application)
-   [Using the Echo Server](#using-the-echo-server)

---

## Supported Devices

The CHIP demo application is intended to work on two categories of ESP32
devices: the
[ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview),
and the [M5Stack](http://m5stack.com). Support for the
[M5Stack](http://m5stack.com) is still a Work in Progress.

## Building the Example Application

Building the example application requires the use of the Espressif ESP32 IoT
Development Framework and the xtensa-esp32-elf toolchain.

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Expressif ESP-IDF and checkout version 4.0

          $ mkdir ${HOME}/tools
          $ cd ${HOME}/tools
          $ git clone https://github.com/espressif/esp-idf.git
          $ cd esp-idf
          $ git checkout release/v4.0
          $ git submodule update --init
          $ export IDF_PATH=${HOME}/tools/esp-idf
          $ ./install.sh

### To build the application, follow these steps:

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   In the root of the example directory, source `idf.sh` and use the
    `defconfig` make target to configure the application with defaults.

          $ source idf.sh
          $ idf make defconfig

-   Run make to build the demo application

          $ idf make

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing.

          $  make flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

## Using the Echo Server

After the application has been flashed, connect the ESP32's Soft-AP. It's
usually something like `CHIP_DEMO-XXXX` where the last 4 digits are from the
device's MAC address.

Once you're connected, the server's IP can be found at the gateway address and
at the listed port number(Default: `8000`).

Then running the following command will ping the ESP32 and cause it to echo. If
necessary replace the `192.168.4.1` with the address printed by the device in
the monitor.

          $ echo "Hello over IP" | nc -w1 -u 192.168.4.1 8000
