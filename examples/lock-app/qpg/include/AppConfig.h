/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
