/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

// ---- Window Example App Config ----
#define APP_TASK_NAME "APP"

// EFR Logging
#ifdef __cplusplus
extern "C" {
#endif

#define LCD_SIZE 128
#define LCD_MARGIN_SIZE 1
#define LCD_BORDER_SIZE 2
#define LCD_FRAME_SIZE (2 * LCD_MARGIN_SIZE + LCD_BORDER_SIZE)
#define LCD_COVER_SIZE (LCD_SIZE - 2 * LCD_FRAME_SIZE)
#define LIFT_OPEN_LIMIT 0
#define LIFT_CLOSED_LIMIT (LCD_COVER_SIZE - 1)
#define LIFT_DELTA 1000 // 10%
#define TILT_OPEN_LIMIT 1
#define TILT_CLOSED_LIMIT (LCD_COVER_SIZE / 10 - 1)
#define TILT_DELTA 1000 // 10%

#define WINDOW_COVER_COUNT 2

#ifndef WINDOW_COVER_ENDPOINT1
#define WINDOW_COVER_ENDPOINT1 1
#endif

#ifndef WINDOW_COVER_ENDPOINT2
#define WINDOW_COVER_ENDPOINT2 2
#endif

#ifndef LONG_PRESS_TIMEOUT
#define LONG_PRESS_TIMEOUT 3000
#endif

#ifndef COVER_LIFT_TILT_TIMEOUT
#define COVER_LIFT_TILT_TIMEOUT 500
#endif

void efr32InitLog(void);

void efr32Log(const char * aFormat, ...);
#define EFR32_LOG(...) efr32Log(__VA_ARGS__);

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
