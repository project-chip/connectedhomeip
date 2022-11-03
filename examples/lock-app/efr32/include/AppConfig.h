/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Door lock Example App Config ----

#define APP_TASK_NAME "Lock"

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 10

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

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
