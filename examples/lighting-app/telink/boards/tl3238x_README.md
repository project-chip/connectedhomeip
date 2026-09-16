# TL3238X Lighting App Build Guide

## 1. 2MB Flash (Matter only, NO OTA/MCUBoot, NO LZMA)

**Kconfig:** boards/tl3238x.conf (default)

-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_CHIP_DFU_OVER_BT_SMP=n, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=n,
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration disables OTA, does not use MCUBoot, and cannot be
upgraded

**DTS Overlay:** src/platform/telink/tl3238x_2m_flash.overlay (default)

-   slot0: 0x15000, size 0xe5000 (916KB)
-   slot1: 0xfa000, size 0xe6000 (920KB)

**Build:**

```bash
west build -p -b tl3238x -d build_tl3238x_default -- \
> build_tl3238x_default.log
# Kconfig: default (boards/tl3238x.conf)
# DTS Overlay: default (src/platform/telink/tl3238x_2m_flash.overlay)
```

**Output:**

```bash
build_tl3238x_default/zephyr/zephyr.bin          -> flash this
```

## 2. 2MB Flash (Matter only, enable BT DFU, LZMA compressed, SW - 1 and 2)

**IMPORTANT**: LZMA compression is REQUIRED for 2MB flash with OTA.

**Kconfig:** boards/tl3238x_2m_flash_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**DTS Overlay:** src/platform/telink/tl3238x_2m_flash_lzma.overlay (default)

-   slot0: 0x15000, size 0x120000 (1152KB)
-   slot1: 0x135000, size 0xAB000 (684KB)

**Build Software Version 1:**

```bash
west build -p -b tl3238x -d build_tl3238x_2m_flash_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl3238x_2m_flash_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl3238x_2m_flash_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl3238x_2m_flash_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl3238x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl3238x -d build_tl3238x_2m_flash_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl3238x_2m_flash_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl3238x_2m_flash_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl3238x_2m_flash_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl3238x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl3238x_2m_flash_lzma_v1/zephyr/merged.bin          -> flash this
build_tl3238x_2m_flash_lzma_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl3238x_2m_flash_lzma_v2/zephyr/matter.ota          -> OTA upgrade
```

## 3. 4MB Flash (Matter + Zigbee dual mode, no LZMA)

**Prerequisite:** copy Zigbee firmware

```bash
cp <zigbee_fw>.bin ${ZEPHYR_BASE}/TL323X_FW/ZB/Zigbee-SampleDemo.bin
e.g.
cp ${ZEPHYR_BASE}/TL323X_FW/ZB/dual_matter_sampleLight_bleAdv_tl323x.bin ${ZEPHYR_BASE}/TL323X_FW/ZB/Zigbee-SampleDemo.bin
```

**Kconfig:** boards/tl3238x_4m_flash_dual_mode_ota.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=2 (auto-switch mode)

**DTS Overlay:** src/platform/telink/tl3238x_4m_flash.overlay (default)

-   slot0: 0x16000, size 0x1E5000 (1940KB)
-   slot1: 0x1FB000, size 0x1E5000 (1940KB)

**Board DTS Overlay:** boards/tl3238x_for_TL3238C-EVK40D.overlay (explicitly
specified)

-   LED/Key definitions for TL3238C-EVK40D board

**Build:**

```bash
west build -p -b tl3238x -d build_tl3238x_4m_dual_mode -- \
-DFLASH_SIZE=4m \
-DCONF_FILE="prj.conf boards/tl3238x_4m_flash_dual_mode_ota.conf" \
-DDTC_OVERLAY_FILE="boards/tl3238x_for_TL3238C-EVK40D.overlay" > build_tl3238x_4m_dual_mode.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl3238x_4m_flash_dual_mode_ota.conf")
# DTS Overlay: default (src/platform/telink/tl3238x_4m_flash.overlay) + explicit (boards/tl3238x_for_TL3238C-EVK40D.overlay)
```

**Output:**

```bash
build_tl3238x_4m_dual_mode/zephyr/merged.bin      -> flash this (sboot+mcuboot+matter+zb)
build_tl3238x_4m_dual_mode/zephyr/merged_dfu.bin  -> DFU over BLE SMP
build_tl3238x_4m_dual_mode/zephyr/matter.ota      -> OTA upgrade
```

## 4. WARNING: 2MB Flash with OTA but NO LZMA (NOT RECOMMENDED)

