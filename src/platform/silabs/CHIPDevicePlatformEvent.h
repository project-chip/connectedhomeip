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
 *          Device Layer on Silabs platforms using the Silicon Labs SDK.
 */

#pragma once

#include <platform/CHIPDeviceEvent.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#include "wfx_host_events.h"
#endif

namespace chip {
namespace DeviceLayer {

namespace DeviceEventType {

/**
 * Enumerates Silabs platform-specific event types that are visible to the application.
 */
enum PublicPlatformSpecificEventTypes
{
    /* None currently defined */
};

/**
 * Enumerates Silabs platform-specific event types that are internal to the Chip Device Layer.
 */
enum InternalPlatformSpecificEventTypes
{
    kWFXSystemEvent = kRange_InternalPlatformSpecific,
};

} // namespace DeviceEventType

/**
 * Represents platform-specific event information for Silicon Labs platforms.
 */

struct ChipDevicePlatformEvent final
{
    union
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        struct
        {
            wfx_event_base_t eventBase;
            union
            {
                sl_wfx_generic_message_t genericMsgEvent;
                sl_wfx_startup_ind_t startupEvent;
                sl_wfx_connect_ind_t connectEvent;
                sl_wfx_disconnect_ind_t disconnectEvent;

                /*
                 * NOT CURRENTLY USED
                 *Some structs might be bigger in size than the one we use
                 * so we reduce the union size by commenting them out.
                 * Keep for possible future implementation.
                 */

                // sl_wfx_generic_ind_t genericEvent;
                // sl_wfx_exception_ind_t exceptionEvent;
                // sl_wfx_error_ind_t errorEvent;
                // sl_wfx_received_ind_t receivedEvent;
                // sl_wfx_scan_result_ind_t scanResultEvent;
                // sl_wfx_scan_complete_ind_t scanCompleteEvent;
                // sl_wfx_start_ap_ind_t startApEvent;
                // sl_wfx_stop_ap_ind_t stopApEvent;
                // sl_wfx_ap_client_connected_ind_t apClientConnectedEvent;
                // sl_wfx_ap_client_rejected_ind_t apClientRejectedEvent;
                // sl_wfx_ap_client_disconnected_ind_t apClientDisconnectedEvent;
            } data;
        } WFXSystemEvent;
#endif
    };
};
}; // namespace DeviceLayer
} // namespace chip
