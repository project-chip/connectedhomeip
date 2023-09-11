/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