**This configuration will show a warning during build and may fail at link
time.**

**Kconfig:** boards/tl3238x_ota.conf (explicitly specified)

-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=n, CONFIG_COMPRESS_LZMA=n
-   CONFIG_DUAL_MODE=0

**WARNING**: Building this configuration will show a warning:

> "tl3238x with 2MB flash and OTA requires LZMA compression. Please use a config
> with 'lzma' suffix and set CONFIG_COMPRESS_LZMA=y. Continuing compilation
> anyway..."

**Note**: Build may fail at link time if firmware size exceeds 916KB (0xe5000)
partition limit.

**Solution**: Use the LZMA-enabled configuration (target #2) instead.

## 5. Concurrent Mode (Matter over Thread + BLE coexistence, NO OTA)

**Kconfig:** boards/tl3238x_concurrent.conf (explicitly specified)

-   CONFIG_CHIP_CONCURRENT_MODE=y
-   CONFIG_IEEE802154_TLX_BLE_COEXIST=y
-   CONFIG_CHIP_OTA_REQUESTOR=n
-   CONFIG_DUAL_MODE=0

**Note**: This configuration enables BLE + 802.15.4 coexistence for Matter over
Thread with BLE advertising. OTA is disabled.

**DTS Overlay:** src/platform/telink/tl3238x_2m_flash.overlay (default)

**Build:**

```bash
west build -p -b tl3238x -d build_tl3238x_concurrent -- \
-DCONF_FILE="prj.conf boards/tl3238x_concurrent.conf" > build_tl3238x_concurrent.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl3238x_concurrent.conf")
# DTS Overlay: default (src/platform/telink/tl3238x_2m_flash.overlay)
```

**Output:**

```bash
build_tl3238x_concurrent/zephyr/zephyr.bin          -> flash this
```

**Note**: TL3238X does NOT support Channel Sounding. Use TL7218X for concurrent
mode + Channel Sounding (see boards/tl7218x_concurrent_cs.conf).

## 6. Concurrent Mode (Matter over Thread + BLE coexistence, with OTA)

**Kconfig:** boards/tl3238x_concurrent_ota_lzma.conf (explicitly specified)

-   CONFIG_CHIP_CONCURRENT_MODE=y
-   CONFIG_IEEE802154_TLX_BLE_COEXIST=y
-   CONFIG_CHIP_OTA_REQUESTOR=y
-   CONFIG_CHIP_DFU_OVER_BT_SMP=y, CONFIG_CHIP_DFU_OVER_BT_SMP_BUILD=y,
-   CONFIG_LZMA=y, CONFIG_COMPRESS_LZMA=y
-   CONFIG_DUAL_MODE=0

**Note**: This configuration enables BLE + 802.15.4 coexistence for Matter over
Thread with BLE advertising and OTA support. LZMA compression is required for
2MB flash with OTA.

**DTS Overlay:** src/platform/telink/tl3238x_2m_flash_lzma.overlay (default)

**Build Software Version 1:**

```bash
west build -p -b tl3238x -d build_tl3238x_concurrent_ota_lzma_v1 -- \
-DCONF_FILE="prj.conf boards/tl3238x_concurrent_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1 > build_tl3238x_concurrent_ota_lzma_v1.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl3238x_concurrent_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl3238x_2m_flash_lzma.overlay)
```

**Build Software Version 2:**

```bash
west build -p -b tl3238x -d build_tl3238x_concurrent_ota_lzma_v2 -- \
-DCONF_FILE="prj.conf boards/tl3238x_concurrent_ota_lzma.conf" \
-DCONFIG_CHIP_DEVICE_SOFTWARE_VERSION=2 > build_tl3238x_concurrent_ota_lzma_v2.log
# Kconfig: explicitly specified (-DCONF_FILE="prj.conf boards/tl3238x_concurrent_ota_lzma.conf")
# DTS Overlay: default (src/platform/telink/tl3238x_2m_flash_lzma.overlay)
```

**Output:**

```bash
build_tl3238x_concurrent_ota_lzma_v1/zephyr/merged.bin          -> flash this
build_tl3238x_concurrent_ota_lzma_v2/zephyr/merged_dfu.lzma.bin -> DFU over BLE SMP
build_tl3238x_concurrent_ota_lzma_v2/zephyr/matter.ota          -> OTA upgrade
```
