# All Devices App for ESP32

This example demonstrates a dynamic Matter device application that allows you to
configure the device type at runtime via CLI commands. The selected device type
is persisted in NVS (Non-Volatile Storage) and automatically restored on
subsequent boots.

## Supported Hardware

This application supports **WiFi-based ESP32 SoCs only**

> **Note:** Thread devices are not supported by this example.

### M5Stack LCD UI

On M5Stack boards, this example also drives an on-device LCD/touch UI (QR
code display, status screen, device-type selection, factory reset) instead
of relying solely on the CLI. Two boards are supported today:

| Board | Build target | Input |
| --- | --- | --- |
| M5Stack Basic / Gray | `esp32-m5stack-all-devices` | 3 physical GPIO buttons |
| M5Stack Core2 | `esp32-m5stack-core2-all-devices` | 3 virtual buttons along the bottom of the touchscreen (capacitive touch, no physical buttons) |

The Core2 differs from the Basic/Gray in ways that go beyond a pin remap: its
LCD backlight/logic rails and RESET line are switched by the onboard AXP192
PMIC over I2C (see `examples/common/screen-framework/AXP192.{h,cpp}` and
`Core2Power.{h,cpp}`), and its touch controller
(`examples/all-devices-app/esp32/main/display/CapacitiveTouchButtons.{h,cpp}`)
is FT6336U-family capacitive touch rather than GPIO buttons. Both were
validated by flashing real M5Stack Core2 hardware.

Known follow-ups for anyone building on this:

-   `sdkconfig_m5stack_core2.defaults` conservatively targets a 4MB flash
    size (matching the Basic/Gray defaults) even though most Core2 units
    ship with 16MB flash and 8MB PSRAM; bump `CONFIG_ESPTOOLPY_FLASHSIZE_*`
    via `idf.py menuconfig` if you need the extra space.
-   The panel's MADCTL orientation byte
    (`examples/common/screen-framework/Display.cpp`) was determined
    empirically against one unit, since none of the vendored TFT library's 4
    named rotation presets (which all set MV and/or MX/MY) render it
    right-side up -- the Core2's panel needs none of those bits, only the
    BGR color-order bit. If text renders reflected or rotated on your unit,
    that's the value to adjust.

## Building the Example

Please
[setup ESP-IDF and CHIP Environment](../../../docs/platforms/esp32/setup_idf_chip.md)
and refer
[building and commissioning](../../../docs/platforms/esp32/build_app_and_commission.md)
guides to get started.

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
