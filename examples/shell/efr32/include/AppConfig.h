/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Lighting Example App Config ----

#define APP_TASK_NAME "Shell_Task"

// EFR32 WSTK Buttons
#define PB0 0
#define PB1 1

// EFR32 WSTK LEDs
#define BSP_LED_0 0
#define BSP_LED_1 1

#define APP_LIGHT_SWITCH PB1
#define APP_FUNCTION_BUTTON PB0
#define APP_BUTTON_DEBOUNCE_PERIOD_MS 50

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1

#define SYSTEM_STATE_LED BSP_LED_0
#define LIGHT_LED BSP_LED_1

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 10

// ---- Light Example SWU Config ----
#define SWU_INTERVAl_WINDOW_MIN_MS (23 * 60 * 60 * 1000) // 23 hours
#define SWU_INTERVAl_WINDOW_MAX_MS (24 * 60 * 60 * 1000) // 24 hours

// EFR Logging
#ifdef __cplusplus
extern "C" {
#endif

void efr32InitLog(void);

void efr32Log(const char * aFormat, ...);
#define EFR32_LOG(...) efr32Log(__VA_ARGS__);
void appError(int err);

#ifdef __cplusplus
}
#endif
