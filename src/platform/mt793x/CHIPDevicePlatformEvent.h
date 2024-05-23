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
 *          Device Layer on MT793x platforms using MT793x SDK.
 */

#pragma once

#include <CHIPDevicePlatformConfig.h>
#include <platform/CHIPDeviceEvent.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include "wifi_api_ex.h"
#endif
#include "filogic.h"

#define MTK_EVENT_BUF_LEN 40

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates MT793x platform-specific event types that are visible to the
 * application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates MT793x platform-specific event types that are internal to the
 * Chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kMtkFilogicEvent = kRange_InternalPlatformSpecific,
    kMtkWiFiEvent    = kRange_InternalPlatformSpecific + 1,
    kMtkIPEvent      = kRange_InternalPlatformSpecific + 2,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for MT793x platforms.
 */
struct ChipDevicePlatformEvent final
{
    union
    {
        struct
        {
            filogic_async_event_id_t event;
            uint8_t payload[MTK_EVENT_BUF_LEN];
            uint32_t length;
        } FilogicEvent;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION || CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
        struct
        {
            filogic_async_event_data event_data;
        } MtkWiFiEvent;
#endif
        struct
        {
            wifi_event_t event;
            uint8_t * payload;
            uint32_t length;
        } MtkIPEvent;
    };
};
}; // namespace DeviceLayer
} // namespace chip
