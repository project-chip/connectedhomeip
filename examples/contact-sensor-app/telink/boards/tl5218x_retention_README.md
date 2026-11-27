# TL5218X Retention Contact-Sensor App Build Guide

> **Note**: TL5218X retention mode is not yet fully implemented on this
> platform. The configurations below are provided only to keep the app can be
> built; full retention (low-power) support will be added in a future release.

## 1. 2MB Flash (Matter only, NO OTA/MCUBoot, NO LZMA)

**Kconfig:** boards/tl5218x_retention.conf (default)

-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_CHIP_DFU_OVER_BT_SMP=n, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=n
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration disables OTA, does not use MCUBoot, and cannot be
upgraded

**DTS Overlay:** src/platform/telink/tl5218x_2m_flash.overlay (default)

-   slot0: 0x15000, size 0xe7000 (924KB)
-   slot1: 0xfc000, size 0xe7000 (924KB)

**Build:**

```bash
west build -p -b tl5218x_retention -d build_tl5218x_retention_contact_default -- \
> build_tl5218x_retention_contact_default.log
# Kconfig: default (boards/tl5218x_retention.conf)
# DTS Overlay: default (src/platform/telink/tl5218x_2m_flash.overlay)
```

**Output:**

```bash
build_tl5218x_retention_contact_default/zephyr/zephyr.bin          -> flash this
```

## 2. 2MB Flash (Matter only, enable BT DFU, LZMA compressed, SW - 1 and 2)

**IMPORTANT**: LZMA compression is REQUIRED for 2MB flash with OTA.

**Kconfig:** boards/tl5218x_retention_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**DTS Overlay:** src/platform/telink/tl5218x_2m_flash_lzma.overlay (default)

-   slot0: 0x15000, size 0x120000 (1152KB)
-   slot1: 0x135000, size 0xAB000 (684KB)

**Build Software Version 1:**

```bash
west build -p -b tl5218x_retention -d build_tl5218x_retention_contact_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl5218x_retention_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl5218x_retention_contact_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl5218x_retention_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl5218x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl5218x_retention -d build_tl5218x_retention_contact_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl5218x_retention_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl5218x_retention_contact_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl5218x_retention_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl5218x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl5218x_retention_contact_lzma_v1/zephyr/merged.bin          -> flash this
build_tl5218x_retention_contact_lzma_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl5218x_retention_contact_lzma_v2/zephyr/matter.ota          -> OTA upgrade
```

## 3. 2MB Flash (Matter + Zigbee dual mode, LZMA compressed, enable BT DFU)

**Prerequisite:** copy Zigbee firmware

```bash
cp <zigbee_fw>.bin ${ZEPHYR_BASE}/TL521X_FW/ZB/Zigbee-SampleDemo.bin
e.g.
cp ${ZEPHYR_BASE}/TL521X_FW/ZB/sampleContact_tl521x_log.bin ${ZEPHYR_BASE}/TL521X_FW/ZB/Zigbee-SampleDemo.bin
```

**Note:** This dual-mode configuration is for 2MB flash only. LZMA compression
is required due to limited partition space.

**Kconfig:** boards/tl5218x_retention_dual_mode_ota_lzma.conf (explicitly
specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=1 (action switch mode)

**DTS Overlay:** src/platform/telink/tl5218x_2m_flash_lzma.overlay (default)

-   slot0: 0x15000, size 0x120000 (1152KB)
-   slot1: 0x135000, size 0xAB000 (684KB)

**Build:**

```bash
west build -p -b tl5218x_retention -d build_tl5218x_retention_contact_dual_mode -- \
-DCONF_FILE="prj.conf boards/tl5218x_retention_dual_mode_ota_lzma.conf" > build_tl5218x_retention_contact_dual_mode.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl5218x_retention_dual_mode_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl5218x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl5218x_retention_contact_dual_mode/zephyr/merged.bin          -> flash this (sboot+mcuboot+matter+zb)
build_tl5218x_retention_contact_dual_mode/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl5218x_retention_contact_dual_mode/zephyr/matter.ota          -> OTA upgrade
```

## 4. 2MB Flash (Matter only, OTA, NO LZMA, SW - 1 and 2)

**This configuration builds successfully for the contact-sensor app.**

**Kconfig:** boards/tl5218x_retention_ota.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**Build Software Version 1:**

```bash
west build -p -b tl5218x_retention -d build_tl5218x_retention_contact_v1 -- \
-DCONF_FILE="prj.conf boards/tl5218x_retention_ota.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl5218x_retention_contact_v1.log
```

**Build Software Version 2:**

```bash
west build -p -b tl5218x_retention -d build_tl5218x_retention_contact_v2 -- \
-DCONF_FILE="prj.conf boards/tl5218x_retention_ota.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl5218x_retention_contact_v2.log
```

**WARNING**: Building this configuration will show a warning:

> "tl5218x with 2MB flash and OTA requires LZMA compression. Please use a config
> with 'lzma' suffix and set CONFIG_COMPRESS_LZMA=y. Continuing compilation
> anyway..."

**Note**: LZMA compression (target #2) is still recommended to maximize
headroom; without LZMA the firmware must stay within the 924KB (0xe7000)
partition limit.

**Output:**

```bash
build_tl5218x_retention_contact_v1/zephyr/merged.bin    -> flash this
build_tl5218x_retention_contact_v2/zephyr/merged_dfu.bin -> DFU over BLE SMP
build_tl5218x_retention_contact_v2/zephyr/matter.ota     -> OTA upgrade
```
