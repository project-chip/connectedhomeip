/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Door lock Example App Config ----

#define APP_TASK_NAME "Lock"

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS (10)

// Maximum number of users supported by lock
#define CONFIG_LOCK_NUM_USERS (5)

// Maximum number of credentials per user supported by lock
#define CONFIG_LOCK_NUM_CREDENTIALS_PER_USER (2)

// Genio Logging
#ifdef __cplusplus
extern "C" {
#endif

void mt793xLogInit(void);

void mt793xLog(const char * aFormat, ...);
#define MT793X_LOG(aFormat...) mt793xLog(aFormat);
void appError(int err);

#ifdef __cplusplus
}
#endif

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
