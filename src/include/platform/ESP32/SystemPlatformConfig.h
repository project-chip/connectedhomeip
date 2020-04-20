/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Platform-specific configuration overrides for the OpenWeave System
 *          Layer on the ESP32 platform.
 *
 */

#ifndef SYSTEM_PLATFORM_CONFIG_H
#define SYSTEM_PLATFORM_CONFIG_H

#include <stdint.h>

#include "esp_err.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
struct WeaveDeviceEvent;
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

// ==================== Platform Adaptations ====================

#define WEAVE_SYSTEM_CONFIG_POSIX_LOCKING 0
#define WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING 0
#define WEAVE_SYSTEM_CONFIG_NO_LOCKING 1
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS 1
#define WEAVE_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define WEAVE_SYSTEM_CONFIG_LWIP_EVENT_TYPE int
#define WEAVE_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE const struct ::nl::Weave::DeviceLayer::WeaveDeviceEvent *

#define WEAVE_SYSTEM_CONFIG_ERROR_TYPE esp_err_t
#define WEAVE_SYSTEM_CONFIG_NO_ERROR ESP_OK
#define WEAVE_SYSTEM_CONFIG_ERROR_MIN 7000000
#define WEAVE_SYSTEM_CONFIG_ERROR_MAX 7000999
#define _WEAVE_SYSTEM_CONFIG_ERROR(e) (WEAVE_SYSTEM_CONFIG_ERROR_MIN + (e))
#define WEAVE_SYSTEM_LWIP_ERROR_MIN 3000000
#define WEAVE_SYSTEM_LWIP_ERROR_MAX 3000999

// ==================== General Configuration Overrides ====================

// NOTE: Values that are mapped to CONFIG_ #defines are settable via the ESP-IDF Kconfig mechanism.

#ifndef WEAVE_SYSTEM_CONFIG_NUM_TIMERS
#define WEAVE_SYSTEM_CONFIG_NUM_TIMERS CONFIG_NUM_TIMERS
#endif // WEAVE_SYSTEM_CONFIG_NUM_TIMERS

#endif // SYSTEM_PLATFORM_CONFIG_H
