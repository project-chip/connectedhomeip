/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file enumerates and defines the different types of events
 *      generated at the CHIP System Layer.
 */

#pragma once

// Include headers
#include <system/SystemConfig.h>

namespace chip {
namespace DeviceLayer {
struct ChipDeviceEvent;
} // namespace DeviceLayer
} // namespace chip

namespace chip {
namespace System {

#if CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 *  @typedef EventType
 *  The basic type for all InetLayer events.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_SYSTEM_CONFIG_LWIP_EVENT_TYPE EventType;

/**
 *  @typedef Event
 *  The basic object for all InetLayer events.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_SYSTEM_CONFIG_LWIP_EVENT_OBJECT_TYPE Event;

/**
 *   The Inet layer event type definitions.
 *
 */
enum
{
    kEvent_ReleaseObj   = _CHIP_SYSTEM_CONFIG_LWIP_EVENT(0), /**< The event for the drop of a SystemLayer object */
    kEvent_ScheduleWork = _CHIP_SYSTEM_CONFIG_LWIP_EVENT(1), /**< The event for scheduling work on the System Layer's thread. */
};

/**
 *  Check to verify if an EventType is a valid one.
 *
 *  @param[in]  aEventType  CHIP System Layer event type.
 *
 *  @return true if it falls within the enumerated range; otherwise, false.
 */
static inline bool IsEventOfType(EventType aEventType)
{
    return (aEventType >= kEvent_ReleaseObj && aEventType <= kEvent_ScheduleWork);
}

#else

#ifndef CHIP_SYSTEM_CONFIG_EVENT_TYPE
#define CHIP_SYSTEM_CONFIG_EVENT_TYPE int
#endif

#ifndef CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE
#define CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE const struct ::chip::DeviceLayer::ChipDeviceEvent *
#endif

/**
 *  @typedef EventType
 *  The basic type for all InetLayer events.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_SYSTEM_CONFIG_EVENT_TYPE EventType;

/**
 *  @typedef Event
 *  The basic object for all InetLayer events.
 *
 *  This is defined to a platform- or system-specific type.
 *
 */
typedef CHIP_SYSTEM_CONFIG_EVENT_OBJECT_TYPE Event;

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace System
} // namespace chip
