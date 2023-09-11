/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
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

static const char * TAG = "echo-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG,
             "PostAttributeChangeCallback - Cluster ID: '" ChipLogFormatMEI
             "', EndPoint ID: '0x%x', Attribute ID: '" ChipLogFormatMEI "'",
             ChipLogValueMEI(clusterId), endpointId, ChipLogValueMEI(attributeId));

    // TODO handle this callback in switch statement
    ESP_LOGI(TAG, "Unhandled cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    ESP_LOGI(TAG, "Current free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}
