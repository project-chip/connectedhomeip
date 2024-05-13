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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <common/CHIPDeviceManager.h>
#include <esp_log.h>
#include <lib/dnssd/Advertiser.h>

#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
#include <app-common/zap-generated/ids/Clusters.h>
#endif

static const char TAG[] = "app-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace chip::app;

constexpr uint32_t kIdentifyTimerDelayMS = 250;

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        statusLED1.Blink(kIdentifyTimerDelayMS * 2);
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
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
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%" PRIx32 "', EndPoint ID: '0x%x' , Attribute ID: '0x%" PRIx32 "'",
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
        ESP_LOGI(TAG, "Unhandled cluster ID: %" PRIu32, clusterId);
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

void AppDeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace app::Clusters::OnOff::Attributes;

    VerifyOrExit(attributeId == OnOff::Id, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%" PRIx32 "'", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;
    endpointId == 1 ? statusLED1.Set(*value) : statusLED2.Set(*value);

exit:
    return;
}

void AppDeviceCallbacks::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace app::Clusters::LevelControl::Attributes;

    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(attributeId == CurrentLevel::Id, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%" PRIx32 "'", attributeId));
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
    using namespace app::Clusters::ColorControl::Attributes;

    VerifyOrExit(attributeId == CurrentHue::Id || attributeId == CurrentSaturation::Id,
                 ESP_LOGI(TAG, "Unhandled AttributeId ID: '0x%" PRIx32 "'", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));
    if (endpointId == 1)
    {
        uint8_t hue, saturation;
        if (attributeId == CurrentHue::Id)
        {
            hue = *value;
            CurrentSaturation::Get(endpointId, &saturation);
        }
        else
        {
            saturation = *value;
            CurrentHue::Get(endpointId, &hue);
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

void AppDeviceCallbacksDelegate::OnIPv4ConnectivityEstablished()
{
    wifiLED.Set(true);
}
void AppDeviceCallbacksDelegate::OnIPv4ConnectivityLost()
{
    wifiLED.Set(false);
}
