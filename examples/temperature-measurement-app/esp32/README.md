# Temperature Sensor Example

## Building the Example Application

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
        and `M5Stack`

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

          $ idf make flash monitor ESPPORT=/dev/ttyUSB0

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

          $ idf make monitor ESPPORT=/dev/ttyUSB0

## Using the Echo Server

### Connect the ESP32 to a 2.4GHz Network of your choice

1.  If the `WiFi Station Options` mentioned above are populated through
    menuconfig, then ESP32 connects to the AP with those credentials (STA mode).

2.  Now flash the device with the same command as before. (Use the right `/dev`
    device)

          $ idf make flash monitor ESPPORT=/dev/ttyUSB0

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

    $ idf make erase_flash ESPPORT=/dev/ttyUSB0

The demo application supports temperaturemeasurement and basic cluster.

## Optimization

Optimization related to WiFi, BLuetooth, Asserts etc are the part of this
example by default. To enable this option set is_debug=false from command-line.

```
idf make build flash monitor 'is_debug=false'
```
