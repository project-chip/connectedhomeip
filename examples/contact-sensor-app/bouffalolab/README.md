# Matter `Bouffalo Lab` Contact Sensor Example

This example demonstrates a Matter contact sensor device (Vendor ID: **0xFFF1**,
Product ID: **0x8005**), supporting BooleanState cluster reporting via GPIO.

---

## Supported Targets

| Board      | Chip   | Transport | Build system                    |
| ---------- | ------ | --------- | ------------------------------- |
| `bl704ldk` | BL704L | Thread    | Matter GN (`build_examples.py`) |
| `bl616dk`  | BL616  | Wi-Fi     | CMake (`make`)                  |

---

## BL704L + Thread

### Architecture

```
BL704L
├── IEEE 802.15.4 radio  →  OpenThread (FTD or MTD)
│                             └── Matter over Thread (UDP/IPv6)
├── BLE                  →  BLE commissioning (PASE over BLE)
└── Application
    ├── CHIP_RESET_PIN   (GPIO 16) — long press → factory reset
    └── CHIP_CONTACT_PIN (GPIO 20) — rising edge = true, falling = false
```

> `CHIP_CONTACT_PIN` should be pulled down to a low level; otherwise, the device
> may wake up unexpectedly.

-   **Transport**: Thread with IPv6 networking
-   **Commissioning**: BLE → Thread credential provisioning
-   **Sleep**: PDS (Power-Down Sleep) via `app_pds.cpp`
-   **Build**: Matter GN build system, managed by `build_examples.py`

### Build

Activate the build environment first:

```shell
source scripts/activate.sh
```

Build with Thread MTD (Minimal Thread Device, lower power):

```shell
./scripts/build/build_examples.py \
    --target bouffalolab-bl704ldk-contact-sensor-thread-mtd-littlefs \
    build
```

The output binary is placed under
`out/bouffalolab-bl704ldk-contact-sensor-thread-mtd-littlefs/`.

### Flash

```shell
./out/bouffalolab-bl704ldk-contact-sensor-thread-mtd-littlefs-mfd/chip-bl702l-contact-sensor-example.flash.py --port <device port>
```

---

## BL616 + Wi-Fi

### Architecture

```
BL616 (RISC-V, Wi-Fi 6 + BLE)
├── Wi-Fi                →  Matter over IP (TCP/UDP)
├── BLE                  →  BLE commissioning (PASE over BLE)
├── Low-power (PDS)      →  Wi-Fi + BLE coordinated sleep (app_lp.cpp)
└── Application
    ├── CHIP_RESET_PIN   (GPIO 2) — long press → factory reset
    └── CHIP_CONTACT_PIN (GPIO 20) — rising edge = true, falling = false
```

-   **Transport**: Wi-Fi with IPv4/IPv6 networking
-   **Commissioning**: BLE → Wi-Fi credential provisioning
-   **Sleep**: PDS (Power-Down Sleep) via `bflb/app_pds.cpp`
-   **Build**: `Bouffalo SDK` + Matter CMake hybrid build

### Build

Activate the build environment first:

```shell
source scripts/activate.sh
```

Build with Wi-Fi:

```shell
make -C examples/contact-sensor-app/bouffalolab
```

#### Clean

```shell
make -C examples/contact-sensor-app/bouffalolab clean
```

### Flash

```shell
make -C examples/contact-sensor-app/bouffalolab flash COMX=<device port>
```

---

## Commissioning

### BL704L (Thread)

Commission over BLE with chip-tool, providing Thread credentials:

```shell
./out/linux-x64-chip-tool/chip-tool pairing ble-thread \
    <device_node_id> hex:<thread_dataset_tlv> 20202021 3840
```

> `<thread_dataset_tlv>` — active Thread dataset in hex TLV format.

### BL616 (Wi-Fi)

Commission over BLE with chip-tool, providing Wi-Fi credentials:

```shell
./out/linux-x64-chip-tool/chip-tool pairing ble-wifi \
    <device_node_id> <wifi_ssid> <wifi_passwd> 20202021 3840
```

> `<device_node_id>` — node ID assigned to the device;<br> > `<wifi_ssid>` —
> Wi-Fi network SSID;<br> > `<wifi_passwd>` — Wi-Fi network password.

---

## Subscribe to BooleanState

Start chip-tool in interactive mode:

```shell
./chip-tool interactive start
```

Subscribe to the BooleanState attribute:

```shell
booleanstate subscribe state-value <min-interval> <max-interval> <device_node_id> 1
```

-   `<min-interval>` — minimum reporting interval (seconds)
-   `<max-interval>` — maximum reporting interval (seconds)
-   `<device_node_id>` — node ID of the contact sensor

Trigger state changes by pressing the contact GPIO button.

---

## References

-   [Bouffalo Lab — Platform overview](../../../docs/platforms/bouffalolab/platform_overview.md)
-   [Bouffalo Lab — Getting Started](../../../docs/platforms/bouffalolab/getting_started.md)
-   [Bouffalo Lab — OTA upgrade](../../../docs/platforms/bouffalolab/ota_upgrade.md)
-   [Bouffalo Lab — Matter factory data](../../../docs/platforms/bouffalolab/matter_factory_data.md)
