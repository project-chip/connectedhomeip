/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#define APP_TASK_NAME "APP"
#define APP_TASK_PRIORITY 15

#define EXT_DISCOVERY_TIMEOUT_SECS 20

#ifdef __cplusplus
#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#else
void appError(int err);
#endif
