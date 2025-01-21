/*
 *
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

#pragma once

#define APP_TASK_NAME "APP"
#define APP_TASK_PRIORITY 15

#define EXT_DISCOVERY_TIMEOUT_SECS 20

typedef void (*app_pds_gpio_irq_handler_t)(void * arg);

#ifdef __cplusplus

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);

extern "C" void platform_port_init(void);
#else

void appError(int err);
void platform_port_init(void);
#endif

void app_pds_init(app_pds_gpio_irq_handler_t pinHandler);
