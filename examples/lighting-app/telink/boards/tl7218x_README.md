# TL7218X Lighting App Build Guide

## 1. 2MB Flash (Matter only, NO OTA/MCUBoot, NO LZMA)

**Kconfig:** boards/tl7218x.conf (default)

-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_CHIP_DFU_OVER_BT_SMP=n, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=n,
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration disables OTA, does not use MCUBoot, and cannot be
upgraded

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash.overlay (default)

-   slot0: 0x15000, size 0xe5000 (916KB)
-   slot1: 0xfa000, size 0xe6000 (920KB)

**Build:**

```bash
west build -p -b tl7218x -d build_tl7218x_default -- \
> build_tl7218x_default.log
# Kconfig: default (boards/tl7218x.conf)
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash.overlay)
```

**Output:**

```bash
build_tl7218x_default/zephyr/zephyr.bin          -> flash this
```

## 2. 2MB Flash (Matter only, enable BT DFU, LZMA compressed, SW - 1 and 2)

**IMPORTANT**: LZMA compression is REQUIRED for 2MB flash with OTA.

**Kconfig:** boards/tl7218x_2m_flash_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash_lzma.overlay (default)

-   slot0: 0x15000, size 0x120000 (1152KB)
-   slot1: 0x135000, size 0xAB000 (684KB)

**Build Software Version 1:**

```bash
west build -p -b tl7218x -d build_tl7218x_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl7218x_2m_flash_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl7218x_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_2m_flash_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl7218x -d build_tl7218x_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl7218x_2m_flash_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl7218x_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_2m_flash_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl7218x_lzma_v1/zephyr/merged.bin          -> flash this
build_tl7218x_lzma_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl7218x_lzma_v2/zephyr/matter.ota          -> OTA upgrade
```

## 3. WARNING: 2MB Flash with OTA but NO LZMA (NOT RECOMMENDED)

**This configuration will show a warning during build and may fail at link
time.**

**Kconfig:** boards/tl7218x_ota.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**WARNING**: Building this configuration will show a warning:

> "tl7218x with 2MB flash and OTA requires LZMA compression. Please use a config
> with 'lzma' suffix and set CONFIG_COMPRESS_LZMA=y. Continuing compilation
> anyway..."

**Note**: Build may fail at link time if firmware size exceeds 916KB (0xe5000)
partition limit.

**Solution**: Use the LZMA-enabled configuration (target #2) instead.

**Important Note**: TL7218X does NOT support dual_mode (Matter + Zigbee)
configuration.

## 4. Concurrent Mode (Matter over Thread + BLE coexistence, NO OTA)

**Kconfig:** boards/tl7218x_concurrent.conf (explicitly specified)

-   CONFIG_CHIP_CONCURRENT_MODE=y
-   CONFIG_IEEE802154_TLX_BLE_COEXIST=y
-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration enables BLE + 802.15.4 coexistence for Matter over
Thread with BLE advertising. OTA is disabled.

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash.overlay (default)

**Build:**

```bash
west build -p -b tl7218x -d build_tl7218x_concurrent -- \
-DCONF_FILE="prj.conf boards/tl7218x_concurrent.conf" > build_tl7218x_concurrent.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_concurrent.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash.overlay)
```

**Output:**

```bash
build_tl7218x_concurrent/zephyr/zephyr.bin          -> flash this
```

## 5. Concurrent Mode + Channel Sounding (Matter over Thread + BLE + CS, NO OTA)

**Kconfig:** boards/tl7218x_concurrent_cs.conf (explicitly specified)

-   CONFIG_CHIP_CONCURRENT_MODE=y
-   CONFIG_IEEE802154_TLX_BLE_COEXIST=y
-   CONFIG_BT_CHANNEL_SOUNDING=y, CONFIG_BT_TLX_CHANNEL_SOUNDING=y
-   CONFIG_BT_SMP=y (unauthenticated pairing, LE Security Mode 1 Level 2)
-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration enables BLE + 802.15.4 coexistence with Channel
Sounding (RAS Reflector) support. BLE buffers, SMP bonding, and stack sizes are
tuned for CS + OT coexistence. OTA is disabled.

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash.overlay (default)

**Build:**

```bash
west build -p -b tl7218x -d build_tl7218x_concurrent_cs -- \
-DCONF_FILE="prj.conf boards/tl7218x_concurrent_cs.conf" > build_tl7218x_concurrent_cs.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_concurrent_cs.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash.overlay)
```

**Output:**

```bash
build_tl7218x_concurrent_cs/zephyr/zephyr.bin          -> flash this
```

## 6. Concurrent Mode (Matter over Thread + BLE coexistence, with OTA)

**Kconfig:** boards/tl7218x_concurrent_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_CONCURRENT_MODE=y
-   CONFIG_IEEE802154_TLX_BLE_COEXIST=y
-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**Note**: This configuration enables BLE + 802.15.4 coexistence for Matter over
Thread with BLE advertising and OTA support. LZMA compression is required for
2MB flash with OTA.

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash_lzma.overlay (default)

**Build Software Version 1:**

```bash
west build -p -b tl7218x -d build_tl7218x_concurrent_ota_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl7218x_concurrent_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl7218x_concurrent_ota_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_concurrent_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl7218x -d build_tl7218x_concurrent_ota_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl7218x_concurrent_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl7218x_concurrent_ota_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_concurrent_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl7218x_concurrent_ota_v1/zephyr/merged.bin          -> flash this
build_tl7218x_concurrent_ota_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl7218x_concurrent_ota_v2/zephyr/matter.ota          -> OTA upgrade
```

## 7. Concurrent Mode + Channel Sounding (Matter over Thread + BLE + CS, with OTA)

**Kconfig:** boards/tl7218x_concurrent_cs_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_CONCURRENT_MODE=y
-   CONFIG_IEEE802154_TLX_BLE_COEXIST=y
-   CONFIG_BT_CHANNEL_SOUNDING=y, CONFIG_BT_TLX_CHANNEL_SOUNDING=y
-   CONFIG_BT_SMP=y (unauthenticated pairing, LE Security Mode 1 Level 2)
-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**Note**: This configuration enables BLE + 802.15.4 coexistence with Channel
Sounding (RAS Reflector) and OTA support. LZMA compression is required for 2MB
flash with OTA.

**DTS Overlay:** src/platform/telink/tl7218x_2m_flash_lzma.overlay (default)

**Build Software Version 1:**

```bash
west build -p -b tl7218x -d build_tl7218x_concurrent_cs_ota_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl7218x_concurrent_cs_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl7218x_concurrent_cs_ota_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_concurrent_cs_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl7218x -d build_tl7218x_concurrent_cs_ota_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl7218x_concurrent_cs_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl7218x_concurrent_cs_ota_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl7218x_concurrent_cs_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl7218x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl7218x_concurrent_cs_ota_v1/zephyr/merged.bin          -> flash this
build_tl7218x_concurrent_cs_ota_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl7218x_concurrent_cs_ota_v2/zephyr/matter.ota          -> OTA upgrade
```
