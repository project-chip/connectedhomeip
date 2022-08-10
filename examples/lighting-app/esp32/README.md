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

---

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
