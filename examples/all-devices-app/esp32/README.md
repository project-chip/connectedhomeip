# All Devices App for ESP32

This example demonstrates a dynamic Matter device application that allows you to
configure the device type at runtime via CLI commands. The selected device type
is persisted in NVS (Non-Volatile Storage) and automatically restored on
subsequent boots.

## Supported Hardware

This application supports **WiFi-based ESP32 SoCs only**

> **Note:** Thread devices are not supported by this example.

### Boards with a dedicated build target

| Board                | Build target                       | Notes                               |
| -------------------- | ---------------------------------- | ----------------------------------- |
| ESP32-DevKitC        | `esp32-devkitc-all-devices`        | headless                            |
| M5Stack Basic / Gray | `esp32-m5stack-all-devices`        | LCD UI with the three front buttons |
| M5Stack CoreS3       | `esp32-m5stack-cores3-all-devices` | ESP32-S3, 16MB flash, 8MB PSRAM     |

> **Note:** The CoreS3 has no physical buttons. It is driven entirely through
> its capacitive touch panel: a breadcrumb-navigated LVGL UI provides the
> commissioning codes, diagnostics, per-endpoint device screens with live
> cluster controls, device type switching and reboot/factory reset. The other
> M5Stack boards keep their button-driven menus. See
> [main/display/README.md](main/display/README.md) for the display architecture.

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

## Audio (Chime)

The M5Stack CoreS3 is the only supported board with a speaker, so it is the only
one that can act as a Chime device. Two switches gate it:

| Switch                     | Where                                                       | Effect                                               |
| -------------------------- | ----------------------------------------------------------- | ---------------------------------------------------- |
| `CONFIG_HAVE_SPEAKER`      | `main/Kconfig.projbuild`, default `y` on CoreS3             | Compiles `main/devices/chime/` and the audio backend |
| `ALL_DEVICES_ENABLE_CHIME` | device list passed to CMake (`enabled_devices_config.h.in`) | Registers the Chime device type in the factory       |

The three installed sounds are synthesized at playback time, so the build
carries no audio assets:

| Chime ID | Name       | Waveform                      |
| -------- | ---------- | ----------------------------- |
| 0        | Ding Dong  | 880 Hz then 660 Hz, 1 s total |
| 1        | Ring Ring  | 1 kHz, pulsed, 1 s            |
| 2        | Alert Beep | 440 Hz, 0.5 s                 |

Each tone is a decaying fundamental plus two harmonics, written to I2S as 16-bit
stereo at 48 kHz.

Audio hardware is brought up lazily, on the first `PlayChimeSound`: the
`AXP2101` PMIC enables the amplifier rails, the `AW9523` I/O controller enables
the boost rail and the amplifier PA, and the `AW88298` amplifier is configured
over I2C, after which the I2S channel is created. Everything stays initialized
afterwards, so only the first chime pays that cost. The tone itself is rendered
on a short-lived task so the CHIP event loop is not blocked for the duration of
the sound.

`CONFIG_CHIME_ATTENUATION_DB` (0-96, default 40) sets how far below the
amplifier's maximum gain the output sits; each 6 dB halves the amplitude.

`PlayChimeSound` returns `Failure` if audio bring-up fails, and `Busy` if a
sound is already playing — the overlapping request is refused rather than queued
or mixed.

## Changing Device Type (Factory Reset)

To change the device type after it has been set, you need to perform a factory
reset to clear the NVS:

### Using Matter Shell

```
matter device factoryreset
```

After factory reset, reboot the device and set a new device type using the
`devtype set` command.
