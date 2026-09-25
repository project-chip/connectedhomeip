# Matter ESP32 Lighting Example (Zephyr)

This example demonstrates the Matter Lighting application on the ESP32-C6
running Zephyr RTOS, with Wi-Fi transport and BLE commissioning.

## Supported Hardware

| Board                            | SoC      | Transport                  |
| -------------------------------- | -------- | -------------------------- |
| `esp32c6_devkitc/esp32c6/hpcore` | ESP32-C6 | Wi-Fi (2.4 GHz), BLE setup |

The light output is GPIO2 (`led0` alias in the board overlay).

## Prerequisites

-   Follow [BUILDING.md](../../../../docs/guides/BUILDING.md) to set up the
    Matter environment.
-   Install the Zephyr SDK (1.0.1 or newer) as described in the
    [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).
-   Build [chip-tool](../../../chip-tool/README.md).

Zephyr **v4.4.2** requires Python 3.12 or newer, so it uses its own venv.

### Setting up the Zephyr workspace

```shell
export ZEPHYR_WORKSPACE=~/zephyr-4.4.2   # west workspace
export CHIP=~/connectedhomeip             # this repository

mkdir -p "$ZEPHYR_WORKSPACE" && cd "$ZEPHYR_WORKSPACE"
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.2 .
west update
west blobs fetch hal_espressif

python3 -m venv "$ZEPHYR_WORKSPACE/.venv"
"$ZEPHYR_WORKSPACE/.venv/bin/pip" install west esptool
"$ZEPHYR_WORKSPACE/.venv/bin/pip" install -r "$ZEPHYR_WORKSPACE/zephyr/scripts/requirements.txt"
"$ZEPHYR_WORKSPACE/.venv/bin/pip" install \
    -r "$CHIP/scripts/setup/requirements.build.txt" \
    -r "$CHIP/scripts/setup/requirements.zephyr.txt"
```

### Setting up the build environment

In every new terminal, activate the Matter environment first, then the Zephyr
venv so that its Python and `west` come first on `PATH`:

```shell
source "$CHIP/scripts/activate.sh"
source "$ZEPHYR_WORKSPACE/.venv/bin/activate"
source "$ZEPHYR_WORKSPACE/zephyr/zephyr-env.sh"
```

> **TODO**: The Matter environment ships Python 3.11, so `scripts/activate.sh`
> alone cannot run this build. Drop the separate venv once it moves to 3.12.

> **Note**: `west: unknown command "build"` means `zephyr-env.sh` was not
> sourced in this terminal. If `which west` points to the Matter `.environment`,
> the Zephyr venv was activated before `scripts/activate.sh`.

## Building the example

```shell
cd "$CHIP/examples/lighting-app/esp32/zephyr"
west build -b esp32c6_devkitc/esp32c6/hpcore
```

-   Pristine build (after changing Kconfig or board files):

    ```shell
    west build -p always -b esp32c6_devkitc/esp32c6/hpcore
    ```

-   Using the helper script:

    ```shell
    export ESP32_ZEPHYR_BASE="$ZEPHYR_WORKSPACE/zephyr"
    scripts/examples/esp32_zephyr_example.sh lighting-app esp32c6_devkitc/esp32c6/hpcore
    ```

-   Using `build_examples.py`:

    ```shell
    export ESP32_ZEPHYR_BASE="$ZEPHYR_WORKSPACE/zephyr"
    export ESP32_ZEPHYR_VENV="$ZEPHYR_WORKSPACE/.venv"
    scripts/run_in_build_env.sh \
        "./scripts/build/build_examples.py --target esp32-zephyr-c6devkitc-light build"
    ```

-   Wi-Fi only build (no BLE, provisioned from the Zephyr shell):

    ```shell
    west build -b esp32c6_devkitc/esp32c6/hpcore -- -DCONFIG_BT=n -DCONFIG_SHELL=y -DCONFIG_NET_L2_WIFI_SHELL=y -DCONFIG_NET_SHELL=y
    ```

    ```
    uart:~$ wifi connect "MyNetwork" -p "MyPassword" -k 1
    ```

-   Configure options with menuconfig:

    ```shell
    west build -t menuconfig
    ```

## Flashing and Monitoring

```shell
west flash --esp-device /dev/ttyUSB0
west espressif monitor -p /dev/ttyUSB0 -e build/zephyr/zephyr.elf
```

> **Note**: Keep only one process open on the serial port.

## Commissioning and Control

The example uses the test passcode `20202021` and discriminator `3840`.

-   Commission over BLE-Wi-Fi:

    ```shell
    ./out/linux-x64-chip-tool/chip-tool pairing ble-wifi <NODE ID> <SSID> <PASSWORD> 20202021 3840
    ```

-   Commission on-network (Wi-Fi only build):

    ```shell
    ./out/linux-x64-chip-tool/chip-tool pairing onnetwork <NODE ID> 20202021
    ```

-   Control the light:

    ```shell
    ./out/linux-x64-chip-tool/chip-tool onoff on <NODE ID> 1
    ./out/linux-x64-chip-tool/chip-tool onoff off <NODE ID> 1
    ./out/linux-x64-chip-tool/chip-tool onoff read on-off <NODE ID> 1
    ```

## Limitations

-   Uses test attestation credentials; factory data is not enabled.
-   No OTA support.
-   Only a single GPIO light output; no buttons or status LEDs.
