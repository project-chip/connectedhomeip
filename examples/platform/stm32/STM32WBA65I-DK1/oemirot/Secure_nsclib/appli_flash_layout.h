/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 ******************************************************************************
 * @file    appli_flash_layout.h
 * @author  MCD Application Team
 * @brief   This file contains definitions of memory regions for stm32wbaxx.
 ******************************************************************************
 */

#ifndef __APPLI_FLASH_LAYOUT_H__
#define __APPLI_FLASH_LAYOUT_H__

/********** Application Flash layout configuration : begin **********/

#define S_HEAP_SIZE 0x0000200    /* EWARM / MDK-ARM / STM32CubeIDE */
#define S_C_STACK_SIZE 0x0000400 /* EWARM / STM32CubeIDE */

#define S_MSP_STACK_SIZE 0x0000200 /* MDK-ARM */
#define S_PSP_STACK_SIZE 0x0000700 /* MDK-ARM */

#define NS_HEAP_SIZE 0x0003000      /* EWARM / MDK-ARM / STM32CubeIDE */
#define NS_C_STACK_SIZE 0x0001000   /* EWARM / STM32CubeIDE */
#define NS_MSP_STACK_SIZE 0x0000C00 /* MDK-ARM */
#define NS_PSP_STACK_SIZE 0x0000C00 /* MDK-ARM */

#define NSC_CODE_SIZE 0x400

#define PARTITION_RESERVED 0x1C00

/********** Application Flash layout configuration : end **********/

/********** All define are updated automatically from ROT_BOOT project : begin **********/

/*#define OEMUROT_ENABLE*/ /* Defined: the project is used for OEMuRoT boot stage
                             Undefined: the project is used for OEMiRoT boot stage */

#define MCUBOOT_OVERWRITE_ONLY /* Defined: the FW installation uses overwrite method.                                              \
                                  UnDefined: The FW installation uses swap mode. */

#define OEMIROT_EXTERNAL_FLASH_ENABLE /* Defined: External OSPI flash used for all secondary slots.                                \
                                            Undefined: External OSPI flash not used. */

#define MCUBOOT_APP_IMAGE_NUMBER                                                                                                   \
    0x2 /* 1: S and NS application binaries are assembled in one single image.                                                     \
           2: Two separated images for S and NS application binaries. */

#define MCUBOOT_S_DATA_IMAGE_NUMBER                                                                                                \
    0x0 /* 1: S data image for S application.                                                                                      \
           0: No S data image. */

#define MCUBOOT_NS_DATA_IMAGE_NUMBER                                                                                               \
    0x0 /* 1: NS data image for NS application.                                                                                    \
           0: No NS data image. */

#define FLASH_AREA_0_OFFSET 0x18000 /* Secure app image primary slot offset */

#define FLASH_AREA_0_SIZE 0x6000 /* Secure app image primary slot size */

#define FLASH_AREA_1_OFFSET 0x1E000 /* Non-secure app image primary slot offset */

#define FLASH_AREA_1_SIZE 0x1C6000 /* Non-secure app image primary slot size */

#define FLASH_AREA_2_OFFSET 0x0 /* Secure app image secondary slot */

#define FLASH_AREA_2_SIZE 0x6000 /* Secure app image secondary slot size */

#define FLASH_AREA_3_OFFSET 0x6000 /* Non-secure app image secondary slot offset */

#define FLASH_AREA_3_SIZE 0x1C6000 /* Non-secure app image secondary slot size */

#define FLASH_AREA_4_OFFSET 0x0 /* Secure data image primary slot offset */

#define FLASH_AREA_4_SIZE 0x0 /* Secure data image primary slot size */

#define FLASH_AREA_5_OFFSET 0x0 /* Non-secure data image primary slot offset */

#define FLASH_AREA_5_SIZE 0x0 /* Non-secure data image primary slot size */

#define FLASH_AREA_6_OFFSET 0x0 /* Non-secure data image primary slot offset */

#define FLASH_AREA_6_SIZE 0x0 /* Non-secure data image primary slot size */

#define FLASH_AREA_7_OFFSET 0x0 /* Non-secure data image primary slot offset */

#define FLASH_AREA_7_SIZE 0x0 /* Non-secure data image primary slot size */

#define FLASH_PARTITION_SIZE 0x1CC000 /* Secure and Non Secure partition size */

#define FLASH_NS_PARTITION_SIZE 0x1C6000 /* Non Secure partition size */

#define FLASH_S_PARTITION_SIZE 0x6000 /* secure partition size */

#define FLASH_S_DATA_PARTITION_SIZE 0x0 /* secure data partition size */

#define FLASH_NS_DATA_PARTITION_SIZE 0x0 /* non secure data partition size */

#define FLASH_B_SIZE 0x100000 /* flash bank size: 1 MBytes*/

#define OEMIROT_AREA_0_OFFSET 0x0 /* Secure app image primary slot for OEMuRoT boot stage */

#define OEMIROT_AREA_0_SIZE 0x0 /* Secure app image primary slot size for OEMuRoT boot stage */

#define OEMIROT_AREA_2_OFFSET 0x0 /* Secure app image secondary slot for OEMuRoT boot stage */

#define OEMIROT_AREA_2_SIZE 0x0 /* Secure app image secondary slot size for OEMuRoT boot stage */

/********** All define are updated automatically from ROT_BOOT project : End **********/

#if defined(STM32WBA65xx)
#define FLASH_TOTAL_SIZE (FLASH_B_SIZE + FLASH_B_SIZE) /* total flash size: 2 MBytes */
#elif defined(STM32WBA52xx) || defined(STM32WBA55xx)
#define FLASH_TOTAL_SIZE (FLASH_B_SIZE) /* total flash size: 1 MBytes */
#else
#error "Unknown target."
#endif

#if defined(OEMIROT_EXTERNAL_FLASH_ENABLE)
/* External SPI Flash layout info for BL2 bootloader */
#define SPI_FLASH_TOTAL_SIZE (0x400000) /* 32 Mbits */
#define SPI_FLASH_BASE_ADDRESS (0x00000000)
#endif /* OEMIROT_EXTERNAL_FLASH_ENABLE */

#define S_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_0_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET)
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_0_OFFSET + FLASH_S_PARTITION_SIZE)
#define NS_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_2_OFFSET + FLASH_S_PARTITION_SIZE)
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
#define S_DATA_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_4_OFFSET)
#define S_DATA_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_6_OFFSET)
#endif /* MCUBOOT_S_DATA_IMAGE_NUMBER == 1 */
#if (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
#define NS_DATA_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_5_OFFSET)
#define NS_DATA_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_7_OFFSET)
#endif /* MCUBOOT_NS_DATA_IMAGE_NUMBER == 1 */

/*
 * Flash device name
 */
#define FLASH_DEV_NAME Driver_FLASH0
#if defined(OEMIROT_EXTERNAL_FLASH_ENABLE)
#define SPI_FLASH_DEV_NAME Driver_SPI_FLASH0
#endif /*  defined(OEMIROT_EXTERNAL_FLASH_ENABLE)  */

#define FLASH_PRIMARY_DEV_NAME FLASH_DEV_NAME

#if defined(OEMIROT_EXTERNAL_FLASH_ENABLE)
#define FLASH_SECONDARY_DEV_NAME SPI_FLASH_DEV_NAME
#else
#define FLASH_SECONDARY_DEV_NAME FLASH_DEV_NAME
#endif /*  defined(OEMIROT_EXTERNAL_FLASH_ENABLE)  */

#endif /* __APPLI_FLASH_LAYOUT_H__ */
