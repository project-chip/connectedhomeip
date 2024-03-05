/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "binding-handler.h"
#include "app/ConcreteCommandPath.h"
#include "app/clusters/bindings/BindingManager.h"
#include "controller/InvokeInteraction.h"
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;

static bool sEnabled = false;

static void ProcessSwitchUnicastBindingCommand(CommandId commandId, const EmberBindingTableEntry & binding,
                                               Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle,
                                               BindingCommandData * data)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Switch command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Switch command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, toggleCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::MoveToLevel::Id: {
        Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
        moveToLevelCommand.level = data->level;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, moveToLevelCommand, onSuccess, onFailure);
    }
    break;
    case Clusters::ColorControl::Commands::MoveToColor::Id: {

        Clusters::ColorControl::Commands::MoveToColor::Type moveToColorCommand;

        moveToColorCommand.colorX = data->colorXY.x;
        moveToColorCommand.colorY = data->colorXY.y;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, moveToColorCommand, onSuccess, onFailure);
    }
    break;

    default:
        ChipLogError(NotSpecified, "Unsupported Command Id");
        break;
    }
}

static void LightSwitchChangedHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "nullptr pointer passed"));

    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    if (binding.type == MATTER_UNICAST_BINDING)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
        case Clusters::LevelControl::Id:
        case Clusters::ColorControl::Id:
            // TODO should not happen?
            VerifyOrReturn(peer_device != nullptr, ChipLogError(NotSpecified, "Peer is nullptr"));

            ProcessSwitchUnicastBindingCommand(data->commandId, binding, peer_device->GetExchangeManager(),
                                               peer_device->GetSecureSession().Value(), data);
            break;

        default:
            ChipLogError(NotSpecified, "Unsupported Cluster Id");
            break;
        }
    }
}

static void LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "nullptr pointer passed"));

    Platform::Delete(static_cast<BindingCommandData *>(context));
}

void InitBindingManager(intptr_t context)
{
    auto & server                          = chip::Server::GetInstance();
    BindingManagerInitParams bindingParams = { &server.GetFabricTable(), server.GetCASESessionManager(),
                                               &server.GetPersistentStorage() };
    CHIP_ERROR error                       = BindingManager::GetInstance().Init(bindingParams);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "BindingManager initialization failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }
    BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
    sEnabled = true;
}

void SwitchWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "nullptr pointer passed"));

    BindingCommandData * data = reinterpret_cast<BindingCommandData *>(context);

    if (!sEnabled)
    {
        ChipLogProgress(NotSpecified, "BindingManager not initialized");
        Platform::Delete(data);
    }

    BindingManager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId, data->clusterId, static_cast<void *>(data));
}
