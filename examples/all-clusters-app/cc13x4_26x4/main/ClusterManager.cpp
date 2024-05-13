/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "ClusterManager.h"
#include "Globals.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/basic-types.h>

#ifdef AUTO_PRINT_METRICS
#include <platform/cc13x4_26x4/DiagnosticDataProviderImpl.h>
#endif

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;

constexpr uint32_t kIdentifyTimerDelayMS     = 250;
constexpr uint32_t kIdentifyTimerDelayPerSec = 4;
ClusterManager ClusterManager::sCluster;

#define ENDPOINT_ID_0 (0)
#define ENDPOINT_ID_1 (1)
#define ENDPOINT_ID_2 (2)

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        PLAT_LOG("Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        PLAT_LOG("Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        PLAT_LOG("Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        PLAT_LOG("Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;
    default:
        PLAT_LOG("No identifier effect");
        break;
    }
    return;
}

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { PLAT_LOG("onIdentifyStart"); },
    [](Identify *) { PLAT_LOG("onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { PLAT_LOG("onIdentifyStart"); },
    [](Identify *) { PLAT_LOG("onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};

void ClusterManager::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace app::Clusters::OnOff::Attributes;
    VerifyOrExit(attributeId == OnOff::Id, PLAT_LOG("Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == ENDPOINT_ID_1 || endpointId == ENDPOINT_ID_2,
                 PLAT_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;
    endpointId == ENDPOINT_ID_1 ? LED_write(sAppGreenHandle, *value) : LED_write(sAppRedHandle, *value);

exit:
    return;
}

void ClusterManager::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace app::Clusters::LevelControl::Attributes;
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(brightness > 0, PLAT_LOG("Brightness set to 0, ignoring"));
    VerifyOrExit(attributeId == CurrentLevel::Id, PLAT_LOG("Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == ENDPOINT_ID_1 || endpointId == ENDPOINT_ID_2,
                 PLAT_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    if (brightness > LED_BRIGHTNESS_MAX)
    {
        brightness = LED_BRIGHTNESS_MAX;
    }

    endpointId == ENDPOINT_ID_1 ? LED_setOn(sAppGreenHandle, brightness) : LED_setOn(sAppRedHandle, brightness);

exit:
    return;
}

void ClusterManager::OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    using namespace app::Clusters::ColorControl::Attributes;

    VerifyOrExit(attributeId == CurrentHue::Id || attributeId == CurrentSaturation::Id,
                 PLAT_LOG("Unhandled AttributeId ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == ENDPOINT_ID_1 || endpointId == ENDPOINT_ID_2,
                 PLAT_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    if (endpointId == ENDPOINT_ID_1)
    {
        uint8_t hue, saturation;
        if (attributeId == CurrentHue::Id)
        {
            hue = *value;
            /* Read Current Saturation value when Attribute change callback for HUE Attribute */
            CurrentSaturation::Get(endpointId, &saturation);
        }
        else
        {
            saturation = *value;
            /* Read Current Hue value when Attribute change callback for SATURATION Attribute */
            CurrentHue::Get(endpointId, &hue);
        }
        PLAT_LOG("Color Control triggered: Hue: %d Saturation: %d", hue, saturation);
    }
exit:
    return;
}

void ClusterManager::OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint16_t size,
                                                           uint8_t * value)
{
    if (attributeId == app::Clusters::Identify::Attributes::IdentifyTime::Id && size == 2)
    {
        uint16_t identifyTime;
        memcpy(&identifyTime, value, size);
        if (identifyTime)
        {
            // Currently we have no separate indicator LEDs on each endpoints.
            // We are using LED1 for endpoint 0,1 and LED2 for endpoint 2
            if (endpointId == ENDPOINT_ID_2)
            {
                LED_startBlinking(sAppGreenHandle, kIdentifyTimerDelayMS, identifyTime * kIdentifyTimerDelayPerSec);
            }
            else
            {
                LED_startBlinking(sAppRedHandle, kIdentifyTimerDelayMS, identifyTime * kIdentifyTimerDelayPerSec);
            }
        }
        else
        {
            bool onOffState;
            endpointId == ENDPOINT_ID_0 ? onOffState = mEndpointOnOffState[0] : onOffState = mEndpointOnOffState[endpointId - 1];
            endpointId == ENDPOINT_ID_2 ? LED_write(sAppRedHandle, onOffState) : LED_write(sAppGreenHandle, onOffState);
        }
    }
}
