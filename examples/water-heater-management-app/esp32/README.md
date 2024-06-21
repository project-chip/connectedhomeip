# Matter ESP32 Energy Management Example

This example demonstrates the Matter Electric Vehicle Supply Equipment
application on ESP platforms.

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

-   Follow the steps
    present[here](https://github.com/espressif/esp-insights/blob/main/examples/README.md#set-up-esp-insights-account)
    to set up an insights_account and the auth key created while setting it up
    will be used in the example.

-   Download the auth key and copy Auth Key to the example

```
cp /path/to/auth/key.txt path/to/connectedhomeip/examples/energy-management-app/esp32/main/insights_auth_key.txt
```

---

-   [Cluster Control](#cluster-control)
-   [Matter OTA guide](../../../docs/guides/esp32/ota.md)

---

### Cluster Control

-   After successful commissioning, use the Energy Electric Vehicle Supply
    Equipment cluster command to disable/enable charging and discharging.

```
        $./out/debug/chip-tool energyevse disable <NODE ID> 1
```

```
        $ ./out/debug/chip-tool energyevse enable-charging 0xFFFFFFFF 6000 32000 <NODE ID> 1 --timedInteractionTimeoutMs <Time>
```
