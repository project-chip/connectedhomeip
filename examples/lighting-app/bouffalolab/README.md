# Matter `Bouffalo Lab` Lighting App Example

This example demonstrates a Matter lighting device (Vendor ID: **0xFFF1**,
Product ID: **0x8005**), supporting OnOff, LevelControl, ColorControl, and
Identify cluster control.

---

## Supported Targets

| Board       | Chip    | Transport                 | SDK            | Build system                    |
| ----------- | ------- | ------------------------- | -------------- | ------------------------------- |
| `bl602dk`   | BL602   | Wi-Fi                     | IoT SDK        | Matter GN (`build_examples.py`) |
| `bl706dk`   | BL702   | Wi-Fi / Thread / Ethernet | IoT SDK        | Matter GN (`build_examples.py`) |
| `bl704ldk`  | BL702L  | Thread                    | IoT SDK        | Matter GN (`build_examples.py`) |
| `bl616dk`   | BL616   | Wi-Fi / Thread / Ethernet | `Bouffalo SDK` | CMake (`make`)                  |
| `bl616cldk` | BL616CL | Wi-Fi                     | `Bouffalo SDK` | CMake (`make`)                  |

---

## Chip Solutions

### BL602 + Wi-Fi

```
BL602
├── Wi-Fi                ->  Matter over IP (TCP/UDP)
├── BLE                  ->  BLE commissioning (PASE over BLE)
└── Application
    ├── BOOT_PIN_RESET   (GPIO 8) - long press -> factory reset
    └── LED_PIN          (GPIO 0) - on/off lighting output
```

-   **Transport**: Wi-Fi with IPv4/IPv6 networking
-   **Commissioning**: BLE -> Wi-Fi credential provisioning
-   **SDK**: IoT SDK

### BL702 + Wi-Fi / Thread / Ethernet

```
BL702
├── Network interface
│   ├── Wi-Fi            ->  Matter over IP (TCP/UDP)
│   │                       └── SPI-attached BL602 Wi-Fi network processor
│   ├── Thread           ->  Matter over Thread (UDP/IPv6)
│   └── Ethernet         ->  Matter over IP (TCP/UDP)
├── BLE                  ->  BLE commissioning (PASE over BLE)
└── Application
    ├── BOOT_PIN_RESET   (GPIO 31) - long press -> factory reset
    └── LED_PIN          (GPIO 22) - on/off lighting output
```

-   **Transport**: Wi-Fi, Thread, or Ethernet; exactly one network interface
    must be enabled for each build. Wi-Fi uses a BL602 network processor
    connected over SPI.
-   **Commissioning**: BLE for Wi-Fi and Thread; on-network commissioning for
    Ethernet
-   **SDK**: IoT SDK

### BL702L + Thread

```
BL702L
├── IEEE 802.15.4 radio  ->  Matter over Thread (UDP/IPv6)
├── BLE                  ->  BLE commissioning (PASE over BLE)
└── Application
    ├── BOOT_PIN_RESET   (GPIO 16) - long press -> factory reset
    └── LED_PIN          (GPIO 18) - on/off lighting output
```

-   **Transport**: Thread with IPv6 networking
-   **Commissioning**: BLE -> Thread credential provisioning
-   **SDK**: IoT SDK

### BL616 / BL616CL + Wi-Fi / Thread / Ethernet

```
BL616 / BL616CL
├── Network interface
│   ├── Wi-Fi            ->  Matter over IP (TCP/UDP)
│   ├── Thread           ->  Matter over Thread (UDP/IPv6)
│   └── Ethernet         ->  Matter over IP (TCP/UDP)
├── BLE                  ->  BLE commissioning (PASE over BLE)
└── Application
    ├── BOOT_PIN_RESET   (GPIO 2 on BL616DK, GPIO 38 on BL616CL)
    ├── LED_B_PIN        (GPIO 0)  - blue PWM channel
    ├── LED_R_PIN        (GPIO 1)  - red PWM channel
    └── LED_G_PIN        (GPIO 30) - green PWM channel
```

-   **Transport**: Wi-Fi, Thread, or Ethernet; exactly one network interface
    must be enabled for each build.
-   **Commissioning**: BLE for Wi-Fi and Thread; on-network commissioning for
    Ethernet.
-   **SDK**: `Bouffalo SDK`

---

## Build and Flash with IoT SDK

The BL602, BL702, and BL702L targets use the Matter GN build system, managed by
`build_examples.py`.

Activate the build environment first:

```shell
source scripts/activate.sh -p bouffalolab
```

### Build

Build BL602 with Wi-Fi:

```shell
./scripts/build/build_examples.py \
    --target bouffalolab-bl602dk-light-wifi-littlefs-mfd-rpc \
    build
```

Build BL702 with Wi-Fi:

```shell
./scripts/build/build_examples.py \
    --target bouffalolab-bl706dk-light-wifi-littlefs-shell \
    build
```

Build BL702 with Thread:

```shell
./scripts/build/build_examples.py \
    --target bouffalolab-bl706dk-light-thread-littlefs-mfd-rpc \
    build
```

Build BL702 with Ethernet:

```shell
./scripts/build/build_examples.py \
    --target bouffalolab-bl706dk-light-ethernet-easyflash \
    build
```

Build BL702L with Thread:

```shell
./scripts/build/build_examples.py \
    --target bouffalolab-bl704ldk-light-thread-littlefs-mfd \
    build
```

### Flash

Flash BL602 Wi-Fi:

