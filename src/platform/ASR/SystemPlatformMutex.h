/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lega_rtos_api.h>

// ==================== Platform Mutex Adaptations ====================

#define platform_mutex_t lega_mutex_t
#define platform_init_mutex lega_rtos_init_mutex
#define platform_lock_mutex lega_rtos_lock_mutex
#define platform_unlock_mutex lega_rtos_unlock_mutex
#define platform_delay_milliseconds lega_rtos_delay_milliseconds
#define PLATFORM_WAIT_FOREVER LEGA_WAIT_FOREVER
