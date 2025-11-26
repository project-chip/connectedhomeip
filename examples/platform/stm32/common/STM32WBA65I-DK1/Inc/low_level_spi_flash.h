/**
  ******************************************************************************
  * @file    low_level_spi_flash.h
  * @author  MCD Application Team
  * @brief   This file contains device definition for low_level_spi_flash driver
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
#ifndef __LOW_LEVEL_SPI_FLASH_H
#define __LOW_LEVEL_SPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif
//#include "stm32_hal.h"
#include <stdint.h>

#define WIP_Pos     0U
#define WIP_Msk     (1U << WIP_Pos)
#define WEL_Pos     1U
#define WEL_Msk     (1U << WEL_Pos)

typedef enum {READY = 0, NOT_READY = !READY} ReadyStatus;

struct spi_flash_range
{
  uint32_t base;
  uint32_t limit;
};
struct spi_flash_vect
{
  uint32_t nb;
  struct spi_flash_range *range;
};
struct low_level_spi_device
{
  struct spi_flash_vect erase;
  struct spi_flash_vect write;
  uint32_t read_error;
};

extern struct low_level_spi_device SPI_FLASH0_DEV;

/* Additional api, not included in cmsis arm driver flash interface */
int32_t Spi_Flash_Config_Exe(void);

#endif /* __LOW_LEVEL_SPI_FLASH_H */
