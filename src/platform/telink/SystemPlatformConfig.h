/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Layer on Telink platform.
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

#ifndef CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS 0
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 0
#else // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#define CHIP_SYSTEM_CONFIG_USE_LWIP 0
#define CHIP_SYSTEM_CONFIG_USE_SOCKETS 1

// ========== Platform-specific Configuration Overrides =========
