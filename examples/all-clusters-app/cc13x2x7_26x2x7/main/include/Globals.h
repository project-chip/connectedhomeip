/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ti/drivers/apps/LED.h>
// Logging
#ifdef __cplusplus
extern "C" {
#endif

int cc13x2_26x2LogInit(void);
void cc13x2_26x2Log(const char * aFormat, ...);
#define PLAT_LOG(...) cc13x2_26x2Log(__VA_ARGS__);

#ifdef __cplusplus
}
#endif
extern LED_Handle sAppRedHandle;
extern LED_Handle sAppGreenHandle;
