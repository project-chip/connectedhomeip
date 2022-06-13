/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include "Globals.h"
#include "LEDWidget.h"
#include "WiFiWidget.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <common/CHIPDeviceManager.h>
#include <lib/dnssd/Advertiser.h>

#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
#include <app-common/zap-generated/cluster-id.h>
#endif

static const char * TAG = "app-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace chip::app;

constexpr uint32_t kIdentifyTimerDelayMS = 250;

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        statusLED1.Blink(kIdentifyTimerDelayMS * 2);
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        break;
    }
    return;
}

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnIdentifyTriggerEffect,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnIdentifyTriggerEffect,
};

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG,
             "PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: "
             "'0x%02x', Attribute ID: '0x%04x'",
             clusterId, endpointId, attributeId);

    switch (clusterId)
    {
    case Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    case Clusters::LevelControl::Id:
        OnLevelControlAttributeChangeCallback(endpointId, attributeId, value);
        break;
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    case Clusters::ColorControl::Id:
        OnColorControlAttributeChangeCallback(endpointId, attributeId, value);
        break;
#endif
    case Clusters::Identify::Id:
        OnIdentifyPostAttributeChangeCallback(endpointId, attributeId, size, value);
        break;
    default:
        ESP_LOGI(TAG, "Unhandled cluster ID: %d", clusterId);
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void AppDeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;
    endpointId == 1 ? statusLED1.Set(*value) : statusLED2.Set(*value);

exit:
    return;
}

void AppDeviceCallbacks::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(attributeId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    endpointId == 1 ? statusLED1.SetBrightness(brightness) : statusLED2.SetBrightness(brightness);

exit:
    return;
}

// Currently we only support ColorControl cluster for ESP32C3_DEVKITM which has
// an on-board RGB-LED
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
void AppDeviceCallbacks::OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID ||
                     attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                 ESP_LOGI(TAG, "Unhandled AttributeId ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));
    if (endpointId == 1)
    {
        uint8_t hue, saturation;
        if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
        {
            hue = *value;
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                                       &saturation, sizeof(uint8_t));
        }
        else
        {
            saturation = *value;
            emberAfReadServerAttribute(endpointId, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue,
                                       sizeof(uint8_t));
        }
        statusLED1.SetColor(hue, saturation);
    }
exit:
    return;
}
#endif

void AppDeviceCallbacks::OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint16_t size,
                                                               uint8_t * value)
{
    if (attributeId == Clusters::Identify::Attributes::IdentifyTime::Id && size == 2)
    {
        uint16_t identifyTime;
        memcpy(&identifyTime, value, size);
        if (identifyTime)
        {
            // Currently we have no separate indicator LEDs on each endpoints.
            // We are using LED1 for endpoint 0,1 and LED2 for endpoint 2
            endpointId == 2 ? statusLED2.Blink(kIdentifyTimerDelayMS * 2) : statusLED1.Blink(kIdentifyTimerDelayMS * 2);
        }
        else
        {
            bool onOffState;
            endpointId == 0 ? onOffState = mEndpointOnOffState[0] : onOffState = mEndpointOnOffState[endpointId - 1];
            endpointId == 2 ? statusLED2.Set(onOffState) : statusLED1.Set(onOffState);
        }
    }
}

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void AppDeviceCallbacksDelegate::OnIPv4ConnectivityEstablished()
{
    wifiLED.Set(true);
}
void AppDeviceCallbacksDelegate::OnIPv4ConnectivityLost()
{
    wifiLED.Set(false);
}
