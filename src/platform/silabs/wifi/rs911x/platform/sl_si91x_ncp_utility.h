/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/*
 * This file contains all the functions specific to the MG24 family for
 * multiplexing the SPI port with WiFi NCP and other WSTK
 * devices such as External Flash and LCD.
 * That can be extended to other families as well.
 */
#ifndef SL_SI91X_NCP_UTILITY_H
#define SL_SI91X_NCP_UTILITY_H

#pragma once
#include "FreeRTOS.h"
#include "semphr.h"
#include "silabs_utils.h"
#include "sl_status.h"

// TODO: This is a WF200 specific include. It is not clear why we need this in the 917 NCP files.
#include <platform/silabs/wifi/wf200/platform/spi_multiplex.h>

#if defined(CHIP_9117)
#include "em_usart.h"
#include "sl_board_configuration_SiWx917.h"
#include "sl_spidrv_exp_config.h"

#endif // CHIP_9117

#define USART_INITSYNC_BAUDRATE 12500000

#if SL_SPICTRL_MUX
sl_status_t spi_board_init(void);
#endif // SL_SPICTRL_MUX

extern uint32_t rx_ldma_channel;
extern uint32_t tx_ldma_channel;
#endif // SL_SI91X_NCP_UTILITY_H
