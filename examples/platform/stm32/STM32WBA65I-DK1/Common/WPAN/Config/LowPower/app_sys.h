/* USER CODE BEGIN Header */
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
/* USER CODE END Header */
#ifndef APP_SYS_H
#define APP_SYS_H

#include <stdint.h>

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Exported constants --------------------------------------------------------*/

/* The RADIO_DEEPSLEEP_WAKEUP_TIME_US macro allows to define when the system
 * needs to wake up before "handle next event".
 * This macro is the sum of the durations of standby exit and Link Layer
 * deep sleep mode exit.
 */
#define RADIO_DEEPSLEEP_WAKEUP_TIME_US (CFG_LPM_STDBY_WAKEUP_TIME)

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported functions prototypes ---------------------------------------------*/

void APP_SYS_BLE_EnterDeepSleep(void);
void APP_SYS_SetWakeupOffset(uint32_t wakeup_offset_us);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* APP_SYS_H */
