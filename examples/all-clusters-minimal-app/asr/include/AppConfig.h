/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Lighting Example App Config ----

#define APP_TASK_NAME "APP"

#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (1024 * 4)
#define APP_WAIT_LOOP 1000

#define MATTER_DEVICE_NAME "ASR-Clusters-m"

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

// ASR Logging
#ifdef __cplusplus
extern "C" {
#endif

void appError(int err);
void ASR_LOG(const char * aFormat, ...);

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
