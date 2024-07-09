/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "BindingHandler.h"
#include "AppConfig.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "platform/CHIPDeviceLayer.h"
#include <app/clusters/bindings/bindings.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;

void BindingHandler::Init()
{
    // The initialization of binding manager will try establishing connection with unicast peers
    // so it requires the Server instance to be correctly initialized. Post the init function to
    // the event queue so that everything is ready when initialization is conducted.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitInternal);
}

void BindingHandler::OnInvokeCommandFailure(BindingData & aBindingData, CHIP_ERROR aError)
{
    CHIP_ERROR error;

    if (aError == CHIP_ERROR_TIMEOUT && !BindingHandler::GetInstance().mCaseSessionRecovered)
    {
        ASR_LOG("Response timeout for invoked command, trying to recover CASE session.");

        // Set flag to not try recover session multiple times.
        BindingHandler::GetInstance().mCaseSessionRecovered = true;

        // Allocate new object to make sure its life time will be appropriate.
        BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
        *data                              = aBindingData;

        // Establish new CASE session and retrasmit command that was not applied.
        error = BindingManager::GetInstance().NotifyBoundClusterChanged(aBindingData.EndpointId, aBindingData.ClusterId,
                                                                        static_cast<void *>(data));

        if (CHIP_NO_ERROR != error)
        {
            ASR_LOG("NotifyBoundClusterChanged failed due to: %" CHIP_ERROR_FORMAT, error.Format());
            return;
        }
    }
    else
    {
        ASR_LOG("Binding command was not applied! Reason: %" CHIP_ERROR_FORMAT, aError.Format());
    }
}

void BindingHandler::OnOffProcessCommand(CommandId aCommandId, const EmberBindingTableEntry & aBinding,
                                         OperationalDeviceProxy * aDevice, void * aContext)
{
    CHIP_ERROR ret     = CHIP_NO_ERROR;
    BindingData * data = reinterpret_cast<BindingData *>(aContext);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ASR_LOG("Binding command applied successfully!");

        // If session was recovered and communication works, reset flag to the initial state.
        if (BindingHandler::GetInstance().mCaseSessionRecovered)
            BindingHandler::GetInstance().mCaseSessionRecovered = false;
    };

    auto onFailure = [dataRef = *data](CHIP_ERROR aError) mutable { BindingHandler::OnInvokeCommandFailure(dataRef, aError); };

    if (aDevice)
    {
        // We are validating connection is ready once here instead of multiple times in each case statement below.
        VerifyOrDie(aDevice->ConnectionReady());
    }

    switch (aCommandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Clusters::OnOff::Commands::Toggle::Type toggleCommand;
        if (aDevice)
        {
            ret = Controller::InvokeCommandRequest(aDevice->GetExchangeManager(), aDevice->GetSecureSession().Value(),
                                                   aBinding.remote, toggleCommand, onSuccess, onFailure);
        }
        else
        {

            Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();
            ret = Controller::InvokeGroupCommandRequest(&exchangeMgr, aBinding.fabricIndex, aBinding.groupId, toggleCommand);
        }
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        if (aDevice)
        {
            ret = Controller::InvokeCommandRequest(aDevice->GetExchangeManager(), aDevice->GetSecureSession().Value(),
                                                   aBinding.remote, onCommand, onSuccess, onFailure);
        }
        else
        {
            Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();
            ret = Controller::InvokeGroupCommandRequest(&exchangeMgr, aBinding.fabricIndex, aBinding.groupId, onCommand);
        }
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        if (aDevice)
        {
            ret = Controller::InvokeCommandRequest(aDevice->GetExchangeManager(), aDevice->GetSecureSession().Value(),
                                                   aBinding.remote, offCommand, onSuccess, onFailure);
        }
        else
        {
            Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();
            ret = Controller::InvokeGroupCommandRequest(&exchangeMgr, aBinding.fabricIndex, aBinding.groupId, offCommand);
        }
        break;
    default:
        ASR_LOG("Invalid binding command data - commandId is not supported");
        break;
    }
    if (CHIP_NO_ERROR != ret)
    {
        ASR_LOG("Invoke OnOff Command Request ERROR: %s", ErrorStr(ret));
    }
}

