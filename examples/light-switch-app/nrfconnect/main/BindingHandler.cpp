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
#ifdef CONFIG_CHIP_LIB_SHELL
#include "ShellCommands.h"
#endif

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app;

void BindingHandler::Init()
{
#ifdef CONFIG_CHIP_LIB_SHELL
    SwitchCommands::RegisterSwitchCommands();
#endif
    DeviceLayer::PlatformMgr().ScheduleWork(InitInternal);
}

void BindingHandler::OnInvokeCommandFailure(BindingData & aBindingData, CHIP_ERROR aError)
{
    CHIP_ERROR error;

    if (aError == CHIP_ERROR_TIMEOUT && !BindingHandler::GetInstance().mCaseSessionRecovered)
    {
        LOG_INF("Response timeout for invoked command, trying to recover CASE session.");

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
            LOG_ERR("NotifyBoundClusterChanged failed due to: %" CHIP_ERROR_FORMAT, error.Format());
            return;
        }
    }
    else
    {
        LOG_ERR("Binding command was not applied! Reason: %" CHIP_ERROR_FORMAT, aError.Format());
    }
}

void BindingHandler::OnOffProcessCommand(CommandId aCommandId, const EmberBindingTableEntry & aBinding,
                                         OperationalDeviceProxy * aDevice, void * aContext)
{
    CHIP_ERROR ret     = CHIP_NO_ERROR;
    BindingData * data = reinterpret_cast<BindingData *>(aContext);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        LOG_DBG("Binding command applied successfully!");

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
        LOG_DBG("Invalid binding command data - commandId is not supported");
        break;
    }
    if (CHIP_NO_ERROR != ret)
    {
        LOG_ERR("Invoke OnOff Command Request ERROR: %s", ErrorStr(ret));
    }
}

void BindingHandler::LevelControlProcessCommand(CommandId aCommandId, const EmberBindingTableEntry & aBinding,
                                                OperationalDeviceProxy * aDevice, void * aContext)
{
    BindingData * data = reinterpret_cast<BindingData *>(aContext);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        LOG_DBG("Binding command applied successfully!");

        // If session was recovered and communication works, reset flag to the initial state.
        if (BindingHandler::GetInstance().mCaseSessionRecovered)
            BindingHandler::GetInstance().mCaseSessionRecovered = false;
    };

    auto onFailure = [dataRef = *data](CHIP_ERROR aError) mutable { BindingHandler::OnInvokeCommandFailure(dataRef, aError); };

    CHIP_ERROR ret = CHIP_NO_ERROR;

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
        LOG_DBG("Invalid binding command data - commandId is not supported");
        break;
    }
    if (CHIP_NO_ERROR != ret)
    {
        LOG_ERR("Invoke Group Command Request ERROR: %s", ErrorStr(ret));
    }
}

void BindingHandler::LightSwitchChangedHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * deviceProxy,
                                               void * context)
{
    VerifyOrReturn(context != nullptr, LOG_ERR("Invalid context for Light switch handler"););
    BindingData * data = static_cast<BindingData *>(context);

    if (binding.type == MATTER_MULTICAST_BINDING && data->IsGroup)
    {
        switch (data->ClusterId)
        {
        case Clusters::OnOff::Id:
            OnOffProcessCommand(data->CommandId, binding, nullptr, context);
            break;
        case Clusters::LevelControl::Id:
            LevelControlProcessCommand(data->CommandId, binding, nullptr, context);
            break;
        default:
            LOG_ERR("Invalid binding group command data");
            break;
        }
    }
    else if (binding.type == MATTER_UNICAST_BINDING && !data->IsGroup)
    {
        switch (data->ClusterId)
        {
        case Clusters::OnOff::Id:
            OnOffProcessCommand(data->CommandId, binding, deviceProxy, context);
            break;
        case Clusters::LevelControl::Id:
            LevelControlProcessCommand(data->CommandId, binding, deviceProxy, context);
            break;
        default:
            LOG_ERR("Invalid binding unicast command data");
            break;
        }
    }
}

void BindingHandler::LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, LOG_ERR("Invalid context for Light switch context release handler"););

    Platform::Delete(static_cast<BindingData *>(context));
}

void BindingHandler::InitInternal(intptr_t aArg)
{
    LOG_INF("Initialize binding Handler");
    auto & server = Server::GetInstance();
    if (CHIP_NO_ERROR !=
        BindingManager::GetInstance().Init(
            { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() }))
    {
        LOG_ERR("BindingHandler::InitInternal failed");
    }

    BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
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

    LOG_INF("Binding Table size: [%d]:", bindingTable.Size());
    uint8_t i = 0;
    for (auto & entry : bindingTable)
    {
        switch (entry.type)
        {
        case MATTER_UNICAST_BINDING:
            LOG_INF("[%d] UNICAST:", i++);
            LOG_INF("\t\t+ Fabric: %d\n \
            \t+ LocalEndpoint %d \n \
            \t+ ClusterId %d \n \
            \t+ RemoteEndpointId %d \n \
            \t+ NodeId %d",
                    (int) entry.fabricIndex, (int) entry.local, (int) entry.clusterId.value_or(kInvalidClusterId),
                    (int) entry.remote, (int) entry.nodeId);
            break;
        case MATTER_MULTICAST_BINDING:
            LOG_INF("[%d] GROUP:", i++);
            LOG_INF("\t\t+ Fabric: %d\n \
            \t+ LocalEndpoint %d \n \
            \t+ RemoteEndpointId %d \n \
            \t+ GroupId %d",
                    (int) entry.fabricIndex, (int) entry.local, (int) entry.remote, (int) entry.groupId);
            break;
        case MATTER_UNUSED_BINDING:
            LOG_INF("[%d] UNUSED", i++);
            break;
        default:
            break;
        }
    }
}

void BindingHandler::SwitchWorkerHandler(intptr_t aContext)
{
    VerifyOrReturn(aContext != 0, LOG_ERR("Invalid Swich data"));

    BindingData * data = reinterpret_cast<BindingData *>(aContext);
    LOG_INF("Notify Bounded Cluster | endpoint: %d cluster: %d", data->EndpointId, data->ClusterId);
    BindingManager::GetInstance().NotifyBoundClusterChanged(data->EndpointId, data->ClusterId, static_cast<void *>(data));
}
