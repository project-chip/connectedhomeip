# Matter ESP32 Light-switch Example

This example demonstrates the Matter Light-switch application on ESP platforms.

---

-   [Matter ESP32 Light-switch Example](#Matter-ESP32-Light-switch-Example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning over BLE using chip-tool](#commissioning-over-ble-using-chip-tool)
    -   [Testing the example](#testing-the-example)

---

## Supported Devices

The demo application is intended to work on following categories of ESP32
devices:

-   [ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview)
-   [ESP32-WROVER-KIT_V4.1](https://www.espressif.com/en/products/hardware/esp-wrover-kit/overview)
-   [ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html).
-   [ESP32S3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html).
-   [ESP32-H2](https://www.espressif.com/en/news/ESP32_H2).

## Building the Example Application

Building the example application requires the use of the Espressif IoT
Development Framework ([ESP-IDF](https://github.com/espressif/esp-idf)).

The VSCode devcontainer has these components pre-installed, so you can skip this
step. To install these components manually, follow these steps:

-   Clone the Espressif ESP-IDF and checkout
    [v4.4 release](https://github.com/espressif/esp-idf/releases/tag/v4.4)

        $ mkdir ${HOME}/tools
        $ cd ${HOME}/tools
        $ git clone https://github.com/espressif/esp-idf.git
        $ cd esp-idf
        $ git checkout v4.4
        $ git submodule update --init
        $ ./install.sh

-   Install ninja-build

        $ sudo apt-get install ninja-build

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

-   Target Set

        $ idf.py set-target esp32
        or
        $ idf.py set-target esp32c3
        or
        $ idf.py set-target esp32s3

-   To build the demo application.

        $ idf.py build

-   After building the application, to flash it outside of VSCode, connect your
    device via USB. Then run the following command to flash the demo application
    onto the device and then monitor its output. If necessary, replace
    `/dev/tty.SLAB_USBtoUART`(MacOS) with the correct USB device name for your
    system(like `/dev/ttyUSB0` on Linux). Note that sometimes you might have to
    press and hold the `boot` button on the device while it's trying to connect
    before flashing.

        $ idf.py -p /dev/tty.SLAB_USBtoUART flash monitor

    Note: Some users might have to install the
    [VCP driver](https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers)
    before the device shows up on `/dev/tty`.

-   Quit the monitor by hitting `Ctrl+]`.

    Note: You can see a menu of various monitor commands by hitting
    `Ctrl+t Ctrl+h` while the monitor is running.

-   If desired, the monitor can be run again like so:

        $ idf.py -p /dev/tty.SLAB_USBtoUART monitor

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool)
-   Commissioning the Lighting device

        $ ./out/debug/chip-tool pairing ble-wifi 12344321 <ssid> <passphrase> 20202021 3840

-   Commissioning the Lighting-switch device

         $ ./out/debug/chip-tool pairing ble-wifi 12344320 <ssid> <passphrase> 20202021 3840

## Testing the example

-   After successful commissioning, use the chip-tool to write the ACL in
    Lighting device to allow access from Lighting-switch device and chip-tool.

        $ ./out/debug/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [12344320], "targets": null }]' 12344321 0

-   After successful commissioning, use the chip-tool for binding in
    Lighting-switch.

        $ ./out/debug/chip-tool binding write binding '[{"fabricIndex": 1, "node":20836, "endpoint":1, "cluster":6}]' 12344320 1

-   Test toggle:

    Press `boot` button to toggle LED.

    Using matter shell on:

        matter switch onoff on

    off:

        matter switch onoff off
