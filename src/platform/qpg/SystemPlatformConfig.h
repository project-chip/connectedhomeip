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
 *          Layer on Qorvo QPG platforms.
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
#define CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME 1
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *
#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 8
// ========== Platform-specific Configuration Overrides =========
/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the buffer reserved for storing CRITICAL events and no
 *   other events.  CRITICAL events will never be evicted until this buffer is
 *   full, so its size and the sizes of events determine how many of the last N
 *   CRITICAL events are guaranteed to be available.
 *
 *   Note: this number must be nonzero.
 */
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE (512)

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the buffer reserved for storing events at INFO
 *   priority and higher.  INFO-priority events will not be evicted until this
 *   buffer is full (with INFO and CRITICAL events in it) and the oldest event
 *   in the buffer is an INFO-priority event (which cannot be evicted into the
 *   CRITICAL event buffer).
 *
 *   Note: set to 0 to treat INFO events as effectively equivalent to DEBUG events.
 */
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE (384)

/**
 * @def CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE
 *
 * @brief
 *   A size, in bytes, of the buffer used for storing newly generated events,
 *   and the only buffer in which DEBUG-priority events are allowed.
 *   DEBUG-priority events will start getting evicted when this buffer is full
 *   (with DEBUG, INFO, and CRITICAL events in it) and the oldest event in the
 *   buffer is a DEBUG-priority event, which cannot be evicted into the INFO
 *   event buffer.
 *
 *   Note: set to 0 to disable storing DEBUG events.
 */
#define CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE (256)
