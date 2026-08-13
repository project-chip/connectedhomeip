# Matter ESP32 Energy Gateway Example

This example demonstrates the Matter `Commodity Price` and
`Electrical Grid Conditions` clusters on ESP platforms.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

### Build time configuration

-   Test Event Trigger support: By default the `CommodityPrice` and
    `ElectricalGridConditions` test event triggers are enabled in the build. To
    turn these off run `idf.py menuconfig` and search for the following config
    entries:

        ENABLE_COMMODITY_PRICE_TRIGGER
        ENABLE_ELECTRICAL_GRID_CONDITIONS_TRIGGER

### Cluster Control

-   More examples using matter-repl are demonstrated in
    [Energy Gateway Linux](../linux/README.md)
