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
#include "spidrv.h"

#define SL_BIT_RATE_LCD 1100000
#define SL_BIT_RATE_EXP_HDR 16000000
#define SL_BIT_RATE_SPI_FLASH 16000000
#define SL_BIT_RATE_UART_CONSOLE 16000000

extern SemaphoreHandle_t spi_sem_sync_hdl;

#ifdef RS911X_WIFI
extern SPIDRV_Handle_t sl_spidrv_eusart_exp_handle;
#define SL_SPIDRV_HANDLE sl_spidrv_eusart_exp_handle
#endif

#ifdef WF200_WIFI
extern SPIDRV_Handle_t sl_spidrv_exp_handle;
#define SL_SPIDRV_HANDLE sl_spidrv_exp_handle
#endif

void spi_drv_reinit(uint32_t);

void spiflash_cs_assert(void);
void spiflash_cs_deassert(void);

void pre_bootloader_spi_transfer(void);
void post_bootloader_spi_transfer(void);

void pre_lcd_spi_transfer(void);
void post_lcd_spi_transfer(void);

#if (defined(EFR32MG24) && defined(WF200_WIFI))
void pre_uart_transfer(void);
void post_uart_transfer(void);
#endif /* EFR32MG24 && WF200_WIFI */

#ifdef __cplusplus
}
#endif
