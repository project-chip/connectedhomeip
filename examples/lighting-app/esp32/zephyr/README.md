# Matter ESP32 Lighting Example (Zephyr)

An example showing the use of Matter on Espressif SoCs running Zephyr RTOS
instead of ESP-IDF. It builds a Matter **On/Off Light** for the ESP32-C6 with
**Wi-Fi** transport and **BLE** commissioning.

<hr>

-   [Matter ESP32 Lighting Example (Zephyr)](#matter-esp32-lighting-example-zephyr)
    -   [Introduction](#introduction)
    -   [Supported Hardware](#supported-hardware)
    -   [Building](#building)
        -   [Setting up the Zephyr workspace](#setting-up-the-zephyr-workspace)
        -   [Setting up the build environment](#setting-up-the-build-environment)
        -   [Building the example](#building-the-example)
        -   [Building with build_examples.py](#building-with-build_examplespy)
    -   [Flashing the Application](#flashing-the-application)
    -   [Running the Complete Example](#running-the-complete-example)
        -   [Commissioning over BLE](#commissioning-over-ble)
        -   [Testing with chip-tool](#testing-with-chip-tool)
    -   [Building Options](#building-options)
        -   [Configuring the application](#configuring-the-application)
        -   [Wi-Fi only build](#wi-fi-only-build)
        -   [Memory footprint](#memory-footprint)
    -   [Limitations](#limitations)

<hr>

## Introduction

The ESP32 Zephyr lighting example provides a baseline demonstration of a Light
control device built using Matter on the ESP32-C6 running Zephyr RTOS. It uses
the generic `zephyr` CHIP device platform (`src/platform/Zephyr/`), the same
path the NXP RW612 Zephyr port takes, so no ESP32-specific `src/platform`
directory is needed.

The device is commissioned over Bluetooth Low Energy: the device and the Matter
controller exchange security information through the Rendez-vous procedure, and
the controller then provisions the Wi-Fi SSID and passphrase. Once joined to the
Wi-Fi network, the device is controlled over IPv6 and discovered through CHIP's
minimal mDNS.

The example drives a single GPIO as the light output and is intended both as a
means to explore Matter on Zephyr for Espressif SoCs and as a template for real
products.

## Supported Hardware

| Board                            | SoC      | Transport                  |
| -------------------------------- | -------- | -------------------------- |
| `esp32c6_devkitc/esp32c6/hpcore` | ESP32-C6 | Wi-Fi (2.4 GHz), BLE setup |

The board files in `boards/` apply automatically for this target. They set the
flash size to 4 MB, route the `led0` alias to GPIO2 and trim the Wi-Fi and
network buffer counts for the C6's SRAM. Adjust the flash size in the overlay if
your module differs.

## Building

Prerequisites:

-   Follow [BUILDING.md](../../../../docs/guides/BUILDING.md) to set up the
    Matter environment (`scripts/bootstrap.sh`).
-   Follow the
    [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
    to install the Zephyr SDK (1.0.1 or newer, `riscv64-zephyr-elf` toolchain).
-   Build [chip-tool](../../../chip-tool/README.md) as the commissioner.

| Component | Version                      | Notes                                                                                 |
| --------- | ---------------------------- | ------------------------------------------------------------------------------------- |
| Zephyr    | **v4.4.2**                   | earlier releases ship mbedTLS 3.x and need the classic crypto backend                 |
| Python    | **3.12** or newer            | required by Zephyr 4.4; the Matter environment ships 3.11, so a separate venv is used |
| ZAP       | see `scripts/setup/zap.json` | installed by `scripts/bootstrap.sh`                                                   |

### Setting up the Zephyr workspace

```shell
export WS=~/zephyr-4.4.2            # west workspace
export CHIP=~/connectedhomeip       # this repository

mkdir -p "$WS" && cd "$WS"
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.2 .
west update
west blobs fetch hal_espressif      # ESP32-C6 Wi-Fi/BLE radio blobs

python3 -m venv "$WS/.venv"
"$WS/.venv/bin/pip" install west esptool
"$WS/.venv/bin/pip" install -r "$WS/zephyr/scripts/requirements.txt"
"$WS/.venv/bin/pip" install \
    -r "$CHIP/scripts/setup/requirements.build.txt" \
    -r "$CHIP/scripts/setup/requirements.zephyr.txt"
```

### Setting up the build environment

In every new terminal, activate the Zephyr venv and source Zephyr's environment
script. This is the Zephyr equivalent of sourcing `export.sh` in ESP-IDF:

```shell
source "$WS/.venv/bin/activate"
source "$WS/zephyr/zephyr-env.sh"
```

`scripts/activate.sh` is not needed. The Matter build finds `gn` and its
environment under `.environment/` on its own, and `west` supplies the venv
Python to CMake. Set `ZEPHYR_SDK_INSTALL_DIR` only if Zephyr does not discover
the SDK by itself.

> **Note**: `west: unknown command "build"` means `ZEPHYR_BASE` is not set in
> this terminal; source `zephyr-env.sh` again (it must be sourced, not run). If
> `which west` shows the Matter `.environment` venv instead of the Zephyr one,
> activate the Zephyr venv after `scripts/activate.sh` or any `.envrc` that
> sources it, so that the Python 3.12 `west` comes first on `PATH`.

### Building the example

Build from the example directory. As with the ESP-IDF examples, the output lands
in the example's `build/` folder.

```shell
cd "$CHIP/examples/lighting-app/esp32/zephyr"
west build -b esp32c6_devkitc/esp32c6/hpcore
```

-   To clean the build:

    ```shell
    west build -t clean
    ```

-   For a pristine build after changing Kconfig or board files:

    ```shell
    west build -p always -b esp32c6_devkitc/esp32c6/hpcore
    ```

The binaries are created in `build/zephyr` as `zephyr.elf` and `zephyr.bin`.

The same build is available as a helper script, following the other platforms'
`scripts/examples/*_example.sh` convention. It needs no activation; point it at
the Zephyr tree and it finds the venv in the workspace root:

```shell
export ESP32_ZEPHYR_BASE="$WS/zephyr"
scripts/examples/esp32_zephyr_example.sh lighting-app esp32c6_devkitc/esp32c6/hpcore
```

### Building with build_examples.py

The example is also a `build_examples.py` target. The builder reads the Zephyr
tree from `ESP32_ZEPHYR_BASE`, and optionally the SDK from
`ESP32_ZEPHYR_SDK_DIR` and a Python 3.12+ venv providing `west` from
`ESP32_ZEPHYR_VENV`:

```shell
export ESP32_ZEPHYR_BASE="$WS/zephyr"
export ESP32_ZEPHYR_VENV="$WS/.venv"
scripts/run_in_build_env.sh \
    "./scripts/build/build_examples.py --target esp32-zephyr-c6devkitc-light build"
```

The output lands in `out/esp32-zephyr-c6devkitc-light/zephyr/`.

## Flashing the Application

-   Flash using west from the example directory:

    ```shell
    west flash --esp-device /dev/ttyUSB0
    ```

    The port can also come from the `ESPTOOL_PORT` environment variable. On
    macOS the port is `/dev/cu.usbserial-*`.

-   Or flash the image directly with esptool:

    ```shell
    python -m esptool --chip esp32c6 --port /dev/ttyUSB0 --baud 460800 \
        write_flash 0x0 build/zephyr/zephyr.bin
    ```

-   Monitor the serial console with backtrace decoding, from the same example
    directory:

    ```shell
    west espressif monitor -p /dev/ttyUSB0 -e build/zephyr/zephyr.elf
    ```

    Without `-e`, the monitor looks for the ELF relative to the current
    directory, so it would have to be run from `build/`. Its `-d` flag means
    `--enable-address-decoding`, not build directory. Always pass `-p`; on macOS
    it otherwise probes the phantom Bluetooth serial ports first and stalls.
    Exit with `Ctrl-]`. Any serial terminal at 115200 baud also works.

> **Note**: Only one process may own the serial port. A monitor left open is the
> usual cause of `Resource busy` or `No serial data received` when flashing.

## Running the Complete Example

### Commissioning over BLE

The example uses the test Device Attestation Credentials, the test setup
passcode `20202021` and discriminator `3840`, so it commissions with chip-tool
without factory data. **Do not ship this configuration.**

BLE advertising starts about 30 s after boot, once the Wi-Fi and network stacks
are up. The console shows:

```
[DL]CHIPoBLE advertising started
[DIS]Advertise commission parameter ... discriminator=3840/15 cm=1 cp=0 jf=0
[SVR]Manual pairing code: [34970112332]
```

`cm=1` means the commissioning window is open. The commissioning procedure does
the following:

-   chip-tool scans BLE and locates the device with the specified discriminator
-   Establishes operational certificates
-   Sends the Wi-Fi credentials over the Network Commissioning cluster
-   The device joins the Wi-Fi network and obtains IPv6 and IPv4 addresses
-   It then starts providing mDNS records on the operational network
-   Future communications happen over Wi-Fi

The fabric is stored in NVS and survives reboot.

### Testing with chip-tool

Commission the device over BLE-Wi-Fi from a host with BLE in range and the 2.4
GHz access point reachable:

```shell
./out/linux-x64-chip-tool/chip-tool pairing ble-wifi ${NODE_ID} ${SSID} ${PASSWORD} 20202021 3840
```

Control the device:

```shell
# Turn on the light
./out/linux-x64-chip-tool/chip-tool onoff on ${NODE_ID} 1

# Turn off the light
./out/linux-x64-chip-tool/chip-tool onoff off ${NODE_ID} 1

# Read the on/off state
./out/linux-x64-chip-tool/chip-tool onoff read on-off ${NODE_ID} 1
```

Where:

-   `${NODE_ID}` is the node ID assigned to the device
-   `20202021` is the setup PIN code
-   `3840` is the discriminator
-   `${SSID}` and `${PASSWORD}` are your Wi-Fi network credentials

Clear chip-tool's state between fresh commissionings with
`rm -rf /tmp/chip_tool_kvs`.

## Building Options

### Configuring the application

Use Zephyr's menuconfig to browse and modify build-time options:

```shell
west build -b esp32c6_devkitc/esp32c6/hpcore -t menuconfig
```

Logging uses immediate mode so the boot log is not truncated on the UART. For
faster boot once the log volume is understood, switch to deferred logging:

```shell
west build -b esp32c6_devkitc/esp32c6/hpcore -- -DCONFIG_LOG_MODE_IMMEDIATE=n -DCONFIG_LOG_MODE_DEFERRED=y
```

### Wi-Fi only build

BLE can be disabled to reclaim about 92 KB of SRAM. The device is then
provisioned from the Zephyr `wifi` shell and commissioned on-network:

```shell
west build -b esp32c6_devkitc/esp32c6/hpcore -- -DCONFIG_BT=n -DCONFIG_SHELL=y -DCONFIG_NET_L2_WIFI_SHELL=y -DCONFIG_NET_SHELL=y
```

```
uart:~$ wifi connect "MyNetwork" -p "MyPassword" -k 1
```

```shell
./out/linux-x64-chip-tool/chip-tool pairing onnetwork ${NODE_ID} 20202021
```

### Memory footprint

The build uses about 81 % of the C6's 497 KiB SRAM and 37 % of a 4 MB flash.
Profile with:

```shell
west build -t ram_report
```

Levers, in order of impact:

-   `CONFIG_CHIP_MALLOC_SYS_HEAP_SIZE` in `prj.conf` (16 KB).
-   `CONFIG_MBEDTLS_PSA_KEY_SLOT_COUNT` in `prj.conf` (16).
-   `CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE` in
    `main/include/CHIPProjectConfig.h` (8).
-   `CONFIG_ESP32_WIFI_*` and `CONFIG_NET_BUF_*` counts in the board `.conf`.

The largest single block (about 100 KB) is the Zephyr system heap sized by
`HEAP_MEM_POOL_ADD_SIZE_ESP_BT` and `HEAP_MEM_POOL_ADD_SIZE_ESP_WIFI`, which
`hal_espressif` hard-codes as the C6 radio-blob minimum.

## Limitations

1. **Prototype configuration.** The example uses test attestation credentials
   and the test passcode. Factory data (`CONFIG_CHIP_FACTORY_DATA`) is not
   enabled.
2. **No OTA.** The image boots through `ESP_SIMPLE_BOOT` without MCUboot slots,
   so `CONFIG_CHIP_OTA_REQUESTOR` is off.
3. **Single GPIO output.** The on-board WS2812 LED on GPIO8 (a strapping pin) is
   not driven; wire a plain LED to GPIO2 or repoint the `led0` alias in the
   board overlay. There are no buttons or status LEDs.
4. **Separate Python environment.** Until the Matter environment provides Python
   3.12, the Zephyr venv must be maintained alongside it.

---

### Additional Resources:

-   [Zephyr ESP32-C6 DevKitC board documentation](https://docs.zephyrproject.org/latest/boards/espressif/esp32c6_devkitc/doc/index.html)
-   [Matter ESP-IDF lighting example](../README.md)
-   [Zephyr RTOS Documentation](https://docs.zephyrproject.org/)
-   [Matter Specification](https://csa-iot.org/all-solutions/matter/)
