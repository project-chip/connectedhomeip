/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
