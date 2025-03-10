/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#include "LightSwitchMgr.h"
#include "BindingHandler.h"

#if defined(ENABLE_CHIP_SHELL)
#include "ShellCommands.h"
#endif // defined(ENABLE_CHIP_SHELL)

// #include "AppConfig.h"
// #include "AppEvent.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/switch-server/switch-server.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

LightSwitchMgr LightSwitchMgr::sSwitch;

/**
 * @brief Configures LightSwitchMgr
 *        This function needs to be call before using the LightSwitchMgr
 *
 * @param lightSwitchEndpoint endpoint for the light switch device type
 * @param genericSwitchEndpoint endpoint for the generic switch device type
 */
CHIP_ERROR LightSwitchMgr::Init(EndpointId lightSwitchEndpoint, chip::EndpointId genericSwitchEndpoint)
{
    VerifyOrReturnError(lightSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(genericSwitchEndpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    mLightSwitchEndpoint   = lightSwitchEndpoint;
    mGenericSwitchEndpoint = genericSwitchEndpoint;

    // Configure Bindings
    CHIP_ERROR error = InitBindingHandler();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "InitBindingHandler() failed!");
    }

#if defined(ENABLE_CHIP_SHELL)
    LightSwtichCommands::RegisterSwitchCommands();
#endif // defined(ENABLE_CHIP_SHELL)

    return error;
}


void LightSwitchMgr::TriggerLightSwitchAction(LightSwitchAction action, bool isGroupCommand, chip::EndpointId endpoint, ClusterId clusterId)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = clusterId;
    data->isGroup   = isGroupCommand;
    data->localEndpointId = endpoint;

    switch (action)
    {
    case LightSwitchAction::Toggle:
        data->commandId = OnOff::Commands::Toggle::Id;
        break;

    case LightSwitchAction::On:
        data->commandId = OnOff::Commands::On::Id;
        break;

    case LightSwitchAction::Off:
        data->commandId = OnOff::Commands::Off::Id;
        break;
    default:
        Platform::Delete(data);
        return;
        break;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}


void LightSwitchMgr::TriggerLightSwitchLevelControl(LightSwitchLevelControl action, bool isGroupCommand, chip::EndpointId endpoint, ClusterId clusterId, uint8_t value){

    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = clusterId;
    data->isGroup   = isGroupCommand;
    data->localEndpointId = endpoint;
    data->value =  value;

    switch (action)
    {
    case LightSwitchLevelControl::MoveToLevel:
        data->commandId = LevelControl::Commands::MoveToLevel::Id;
        break;
    default:
        Platform::Delete(data);
        return;
        break;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchMgr::TriggerLightSwitchColorControl(LightSwitchColorControl action, bool isGroupCommand, chip::EndpointId endpoint, ClusterId clusterId, uint8_t value){

    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = clusterId;
    data->isGroup   = isGroupCommand;
    data->localEndpointId = endpoint;
    data->value =  value;

    switch (action)
    {
    case LightSwitchColorControl::MoveToColor:
        data->commandId = ColorControl::Commands::MoveToColorTemperature::Id;
        break;
    default:
        Platform::Delete(data);
        return;
        break;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}
