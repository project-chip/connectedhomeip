# Matter ESP32 Lighting Example

This example demonstrates the Matter Lighting application on ESP platforms.

---

-   [Matter ESP32 Lighting Example](#matter-esp32-lighting-example)
    -   [Supported Devices](#supported-devices)
    -   [Building the Example Application](#building-the-example-application)
    -   [Commissioning over BLE using chip-tool](#commissioning-over-ble-using-chip-tool)
    -   [Cluster Control](#cluster-control)
-   [Steps to Try Lighting app OTA Requestor](#steps-to-try-lighting-app-ota-requestor)

---

## Supported Devices

The demo application is intended to work on following categories of ESP32
devices:

-   [ESP32-DevKitC](https://www.espressif.com/en/products/hardware/esp32-devkitc/overview)
-   [ESP32-WROVER-KIT_V4.1](https://www.espressif.com/en/products/hardware/esp-wrover-kit/overview)
-   [ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html).
-   [ESP32S3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html).

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

        $ ./out/debug/chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 20202021 3840

## Cluster Control

-   After successful commissioning, use the OnOff cluster command to control the
    OnOff attribute. This allows you to toggle a parameter implemented by the
    device to be On or Off.

        $ ./out/debug/chip-tool onoff on 12345 1

-   On
    [ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
    or
    [ESP32S3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html)
    board, there is an on-board RGB-LED. Use ColorControl cluster command to
    control the color attributes:

        $ ./out/debug/chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 12345 1

# Steps to Try Lighting app OTA Requestor

Before moving ahead, make sure your device is commissioned and running.

-   Build the Linux OTA Provider

```
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
```

-   Run the Linux OTA Provider with OTA image.

```
./out/debug/chip-ota-provider-app -f hello-world.bin
```

hello-world.bin can be obtained from compiling the hello-world ESP-IDF example.

-   Commission the Linux OTA Provider using chip-tool

```
./out/debug/chip-tool pairing onnetwork 12346 20202021
```

## Query for an OTA Image

After commissioning is successful, press Enter in requestor device console and
type below query.

```
>matter ota query 1 12346 0
```

## Apply update

Once the transfer is complete, OTA requestor sends ApplyUpdateRequest command to
OTA provider for applying the image. Device will restart on successful
application of OTA image.
