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

#include "AppConfig.h"
#include "AppEvent.h"

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
        SILABS_LOG("InitBindingHandler() failed!");
        appError(error);
    }

#if defined(ENABLE_CHIP_SHELL)
    LightSwitchCommands::RegisterSwitchCommands();
#endif // defined(ENABLE_CHIP_SHELL)

    return error;
}

/**
 * @brief Function that triggers a generic switch OnInitialPress event
 */
void LightSwitchMgr::GenericSwitchOnInitialPress()
{
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::InitialPress::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

/**
 * @brief Function that triggers a generic switch OnShortRelease event
 */
void LightSwitchMgr::GenericSwitchOnShortRelease()
{
    GenericSwitchEventData * data = Platform::New<GenericSwitchEventData>();

    data->endpoint = mGenericSwitchEndpoint;
    data->event    = Switch::Events::ShortRelease::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(GenericSwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
}

void LightSwitchMgr::TriggerLightSwitchAction(LightSwitchAction action, bool isGroupCommand)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();

    data->clusterId = chip::app::Clusters::OnOff::Id;
    data->isGroup   = isGroupCommand;

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

void LightSwitchMgr::GenericSwitchWorkerFunction(intptr_t context)
{

    GenericSwitchEventData * data = reinterpret_cast<GenericSwitchEventData *>(context);

    switch (data->event)
    {
    case Switch::Events::InitialPress::Id: {
        uint8_t currentPosition = 1;

        // Set new attribute value
        Clusters::Switch::Attributes::CurrentPosition::Set(data->endpoint, currentPosition);

        // Trigger event
        Clusters::SwitchServer::Instance().OnInitialPress(data->endpoint, currentPosition);
        break;
    }
    case Switch::Events::ShortRelease::Id: {
        uint8_t previousPosition = 1;
        uint8_t currentPosition  = 0;

        // Set new attribute value
        Clusters::Switch::Attributes::CurrentPosition::Set(data->endpoint, currentPosition);

        // Trigger event
        Clusters::SwitchServer::Instance().OnShortRelease(data->endpoint, previousPosition);
        break;
    }
    default:
        break;
    }

    Platform::Delete(data);
}
