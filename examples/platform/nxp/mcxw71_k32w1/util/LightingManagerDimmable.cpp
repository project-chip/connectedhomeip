/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "LightingManagerDimmable.h"

#include "AppConfig.h"
#include "AppTask.h"

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>

LightingManagerDimmable LightingManagerDimmable::sLightingManager;

#if CONFIG_ENABLE_FEEDBACK
UserInterfaceFeedback & FeedbackMgr()
{
    return LightingMgr();
}
#endif

void LightingManagerDimmable::ApplyDim(uint8_t value)
{
    ChipLogProgress(DeviceLayer, "Dim action has been initiated");
    LightingManagerDimmable::lightLed.Set(LightingManagerDimmable::lightLed.IsTurnedOff() ? 1 : value);
    ChipLogProgress(DeviceLayer, "Move to level %d completed", value);
}

void LightingManagerDimmable::Init()
{
    /* The parameters will not be used by the dimmer init. */
    lightLed.Init(LIGHTING_MANAGER_LIGHT_LED_INDEX, false);

    RestoreState();
}

void LightingManagerDimmable::DisplayInLoop()
{
    lightLed.Animate();
}

void LightingManagerDimmable::DisplayOnAction(Action action)
{
    // Do nothing for now.
}

void LightingManagerDimmable::RestoreState()
{
    /* restore initial state for the LED indicating Lighting state */
    lightLed.Set(false);

    chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t arg) {
        chip::app::DataModel::Nullable<uint8_t> currentLevel;
        chip::app::Clusters::LevelControl::Attributes::CurrentLevel::Get(LIGHTING_MANAGER_APP_DEVICE_TYPE_ENDPOINT, currentLevel);
        LightingMgr().ApplyDim(currentLevel.Value());
    });
}

void LightingManagerDimmable::UpdateState()
{
    // Do nothing for now.
}
