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

#include "binding-handler.h"

#include "AppConfig.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "platform/CHIPDeviceLayer.h"

#if defined(ENABLE_CHIP_SHELL)
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;

#if defined(ENABLE_CHIP_SHELL)
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellSwitchSubCommands;
Engine sShellSwitchGroupsSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

namespace {

void ProcessOnOffBindingCommand(CommandId commandId, const EmberBindingTableEntry & binding, DeviceProxy * peer_device)
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
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         toggleCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Clusters::OnOff::Commands::On::Type onCommand;
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         onCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Clusters::OnOff::Commands::Off::Type offCommand;
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         offCommand, onSuccess, onFailure);
        break;

    default:
        ChipLogError(NotSpecified, "Invalid binding command data - commandId is not supported");
        break;
    }
}

void LightSwitchChangedHandler(const EmberBindingTableEntry & binding, DeviceProxy * peer_device, void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "Invalid context for Light switch handler"););
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    if (binding.type == EMBER_MULTICAST_BINDING)
    {
        ChipLogError(NotSpecified, "Group binding is not supported now");
    }
    else if (binding.type == EMBER_UNICAST_BINDING && binding.local == 1 &&
             (!binding.clusterId.HasValue() || binding.clusterId.Value() == data->clusterId))
    {

        switch (data->clusterId)
        {
        case Clusters::OnOff::Id:
            ProcessOnOffBindingCommand(data->commandId, binding, peer_device);
            break;
        default:
            ChipLogError(NotSpecified, "Invalid binding command data - clusterId is not supported");
            break;
        }
    }

    Platform::Delete(data);
}

#ifdef ENABLE_CHIP_SHELL

/********************************************************
 * Switch shell functions
 *********************************************************/

CHIP_ERROR SwitchHelpHandler(int argc, char ** argv)
{
    sShellSwitchSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnSwitchCommandHandler(int argc, char ** argv)
{
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchOnOffOn, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OffSwitchCommandHandler(int argc, char ** argv)
{
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchOnOffOff, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ToggleSwitchCommandHandler(int argc, char ** argv)
{
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchToggleOnOff, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return SwitchHelpHandler(argc, argv);
    }

    return sShellSwitchSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Groups switch shell functions
 *********************************************************/

CHIP_ERROR GroupsHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsSubCommands.ExecCommand(argc, argv);
}

static void RegisterSwitchCommands()
{
    static const shell_command_t sSwitchSubCommands[] = {
        { &SwitchHelpHandler, "help", "Usage: switch <subcommand>" },
        { &GroupsSwitchCommandHandler, "groups", "Usage: switch groups <subcommand>" },
        { &OnSwitchCommandHandler, "on", "Sends on command to bound lighting app" },
        { &OffSwitchCommandHandler, "off", "Sends off command to bound lighting app" },
        { &ToggleSwitchCommandHandler, "toggle", "Sends toggle command to bound lighting app" }
    };

    static const shell_command_t sSwitchGroupsSubCommands[] = { { &GroupsHelpHandler, "help",
                                                                  "Usage: switch groups <subcommand>" } };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch <subcommand>" };

    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, ArraySize(sSwitchGroupsSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, ArraySize(sSwitchSubCommands));
    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}
#endif // ENABLE_CHIP_SHELL

void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
}

} // namespace

void SwitchToggleOnOff(intptr_t context)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "SwitchToggleOnOff -  Out of Memory of work data"));

    data->clusterId = Clusters::OnOff::Id;
    data->commandId = Clusters::OnOff::Commands::Toggle::Id;

    BindingManager::GetInstance().NotifyBoundClusterChanged(1 /* endpointId */, Clusters::OnOff::Id, static_cast<void *>(data));
}

void SwitchOnOffOn(intptr_t context)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "SwitchOnOffOn -  Out of Memory of work data"));

    data->clusterId = Clusters::OnOff::Id;
    data->commandId = Clusters::OnOff::Commands::On::Id;

    BindingManager::GetInstance().NotifyBoundClusterChanged(1 /* endpointId */, Clusters::OnOff::Id, static_cast<void *>(data));
}

void SwitchOnOffOff(intptr_t context)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "SwitchOnOffOff -  Out of Memory of work data"));

    data->clusterId = Clusters::OnOff::Id;
    data->commandId = Clusters::OnOff::Commands::Off::Id;

    BindingManager::GetInstance().NotifyBoundClusterChanged(1 /* endpointId */, Clusters::OnOff::Id, static_cast<void *>(data));
}

CHIP_ERROR InitBindingHandler()
{
    // The initialization of binding manager will try establishing connection with unicast peers
    // so it requires the Server instance to be correctly initialized. Post the init function to
    // the event queue so that everything is ready when initialization is conducted.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
#if defined(ENABLE_CHIP_SHELL)
    RegisterSwitchCommands();
#endif
    return CHIP_NO_ERROR;
}
