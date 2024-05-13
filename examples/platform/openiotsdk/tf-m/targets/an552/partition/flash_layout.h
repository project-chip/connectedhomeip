/*
 * Copyright (c) 2019-2021 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

#include "platform_base_address.h"

/* WARNING: Layout below has been moved to QSPI_SRAM and addresses so different now */
/* Flash layout on AN552 with BL2 (multiple image boot):
 *
 * QSPI SRAM S
 * 0x0000_0000 Secure image     primary slot (384 KB)
 * 0x0006_0000 Non-secure image primary slot (2.25 MB)
 * 0x002A_0000 Secure image     secondary slot (384 KB)
 * 0x0030_0000 Non-secure image secondary slot (2.25 MB)
 * 0x0054_0000 Scratch area (1.125 MB)
 * 0x0066_0000 Protected Storage Area (64 KB)
 * 0x0067_0000 OTP / NV counters  area (8 KB)
 * 0x0067_2000 Internal Trusted Storage Area (64 KB)
 * 0x0068_2000 Unused
 *
 * Flash layout on AN552 with BL2 (single image boot):
 *
 * QSPI SRAM S (part1)
 * 0x0000_0000 Primary image area (2.25MB + 384 KB):
 *    0x0000_0000 Secure     image primary (384 KB)
 *    0x0006_0000 Non-secure image primary (2.25 MB)
 * QSPI SRAM S (part2)
 * 0x002A_0000 Secondary image area (2.25MB + 384 KB):
 *    0x002A_0000 Secure     image secondary (384 KB)
 *    0x0030_0000 Non-secure image secondary (2.25 KB)
 * 0x0054_0000 Scratch area (1.125 MB)
 * 0x0066_0000 Protected Storage Area (64 KB)
 * 0x0067_0000 OTP / NV counters  area (8 KB)
 * 0x0067_2000 Internal Trusted Storage Area (64 KB)
 * 0x0068_2000 Unused
 */

/* This header file is included from linker scatter file as well, where only a
 * limited C constructs are allowed. Therefore it is not possible to include
 * here the platform_retarget.h to access flash related defines. To resolve this
 * some of the values are redefined here with different names, these are marked
 * with comment.
 */

/* Size of a Secure and of a Non-secure image */
#define FLASH_S_PARTITION_SIZE (0x60000)   /* S  partition: 384 KB */
#define FLASH_NS_PARTITION_SIZE (0x240000) /* NS partition: 2.25 MB */
#define FLASH_MAX_PARTITION_SIZE                                                                                                   \
    ((FLASH_S_PARTITION_SIZE > FLASH_NS_PARTITION_SIZE) ? FLASH_S_PARTITION_SIZE : FLASH_NS_PARTITION_SIZE)

/* Sector size of the flash hardware; same as FLASH0_SECTOR_SIZE */
#define FLASH_AREA_IMAGE_SECTOR_SIZE (0x1000) /* 4 KB */
/* Same as FLASH0_SIZE */
#define FLASH_TOTAL_SIZE (QSPI_SRAM_SIZE) /* 8 MB */

/* Flash layout info for BL2 bootloader */
/* Same as FLASH0_BASE_S */
#define FLASH0_BASE_ADDRESS (QSPI_SRAM_BASE_S)
#define FLASH_BASE_ADDRESS (QSPI_SRAM_BASE_S)

/* Offset and size definitions of the flash partitions that are handled by the
 * bootloader. The image swapping is done between IMAGE_PRIMARY and
 * IMAGE_SECONDARY, SCRATCH is used as a temporary storage during image
 * swapping.
 */
#define FLASH_AREA_BL2_OFFSET (0x0)
#define FLASH_AREA_BL2_SIZE (ITCM_SIZE) /* 256 KB */

#if !defined(MCUBOOT_IMAGE_NUMBER) || (MCUBOOT_IMAGE_NUMBER == 1)
/* In QSPI SRAM S */
/* Secure + Non-secure image primary slot */
#define FLASH_AREA_0_ID (1)
#define FLASH_AREA_0_OFFSET (0)
#define FLASH_AREA_0_SIZE (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)

