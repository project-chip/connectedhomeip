# Matter ESP32 EVSE Example

This example demonstrates the Matter Electric Vehicle Supply Equipment example
application along with several other energy management clusters on ESP
platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

### Build time configuration

-   Test Event Trigger support: By default the EVSE, Device Energy Management
    (DEM), Energy Reporting test event triggers are enabled in the build. To
    turn these off run `idf.py menuconfig` and search for the following config
    entries:

        ENABLE_DEVICE_ENERGY_MANAGEMENT_TRIGGER
        ENABLE_ENERGY_EVSE_TRIGGER
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

```bash
./out/debug/chip-tool energyevse disable <NODE ID> 1
```

```bash
./out/debug/chip-tool energyevse enable-charging 0xFFFFFFFF 6000 32000 <NODE ID> 1 --timedInteractionTimeoutMs <Time>
```

-   More examples using matter-repl are demonstrated in
    [EVSE Linux](../linux/README.md)
