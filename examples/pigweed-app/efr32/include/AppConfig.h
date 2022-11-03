/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// ---- PW Example App Config ----

// EFR32 WSTK Buttons
#define PB0 0
#define PB1 1

// EFR32 WSTK LEDs
#define BSP_LED_0 0
#define BSP_LED_1 1

#define APP_BUTTON_DEBOUNCE_PERIOD_MS 50
#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1

#define SYSTEM_STATE_LED BSP_LED_0

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
#endif
