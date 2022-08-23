# Matter ESP32 Light-switch Example

This example demonstrates the Matter Light-switch application on ESP platforms.

---

-   [Matter ESP32 Light-switch Example](#matter-esp32-light-switch-example)
    -   [Commissioning over BLE using chip-tool](#commissioning-over-ble-using-chip-tool)
    -   [Testing the example](#testing-the-example)

---

-   [Testing the example](#testing-the-example)

    It is recommended to have Ccache installed for faster builds

    ```
    $ export IDF_CCACHE_ENABLE=1
    ```

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

-   Please build the standalone chip-tool as described [here](../../chip-tool/README.md)
-   Commissioning the Lighting device

        $ ./out/debug/chip-tool pairing ble-wifi 12344321 <ssid> <passphrase> 20202021 3840

-   Commissioning the Lighting-switch device

         $ ./out/debug/chip-tool pairing ble-wifi 12344320 <ssid> <passphrase> 20202021 3840

## Testing the example

-   After successful commissioning, use the chip-tool to write the ACL in
    Lighting device to allow access from Lighting-switch device and chip-tool.

        $ ./out/debug/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null },{"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": [<LIGHT SWITCH NODE ID>], "targets": null }]' <LIGHTING APP NODE ID> 0

-   After successful commissioning, use the chip-tool for binding in
    Lighting-switch.

        $ ./out/debug/chip-tool binding write binding '[{"fabricIndex": 1, "node":<LIGHTING APP NODE ID>, "endpoint":1, "cluster":6}]' <LIGHT SWITCH NODE ID> 1

-   Test toggle:

    Press `boot` button to toggle LED.

    Using matter shell on:

        matter switch onoff on

    off:

        matter switch onoff off
