/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */

#include "appli_flash_layout.h"
#include "low_level_spi_flash.h"

#if defined(OEMIROT_EXTERNAL_FLASH_ENABLE)
static struct spi_flash_range erase_vect[] = {
    { FLASH_AREA_2_OFFSET, FLASH_AREA_2_OFFSET + FLASH_PARTITION_SIZE - 1 },
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
    { FLASH_AREA_6_OFFSET, FLASH_AREA_6_OFFSET + FLASH_S_DATA_PARTITION_SIZE - 1 },
#endif /* (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */
#if (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
    { FLASH_AREA_7_OFFSET, FLASH_AREA_7_OFFSET + FLASH_NS_DATA_PARTITION_SIZE - 1 },
#endif /* (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1) */
};
static struct spi_flash_range write_vect[] = {
    { FLASH_AREA_2_OFFSET, FLASH_AREA_2_OFFSET + FLASH_PARTITION_SIZE - 1 },
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
    { FLASH_AREA_6_OFFSET, FLASH_AREA_6_OFFSET + FLASH_S_DATA_PARTITION_SIZE - 1 },
#endif /* (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */
#if (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
    { FLASH_AREA_7_OFFSET, FLASH_AREA_7_OFFSET + FLASH_NS_DATA_PARTITION_SIZE - 1 },
#endif /* (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1) */
};

struct low_level_spi_device SPI_FLASH0_DEV = {
    .erase      = { .nb = sizeof(erase_vect) / sizeof(struct spi_flash_range), .range = erase_vect },
    .write      = { .nb = sizeof(write_vect) / sizeof(struct spi_flash_range), .range = write_vect },
    .read_error = 1
};
#endif /* OEMIROT_EXTERNAL_FLASH_ENABLE */
