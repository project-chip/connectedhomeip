/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// ---- Lock Example App Config ----

#define APP_TASK_NAME "LOCK-APP"
#if CONFIG_IDF_TARGET_ESP32
#define SYSTEM_STATE_LED GPIO_NUM_25
#define LOCK_STATE_LED GPIO_NUM_26

#define APP_LOCK_BUTTON GPIO_NUM_34
#define APP_FUNCTION_BUTTON GPIO_NUM_35
#elif CONFIG_IDF_TARGET_ESP32C3
#define SYSTEM_STATE_LED GPIO_NUM_8
#define LOCK_STATE_LED GPIO_NUM_9

#define APP_LOCK_BUTTON GPIO_NUM_18
#define APP_FUNCTION_BUTTON GPIO_NUM_19
#endif
#define APP_BUTTON_DEBOUNCE_PERIOD_MS 50

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000
