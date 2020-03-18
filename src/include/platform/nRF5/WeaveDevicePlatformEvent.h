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
 *          Defines platform-specific event types and data for the Weave
 *          Device Layer on nRF52 platforms using the Nordic nRF5 SDK.
 */

#ifndef WEAVE_DEVICE_PLATFORM_EVENT_H
#define WEAVE_DEVICE_PLATFORM_EVENT_H

#include <Weave/DeviceLayer/WeaveDeviceEvent.h>

#include "ble.h"
#include "nrf_ble_gatt.h"

namespace nl {
namespace Weave {
namespace System {
class PacketBuffer;
}
}
}

namespace nl {
namespace Weave {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates nRF52 platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates nRF52 platform-specific event types that are internal to the Weave Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kSoftDeviceBLEEvent                     = kRange_InternalPlatformSpecific,
    kWoBLERXCharWriteEvent,
    kWoBLEOutOfBuffersEvent,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Nordic nRF52 platforms.
 */
struct WeaveDevicePlatformEvent final
{
    union
    {
        struct
        {
            ble_evt_t EventData;
            uint8_t PayloadPadding[2];
        } SoftDeviceBLEEvent;
        struct
        {
            uint16_t ConId;
            ble_gatts_evt_write_t WriteArgs;
            ::nl::Weave::System::PacketBuffer * Data;
        } RXCharWriteEvent;
    };
};

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl


#endif // WEAVE_DEVICE_PLATFORM_EVENT_H
