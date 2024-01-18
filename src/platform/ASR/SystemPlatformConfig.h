/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include <stdint.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

// ==================== Platform Adaptations ====================

#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_EVENT_FUNCTIONS 1
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 0
#define CHIP_SYSTEM_CONFIG_LWIP_EVENT_TYPE int
#define CHIP_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *
#define CHIP_SYSTEM_CONFIG_ERROR_TYPE uint32_t
#define CHIP_SYSTEM_CONFIG_NO_ERROR 0
#define CHIP_SYSTEM_CONFIG_ERROR_MIN 7000000
#define CHIP_SYSTEM_CONFIG_ERROR_MAX 7000999
#define _CHIP_SYSTEM_CONFIG_ERROR(e) (CHIP_SYSTEM_CONFIG_ERROR_MIN + (e))
#define CHIP_SYSTEM_LWIP_ERROR_MIN 3000000
#define CHIP_SYSTEM_LWIP_ERROR_MAX 3000128
