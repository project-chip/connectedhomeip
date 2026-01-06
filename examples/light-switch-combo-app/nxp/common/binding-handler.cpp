/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "controller/ReadInteraction.h"
#include "platform/CHIPDeviceLayer.h"
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>


using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

void ProcessOnOffUnicastBindingCommand(CommandId commandId, const Binding::TableEntry & binding, OperationalDeviceProxy * peer_device)
{

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
        TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         toggleCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         onCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         offCommand, onSuccess, onFailure);
        break;
    }
}


void ProcessOnOffGroupBindingCommand(CommandId commandId, const Binding::TableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    switch (commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, toggleCommand);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, onCommand);

        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, offCommand);
        break;
    }
}

void SwitchChangedHandler(const Binding::TableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{
	VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
	BindingCommandData * data = static_cast<BindingCommandData *>(context);

	if (data->isGroup)
	{
		/*Group commands*/
	}
	else if (binding.type == Binding::MATTER_UNICAST_BINDING && !data->isGroup)
	{
		ProcessOnOffUnicastBindingCommand(data->commandId, binding, peer_device);
	}
  	
}

void SwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "SwitchContextReleaseHandler: context is null"));

    Platform::Delete(static_cast<BindingCommandData *>(context));
}


void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    Binding::Manager::GetInstance().RegisterBoundDeviceChangedHandler(SwitchChangedHandler);
    Binding::Manager::GetInstance().RegisterBoundDeviceContextReleaseHandler(SwitchContextReleaseHandler);
}


} // namespace


/********************************************************
 * Switch functions
 *********************************************************/

void SwitchWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));

    BindingCommandData * data = reinterpret_cast<BindingCommandData *>(context);
    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId, data->clusterId, static_cast<void *>(data));
}

void BindingWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "BindingWorkerFunction - Invalid work data"));

    Binding::TableEntry * entry = reinterpret_cast<Binding::TableEntry *>(context);
    TEMPORARY_RETURN_IGNORED AddBindingEntry(*entry);
}

CHIP_ERROR InitBindingHandlers()
{
    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
    return CHIP_NO_ERROR;
}
