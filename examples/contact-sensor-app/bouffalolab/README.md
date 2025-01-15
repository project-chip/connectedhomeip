# Matter `Bouffalo Lab` Contact Sensor Example

This example functions as a contact sensor uses a test Vendor ID (VID) and a
Product ID (PID) of **0x8005**.

Supported board:

-   `bl704ldk`

Please refer to the following documents for more information

-   [Bouffalo Lab - Platform overview](../../../docs/platforms/bouffalolab/platform_overview.md)
-   [Bouffalo Lab - Getting Started](../../../docs/platforms/bouffalolab/getting_started.md)
-   [Bouffalo Lab - OTA upgrade](../../../docs/platforms/bouffalolab/ota_upgrade.md)
-   [Bouffalo Lab - Matter factory data generation](../../../docs/platforms/bouffalolab/matter_factory_data.md)

## Build CHIP Contact Sensor example

```shell
./scripts/build/build_examples.py --target bouffalolab-bl704ldk-contact-sensor-thread-mtd-littlefs build
```

## Test with chip-tool

### Commissioning over BLE

-   Reset the board or factory reset the board

-   Enter build out folder of chip-tool and running the following command to do
    BLE commission

    -   Thread

            ```shell
            ./out/linux-x64-chip-tool/chip-tool pairing ble-thread <deivce_node_id> hex:<thread_operational_dataset> 20202021 3840
            ```

        > `<deivce_node_id>`, which is node ID assigned to device within
        > chip-tool `<thread_operational_dataset>`, Thread network credential
        > which running `sudo ot-ctl dataset active -x` command on border router
        > to get.

### Subscribe booleanstate of contact sensor

-   Start chip-tool with interactive mode

    ```shell
    ./chip-tool interactive start
    ```

-   Subscribe booleanstate under chip-tool interactive mode

    ```shell
    booleanstate subscribe state-value <min-interva> <max-interval> <deivce_node_id> 1
    ```

    -   `<min-interva>`, min interval to report to boolean state of contact
        sensor
    -   `<max-interva>`, max interval to report to boolean state of contact
        sensor
    -   `<deivce_node_id>`, node id of contact sensor

-   Press down `INT` button, boolean state True will report to chip-tool

    > GPIO 20 connects to `INT` button.

-   After press down `INT` button over `min-interva>` seconds, and release `INT`
    button, boolean state False will report to chip-tool
