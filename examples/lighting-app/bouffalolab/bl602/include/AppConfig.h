/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// #include "driver/gpio.h"

extern "C" {
#include <blog.h>
}
// ---- Light Example App Config ----

#define APP_TASK_NAME "LIGHT-APP"

// 3R: TODO
//#define SYSTEM_STATE_LED (25)
//#define LOCK_STATE_LED (26)

//#define APP_LIGHT_BUTTON (34)
#define APP_FUNCTION_BUTTON (35)
#define APP_LOCK_BUTTON (8)
#define APP_BUTTON_DEBOUNCE_PERIOD_MS 50

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_LONGPRESSED 1
// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000