void BindingHandler::LevelControlProcessCommand(CommandId aCommandId, const EmberBindingTableEntry & aBinding,
                                                OperationalDeviceProxy * aDevice, void * aContext)
{
    CHIP_ERROR ret     = CHIP_NO_ERROR;
    BindingData * data = reinterpret_cast<BindingData *>(aContext);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ASR_LOG("Binding command applied successfully!");

        // If session was recovered and communication works, reset flag to the initial state.
        if (BindingHandler::GetInstance().mCaseSessionRecovered)
            BindingHandler::GetInstance().mCaseSessionRecovered = false;
    };

    auto onFailure = [dataRef = *data](CHIP_ERROR aError) mutable { BindingHandler::OnInvokeCommandFailure(dataRef, aError); };

    if (aDevice)
    {
        // We are validating connection is ready once here instead of multiple times in each case statement below.
        VerifyOrDie(aDevice->ConnectionReady());
    }

    switch (aCommandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id: {
        Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
        moveToLevelCommand.level = data->Value;
        if (aDevice)
        {
            ret = Controller::InvokeCommandRequest(aDevice->GetExchangeManager(), aDevice->GetSecureSession().Value(),
                                                   aBinding.remote, moveToLevelCommand, onSuccess, onFailure);
        }
        else
        {
            Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();
            ret = Controller::InvokeGroupCommandRequest(&exchangeMgr, aBinding.fabricIndex, aBinding.groupId, moveToLevelCommand);
        }
    }
    break;
    default:
        ASR_LOG("Invalid binding command data - CommandId is not supported");
        break;
    }
    if (CHIP_NO_ERROR != ret)
    {
        ASR_LOG("Invoke Group Command Request ERROR: %s", ErrorStr(ret));
    }
}

void BindingHandler::LightSwitchChangedHandler(const EmberBindingTableEntry & aBinding, OperationalDeviceProxy * deviceProxy,
                                               void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingData * data = static_cast<BindingData *>(context);

    if (aBinding.type == MATTER_MULTICAST_BINDING && data->IsGroup)
    {
        switch (data->ClusterId)
        {
        case Clusters::OnOff::Id:
            OnOffProcessCommand(data->CommandId, aBinding, nullptr, context);
            break;
        case Clusters::LevelControl::Id:
            LevelControlProcessCommand(data->CommandId, aBinding, nullptr, context);
            break;
        default:
            ChipLogError(NotSpecified, "Invalid binding group command data");
            break;
        }
    }
    else if (aBinding.type == MATTER_UNICAST_BINDING && !data->IsGroup)
    {
        switch (data->ClusterId)
        {
        case Clusters::OnOff::Id:
            OnOffProcessCommand(data->CommandId, aBinding, deviceProxy, context);
            break;
        case Clusters::LevelControl::Id:
            LevelControlProcessCommand(data->CommandId, aBinding, deviceProxy, context);
            break;
        default:
            ChipLogError(NotSpecified, "Invalid binding unicast command data");
            break;
        }
    }
}

void BindingHandler::LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "LightSwitchContextReleaseHandler: context is null"));

    Platform::Delete(static_cast<BindingData *>(context));
}

void BindingHandler::InitInternal(intptr_t arg)
{
    ASR_LOG("Initialize binding Handler");
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    chip::BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
    BindingHandler::GetInstance().PrintBindingTable();
}

bool BindingHandler::IsGroupBound()
{
    BindingTable & bindingTable = BindingTable::GetInstance();

    for (auto & entry : bindingTable)
    {
        if (MATTER_MULTICAST_BINDING == entry.type)
        {
            return true;
        }
    }
    return false;
}

void BindingHandler::PrintBindingTable()
{
    BindingTable & bindingTable = BindingTable::GetInstance();

    ASR_LOG("Binding Table size: [%d]:", bindingTable.Size());
    uint8_t i = 0;
    for (auto & entry : bindingTable)
    {
        switch (entry.type)
        {
        case MATTER_UNICAST_BINDING:
            ASR_LOG("[%d] UNICAST:", i++);
            ASR_LOG("\t\t+ Fabric: %d\n \
            \t+ LocalEndpoint %d \n \
            \t+ ClusterId %d \n \
            \t+ RemoteEndpointId %d \n \
            \t+ NodeId %d",
                    (int) entry.fabricIndex, (int) entry.local, (int) entry.clusterId.value_or(kInvalidClusterId),
                    (int) entry.remote, (int) entry.nodeId);
            break;
        case MATTER_MULTICAST_BINDING:
            ASR_LOG("[%d] GROUP:", i++);
            ASR_LOG("\t\t+ Fabric: %d\n \
            \t+ LocalEndpoint %d \n \
            \t+ RemoteEndpointId %d \n \
            \t+ GroupId %d",
                    (int) entry.fabricIndex, (int) entry.local, (int) entry.remote, (int) entry.groupId);
            break;
        case MATTER_UNUSED_BINDING:
            ASR_LOG("[%d] UNUSED", i++);
            break;
        // case MATTER_MANY_TO_ONE_BINDING:
        //     ASR_LOG("[%d] MANY TO ONE", i++);
        //     break;
        default:
            break;
        }
    }
}

/********************************************************
 * Switch functions
 *********************************************************/

void BindingHandler::SwitchWorkerHandler(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));

    BindingData * data = reinterpret_cast<BindingData *>(context);
    BindingManager::GetInstance().NotifyBoundClusterChanged(data->EndpointId, data->ClusterId, static_cast<void *>(data));
}
