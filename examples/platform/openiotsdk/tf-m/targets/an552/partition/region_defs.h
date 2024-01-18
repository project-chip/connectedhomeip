/*
 * Copyright (c) 2019-2022 Arm Limited. All rights reserved.
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

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"
#include "platform_base_address.h"

#define BL2_HEAP_SIZE (0x0001000)
#define BL2_MSP_STACK_SIZE (0x0001800)

#ifdef ENABLE_HEAP
#define S_HEAP_SIZE (0x0000200)
#endif

#define S_MSP_STACK_SIZE (0x0000800)
#define S_PSP_STACK_SIZE (0x0000800)

#define NS_HEAP_SIZE (0x0008000)
#define NS_STACK_SIZE (0x0002000)

/* This size of buffer is big enough to store an attestation
 * token produced by initial attestation service
 */
#define PSA_INITIAL_ATTEST_TOKEN_MAX_SIZE (0x250)

#ifdef BL2
#ifndef LINK_TO_SECONDARY_PARTITION
#define S_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_0_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET)
#else
#define S_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_0_OFFSET)
#endif /* !LINK_TO_SECONDARY_PARTITION */
#else
#define S_IMAGE_PRIMARY_PARTITION_OFFSET (0x0)
#endif /* BL2 */

#ifndef LINK_TO_SECONDARY_PARTITION
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_0_OFFSET + FLASH_S_PARTITION_SIZE)
#else
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET + FLASH_S_PARTITION_SIZE)
#endif /* !LINK_TO_SECONDARY_PARTITION */

/* Boot partition structure if MCUBoot is used:
 * 0x0_0000 Bootloader header
 * 0x0_0400 Image area
 * 0x5_0000 Trailer
 */
/* IMAGE_CODE_SIZE is the space available for the software binary image.
 * It is less than the FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE
 * because we reserve space for the image header and trailer introduced
 * by the bootloader.
 */
#if (!defined(MCUBOOT_IMAGE_NUMBER) || (MCUBOOT_IMAGE_NUMBER == 1)) &&                                                             \
    (NS_IMAGE_PRIMARY_PARTITION_OFFSET > S_IMAGE_PRIMARY_PARTITION_OFFSET)
/* If secure image and nonsecure image are concatenated, and nonsecure image
 * locates at the higher memory range, then the secure image does not need
 * the trailer area.
 */
#define IMAGE_S_CODE_SIZE (FLASH_S_PARTITION_SIZE - BL2_HEADER_SIZE)
#else
#define IMAGE_S_CODE_SIZE (FLASH_S_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#endif

#define IMAGE_NS_CODE_SIZE (FLASH_NS_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)

/* Secure regions */
#define S_IMAGE_PRIMARY_AREA_OFFSET (S_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
/* Secure Code stored in Code SRAM */
#define S_CODE_START ((QSPI_SRAM_BASE_S) + (S_IMAGE_PRIMARY_AREA_OFFSET))
#define S_CODE_SIZE (IMAGE_S_CODE_SIZE)
#define S_CODE_LIMIT (S_CODE_START + S_CODE_SIZE - 1)

/* Secure Data stored in DTCM */
#define S_DATA_START (DTCM0_BASE_S)
#define S_DATA_SIZE (DTCM_BLK_SIZE * DTCM_BLK_NUM)
#define S_DATA_LIMIT (S_DATA_START + S_DATA_SIZE - 1)

/* Size of vector table: 146 interrupt handlers + 4 bytes MPS initial value */
#define S_CODE_VECTOR_TABLE_SIZE (0x24C)

/* Non-secure regions */
#define NS_IMAGE_PRIMARY_AREA_OFFSET (NS_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
/* Non-Secure Code stored in Code SRAM memory */
#define NS_CODE_START (QSPI_SRAM_BASE_NS + (NS_IMAGE_PRIMARY_AREA_OFFSET))
#define NS_CODE_SIZE (IMAGE_NS_CODE_SIZE)
#define NS_CODE_LIMIT (NS_CODE_START + NS_CODE_SIZE - 1)

/* Non-Secure Data stored in ISRAM0 + ISRAM1 */
#define NS_DATA_START (ISRAM0_BASE_NS)
#define NS_DATA_SIZE (ISRAM0_SIZE + ISRAM1_SIZE)
#define NS_DATA_LIMIT (NS_DATA_START + NS_DATA_SIZE - 1)

/* NS partition information is used for MPC and SAU configuration */
#define NS_PARTITION_START ((QSPI_SRAM_BASE_NS) + (NS_IMAGE_PRIMARY_PARTITION_OFFSET))
#define NS_PARTITION_SIZE (FLASH_NS_PARTITION_SIZE)

/* Secondary partition for new images in case of firmware upgrade */
#define SECONDARY_PARTITION_START ((QSPI_SRAM_BASE_NS) + (S_IMAGE_SECONDARY_PARTITION_OFFSET))
#define SECONDARY_PARTITION_SIZE (FLASH_S_PARTITION_SIZE + FLASH_NS_PARTITION_SIZE)

#ifdef BL2
/* Bootloader regions */
/* Use ITCM to store Bootloader */
#define BL2_CODE_START (ITCM_BASE_S)
#define BL2_CODE_SIZE (ITCM_SIZE)
#define BL2_CODE_LIMIT (BL2_CODE_START + BL2_CODE_SIZE - 1)

/* Bootloader uses same memory as for secure image */
#define BL2_DATA_START (S_DATA_START)
#define BL2_DATA_SIZE (S_DATA_SIZE)
#define BL2_DATA_LIMIT (BL2_DATA_START + BL2_DATA_SIZE - 1)
#endif /* BL2 */

/* Shared data area between bootloader and runtime firmware.
 * Shared data area is allocated at the beginning of the RAM, it is overlapping
 * with TF-M Secure code's MSP stack
 */
#define BOOT_TFM_SHARED_DATA_BASE S_DATA_START
#define BOOT_TFM_SHARED_DATA_SIZE (0x400)
#define BOOT_TFM_SHARED_DATA_LIMIT (BOOT_TFM_SHARED_DATA_BASE + BOOT_TFM_SHARED_DATA_SIZE - 1)

#endif /* __REGION_DEFS_H__ */
