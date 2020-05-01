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

          $ idf make flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

## Using the Echo Server

There are two ways to use the Echo Server running on the device.

### Connect the ESP32 to a 2.4GHz Network of your choice

1.  To connect the device to your network, give it the network details via the
    `menuconfig` target.

                $ idf make menuconfig

2.  While in the configurator, navigate to
    `Component Config`->`CHIP Device Layer`->`WiFi Station Options` and fill out
    the WiFi SSID and Password.

3.  Now flash the device with the same command as before. (Use the right `/dev`
    device)

                $ idf make flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

4.  The device should boot up and connect to your network. When that happens you
    will see a log like this in the monitor.

                I (5524) chip[DL]: SYSTEM_EVENT_STA_GOT_IP
                I (5524) chip[DL]: IPv4 address changed on WiFi station interface: <IP_ADDRESS>...

5.  Then running the following command will ping the ESP32 and cause it to echo.
    If necessary replace the `<IP_ADDRESS>` with the address printed by the
    device in the monitor.

                $ echo "Hello over IP" | nc -w1 -u 192.168.4.1 8000

Note: The ESP32 does not support 5GHz networks. Also, the Device will persist
your network configuration. To erase it, simply run.

                $ idf make erase_flash ESPPORT=/dev/tty.SLAB_USBtoUART

### Use the ESP32's Network

Alternatively, you can connect to the ESP32's Soft-AP directly.

1.  After the application has been flashed, connect the ESP32's Soft-AP. It's
    usually something like `CHIP_DEMO-XXXX` where the last 4 digits are from the
    device's MAC address.

2.  Once you're connected, the server's IP can be found at the gateway address
    and at the listed port number(Default: `8000`).

3.  Then running the following command will ping the ESP32 and cause it to echo.
    If necessary replace the `192.168.4.1` with the address printed by the
    device in the monitor.

              $ echo "Hello over IP" | nc -w1 -u 192.168.4.1 8000
