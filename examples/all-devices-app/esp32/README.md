# All Devices App for ESP32

This example demonstrates a dynamic Matter device application that allows you to
configure the device type at runtime via CLI commands. The selected device type
is persisted in NVS (Non-Volatile Storage) and automatically restored on
subsequent boots.

## Supported Hardware

This application supports **WiFi-based ESP32 SoCs only**

> **Note:** Thread devices are not supported by this example.

### Boards with a dedicated build target

| Board                | Build target                       | Notes                                 |
| -------------------- | ---------------------------------- | ------------------------------------- |
| ESP32-DevKitC        | `esp32-devkitc-all-devices`        | headless                              |
| M5Stack Basic / Gray | `esp32-m5stack-all-devices`        | LCD UI with the three front buttons   |
| M5Stack CoreS3       | `esp32-m5stack-cores3-all-devices` | ESP32-S3, 16MB flash, 8MB PSRAM       |

> **Note:** On the CoreS3 the LCD and capacitive touch panel are not driven yet.
> The display library used by the other M5Stack boards is ESP32-only, so the
> CoreS3 currently runs headless and is controlled through the serial shell.

## Building the Example

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

```
./scripts/build/build_examples.py --target esp32-m5stack-cores3-all-devices build
```

## Usage

### First Boot (No Stored Device Type)

On first boot, the application will display:

```
==================================================
No stored device type found.
Use command: devtype set <device-type>
==================================================
```

### Setting a Device Type

Use the shell command to set and initialize a device type:

```
devtype set <device-type>
```

Example:

```
devtype set contact-sensor
```

On successful initialization, the device type is saved to NVS and the Matter
server starts.

### Subsequent Boots

On subsequent boots, the application automatically:

1. Reads the stored device type from NVS
2. Initializes the device with that type
3. Starts the Matter server

```
==================================================
Found stored device type: contact-sensor
Auto-initializing...
==================================================
```

### Supported Device Types

The available device types depend on what's registered in the `DeviceFactory`.
Check `all-devices-common/device-factory/DeviceFactory.h` for the list of
supported types.

## Changing Device Type (Factory Reset)

To change the device type after it has been set, you need to perform a factory
reset to clear the NVS:

### Using Matter Shell

```
matter device factoryreset
```

After factory reset, reboot the device and set a new device type using the
`devtype set` command.
