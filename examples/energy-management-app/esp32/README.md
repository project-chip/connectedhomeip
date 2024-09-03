# Matter ESP32 Energy Management Example

This example demonstrates the Matter Electric Vehicle Supply Equipment and Water
Heater example application along with several other energy management clusters
on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

### Enabling ESP-Insights:

-   Before building the app, enable the option: `ESP_INSIGHTS_ENABLED` through
    menuconfig.

-   Create a file named `insights_auth_key.txt` in the main directory of the
    example.

-   Follow the steps present
    [here](https://github.com/espressif/esp-insights/blob/main/examples/README.md#set-up-esp-insights-account)
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

### Build time configuration

-   Application mode: By default the example app will run the EVSE example,
    however this can be changed using the `idf.py menuconfig` command and
    searching for:

        ENABLE_EXAMPLE_WATER_HEATER_DEVICE
        ENABLE_EXAMPLE_EVSE_DEVICE

-   Test Event Trigger support: By default the EVSE, Water Heater, Device Energy
    Management (DEM), Energy Reporting test event triggers are enabled in the
    build. To turn these off run `idf.py menuconfig` and search for the
    following config entries:

        ENABLE_DEVICE_ENERGY_MANAGEMENT_TRIGGER
        ENABLE_ENERGY_EVSE_TRIGGER
        ENABLE_WATER_HEATER_MANAGEMENT_TRIGGER
        ENABLE_ENERGY_REPORTING_TRIGGER

-   Device Energy Management feature support: Depending on the capabilities of
    the device it may support Power Forecast Reporting `PFR` or State Forecast
    Reporting `SFR`. However it is not allowed to support both of these at once.

    By default `PFR` is enabled in the build. To change this run
    `idf.py menuconfig` and search for the following config entries:

        DEM_SUPPORT_STATE_FORECAST_REPORTING
        DEM_SUPPORT_POWER_FORECAST_REPORTING

    Note only one of these can be enabled to meet Matter specification
    conformance.

    When running in test events, some test cases (e.g. TC_DEM_2.7 and
    TC_DEM_2.8) depend on the correct setting to be used in the binary DUT.

### Cluster Control

-   After successful commissioning, use the Energy Electric Vehicle Supply
    Equipment cluster command to disable/enable charging and discharging.

```
        $./out/debug/chip-tool energyevse disable <NODE ID> 1
```

```
        $ ./out/debug/chip-tool energyevse enable-charging 0xFFFFFFFF 6000 32000 <NODE ID> 1 --timedInteractionTimeoutMs <Time>
```

-   More examples using chip-repl are demonstrated in
    [Energy Management Linux](../linux/README.md)
