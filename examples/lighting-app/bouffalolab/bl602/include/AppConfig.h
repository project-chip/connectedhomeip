/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
