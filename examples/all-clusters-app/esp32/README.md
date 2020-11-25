# CHIP All Clusters Example

A prototype appplication that uses CHIP to setup WiFi on the ESP32 and runs an
Echo Server. This example will evolve as more complex messaging is supported in
CHIP.

---

-   [CHIP App Server Example](#chip-app-server-example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
        -   [To build the application, follow these steps:](#to-build-the-application-follow-these-steps)
    -   [Using the Echo Server](#using-the-echo-server)
        -   [Connect the ESP32 to a 2.4GHz Network of your choice](#connect-the-esp32-to-a-24ghz-network-of-your-choice)
        -   [Use the ESP32's Network](#use-the-esp32s-network)

---

## Supported Devices

The CHIP demo application is intended to work on two categories of ESP32
devices: the
[ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview),
and the [M5Stack](http://m5stack.com). On the [M5Stack](http://m5stack.com) this
example displays a CHIP QRCode with the device's Soft-AP SSID encoded in the TLV
section.

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
          $ git checkout release/v4.1
          $ git submodule update --init
          $ export IDF_PATH=${HOME}/tools/esp-idf
          $ ./install.sh

-   Install ninja-build

          $ sudo apt-get install ninja-build

### To build the application, follow these steps:

Currently building in VSCode _and_ deploying from native is not supported, so
make sure the IDF_PATH has been exported(See the manual setup steps above).

-   In the root of the example directory, sync the CHIP tree's submodules and
    source `idf.sh`. Note: This does not have to be repeated for incremental
    builds.

          $ source idf.sh

-   Configuration Options

        To choose from the different configuration options, run menuconfig

          $ idf make menuconfig

        Select ESP32 based `Device Type` through `Demo`->`Device Type`.
        The device types that are currently supported include `ESP32-DevKitC` (default),
        `ESP32-WROVER-KIT_V4.1` and `M5Stack`

        If you are using `standalone chip-tool` to communicate with the ESP32, bypass the
        Rendezvous mode so that the device can communicate over an insecure channel.
        This can be done through `Demo`->`Rendezvous Mode`->`Bypass`

        To connect the ESP32 to your network, configure the Wi-Fi SSID and Passphrase through
        `Component config`->`CHIP Device Layer`->`WiFi Station Options`->`Default WiFi SSID` and
        `Default WiFi Password` respectively.

        To use the default configuration options, run the default config

          $ idf make defconfig

-   Run make to build the demo application.

          $ idf make

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing. For ESP32-DevKitC devices this is labeled in the
    [functional description diagram](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html#functional-description).

          $ idf make flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

          $ idf make monitor ESPPORT=/dev/tty.SLAB_USBtoUART

## Using the Echo Server

There are two ways to use the Echo Server running on the device.

### Connect the ESP32 to a 2.4GHz Network of your choice

1.  If the `WiFi Station Options` mentioned above are populated through
    menuconfig, then ESP32 connects to the AP with those credentials (STA mode).

2.  Now flash the device with the same command as before. (Use the right `/dev`
    device)

          $ idf make flash monitor ESPPORT=/dev/tty.SLAB_USBtoUART

3.  The device should boot up and connect to your network. When that happens you
    will see a log like this in the monitor.

          I (5524) chip[DL]: SYSTEM_EVENT_STA_GOT_IP
          I (5524) chip[DL]: IPv4 address changed on WiFi station interface: <IP_ADDRESS>...

    Note: If you are using the M5Stack, the screen will display the server's IP
    Address if it successfully connects to the configured 2.4GHz Network.

4.  Use
    [standalone chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
    or
    [iOS chip-tool app](https://github.com/project-chip/connectedhomeip/tree/master/src/darwin)
    to communicate with the device.

Note: The ESP32 does not support 5GHz networks. Also, the Device will persist
your network configuration. To erase it, simply run.

          $ idf make erase_flash ESPPORT=/dev/tty.SLAB_USBtoUART

### Use the ESP32's Network

Alternatively, you can connect to the ESP32's Soft-AP directly.

1.  After the application has been flashed, connect to the ESP32's Soft-AP. If
    you use the M5Stack, the Soft-AP's SSID is encoded in the TLV section of the
    QRCode on screen. It's usually something like `CHIP-XXX` where the last 3
    digits are from the setup payload discriminator.

2.  Once you're connected, the server's IP can be found at the gateway address.

3.  Use
    [standalone chip-tool](https://github.com/project-chip/connectedhomeip/tree/master/examples/chip-tool)
    or
    [iOS chip-tool app](https://github.com/project-chip/connectedhomeip/tree/master/src/darwin)
    to communicate with the device.

In addition to the echo server, this demo also supports controlling OnOff
cluster (Server) attributes of an endpoint. For `ESP32-DevKitC` and
`ESP32-WROVER-KIT_V4.1`, a GPIO (configurable through `STATUS_LED_GPIO_NUM` in
`main/main.cpp`) is updated through the on/off/toggle commands from the
`chip-tool`. For `M5Stack`, a virtual Green LED on the display is used for the
same.

Note: If you wish to see the actual effect of the commands on `ESP32-DevKitC`
and `ESP32-WROVER-KIT_V4.1`, you will have to connect an external LED to GPIO
`STATUS_LED_GPIO_NUM`.
