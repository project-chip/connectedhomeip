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
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/bindings.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <controller/ReadInteraction.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;

#if CONFIG_ENABLE_CHIP_SHELL
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>

using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellSwitchSubCommands;
Engine sShellSwitchOnOffSubCommands;

Engine sShellSwitchGroupsSubCommands;
Engine sShellSwitchGroupsOnOffSubCommands;

Engine sShellSwitchBindingSubCommands;

#endif // defined(ENABLE_CHIP_SHELL)

#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
extern void UpdateLightingStatetoGUI(EndpointId endpointId, uint8_t status);
#endif

void BindingHandler::Init()
{
    // The initialization of binding manager will try establishing connection with unicast peers
    // so it requires the Server instance to be correctly initialized. Post the init function to
    // the event queue so that everything is ready when initialization is conducted.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitInternal);
#if CONFIG_ENABLE_CHIP_SHELL
    RegisterSwitchCommands();
#endif
}

void BindingHandler::OnInvokeCommandFailure(BindingData & aBindingData, CHIP_ERROR aError)
{
    CHIP_ERROR error;

    if (aError == CHIP_ERROR_TIMEOUT && !BindingHandler::GetInstance().mCaseSessionRecovered)
    {
        ChipLogProgress(NotSpecified, "Response timeout for invoked command, trying to recover CASE session.");

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
            ChipLogProgress(NotSpecified, "NotifyBoundClusterChanged failed due to: %" CHIP_ERROR_FORMAT, error.Format());
            return;
        }
    }
    else
    {
        ChipLogProgress(NotSpecified, "Binding command was not applied! Reason: %" CHIP_ERROR_FORMAT, aError.Format());
    }
}

void BindingHandler::OnOffProcessCommand(CommandId aCommandId, const EmberBindingTableEntry & aBinding,
                                         OperationalDeviceProxy * aDevice, void * aContext)
{
    CHIP_ERROR ret     = CHIP_NO_ERROR;
    BindingData * data = reinterpret_cast<BindingData *>(aContext);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Binding command applied successfully!");

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
        ChipLogProgress(NotSpecified, "Invalid binding command data - commandId is not supported");
        break;
    }
    if (CHIP_NO_ERROR != ret)
    {
        ChipLogProgress(NotSpecified, "Invoke OnOff Command Request ERROR: %s", ErrorStr(ret));
    }
}

