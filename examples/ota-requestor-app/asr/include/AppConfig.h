/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Lighting Example App Config ----

#define MATTER_DEVICE_NAME "ASR-OTA-REQ"
#define APP_TASK_NAME "APP"
#define APP_TASK_STACK_SIZE (1024 * 4)

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
