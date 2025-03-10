/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "BindingHandler.h"

// #include "AppConfig.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "platform/CHIPDeviceLayer.h"
#include <app/clusters/bindings/bindings.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;

namespace {

void ProcessOnOffUnicastBindingCommand(CommandId commandId, const EmberBindingTableEntry & binding,
                                       Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle)
{

    // ChipLogError(NotSpecified, "-------------------------------");
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, toggleCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, onCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, offCommand, onSuccess, onFailure);
        break;
    }
}

/*----------------------------------------------------------------
-----------------------------PTIT Dev-----------------------------
----------------------------------------------------------------*/

void ProcessLevelControlBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                    Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle){

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Level Control command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Level Control command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };
    switch (data->commandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id:
        Clusters::LevelControl::Commands::MoveToLevel::Type MoveToLevel;
        MoveToLevel.level = data->value;
        MoveToLevel.transitionTime =  (DataModel::Nullable<uint16_t>) data->transitionTime;
        MoveToLevel.optionsMask = (Clusters::LevelControl::LevelControlOptions) data->optionsMask;
        MoveToLevel.optionsOverride = (Clusters::LevelControl::LevelControlOptions) data->optionsOverride;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, MoveToLevel, onSuccess, onFailure);
        break;
    }
}


void ProcessColorControlBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                    Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle){

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Color Temperature Control command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Color Temperature Control command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };
    switch (data->commandId)
    {
    case Clusters::ColorControl::Commands::MoveToColorTemperature::Id:
        Clusters::ColorControl::Commands::MoveToColorTemperature::Type MoveToColorTemp;
        MoveToColorTemp.colorTemperatureMireds = data->value;
        MoveToColorTemp.transitionTime =   data->transitionTime;
        MoveToColorTemp.optionsMask =  (chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap>)data->optionsMask;
        MoveToColorTemp.optionsOverride =  (chip::BitMask<chip::app::Clusters::ColorControl::OptionsBitmap>)data->optionsOverride;
        Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, MoveToColorTemp, onSuccess, onFailure);
        break;
    }
}

/*----------------------------------------------------------------
-------------------------------End--------------------------------
----------------------------------------------------------------*/

void ProcessOnOffGroupBindingCommand(CommandId commandId, const EmberBindingTableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, toggleCommand);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, onCommand);

        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, offCommand);
        break;
    }
}

void LightSwitchChangedHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{

    // ChipLogError(NotSpecified, "SwitchWorkerFunction - ----------------");
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    if (binding.type == MATTER_MULTICAST_BINDING && data->isGroup)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            ProcessOnOffGroupBindingCommand(data->commandId, binding);
            break;
        }
    }
    else if (binding.type == MATTER_UNICAST_BINDING && !data->isGroup)
    {
        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
            ProcessOnOffUnicastBindingCommand(data->commandId, binding, peer_device->GetExchangeManager(),
                                              peer_device->GetSecureSession().Value());
            break;
        
        case Clusters::LevelControl::Id:
            VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
            ProcessLevelControlBindingCommand(data, binding, peer_device->GetExchangeManager(),
                                              peer_device->GetSecureSession().Value());
            break;
        case Clusters::ColorControl::Id:
            VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
            ProcessColorControlBindingCommand(data, binding, peer_device->GetExchangeManager(),
                                              peer_device->GetSecureSession().Value());
        }
    }
}

void LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "LightSwitchContextReleaseHandler: context is null"));
    Platform::Delete(static_cast<BindingCommandData *>(context));
}

void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        {   &server.GetFabricTable(), 
            server.GetCASESessionManager(), 
            &server.GetPersistentStorage() 
        }
    );
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    chip::BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
}

} // namespace

/********************************************************
 * Switch functions
 *********************************************************/

void SwitchWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));

    BindingCommandData * data = reinterpret_cast<BindingCommandData *>(context);

    // ChipLogError(NotSpecified, "SwitchWorkerFunction - EndpointId = %d --- ClusterId = %d", data->localEndpointId, data->clusterId);
    BindingManager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId, data->clusterId, static_cast<void *>(data));
}

void BindingWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "BindingWorkerFunction - Invalid work data"));

    EmberBindingTableEntry * entry = reinterpret_cast<EmberBindingTableEntry *>(context);
    AddBindingEntry(*entry);

    Platform::Delete(entry);
}

CHIP_ERROR InitBindingHandler()
{
    // The initialization of binding manager will try establishing connection with unicast peers
    // so it requires the Server instance to be correctly initialized. Post the init function to
    // the event queue so that everything is ready when initialization is conducted.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
    return CHIP_NO_ERROR;
}
