# Matter ESP32 Temperature Sensor Example

This example is meant to represent a minimal-sized application.

Please
[setup ESP-IDF and CHIP Environment](../../../docs/guides/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/guides/esp32/build_app_and_commission.md)
guides to get started.

---

-   [Cluster control](#cluster-control)
-   [Optimization](#optimization)

---

### Cluster control

#### temperaturemeasurement

```bash
Usage:
  ./out/debug/chip-tool temperaturemeasurement read measured-value <NODE ID> 1
```

## Additional details

This example demonstrates the utilization of the diagnostic logs cluster to send
diagnostic logs to the client.

In this scenario, the [main/diagnostic_logs](main/diagnostic_logs) directory
contains three files:

```
main/diagnostic_logs
├── end_user_support.log
└── network_diag.log
```

These files contain dummy data.

#### To test the diagnostic logs cluster

```
# Commission the app
chip-tool pairing ble-wifi 1 SSID PASSPHRASE 20202021 3840

# Read end user support logs using response payload protocol
chip-tool diagnosticlogs retrieve-logs-request 0 0 1 0

# Read network diagnostic using BDX protocol
chip-tool interactive start
> diagnosticlogs retrieve-logs-request 1 1 1 0 --TransferFileDesignator network-diag.log
# Retrieve crash summary over BDX
> diagnosticlogs retrieve-logs-request 2 1 1 0 --TransferFileDesignator crash-summary.bin
```

esp-idf supports storing and retrieving
[core dump in flash](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/core_dump.html#core-dump-to-flash).

To support that, application needs to add core dump partition's entry in
[partitons.csv](partitions.csv#7) and we need to enable few menuconfig options.

```
CONFIG_ESP32_ENABLE_COREDUMP_TO_FLASH=y
CONFIG_ESP32_COREDUMP_DATA_FORMAT_ELF=y
```

This example's partition table and sdkconfig.default are already modified

-   Retrieve the core dump using diagnostic logs cluster

    ```
    # Read crash summary over BDX
    chip-tool interactive start
    > diagnosticlogs retrieve-logs-request 2 1 1 0 --TransferFileDesignator crash-summary.bin
    ```

## Optimization

Optimization related to WiFi, BLuetooth, Asserts etc are the part of this
example by default. To enable this option set is_debug=false from command-line.

```
# Reconfigure the project for additional optimizations
rm -rf sdkconfig build/
idf.py -Dis_debug=false reconfigure

# Set additional configurations if required
idf.py menuconfig

# Build, flash, and monitor the device
idf.py -p /dev/tty.SLAB_USBtoUART build flash monitor
```
