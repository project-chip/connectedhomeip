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
├── crash.log
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
chip-tool diagnosticlogs retrieve-logs-request 1 0 1 0 --TransferFileDesignator network-diag.log
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
