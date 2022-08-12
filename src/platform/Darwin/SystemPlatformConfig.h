/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          Layer on Darwin platforms.
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

#if !CHIP_SYSTEM_CONFIG_USE_LIBEV
// FIXME: these should not be hardcoded here, it is set via build config
// Need to exclude these for now in libev case
#define CHIP_SYSTEM_CONFIG_POSIX_LOCKING 0
#define CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#define CHIP_SYSTEM_CONFIG_NO_LOCKING 1
#endif

#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 1
#define CHIP_SYSTEM_CONFIG_POOL_USE_HEAP 1

// ========== Platform-specific Configuration Overrides =========
