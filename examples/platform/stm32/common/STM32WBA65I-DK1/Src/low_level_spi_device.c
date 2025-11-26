/**
  ******************************************************************************
  * @file    low_level_spi_device.c
  * @author  MCD Application Team
  * @brief   This file contains device definition for low_level_spi_device
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "appli_flash_layout.h"
#include "low_level_spi_flash.h"

#if defined (OEMIROT_EXTERNAL_FLASH_ENABLE)
static struct spi_flash_range erase_vect[] =
{
  {FLASH_AREA_2_OFFSET, FLASH_AREA_2_OFFSET + FLASH_PARTITION_SIZE - 1},
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
  { FLASH_AREA_6_OFFSET, FLASH_AREA_6_OFFSET + FLASH_S_DATA_PARTITION_SIZE - 1},
#endif /* (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */
#if (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
  { FLASH_AREA_7_OFFSET, FLASH_AREA_7_OFFSET + FLASH_NS_DATA_PARTITION_SIZE - 1},
#endif /* (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1) */
};
static struct spi_flash_range write_vect[] =
{
  {FLASH_AREA_2_OFFSET, FLASH_AREA_2_OFFSET + FLASH_PARTITION_SIZE - 1},
#if (MCUBOOT_S_DATA_IMAGE_NUMBER == 1)
  { FLASH_AREA_6_OFFSET, FLASH_AREA_6_OFFSET + FLASH_S_DATA_PARTITION_SIZE - 1},
#endif /* (MCUBOOT_S_DATA_IMAGE_NUMBER == 1) */
#if (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1)
  { FLASH_AREA_7_OFFSET, FLASH_AREA_7_OFFSET + FLASH_NS_DATA_PARTITION_SIZE - 1},
#endif /* (MCUBOOT_NS_DATA_IMAGE_NUMBER == 1) */
};

struct low_level_spi_device SPI_FLASH0_DEV =
{
  .erase = { .nb = sizeof(erase_vect) / sizeof(struct spi_flash_range), .range = erase_vect},
  .write = { .nb = sizeof(write_vect) / sizeof(struct spi_flash_range), .range = write_vect},
  .read_error = 1
};
#endif /* OEMIROT_EXTERNAL_FLASH_ENABLE */
