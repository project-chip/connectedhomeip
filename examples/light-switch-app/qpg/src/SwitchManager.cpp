/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "SwitchManager.h"
#include "binding-handler.h"
#include "qvIO.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

SwitchManager SwitchManager::sSwitch;
using namespace ::chip;
using namespace chip::DeviceLayer;

void SwitchManager::Init(void)
{
    // init - TODO
}

void SwitchManager::ToggleHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "CHIP_ERROR_NO_MEMORY"));

    data->localEndpointId = SWITCH_ENDPOINT_ID;
    data->clusterId       = chip::app::Clusters::OnOff::Id;
    data->commandId       = chip::app::Clusters::OnOff::Commands::Toggle::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void SwitchManager::LevelHandler(AppEvent * aEvent)
{
    static uint8_t sLevel;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "CHIP_ERROR_NO_MEMORY"));

    data->localEndpointId = SWITCH_ENDPOINT_ID;
    data->clusterId       = chip::app::Clusters::LevelControl::Id;
    data->commandId       = chip::app::Clusters::LevelControl::Commands::MoveToLevel::Id;
    data->level           = (sLevel == MIN_LEVEL) ? MAX_LEVEL : MIN_LEVEL;
    sLevel                = data->level;

    ChipLogProgress(NotSpecified, "Level - %d", sLevel);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void SwitchManager::ColorHandler(AppEvent * aEvent)
{
    static uint8_t color;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "CHIP_ERROR_NO_MEMORY"));

    data->localEndpointId = SWITCH_ENDPOINT_ID;
    data->clusterId       = chip::app::Clusters::ColorControl::Id;
    data->commandId       = chip::app::Clusters::ColorControl::Commands::MoveToColor::Id;
    if (color == COLOR_AMBER)
    {
        color           = COLOR_GREEN;
        data->colorXY.x = 36044;
        data->colorXY.y = 29490;
        ChipLogProgress(NotSpecified, "Color - AMBER");
    }
    else
    {
        color           = COLOR_AMBER;
        data->colorXY.x = 7536;
        data->colorXY.y = 54131;
        ChipLogProgress(NotSpecified, "Color - GREEN");
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void SwitchManager::GenericSwitchInitialPress(void)
{
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t newPosition = 1;

    SystemLayer().ScheduleLambda(
        [newPosition] { chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(GENERICSWITCH_ENDPOINT_ID, newPosition); });
}

void SwitchManager::GenericSwitchReleasePress(void)
{
    // Release moves Position from 1 (press) to 0
    uint8_t newPosition = 0;

    SystemLayer().ScheduleLambda(
        [newPosition] { chip::app::Clusters::Switch::Attributes::CurrentPosition::Set(GENERICSWITCH_ENDPOINT_ID, newPosition); });
}