void BindingHandler::LightSwitchChangedHandler(const EmberBindingTableEntry & aBinding, OperationalDeviceProxy * deviceProxy,
                                               void * context)
{
#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
    if (context == nullptr)
    {
        chip::EndpointId localEndpointId = aBinding.local;
        ChipLogProgress(NotSpecified, "localEndpointId=%d", localEndpointId);

        if (aBinding.type == MATTER_UNICAST_BINDING &&
            (!aBinding.clusterId.has_value() || aBinding.clusterId.value() == Clusters::OnOff::Id))
        {
            auto onReport = [localEndpointId](const app::ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
                ChipLogProgress(NotSpecified, "SubscribeAttribute onReport OnOff=%d", dataResponse);
                UpdateLightingStatetoGUI(localEndpointId, (uint8_t) dataResponse);
            };

            auto onError = [](const app::ConcreteDataAttributePath * attributePath, CHIP_ERROR aError) {
                ChipLogError(NotSpecified, "SubscribeAttribute failed: %" CHIP_ERROR_FORMAT, aError.Format());
                // todo
            };

            auto onSubscriptionEstablishedCb = [localEndpointId](const app::ReadClient & readClient,
                                                                 SubscriptionId subscriptionId) {
                ChipLogProgress(NotSpecified, "onSubscriptionEstablishedCb %d", localEndpointId); // online
                UpdateLightingStatetoGUI(localEndpointId, 3);
            };

            auto onResubscriptionAttemptCb = [localEndpointId](const app::ReadClient & readClient, CHIP_ERROR aError,
                                                               uint32_t aNextResubscribeIntervalMsec) {
                ChipLogProgress(NotSpecified, "onResubscriptionAttemptCb %d", localEndpointId); // offline
                UpdateLightingStatetoGUI(localEndpointId, 2);
            };

            Controller::SubscribeAttribute<Clusters::OnOff::Attributes::OnOff::TypeInfo>(
                deviceProxy->GetExchangeManager(), deviceProxy->GetSecureSession().Value(), aBinding.remote, onReport, onError, 1,
                15, onSubscriptionEstablishedCb, onResubscriptionAttemptCb);
        }

        return;
    }
#endif

    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingData * data = static_cast<BindingData *>(context);

    if (aBinding.type == MATTER_MULTICAST_BINDING && data->IsGroup)
    {
        switch (data->ClusterId)
        {
        case Clusters::OnOff::Id:
            OnOffProcessCommand(data->CommandId, aBinding, nullptr, context);
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
    ChipLogProgress(NotSpecified, "Initialize binding Handler");
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    chip::BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
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

#ifdef CONFIG_ENABLE_CHIP_SHELL

/********************************************************
 * Switch shell functions
 *********************************************************/

CHIP_ERROR BindingHandler::SwitchHelpHandler(int argc, char ** argv)
{
    sShellSwitchSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return SwitchHelpHandler(argc, argv);
    }

    return sShellSwitchSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * OnOff switch shell functions
 *********************************************************/

CHIP_ERROR BindingHandler::OnOffHelpHandler(int argc, char ** argv)
{
    sShellSwitchOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::OnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return sShellSwitchOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR BindingHandler::OnSwitchCommandHandler(int argc, char ** argv)
{
    BindingData * data = Platform::New<BindingData>();
    data->EndpointId   = atoi(argv[0]) + 1;
    data->CommandId    = Clusters::OnOff::Commands::On::Id;
    data->ClusterId    = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::OffSwitchCommandHandler(int argc, char ** argv)
{
    BindingData * data = Platform::New<BindingData>();
    data->EndpointId   = atoi(argv[0]) + 1;
    data->CommandId    = Clusters::OnOff::Commands::Off::Id;
    data->ClusterId    = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::ToggleSwitchCommandHandler(int argc, char ** argv)
{
    BindingData * data = Platform::New<BindingData>();
    data->EndpointId   = atoi(argv[0]) + 1;
    data->CommandId    = Clusters::OnOff::Commands::Toggle::Id;
    data->ClusterId    = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * bind switch shell functions
 *********************************************************/

CHIP_ERROR BindingHandler::BindingHelpHandler(int argc, char ** argv)
{
    sShellSwitchBindingSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::BindingSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return BindingHelpHandler(argc, argv);
    }

    return sShellSwitchBindingSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR BindingHandler::BindingGroupBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 2, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = MATTER_MULTICAST_BINDING;
    entry->fabricIndex             = atoi(argv[0]);
    entry->groupId                 = atoi(argv[1]);
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->clusterId.emplace(6);        // Hardcoded to OnOff cluster for now

    DeviceLayer::PlatformMgr().ScheduleWork(BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::BindingUnicastBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 3, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = MATTER_UNICAST_BINDING;
    entry->fabricIndex             = atoi(argv[0]);
    entry->nodeId                  = atoi(argv[1]);
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->remote                  = atoi(argv[2]);
    entry->clusterId.emplace(6); // Hardcode to OnOff cluster for now

    DeviceLayer::PlatformMgr().ScheduleWork(BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Groups switch shell functions
 *********************************************************/

CHIP_ERROR BindingHandler::GroupsHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::GroupsSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Groups OnOff switch shell functions
 *********************************************************/

CHIP_ERROR BindingHandler::GroupsOnOffHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::GroupsOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsOnOffHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR BindingHandler::GroupOnSwitchCommandHandler(int argc, char ** argv)
{
    BindingData * data = Platform::New<BindingData>();
    data->EndpointId   = 1;
    data->CommandId    = Clusters::OnOff::Commands::On::Id;
    data->ClusterId    = Clusters::OnOff::Id;
    data->IsGroup      = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::GroupOffSwitchCommandHandler(int argc, char ** argv)
{
    BindingData * data = Platform::New<BindingData>();
    data->EndpointId   = 1;
    data->CommandId    = Clusters::OnOff::Commands::Off::Id;
    data->ClusterId    = Clusters::OnOff::Id;
    data->IsGroup      = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::GroupToggleSwitchCommandHandler(int argc, char ** argv)
{
    BindingData * data = Platform::New<BindingData>();
    data->EndpointId   = 1;
    data->CommandId    = Clusters::OnOff::Commands::Toggle::Id;
    data->ClusterId    = Clusters::OnOff::Id;
    data->IsGroup      = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingHandler::TableCommandHelper(int argc, char ** argv)
{
    BindingTable & bindingTable = BindingTable::GetInstance();

    streamer_printf(streamer_get(), "Binding Table size: [%d]:\r\n", bindingTable.Size());
    uint8_t i = 0;
    for (auto & entry : bindingTable)
    {
        switch (entry.type)
        {
        case MATTER_UNICAST_BINDING:
            streamer_printf(streamer_get(), "[%d] UNICAST:\r\n", i++);
            streamer_printf(streamer_get(), "\t\t+ Fabric: %d\r\n \
            \t+ LocalEndpoint %d \r\n \
            \t+ ClusterId %d \r\n \
            \t+ RemoteEndpointId %d \r\n \
            \t+ NodeId %d\r\n",
                            (int) entry.fabricIndex, (int) entry.local, (int) entry.clusterId.value_or(kInvalidClusterId),
                            (int) entry.remote, (int) entry.nodeId);
            break;
        case MATTER_MULTICAST_BINDING:
            streamer_printf(streamer_get(), "[%d] GROUP:\r\n", i++);
            streamer_printf(streamer_get(), "\t\t+ Fabric: %d\r\n \
            \t+ LocalEndpoint %d \r\n \
            \t+ RemoteEndpointId %d \r\n \
            \t+ GroupId %d\r\n",
                            (int) entry.fabricIndex, (int) entry.local, (int) entry.remote, (int) entry.groupId);
            break;
        case MATTER_UNUSED_BINDING:
            streamer_printf(streamer_get(), "[%d] UNUSED\r\n", i++);
            break;
        default:
            break;
        }
    }
    return CHIP_NO_ERROR;
}

/**
 * @brief configures switch matter shell
 *
 */
void BindingHandler::RegisterSwitchCommands()
{
    static const shell_command_t sSwitchSubCommands[] = {
        { &SwitchHelpHandler, "help", "Usage: switch <subcommand>" },
        { &OnOffSwitchCommandHandler, "onoff", " Usage: switch onoff <subcommand>" },
        { &GroupsSwitchCommandHandler, "groups", "Usage: switch groups <subcommand>" },
        { &BindingSwitchCommandHandler, "binding", "Usage: switch binding <subcommand>" },
        { &TableCommandHelper, "table", "Usage: switch table" }
    };

    static const shell_command_t sSwitchOnOffSubCommands[] = {
        { &OnOffHelpHandler, "help", "Usage : switch ononff <subcommand>" },
        { &OnSwitchCommandHandler, "on", "Sends on command to bound lighting app" },
        { &OffSwitchCommandHandler, "off", "Sends off command to bound lighting app" },
        { &ToggleSwitchCommandHandler, "toggle", "Sends toggle command to bound lighting app" }
    };

    static const shell_command_t sSwitchGroupsSubCommands[] = { { &GroupsHelpHandler, "help", "Usage: switch groups <subcommand>" },
                                                                { &GroupsOnOffSwitchCommandHandler, "onoff",
                                                                  "Usage: switch groups onoff <subcommand>" } };

    static const shell_command_t sSwitchGroupsOnOffSubCommands[] = {
        { &GroupsOnOffHelpHandler, "help", "Usage: switch groups onoff <subcommand>" },
        { &GroupOnSwitchCommandHandler, "on", "Sends on command to bound group" },
        { &GroupOffSwitchCommandHandler, "off", "Sends off command to bound group" },
        { &GroupToggleSwitchCommandHandler, "toggle", "Sends toggle command to group" }
    };

    static const shell_command_t sSwitchBindingSubCommands[] = {
        { &BindingHelpHandler, "help", "Usage: switch binding <subcommand>" },
        { &BindingGroupBindCommandHandler, "group", "Usage: switch binding group <fabric index> <group id>" },
        { &BindingUnicastBindCommandHandler, "unicast", "Usage: switch binding unicast <fabric index> <node id> <endpoint>" }
    };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch <subcommand>" };

    sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwitchGroupsOnOffSubCommands,
                                                        MATTER_ARRAY_SIZE(sSwitchGroupsOnOffSubCommands));
    sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, MATTER_ARRAY_SIZE(sSwitchOnOffSubCommands));
    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, MATTER_ARRAY_SIZE(sSwitchGroupsSubCommands));
    sShellSwitchBindingSubCommands.RegisterCommands(sSwitchBindingSubCommands, MATTER_ARRAY_SIZE(sSwitchBindingSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, MATTER_ARRAY_SIZE(sSwitchSubCommands));

    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}

void BindingHandler::BindingWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "BindingWorkerFunction - Invalid work data"));

    EmberBindingTableEntry * entry = reinterpret_cast<EmberBindingTableEntry *>(context);
    AddBindingEntry(*entry);

    Platform::Delete(entry);
}

#endif // ENABLE_CHIP_SHELL

/********************************************************
 * Switch functions
 *********************************************************/
void BindingHandler::SwitchWorkerFunction(intptr_t context)
{
#if CONFIG_ENABLE_CHIP_SHELL
    VerifyOrReturn(context != 0, streamer_printf(streamer_get(), "SwitchWorkerFunction - Invalid work data\r\n"));
#else
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));
#endif
    BindingData * data = reinterpret_cast<BindingData *>(context);

#if CONFIG_ENABLE_CHIP_SHELL
    streamer_printf(streamer_get(), "Notify Bounded Cluster | endpoint: %d CLuster: %d\r\n", data->EndpointId, data->ClusterId);
#endif
    BindingManager::GetInstance().NotifyBoundClusterChanged(data->EndpointId, data->ClusterId, static_cast<void *>(data));
}

void BindingHandler::SwitchWorkerFunction2(int localEndpointId)
{
    BindingManager::GetInstance().NotifyBoundClusterChanged(localEndpointId, Clusters::OnOff::Id, nullptr);
}

void BindingHandler::SwitchWorkerFunction3(intptr_t context)
{
    SubscribeCommandData * data = reinterpret_cast<SubscribeCommandData *>(context);
    chip::app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(data->fabricIndex, data->nodeId);
}
