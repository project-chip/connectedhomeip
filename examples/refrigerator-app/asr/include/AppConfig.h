/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- Refrigerator Example App Config ----

#if (CFG_EASY_LOG_ENABLE == 1)
#include "elog.h"
#endif

#define APP_TASK_NAME "APP"

#define APP_TASK_STACK_SIZE (1024 * 4)

#define MATTER_DEVICE_NAME "Refrigerator"

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
