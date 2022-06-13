
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
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/basic-types.h>

#ifdef AUTO_PRINT_METRICS
#include <platform/cc13x2_26x2/DiagnosticDataProviderImpl.h>
#endif

using namespace ::chip;
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
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        PLAT_LOG("EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        PLAT_LOG("EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        PLAT_LOG("EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        PLAT_LOG("EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
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
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnIdentifyTriggerEffect,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { PLAT_LOG("onIdentifyStart"); },
    [](Identify *) { PLAT_LOG("onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
    OnIdentifyTriggerEffect,
};

void ClusterManager::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID, PLAT_LOG("Unhandled Attribute ID: '0x%04x", attributeId));
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
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(brightness > 0, PLAT_LOG("Brightness set to 0, ignoring"));
    VerifyOrExit(attributeId == ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, PLAT_LOG("Unhandled Attribute ID: '0x%04x", attributeId));
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
    VerifyOrExit(attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID ||
                     attributeId == ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                 PLAT_LOG("Unhandled AttributeId ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == ENDPOINT_ID_1 || endpointId == ENDPOINT_ID_2,
                 PLAT_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    if (endpointId == ENDPOINT_ID_1)
    {
        uint8_t hue, saturation;
        if (attributeId == ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID)
        {
            hue = *value;
            /* Read Current Saturation value when Attribute change callback for HUE Attribute */
            app::Clusters::ColorControl::Attributes::CurrentSaturation::Get(endpointId, &saturation);
        }
        else
        {
            saturation = *value;
            /* Read Current Hue value when Attribute change callback for SATURATION Attribute */
            app::Clusters::ColorControl::Attributes::CurrentHue::Get(endpointId, &hue);
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
