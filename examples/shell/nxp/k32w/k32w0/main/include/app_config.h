/*
 *
 *    Copyright (c) 2020 Google LLC.
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

#include "LED.h"

#pragma once

// ---- Lock Example App Config ----

#define RESET_BUTTON 1
#define LOCK_BUTTON 2
#define JOIN_BUTTON 3
#define BLE_BUTTON 4

#define RESET_BUTTON_PUSH 1
#define LOCK_BUTTON_PUSH 2
#define JOIN_BUTTON_PUSH 3
#define BLE_BUTTON_PUSH 4

#define APP_BUTTON_PUSH 1

#define SYSTEM_STATE_LED LED1
#define LOCK_STATE_LED LED2

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

// ---- Lock Example SWU Config ----
#define SWU_INTERVAl_WINDOW_MIN_MS (23 * 60 * 60 * 1000) // 23 hours
#define SWU_INTERVAl_WINDOW_MAX_MS (24 * 60 * 60 * 1000) // 24 hours

#if K32W_LOG_ENABLED
#define K32W_LOG(...) otPlatLog(OT_LOG_LEVEL_NONE, OT_LOG_REGION_API, ##__VA_ARGS__);
#else
#define K32W_LOG(...)
#endif
