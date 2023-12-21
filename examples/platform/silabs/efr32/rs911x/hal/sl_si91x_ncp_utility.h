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


#include "spi_multiplex.h"
#include "silabs_utils.h"
#include "sl_status.h"

#if defined(CHIP_9117)
#include "sl_board_configuration_SiWx917.h"
#else
#include "sl_board_configuration.h"
#if defined(EFR32MG12)
#include "em_usart.h"
#include "sl_spidrv_exp_config.h"
extern SPIDRV_Handle_t sl_spidrv_exp_handle;
#define SL_SPIDRV_HANDLE sl_spidrv_exp_handle
#elif defined(EFR32MG24)
#include "em_eusart.h"
#include "sl_spidrv_instances.h"
#include "sl_spidrv_eusart_exp_config.h"
#define SL_SPIDRV_HANDLE sl_spidrv_eusart_exp_handle
#else // EFR32MG12 || EFR32MG24
#error "Unknown platform"
#endif
#endif // CHIP_9117

// variable to identify spi configured for expansion header
// EUSART configuration available on the SPIDRV

#if SL_SPICTRL_MUX
sl_status_t spi_board_init(void);
#endif // SL_SPICTRL_MUX

