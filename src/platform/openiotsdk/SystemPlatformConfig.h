/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Platform-specific configuration overrides for the CHIP System
 *          Layer on Open IoT SDK platform.
 *
 */

#pragma once

#include <stdint.h>

// ==================== Platform Adaptations ====================
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#define CHIP_SYSTEM_CONFIG_MBED_LOCKING 0
#define CHIP_SYSTEM_CONFIG_NO_LOCKING 0
#define CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING 1

#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#ifndef CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
#define CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME 0
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

#define CHIP_SYSTEM_CONFIG_LWIP_SKIP_INIT 1

// ========== Platform-specific Configuration Overrides =========
