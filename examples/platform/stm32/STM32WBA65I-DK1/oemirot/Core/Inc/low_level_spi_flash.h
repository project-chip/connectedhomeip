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
  * @file    low_level_spi_flash.h
  * @author  MCD Application Team
  * @brief   This file contains device definition for low_level_spi_flash driver
  ******************************************************************************
  */
#ifndef __LOW_LEVEL_SPI_FLASH_H
#define __LOW_LEVEL_SPI_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32_hal.h"

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
