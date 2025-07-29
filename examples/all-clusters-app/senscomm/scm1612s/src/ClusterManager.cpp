/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
 * @file
 *   This file implements the handler for data model messages.
 */

#include "ClusterManager.h"
#include "AppConfig.h"
#include "LEDWidget.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/server/Dnssd.h>
#include <app/util/basic-types.h>
#include <app/util/util.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace ::chip::app::Clusters;

#define ENDPOINT_FIRST_IDX 1
#define ENDPOINT_SECOND_IDX 2

uint32_t identifyTimerCount;
constexpr uint32_t kIdentifyTimerDelayMS = 250;
ClusterManager ClusterManager::sCluster;

void ClusterManager::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == OnOff::Attributes::OnOff::Id,
                 P6_LOG("Unhandled Attribute ID: '" ChipLogFormatMEI "'", ChipLogValueMEI(attributeId)));
    VerifyOrExit(endpointId == ENDPOINT_FIRST_IDX || endpointId == ENDPOINT_SECOND_IDX,
                 P6_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    // At this point we can assume that value points to a bool value.
    mEndpointOnOffState[endpointId - 1] = *value;

    // On/Off Cluster LED for endpoint 1 and Light Led for endpoint 2
    endpointId == ENDPOINT_FIRST_IDX ? sClusterLED.RGB_set(*value) : sLightLED.Set(*value);

exit:
    return;
}

void ClusterManager::OnLevelControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    bool onOffState    = mEndpointOnOffState[endpointId - 1];
    uint8_t brightness = onOffState ? *value : 0;

    VerifyOrExit(attributeId == LevelControl::Attributes::CurrentLevel::Id,
                 P6_LOG("Unhandled Attribute ID: '" ChipLogFormatMEI "'", ChipLogValueMEI(attributeId)));
    VerifyOrExit(endpointId == ENDPOINT_FIRST_IDX || endpointId == ENDPOINT_SECOND_IDX,
                 P6_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    endpointId == ENDPOINT_FIRST_IDX ? sClusterLED.SetBrightness(brightness) : sLightLED.SetBrightness(brightness);

exit:
    return;
}

void ClusterManager::OnColorControlAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ColorControl::Attributes::CurrentHue::Id ||
                     attributeId == ColorControl::Attributes::CurrentSaturation::Id,
                 P6_LOG("Unhandled Attribute ID: '" ChipLogFormatMEI "'", ChipLogValueMEI(attributeId)));
    VerifyOrExit(endpointId == ENDPOINT_FIRST_IDX || endpointId == ENDPOINT_SECOND_IDX,
                 P6_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));
    if (endpointId == 1)
    {
        uint8_t hue, saturation;
        /* If the Current Attribute is CurrentHue, read the saturation value and
         * set the color on Cluster LED using both Saturation and Hue.
         */
        if (attributeId == ColorControl::Attributes::CurrentHue::Id)
        {
            hue = *value;
            /* Read Current Saturation value when Attribute change callback for HUE Attribute */
            ColorControl::Attributes::CurrentSaturation::Get(endpointId, &saturation);
        }
        else
        {
            /* If the Current Attribute is CurrentSaturation, read the Hue value and
             * set the color on Cluster LED using both Saturation and Hue.
             */
            saturation = *value;
            /* Read Current Hue value when Attribute change callback for SATURATION Attribute */
            ColorControl::Attributes::CurrentHue::Get(endpointId, &hue);
        }
        /* Set RGB Color on Cluster Indication LED */
        sClusterLED.SetColor(hue, saturation);
    }
exit:
    return;
}

/* Identify Cluster puts device into identification mode like flashing a device
 * This function blinks LED by running timer every 500ms as expected by
 * IdentifyTimer Attribute
 */

void IdentifyTimerHandler(Layer * systemLayer, void * appState)
{
    /* Status LED On/Off based on Blink Configuration */
    sStatusLED.Animate();

    if (identifyTimerCount)
    {
        systemLayer->StartTimer(Clock::Milliseconds32(kIdentifyTimerDelayMS), IdentifyTimerHandler, appState);
        // Decrement the timer count.
        identifyTimerCount--;
    }
}

void ClusterManager::OnIdentifyPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == Identify::Attributes::IdentifyTime::Id,
                 P6_LOG("Unhandled Attribute ID: '" ChipLogFormatMEI "'", ChipLogValueMEI(attributeId)));
    VerifyOrExit(endpointId == ENDPOINT_FIRST_IDX, P6_LOG("Unexpected EndPoint ID: `0x%02x'", endpointId));

    /* IdentifyTime Attribute Spec mentions "flashing a light with a period of 0.5 seconds" */
    sStatusLED.Blink(kIdentifyTimerDelayMS * 2);

    // timerCount represents the number of callback executions before we stop the timer.
    // value is expressed in seconds and the timer is fired every 250ms, so just multiply value by 4.
    // Also, we want timerCount to be odd number, so the light state ends in the same state it starts.
    identifyTimerCount = (*value) * 4;

    DeviceLayer::SystemLayer().CancelTimer(IdentifyTimerHandler, this);
    DeviceLayer::SystemLayer().StartTimer(Clock::Milliseconds32(kIdentifyTimerDelayMS), IdentifyTimerHandler, this);

exit:
    return;
}
