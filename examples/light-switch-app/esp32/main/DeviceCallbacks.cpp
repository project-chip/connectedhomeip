/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/

#include "DeviceCallbacks.h"
#include <lib/support/logging/CHIPLogging.h>

static const char * TAG = "light-switch-app-callbacks";

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;
using namespace chip::app::Clusters;

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t mask, uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);

    switch (clusterId)
    {
    case OnOffSwitchConfiguration::Id:
        OnOffSwitchConfigurationAttributeChangeCallback(endpointId, attributeId, type, value, size);
        break;
    default:
        ESP_LOGI(TAG, "Unhandled cluster ID: %d", clusterId);
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void AppDeviceCallbacks::OnOffSwitchConfigurationAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId,
                                                                         uint8_t type, uint8_t * value, uint16_t size)
{
    ChipLogProgress(Zcl, "OnOff Switch Configuration attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                    ChipLogValueMEI(attributeId), type, *value, size);
}
