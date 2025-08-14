/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Defines platform-specific event types and data for the Chip
 *          Device Layer on scm1612s platform using wise-sdk.
 */

#pragma once

#include <CHIPDevicePlatformConfig.h>
#include <platform/CHIPDeviceEvent.h>

#include "wise_event_loop.h"

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates scm1612s platform-specific event types that are visible to the
 * application.
 */
enum PublicPlatformSpecificEventTypes
{
    kSCMSystemEvent = kRange_PublicPlatformSpecific,
};

#if CONFIG_ENABLE_SCM1612S_BLE_CONTROLLER
/**
 * Enumerates scm1612s platform-specific event types that are internal to the
 * Chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kPlatformSCMEvent = kRange_InternalPlatformSpecific,
    kPlatformSCMBLECentralConnected,
    kPlatformSCMBLECentralConnectFailed,
    kPlatformSCMBLEWriteComplete,
    kPlatformSCMBLESubscribeOpComplete,
    kPlatformSCMBLEIndicationReceived,
};

#endif
} // namespace DeviceEventType

/**
 * Represents platform-specific event information for scm1612s platforms.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        struct
        {
            system_event_t event;
        } SCMSystemEvent;
#if CONFIG_ENABLE_SCM1612S_BLE_CONTROLLER
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
        } BLECentralConnected;
        struct
        {
            CHIP_ERROR mError;
        } BLECentralConnectFailed;
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
        } BLEWriteComplete;
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
            bool mIsSubscribed;
        } BLESubscribeOpComplete;
        struct
        {
            BLE_CONNECTION_OBJECT mConnection;
            chip::System::PacketBuffer * mData;
        } BLEIndicationReceived;
#endif
    };
};
}; // namespace DeviceLayer
} // namespace chip
