/* USER CODE BEGIN Header */
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef CRC_CTRL_CONF_H
#define CRC_CTRL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Own header files */
#include "crc_ctrl.h"

/* CRC configuration types */
#include "stm32wbaxx_hal_crc.h"

/* Exported defines ----------------------------------------------------------*/
/**
 * @brief Physical address of the CRC to use
 */
#define CRCCTRL_HWADDR CRC

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/**
 * @brief Handle used by the Simple NVM Arbiter to access the CRC functions
 */
extern CRCCTRL_Handle_t SNVMA_Handle;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* CRC_CTRL_CONF_H */
