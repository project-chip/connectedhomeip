/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "FreeRTOS.h"
#include "semphr.h"
#include "sl_mx25_flash_shutdown_usart_config.h"
#include "sl_spidrv_exp_config.h"
#include "sl_wfx_host_api.h"

#define LCD_BIT_RATE 1100000
#define EXP_HDR_BIT_RATE 16000000
#define SPI_FLASH_BIT_RATE 16000000

typedef enum PERIPHERAL_TYPE
{
    EXP_HDR = 0,
    LCD,
    EXT_SPIFLASH,
} peripheraltype_t;

extern SemaphoreHandle_t spi_sem_sync_hdl;
extern peripheraltype_t pr_type;

void spi_drv_reinit(uint32_t);
void set_spi_baudrate(peripheraltype_t);

void spiflash_cs_assert(void);
void spiflash_cs_deassert(void);

void pre_bootloader_spi_transfer(void);
void post_bootloader_spi_transfer(void);

void pre_lcd_spi_transfer(void);
void post_lcd_spi_transfer(void);

#ifdef __cplusplus
}
#endif
