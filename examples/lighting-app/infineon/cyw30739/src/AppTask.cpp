/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppTask.h"
#include <LEDWidget.h>
#include <app/clusters/identify-server/identify-server.h>
#include <platform/CHIPDeviceLayer.h>
#include <wiced_led_manager.h>

using namespace ::chip;
using namespace ::chip::app;

/* config LED 1 */
static wiced_led_config_t chip_lighting_led_config[] = {
    {
        .led    = PLATFORM_LED_1,
        .bright = 50,
    },
};

/**********************************************************
 * Identify Callbacks
 *********************************************************/

void OnIdentifyTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        break;
    }
}

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnIdentifyTriggerEffect,
};

void InitAppTask()
{
    LEDWidget().GetInstance().Init(chip_lighting_led_config, ArraySize(chip_lighting_led_config));
    LightMgr().SetCallbacks(LightManagerCallback, nullptr);
    LightMgr().WriteClusterLevel(254);
}

void LightManagerCallback(LightingManager::Actor_t actor, LightingManager::Action_t action, uint8_t level)
{
    if (action == LightingManager::ON_ACTION)
    {
        printf("Turning light ON\n");
        wiced_led_manager_enable_led(PLATFORM_LED_1);
    }
    else if (action == LightingManager::OFF_ACTION)
    {
        printf("Turning light OFF\n");
        wiced_led_manager_disable_led(PLATFORM_LED_1);
    }
    else if (action == LightingManager::LEVEL_ACTION)
    {
        printf("Set light level = %d\n", level);
        chip_lighting_led_config[0].bright = (uint16_t) level * 100 / 0xfe;
        wiced_led_manager_reconfig_led(&chip_lighting_led_config[0]);
    }
}
