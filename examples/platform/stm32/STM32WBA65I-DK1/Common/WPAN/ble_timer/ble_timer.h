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

#ifndef BLE_TIMER_H__
#define BLE_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_common.h"

void BLE_TIMER_Init(void);
void BLE_TIMER_Deinit(void);

uint8_t BLE_TIMER_Start(uint16_t id, uint32_t ms_timeout);

void BLE_TIMER_Stop(uint16_t id);

void BLE_TIMER_Background(void);

/* Callback
 */
void BLE_TIMERCB_Expiry(uint16_t id);

#ifdef __cplusplus
}
#endif

#endif /* BLE_TIMER_H__ */
