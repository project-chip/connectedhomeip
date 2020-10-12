/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Defines platform-specific event types and data for the chip
 *          Device Layer on nRF52 platforms using the Nordic nRF5 SDK.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>

#include "ble.h"
#include "nrf_ble_gatt.h"

namespace chip {
namespace System {
class PacketBuffer;
}
} // namespace chip

namespace chip {
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
 * Enumerates nRF52 platform-specific event types that are internal to the chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kSoftDeviceBLEEvent = kRange_InternalPlatformSpecific,
    kCHIPoBLERXCharWriteEvent,
    kCHIPoBLEOutOfBuffersEvent,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Nordic nRF52 platforms.
 */
struct ChipDevicePlatformEvent final
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
            ::chip::System::PacketBuffer * Data;
        } RXCharWriteEvent;
    };
};

} // namespace DeviceLayer
} // namespace chip
