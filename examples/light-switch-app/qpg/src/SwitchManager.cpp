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
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Switch;
using namespace chip::DeviceLayer;

SwitchManager SwitchManager::sSwitch;

static uint8_t multiPressCount = 1;

void SwitchManager::Init(void)
{
    // MultiPressMax is an optional attribute, it has to be enabled in ember with an appropriate default value (minimum value is 2).
    // If this attribute is enabled, the cluster will take the value as a configuration value that can not be changed.
    // Cluster default is used currently (so MultiPressMax will be 2)
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
    data->isGroup         = true;

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
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
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
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

    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void SwitchManager::GenericSwitchInitialPressHandler(AppEvent * aEvent)
{
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t newPosition = 1;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    ChipLogDetail(NotSpecified, "GenericSwitchInitialPress new position %d", newPosition);
    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([newPosition] {
        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(GENERICSWITCH_ENDPOINT_ID);
        VerifyOrReturn(switchCluster != nullptr);

        CHIP_ERROR status = switchCluster->SetCurrentPosition(newPosition);
        VerifyOrReturn(CHIP_NO_ERROR == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));

        RETURN_SAFELY_IGNORED switchCluster->OnInitialPress(newPosition);
    });
}

void SwitchManager::GenericSwitchLongPressHandler(AppEvent * aEvent)
{
    // Press moves Position from 0 (idle) to 1 (press)
    uint8_t newPosition = 1;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    ChipLogDetail(NotSpecified, "GenericSwitchLongPress new position %d", newPosition);
    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([newPosition] {
        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(GENERICSWITCH_ENDPOINT_ID);
        VerifyOrReturn(switchCluster != nullptr);

        RETURN_SAFELY_IGNORED switchCluster->OnLongPress(newPosition);
    });
}

void SwitchManager::GenericSwitchShortReleaseHandler(AppEvent * aEvent)
{
    // Release moves Position from 1 (press) to 0
    uint8_t newPosition      = 0;
    uint8_t previousPosition = 1;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    ChipLogDetail(NotSpecified, "GenericSwitchShortRelease new position %d", newPosition);
    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([newPosition, previousPosition] {
        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(GENERICSWITCH_ENDPOINT_ID);
        VerifyOrReturn(switchCluster != nullptr);

        CHIP_ERROR status = switchCluster->SetCurrentPosition(newPosition);
        VerifyOrReturn(CHIP_NO_ERROR == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));

        RETURN_SAFELY_IGNORED switchCluster->OnShortRelease(previousPosition);
    });
}

void SwitchManager::GenericSwitchLongReleaseHandler(AppEvent * aEvent)
{
    // Release moves Position from 1 (press) to 0
    uint8_t newPosition      = 0;
    uint8_t previousPosition = 1;

    if (aEvent->Type != AppEvent::kEventType_Button)
    {
        ChipLogError(NotSpecified, "Event type not supported!");
        return;
    }

    ChipLogDetail(NotSpecified, "GenericSwitchLongRelease new position %d", newPosition);
    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([newPosition, previousPosition] {
        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(GENERICSWITCH_ENDPOINT_ID);
        VerifyOrReturn(switchCluster != nullptr);

        CHIP_ERROR status = switchCluster->SetCurrentPosition(newPosition);
        VerifyOrReturn(CHIP_NO_ERROR == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));

        RETURN_SAFELY_IGNORED switchCluster->OnLongRelease(previousPosition);
    });
}

void SwitchManager::GenericSwitchMultipressOngoingHandler(AppEvent * aEvent)
{
    uint8_t newPosition = 1;

    multiPressCount++;

    ChipLogDetail(NotSpecified, "GenericSwitchMultiPressOngoing (%d)", multiPressCount);

    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([newPosition] {
        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(GENERICSWITCH_ENDPOINT_ID);
        VerifyOrReturn(switchCluster != nullptr);

        RETURN_SAFELY_IGNORED switchCluster->OnMultiPressOngoing(newPosition, multiPressCount);
    });
}

void SwitchManager::GenericSwitchMultipressCompleteHandler(AppEvent * aEvent)
{
    uint8_t previousPosition = 0;

    ChipLogProgress(NotSpecified, "GenericSwitchMultiPressComplete (%d)", multiPressCount);

    TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([previousPosition] {
        auto switchCluster = Clusters::Switch::FindClusterOnEndpoint(GENERICSWITCH_ENDPOINT_ID);
        VerifyOrReturn(switchCluster != nullptr);

        RETURN_SAFELY_IGNORED switchCluster->OnMultiPressComplete(previousPosition, multiPressCount);
    });

    multiPressCount = 1;
}
