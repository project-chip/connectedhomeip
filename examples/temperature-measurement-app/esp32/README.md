# CHIP ESP32 Temperature Sensor Example

This example is meant to represent a minimal-sized application.

Refer to
[setup ESP-IDF and CHIP Environment](#setup-esp-idf-and-chip-environment) and
[Build, Flash, Monitor, and Commissioning](#build-flash-monitor-and-commissioning)
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
