/*
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Logging
#ifdef __cplusplus
extern "C" {
#endif

#ifdef ti_log_Log_ENABLE
#include "ti_drivers_config.h"
#include "ti_log_config.h"
#endif

int cc13xx_26xxLogInit(void);
void cc13xx_26xxLog(const char * aFormat, ...);
#ifndef ti_log_Log_ENABLE
#define PLAT_LOG(...) cc13xx_26xxLog(__VA_ARGS__);
#else // SILK Logging
#define PLAT_LOG(...) Log_printf(LogModule_App1, Log_DEBUG, __VA_ARGS__);
#endif

#define ACTUATOR_MOVEMENT_PERIOD_MS 1000

#ifdef __cplusplus
}
#endif
#endif // APP_CONFIG_H
