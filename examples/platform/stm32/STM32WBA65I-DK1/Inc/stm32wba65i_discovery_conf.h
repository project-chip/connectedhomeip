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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32WBA65I_DK1_CONF_H
#define STM32WBA65I_DK1_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx_hal.h"

/* Usage of STM32WBA65I_DK1 board */
#define USE_STM32WBA65I_DK1 1U

/* COM define */
#define USE_BSP_COM_FEATURE 1U
#define USE_COM_LOG 0U

/* Joystick Debounce Delay in ms */
#define BSP_JOY_DEBOUNCE_DELAY 200

#ifdef __cplusplus
}
#endif

#endif /* STM32WBA65I_DK1_CONF_H */
