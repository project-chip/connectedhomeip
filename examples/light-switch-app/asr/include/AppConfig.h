/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#ifdef CFG_PLF_RV32
#include "asr_gpio.h"
#include "asr_pinmux.h"
#define duet_gpio_dev_t asr_gpio_dev_t
#else
#include "duet_gpio.h"
#include "duet_pinmux.h"
#endif

#define TASK_NAME "APP"
#define MATTER_DEVICE_NAME "ASR-Switch"
#define APP_TASK_STACK_SIZE (4096)
#define APP_EVENT_QUEUE_SIZE 10

#define GPIO_TASK_NAME "gpio"
#define GPIO_TASK_STACK_SIZE 1024

#define BUTTON_DEBOUNCE_PERIOD_MS 50

#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1

#define SWITCH1_BUTTON GPIO6_INDEX
#define SWITCH2_BUTTON GPIO7_INDEX
#define GENERIC_SWITCH_BUTTON GPIO7_INDEX
#define SYSTEM_STATE_LED GPIO12_INDEX
#define SWITCH_LED GPIO13_INDEX

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

// ---- Lock Example SWU Config ----
#define SWU_INTERVAl_WINDOW_MIN_MS (23 * 60 * 60 * 1000) // 23 hours
#define SWU_INTERVAl_WINDOW_MAX_MS (24 * 60 * 60 * 1000) // 24 hours

// ---- Thread Polling Config ----
#define THREAD_ACTIVE_POLLING_INTERVAL_MS 100
#define THREAD_INACTIVE_POLLING_INTERVAL_MS 1000

// ASR Logging
#ifdef __cplusplus
extern "C" {
#endif

void appError(int err);
void ASR_LOG(const char * aFormat, ...);

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
