# Matter ESP32 Lighting Example

This example demonstrates the Matter Lighting application on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

### Enabling ESP-Insights:

-   Before building the app, enable the option: ESP_INSIGHTS_ENABLED through
    menuconfig.

-   Create a file named insights_auth_key.txt in the main directory of the
    example.

-   Follow the steps present
    [here](https://github.com/espressif/esp-insights/blob/main/examples/README.md#set-up-esp-insights-account)
    to set up an insights_account and the auth key created while setting it up
    will be used in the example.

-   Download the auth key and copy Auth Key to the example

```
cp /path/to/auth/key.txt path/to/connectedhomeip/examples/lighting-app/esp32/main/insights_auth_key.txt
```

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

### Matter OTA

For Matter OTA please take a look at
[Matter OTA guide](../../../docs/guides/esp32/ota.md).
