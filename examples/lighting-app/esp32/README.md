# Matter ESP32 Lighting Example

This example demonstrates the Matter Lighting application on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Cluster Control](#cluster-control)
-   [Matter OTA guide](../../../docs/guides/esp32/ota.md)

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

-   Commissioning over ble after flashing script, change the passcode, replace
    `20202021` with `99663300`.

## Commissioning over BLE using chip-tool

-   Please build the standalone chip-tool as described [here](../../chip-tool/README.md)
-   Commissioning the WiFi Lighting devices(ESP32, ESP32C3, ESP32S3)

        $ ./out/debug/chip-tool pairing ble-wifi 12345 <ssid> <passphrase> 20202021 3840

-   For ESP32-H2, firstly start OpenThread Border Router, you can either use
    [Raspberry Pi OpenThread Border Router](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/openthread_border_router_pi.md)
    OR
    [ESP32 OpenThread Border Router](https://github.com/espressif/esp-idf/tree/master/examples/openthread/ot_br)
-   Get the active operational dataset.

        $ ot-ctl> dataset active -x

-   Commissioning the Thread Lighting device(ESP32H2)

         $ ./out/debug/chip-tool pairing ble-thread 12345 hex:<operational-dataset> 20202021 3840

    NOTE: If using ESP32 factory data provider then commission the device using
    discriminator and passcode provided while building the factory NVS binary.

### Cluster Control

-   After successful commissioning, use the OnOff cluster command to control the
    OnOff attribute. This allows you to toggle a parameter implemented by the
    device to be On or Off.

        $ ./out/debug/chip-tool onoff on <NODE ID> 1

-   On
    [ESP32C3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/hw-reference/esp32c3/user-guide-devkitm-1.html)
    or
    [ESP32S3-DevKitM](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitm-1.html)
    board, there is an on-board RGB-LED. Use ColorControl cluster command to
    control the color attributes:

        $ ./out/debug/chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 <NODE ID> 1