/* In QSPI SRAM S */
/* Secure + Non-secure secondary slot */
#define FLASH_AREA_2_ID (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_2_OFFSET (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
#define FLASH_AREA_2_SIZE (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)
/* Scratch area */
#define FLASH_AREA_SCRATCH_ID (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
#define FLASH_AREA_SCRATCH_SIZE (FLASH_MAX_PARTITION_SIZE / 2)
/* The maximum number of status entries supported by the bootloader. */
#define MCUBOOT_STATUS_MAX_ENTRIES ((FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE) / FLASH_AREA_SCRATCH_SIZE)
/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS ((FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE) / FLASH_AREA_IMAGE_SECTOR_SIZE)

#if (FLASH_AREA_SCRATCH_OFFSET + FLASH_AREA_SCRATCH_SIZE > QSPI_SRAM_SIZE)
#error "Out of QSPI SRAM S memory!"
#endif
#elif (MCUBOOT_IMAGE_NUMBER == 2)
/* In QSPI SRAM S */
/* Secure image primary slot */
#define FLASH_AREA_0_ID (1)
#define FLASH_AREA_0_OFFSET (0)
#define FLASH_AREA_0_SIZE (FLASH_S_PARTITION_SIZE)
/* Non-secure image primary slot */
#define FLASH_AREA_1_ID (FLASH_AREA_0_ID + 1)
#define FLASH_AREA_1_OFFSET (FLASH_AREA_0_OFFSET + FLASH_AREA_0_SIZE)
#define FLASH_AREA_1_SIZE (FLASH_NS_PARTITION_SIZE)
/* In QSPI SRAM */
/* Secure image secondary slot */
#define FLASH_AREA_2_ID (FLASH_AREA_1_ID + 1)
#define FLASH_AREA_2_OFFSET (FLASH_AREA_1_OFFSET + FLASH_AREA_1_SIZE)
#define FLASH_AREA_2_SIZE (FLASH_S_PARTITION_SIZE)
/* Non-secure image secondary slot */
#define FLASH_AREA_3_ID (FLASH_AREA_2_ID + 1)
#define FLASH_AREA_3_OFFSET (FLASH_AREA_2_OFFSET + FLASH_AREA_2_SIZE)
#define FLASH_AREA_3_SIZE (FLASH_NS_PARTITION_SIZE)
/* Scratch area */
#define FLASH_AREA_SCRATCH_ID (FLASH_AREA_3_ID + 1)
#define FLASH_AREA_SCRATCH_OFFSET (FLASH_AREA_3_OFFSET + FLASH_AREA_3_SIZE)
#define FLASH_AREA_SCRATCH_SIZE (FLASH_MAX_PARTITION_SIZE / 2)
/* The maximum number of status entries supported by the bootloader. */
#define MCUBOOT_STATUS_MAX_ENTRIES (FLASH_MAX_PARTITION_SIZE / FLASH_AREA_SCRATCH_SIZE)
/* Maximum number of image sectors supported by the bootloader. */
#define MCUBOOT_MAX_IMG_SECTORS (FLASH_MAX_PARTITION_SIZE / FLASH_AREA_IMAGE_SECTOR_SIZE)

#if (FLASH_AREA_SCRATCH_OFFSET + FLASH_AREA_SCRATCH_SIZE > QSPI_SRAM_SIZE)
#error "Out of QSPI SRAM S memory!"
#endif

#else /* MCUBOOT_IMAGE_NUMBER > 2 */
#error "Only MCUBOOT_IMAGE_NUMBER 1 and 2 are supported!"
#endif /* MCUBOOT_IMAGE_NUMBER */

/* Protected Storage (PS) Service definitions */
#define FLASH_PS_AREA_OFFSET (FLASH_AREA_SCRATCH_OFFSET + FLASH_AREA_SCRATCH_SIZE)
#define FLASH_PS_AREA_SIZE (0x10000) /* 64 KB */

/* OTP_definitions */
#define FLASH_OTP_NV_COUNTERS_AREA_OFFSET (FLASH_PS_AREA_OFFSET + FLASH_PS_AREA_SIZE)
#define FLASH_OTP_NV_COUNTERS_AREA_SIZE (FLASH_AREA_IMAGE_SECTOR_SIZE * 2)
#define FLASH_OTP_NV_COUNTERS_SECTOR_SIZE FLASH_AREA_IMAGE_SECTOR_SIZE

