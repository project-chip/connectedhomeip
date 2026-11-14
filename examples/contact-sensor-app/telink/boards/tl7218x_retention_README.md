# TL7218X Retention Contact-Sensor App Build Guide

## 1. 2MB Flash (Matter only, NO OTA/MCUBoot, NO LZMA)

**Kconfig:** boards/tl7218x_retention.conf (default)

-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_CHIP_DFU_OVER_BT_SMP=n, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=n
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration disables OTA, does not use MCUBoot, and cannot be
upgraded

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash.overlay (default)

-   slot0: 0x15000, size 0xe5000 (916KB)
-   slot1: 0xfa000, size 0xe6000 (920KB)

**Build:**

```bash
west build -p -b tl7218x_retention -d build_tl7218x_retention_contact_default -- \
> build_tl7218x_retention_contact_default.log
# Kconfig: default (boards/tl7218x_retention.conf)
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash.overlay)
```

**Output:**

```bash
build_tl7218x_retention_contact_default/zephyr/zephyr.bin          -> flash this
```

## 2. 2MB Flash (Matter only, enable BT DFU, LZMA compressed, SW - 1 and 2)

**IMPORTANT**: LZMA compression is REQUIRED for 2MB flash with OTA.

**Kconfig:** boards/tl7218x_retention_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash_lzma.overlay (default)

-   slot0: 0x15000, size 0x120000 (1152KB)
-   slot1: 0x135000, size 0xAB000 (684KB)

**Build Software Version 1:**

```bash
west build -p -b tl7218x_retention -d build_tl7218x_retention_contact_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl7218x_retention_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl7218x_retention_contact_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_retention_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl7218x_retention -d build_tl7218x_retention_contact_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl7218x_retention_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl7218x_retention_contact_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_retention_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl7218x_retention_contact_lzma_v1/zephyr/merged.bin          -> flash this
build_tl7218x_retention_contact_lzma_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl7218x_retention_contact_lzma_v2/zephyr/matter.ota          -> OTA upgrade
```

## 3. 2MB Flash (Matter only, OTA, NO LZMA, SW - 1 and 2)

**This configuration builds successfully for the contact-sensor app.**

**Kconfig:** boards/tl7218x_retention_ota.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**Build Software Version 1:**

```bash
west build -p -b tl7218x_retention -d build_tl7218x_retention_contact_v1 -- \
-DCONF_FILE="prj.conf boards/tl7218x_retention_ota.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl7218x_retention_contact_v1.log
```

**Build Software Version 2:**

```bash
west build -p -b tl7218x_retention -d build_tl7218x_retention_contact_v2 -- \
-DCONF_FILE="prj.conf boards/tl7218x_retention_ota.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl7218x_retention_contact_v2.log
```

**WARNING**: Building this configuration will show a warning:

> "tl7218x with 2MB flash and OTA requires LZMA compression. Please use a config
> with 'lzma' suffix and set CONFIG_COMPRESS_LZMA=y. Continuing compilation
> anyway..."

**Note**: LZMA compression (target #2) is still recommended to maximize
headroom; without LZMA the firmware must stay within the 916KB (0xe5000)
partition limit.

**Output:**

```bash
build_tl7218x_retention_contact_v1/zephyr/merged.bin    -> flash this
build_tl7218x_retention_contact_v2/zephyr/merged_dfu.bin -> DFU over BLE SMP
build_tl7218x_retention_contact_v2/zephyr/matter.ota     -> OTA upgrade
```

**Important Note**: TL7218X does NOT support dual_mode (Matter + Zigbee)
configuration.
