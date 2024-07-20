/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#pragma once

#include "cybsp.h"
#include "cyhal.h"

// ---- Lock Example App Config ----

#define APP_TASK_NAME "APP"

#define APP_LOCK_BUTTON_IDX 0
#define APP_FUNCTION_BUTTON_IDX 1

#define APP_LOCK_BUTTON CYBSP_USER_BTN1
#define APP_FUNCTION_BUTTON CYBSP_USER_BTN2
#define APP_BUTTON_MIN_ASSERT_TIME_MS 500

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1
#define APP_BUTTON_LONG_PRESS 2

#define SYSTEM_STATE_LED CYBSP_USER_LED1
#define LOCK_STATE_LED CYBSP_USER_LED2

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

// ---- Lock Example SWU Config ----
#define SWU_INTERVAl_WINDOW_MIN_MS (23 * 60 * 60 * 1000) // 23 hours
#define SWU_INTERVAl_WINDOW_MAX_MS (24 * 60 * 60 * 1000) // 24 hours

// ---- Thread Polling Config ----
#define THREAD_ACTIVE_POLLING_INTERVAL_MS 100
#define THREAD_INACTIVE_POLLING_INTERVAL_MS 1000

// PSOC6 Logging
#ifdef __cplusplus
extern "C" {
#endif

void appError(int err);
void PSOC6Log(const char * aFormat, ...);
#define PSOC6_LOG(...) PSOC6Log(__VA_ARGS__)

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