/* Internal Trusted Storage (ITS) Service definitions */
#define FLASH_ITS_AREA_OFFSET (FLASH_OTP_NV_COUNTERS_AREA_OFFSET + FLASH_OTP_NV_COUNTERS_AREA_SIZE)
#define FLASH_ITS_AREA_SIZE (0x10000) /* 64 KB */

#if (FLASH_ITS_AREA_OFFSET + FLASH_ITS_AREA_SIZE > QSPI_SRAM_SIZE)
#error "Out of QSPI SRAM memory!"
#endif

/* Offset and size definition in flash area used by assemble.py */
#define SECURE_IMAGE_OFFSET (0x0)
#define SECURE_IMAGE_MAX_SIZE FLASH_S_PARTITION_SIZE

#define NON_SECURE_IMAGE_OFFSET (SECURE_IMAGE_OFFSET + SECURE_IMAGE_MAX_SIZE)
#define NON_SECURE_IMAGE_MAX_SIZE FLASH_NS_PARTITION_SIZE

/* Flash device name used by BL2
 * Name is defined in flash driver file: Driver_Flash.c
 */
#define FLASH_DEV_NAME_0 Driver_FLASH0
#define FLASH_DEVICE_ID_0 102
#define FLASH_DEV_NAME_1 Driver_FLASH0
#define FLASH_DEVICE_ID_1 102
#define FLASH_DEV_NAME_SCRATCH Driver_FLASH0
#define FLASH_DEVICE_ID_SCRATCH 103

#define FLASH_DEV_NAME FLASH_DEV_NAME_0

/* Smallest flash programmable unit in bytes */
#define TFM_HAL_FLASH_PROGRAM_UNIT (0x1)

/* Protected Storage (PS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M PS Integration Guide.
 */
#define TFM_HAL_PS_FLASH_DRIVER Driver_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
/* Base address of dedicated flash area for PS */
#define TFM_HAL_PS_FLASH_AREA_ADDR FLASH_PS_AREA_OFFSET
/* Size of dedicated flash area for PS */
#define TFM_HAL_PS_FLASH_AREA_SIZE FLASH_PS_AREA_SIZE
#define PS_RAM_FS_SIZE TFM_HAL_PS_FLASH_AREA_SIZE
/* Number of physical erase sectors per logical FS block */
#define TFM_HAL_PS_SECTORS_PER_BLOCK (1)
/* Smallest flash programmable unit in bytes */
#define TFM_HAL_PS_PROGRAM_UNIT (0x1)

/* Internal Trusted Storage (ITS) Service definitions
 * Note: Further documentation of these definitions can be found in the
 * TF-M ITS Integration Guide. The ITS should be in the internal flash, but is
 * allocated in the external flash just for development platforms that don't
 * have internal flash available.
 */
#define TFM_HAL_ITS_FLASH_DRIVER Driver_FLASH0

/* In this target the CMSIS driver requires only the offset from the base
 * address instead of the full memory address.
 */
/* Base address of dedicated flash area for ITS */
#define TFM_HAL_ITS_FLASH_AREA_ADDR FLASH_ITS_AREA_OFFSET
/* Size of dedicated flash area for ITS */
#define TFM_HAL_ITS_FLASH_AREA_SIZE FLASH_ITS_AREA_SIZE
#define ITS_RAM_FS_SIZE TFM_HAL_ITS_FLASH_AREA_SIZE
/* Number of physical erase sectors per logical FS block */
#define TFM_HAL_ITS_SECTORS_PER_BLOCK (1)
/* Smallest flash programmable unit in bytes */
#define TFM_HAL_ITS_PROGRAM_UNIT (0x1)

/* OTP / NV counter definitions */
#define TFM_OTP_NV_COUNTERS_AREA_SIZE (FLASH_OTP_NV_COUNTERS_AREA_SIZE / 2)
#define TFM_OTP_NV_COUNTERS_AREA_ADDR FLASH_OTP_NV_COUNTERS_AREA_OFFSET
#define TFM_OTP_NV_COUNTERS_SECTOR_SIZE FLASH_OTP_NV_COUNTERS_SECTOR_SIZE
#define TFM_OTP_NV_COUNTERS_BACKUP_AREA_ADDR (TFM_OTP_NV_COUNTERS_AREA_ADDR + TFM_OTP_NV_COUNTERS_AREA_SIZE)

#endif /* __FLASH_LAYOUT_H__ */
