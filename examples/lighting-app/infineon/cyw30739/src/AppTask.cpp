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
#include "ButtonHandler.h"
#include <LEDWidget.h>
#include <app/clusters/identify-server/identify-server.h>
#include <cycfg_pins.h>
#include <matter_config.h>
#include <platform/CHIPDeviceLayer.h>
#include <wiced_led_manager.h>

using namespace chip::app;

/* config LED 1 */
static wiced_led_config_t chip_lighting_led_config[] = {
    {
        .led    = PLATFORM_LED_RED,
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
}

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
    OnIdentifyTriggerEffect,
};

void AppTaskMain(intptr_t args)
{
    wiced_result_t result;
    result = app_button_init();
    if (result != WICED_SUCCESS)
    {
        ChipLogError(Zcl, "ERROR app_button_init %d\n", result);
    }
    LEDWid().Init(chip_lighting_led_config, MATTER_ARRAY_SIZE(chip_lighting_led_config));
    LightMgr().SetCallbacks(LightManagerCallback, nullptr);

    CYW30739MatterConfig::InitApp();
    LightMgr().WriteClusterLevel(127);
}

void LightManagerCallback(LightingManager::Actor_t actor, LightingManager::Action_t action, uint8_t level)
{
    if (action == LightingManager::ON_ACTION)
    {
        printf("Turning light ON\n");
        wiced_led_manager_enable_led(PLATFORM_LED_RED);
    }
    else if (action == LightingManager::OFF_ACTION)
    {
        printf("Turning light OFF\n");
        wiced_led_manager_disable_led(PLATFORM_LED_RED);
    }
    else if (action == LightingManager::LEVEL_ACTION)
    {
        printf("Set light level = %d\n", level);
        chip_lighting_led_config[0].bright = (uint16_t) level * 100 / 0xfe;
        wiced_led_manager_reconfig_led(chip_lighting_led_config[0].led);
    }
}
