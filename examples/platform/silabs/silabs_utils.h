/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2022 Silabs
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// EFR Logging
#ifdef __cplusplus
extern "C" {
#endif

void silabsInitLog(void);

void silabsLog(const char * aFormat, ...);
#define SILABS_LOG(...) silabsLog(__VA_ARGS__);
void appError(int err);

#ifdef __cplusplus
}

// Output logs to RTT by defaults
#ifndef SILABS_LOG_OUT_UART
#define SILABS_LOG_OUT_UART 0
#endif

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
