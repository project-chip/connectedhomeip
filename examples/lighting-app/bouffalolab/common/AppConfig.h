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

// ---- Lighting Example App Config ----

#define APP_TASK_NAME "APP"

#define APP_BUTTON_PRESS_JITTER 50
#define APP_BUTTON_PRESS_SHORT 1000
#define APP_BUTTON_PRESS_LONG 4000

#define APP_TASK_PRIORITY 15
#define EXT_DISCOVERY_TIMEOUT_SECS 20
#define APP_LIGHT_ENDPOINT_ID 1
#define APP_REBOOT_RESET_COUNT 3
#define APP_REBOOT_RESET_COUNT_KEY "app_reset_cnt"

#ifdef __cplusplus
extern "C" {
#endif

#define BL_LOG(...) printf(__VA_ARGS__);
void appError(int err);

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
