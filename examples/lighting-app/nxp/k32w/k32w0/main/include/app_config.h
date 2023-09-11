/*
 *
 * SPDX-FileCopyrightText: 2021 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "LED.h"

#pragma once

// ---- Light Example App Config ----

#define RESET_BUTTON 1
#define LIGHT_BUTTON 2
#define OTA_BUTTON 3
#define BLE_BUTTON 4

#define RESET_BUTTON_PUSH 1
#define LIGHT_BUTTON_PUSH 2
#define OTA_BUTTON_PUSH 3
#define BLE_BUTTON_PUSH 4

#define APP_BUTTON_PUSH 1

#define SYSTEM_STATE_LED LED1
#define LIGHT_STATE_LED LED2

// Time it takes for the light to switch on/off
#define ACTUATOR_MOVEMENT_PERIOS_MS 50

// ---- Light Example SWU Config ----
#define SWU_INTERVAl_WINDOW_MIN_MS (23 * 60 * 60 * 1000) // 23 hours
#define SWU_INTERVAl_WINDOW_MAX_MS (24 * 60 * 60 * 1000) // 24 hours

#if K32W_LOG_ENABLED
#if CHIP_PW_TOKENIZER_LOGGING
#define K32W_LOG(MSG, ...) ChipLogDetail(Echo, MSG, __VA_ARGS__);
#else
#define K32W_LOG(...) otPlatLog(OT_LOG_LEVEL_NONE, OT_LOG_REGION_API, ##__VA_ARGS__);
#endif
#else
#define K32W_LOG(...)
#endif