```shell
./out/bouffalolab-bl602dk-light-wifi-littlefs-mfd-rpc/chip-bl602-lighting-example.flash.py --port <device port>
```

Flash BL702 Wi-Fi:

```shell
./out/bouffalolab-bl706dk-light-wifi-littlefs-shell/chip-bl702-lighting-example.flash.py --port <device port>
```

Flash BL702 Thread:

```shell
./out/bouffalolab-bl706dk-light-thread-littlefs-mfd-rpc/chip-bl702-lighting-example.flash.py --port <device port>
```

Flash BL702 Ethernet:

```shell
./out/bouffalolab-bl706dk-light-ethernet-easyflash/chip-bl702-lighting-example.flash.py --port <device port>
```

Flash BL702L Thread:

```shell
./out/bouffalolab-bl704ldk-light-thread-littlefs-mfd/chip-bl702l-lighting-example.flash.py --port <device port>
```

---

## Build and Flash with `Bouffalo SDK`

The BL616 and BL616CL targets use a `Bouffalo SDK` + Matter CMake hybrid build.

Activate the build environment first:

```shell
source scripts/activate.sh -p bouffalolab
```

### Build

Build BL616DK with Wi-Fi:

```shell
make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y
```

Build BL616DK with Thread:

```shell
make -C examples/lighting-app/bouffalolab CONFIG_THREAD=y
```

Build BL616DK with Ethernet:

```shell
make -C examples/lighting-app/bouffalolab CONFIG_ETHERNET=y
```

Build BL616CL with Wi-Fi:

```shell
make -C examples/lighting-app/bouffalolab CHIP=bl616cl CONFIG_WIFI=y
```

Build with optional features:

```shell
make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y CONFIG_SHELL=y
make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y CONFIG_MFD=n
```

#### Optional Features

| Flag                                 | Default | Description                       |
| ------------------------------------ | ------- | --------------------------------- |
| `CONFIG_MFD=y/n`                     | `y`     | Enable factory/manufacturing data |
| `CONFIG_SHELL=y/n`                   | `n`     | Enable Matter interactive shell   |
| `CONFIG_CHIP_ROTATING_DEVICE_ID=y/n` | `n`     | Enable rotating device ID         |
| `CONFIG_CHIP_HEAP_MONITOR=y/n`       | `n`     | Enable heap monitoring            |
| `CONFIG_COREDUMP=y/n`                | `n`     | Enable SDK `coredump` capture     |
| `CONFIG_OT_FTD=1/0`                  | `1`     | Enable or disable Thread FTD mode |
| `CONFIG_OT_MTD=1/0`                  | `0`     | Enable or disable Thread MTD mode |

#### Clean

```shell
make -C examples/lighting-app/bouffalolab clean
```

### Flash

```shell
make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y flash
make -C examples/lighting-app/bouffalolab CONFIG_THREAD=y flash
make -C examples/lighting-app/bouffalolab CONFIG_ETHERNET=y flash
make -C examples/lighting-app/bouffalolab CHIP=bl616cl BOARD=bl616cldk CONFIG_WIFI=y flash
```

Flash with a manufacturing data file:

```shell
make -C examples/lighting-app/bouffalolab CONFIG_WIFI=y flash MFD_FILE=/path/to/mfd.bin
```

## Test with chip-tool

### Commissioning

Reset or factory reset the board first, then enter the chip-tool output
directory and run the commissioning command for the selected transport.

-   Wi-Fi

    ```shell
    ./chip-tool pairing ble-wifi <device_node_id> <wifi_ssid> <wifi_passwd> 20202021 3840
    ```

-   Thread

    ```shell
    ./chip-tool pairing ble-thread <device_node_id> hex:<thread_operational_dataset> 20202021 3840
    ```

-   Ethernet

    ```shell
    ./chip-tool pairing onnetwork <device_node_id> 20202021
    ```

> `<device_node_id>` - node ID assigned to the device with chip-tool;<br> >
> `<wifi_ssid>` - Wi-Fi network SSID;<br> > `<wifi_passwd>` - Wi-Fi network
> password;<br> > `<thread_operational_dataset>` - Thread network credentials,
> which can be obtained by running `sudo ot-ctl dataset active -x` on the border
> router.

### Cluster Control

After successful commissioning, use the cluster commands below to control the
board.

-   OnOff cluster

    The following command toggles the LED on the board:

    ```shell
    ./chip-tool onoff toggle <device_node_id> 1
    ```

-   LevelControl cluster

    The following command moves the level to 128:

    ```shell
    ./chip-tool levelcontrol move-to-level 128 10 0 0 <device_node_id> 1
    ```

-   ColorControl cluster

    The following command changes hue and saturation to 240 and 100:

    ```shell
    ./chip-tool colorcontrol move-to-hue-and-saturation 240 100 0 0 0 <device_node_id> 1
    ```

-   Identify cluster

    The following command identifies the board for 10 seconds:

    ```shell
    ./chip-tool identify identify 10 <device_node_id> 1
    ```

---

## References

-   [Bouffalo Lab - Platform overview](../../../docs/platforms/bouffalolab/platform_overview.md)
-   [Bouffalo Lab - Getting Started](../../../docs/platforms/bouffalolab/getting_started.md)
-   [Bouffalo Lab - OTA upgrade](../../../docs/platforms/bouffalolab/ota_upgrade.md)
-   [Bouffalo Lab - Matter factory data](../../../docs/platforms/bouffalolab/matter_factory_data.md)
-   [Bouffalo Lab - RPC console](../../../docs/platforms/bouffalolab/rpc_console.md)
