/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// ---- Lock Example App Config ----
#define APP_TASK_NAME "APP"
#define APP_LOCK_BUTTON BTN_SW4
#define APP_FUNCTION_BUTTON BTN_SW5

#define SYSTEM_STATE_LED LED_GREEN
#define LOCK_STATE_LED LED_WHITE

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

// ---- Lock Example SWU Config ----
#define SWU_INTERVAl_WINDOW_MIN_MS (23 * 60 * 60 * 1000) // 23 hours
#define SWU_INTERVAl_WINDOW_MAX_MS (24 * 60 * 60 * 1000) // 24 hours

// Maximum number of users supported by lock
#define CONFIG_LOCK_NUM_USERS (5)
// Maximum number of credentials supported by lock
#define CONFIG_LOCK_NUM_CREDENTIALS (10)
// Maximum number of credentials per user supported by lock
#define CONFIG_LOCK_NUM_CREDENTIALS_PER_USER (2)

#endif // APP_CONFIG_H
