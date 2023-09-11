/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Lighting Example App Config ----

#define APP_TASK_NAME "Lit"

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 10

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

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
