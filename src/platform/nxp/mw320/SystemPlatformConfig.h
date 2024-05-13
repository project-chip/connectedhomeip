/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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
 *          Layer on NXP MW320 Platforms.
 *
 */

#pragma once

#include <stdint.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================

#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
#ifndef CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#ifndef CHIP_SYSTEM_CONFIG_NO_LOCKING
#define CHIP_SYSTEM_CONFIG_NO_LOCKING 1
#endif // CHIP_SYSTEM_CONFIG_NO_LOCKING
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS 1
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define CHIP_SYSTEM_CONFIG_LWIP_EVENT_TYPE int
#define CHIP_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *

#define CHIP_SYSTEM_CONFIG_ERROR_TYPE int32_t
#define CHIP_SYSTEM_CONFIG_NO_ERROR 0
#define CHIP_SYSTEM_CONFIG_ERROR_MIN 7000000
#define CHIP_SYSTEM_CONFIG_ERROR_MAX 7000999
#define _CHIP_SYSTEM_CONFIG_ERROR(e) (CHIP_SYSTEM_CONFIG_ERROR_MIN + (e))
#define CHIP_SYSTEM_LWIP_ERROR_MIN 3000000
#define CHIP_SYSTEM_LWIP_ERROR_MAX 3000128

// ========== Platform-specific Configuration Overrides =========
