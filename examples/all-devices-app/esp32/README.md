# All Devices App for ESP32

This example demonstrates a dynamic Matter device application that allows you to
configure the device type at runtime via CLI commands. The selected device type
is persisted in NVS (Non-Volatile Storage) and automatically restored on
subsequent boots.

## Supported Hardware

This application supports **WiFi-based ESP32 SoCs only**

> **Note:** Thread devices are not supported by this example.

Boards with a display get the UI described in
[main/display/README.md](main/display/README.md): a button-driven menu on the
M5Stack Basic/Gray and a touch-driven LVGL UI on the others.

| Board              | SoC       | Defaults file                       |
| :----------------- | :-------- | :---------------------------------- |
| M5Stack Basic/Gray | ESP32     | `sdkconfig_m5stack.defaults`        |
| M5Stack CoreS3     | ESP32-S3  | `sdkconfig_m5stack_cores3.defaults` |
| ESP-Mosaico        | ESP32-S31 | `sdkconfig_mosaico.defaults`        |

### ESP-Mosaico (ESP32-S31)

#### ESP-IDF

The Mosaico needs [ESP-IDF](https://github.com/espressif/esp-idf) 6.2, which at
the time of writing is only on master. This port was built with:

```
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout 4d59230ddff
git submodule update --init --recursive
./install.sh esp32s31
. ./export.sh
```

#### Mosaico BSP

Clone the [esp-mosaico-bsp](https://github.com/esp-mosaico/esp-mosaico-bsp)
anywhere and point `ESP_MOSAICO_BSP_PATH` at the checkout before building; the
port was tested at commit `6edd088e`:

```
git clone https://github.com/esp-mosaico/esp-mosaico-bsp.git /path/to/esp-mosaico-bsp
export ESP_MOSAICO_BSP_PATH=/path/to/esp-mosaico-bsp
```

A Mosaico build stops with a message if the variable is unset. Other boards
ignore it.

#### Build and flash

Build with the Mosaico defaults:

```
export SDKCONFIG_DEFAULTS=sdkconfig_mosaico.defaults
idf.py --preview set-target esp32s31
idf.py build
```

The board's USB-C port is the ESP32-S31's USB OTG controller and there is no
USB-to-UART bridge, so the application runs a USB CDC console on that port
(`main/board/MosaicoUsbConsole.cpp`). `idf.py monitor` shows the logs there, and
the device answers the USB Serial/JTAG reset sequence esptool uses, so once this
firmware is on the board `idf.py flash` resets it into the bootloader by itself:

```
idf.py -p /dev/ttyACM0 flash monitor
```

For the first flash, or whenever the board runs firmware without that console,
put it into ROM download mode by hand first:

1. Press and hold the Boot button (left of the USB-C port).
2. Press and release the Reset button while still holding Boot.
3. Release Boot. The ROM enumerates as an Espressif USB device.

### Boards with a dedicated build target

| Board                | Build target                       | Notes                               |
| -------------------- | ---------------------------------- | ----------------------------------- |
| ESP32-DevKitC        | `esp32-devkitc-all-devices`        | headless                            |
| M5Stack Basic / Gray | `esp32-m5stack-all-devices`        | LCD UI with the three front buttons |
| M5Stack CoreS3       | `esp32-m5stack-cores3-all-devices` | ESP32-S3, 16MB flash, 8MB PSRAM     |

> **Note:** The CoreS3 and the Mosaico have no menu buttons. They are driven
> entirely through the capacitive touch panel: a breadcrumb-navigated LVGL UI
> provides the commissioning codes, diagnostics, per-endpoint device screens
> with live cluster controls, device type switching and reboot/factory reset.
> The other M5Stack boards keep their button-driven menus. See
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

The M5Stack CoreS3 and the ESP-Mosaico have a speaker, so they can act as a
Chime device. Two switches gate it:

| Switch                     | Where                                                       | Effect                                               |
| -------------------------- | ----------------------------------------------------------- | ---------------------------------------------------- |
| `CONFIG_HAVE_SPEAKER`      | `main/Kconfig.projbuild`, default `y` on CoreS3 and Mosaico | Compiles `main/devices/chime/` and the audio backend |
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

Tone synthesis lives in `TonePlayingChime`; each board supplies only its audio
output path. Audio hardware is brought up lazily, on the first `PlayChimeSound`.
On the CoreS3 the `AXP2101` PMIC enables the amplifier rails, the `AW9523` I/O
controller enables the boost rail and the amplifier PA, and the `AW88298`
amplifier is configured over I2C, after which the I2S channel is created. On the
Mosaico the BSP opens the `ES8311` codec. Everything stays initialized
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
