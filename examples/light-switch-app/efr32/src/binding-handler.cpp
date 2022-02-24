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

#if defined(ENABLE_CHIP_SHELL)
#include "lib/shell/Engine.h"

using chip::Shell::Engine;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;
#endif // defined(ENABLE_CHIP_SHELL)

using namespace chip;
using namespace chip::app;

namespace {

void ProcessOnOffBindingCommand(chip::CommandId commandId, const EmberBindingTableEntry & binding, chip::DeviceProxy * peer_device)
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

void LightSwitchChangedHandler(const EmberBindingTableEntry & binding, chip::DeviceProxy * peer_device, void * context)
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

    chip::Platform::Delete(data);
}

#ifdef ENABLE_CHIP_SHELL
CHIP_ERROR SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 1 && strcmp(argv[0], "on") == 0)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchOnOffOn, 0);
    }
    else if (argc == 1 && strcmp(argv[0], "off") == 0)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchOnOffOff, 0);
    }
    else if (argc == 1 && strcmp(argv[0], "toggle") == 0)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchToggleOnOff, 0);
    }
    else
    {
        streamer_printf(streamer_get(), "Usage: switch [on|off|toggle]");
    }
    return CHIP_NO_ERROR;
}

static void RegisterSwitchCommands()
{
    static const shell_command_t sSwitchCommand = { SwitchCommandHandler, "switch",
                                                    "Switch commands. Usage: switch [on|off|toggle]" };
    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
    return;
}
#endif // ENABLE_CHIP_SHELL
} // namespace

void SwitchToggleOnOff(intptr_t context)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "SwitchToggleOnOff -  Out of Memory of work data"));

    data->clusterId = chip::app::Clusters::OnOff::Id;
    data->commandId = chip::app::Clusters::OnOff::Commands::Toggle::Id;

    chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1 /* endpointId */, chip::app::Clusters::OnOff::Id,
                                                                  static_cast<void *>(data));
}

void SwitchOnOffOn(intptr_t context)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "SwitchOnOffOn -  Out of Memory of work data"));

    data->clusterId = chip::app::Clusters::OnOff::Id;
    data->commandId = chip::app::Clusters::OnOff::Commands::On::Id;

    chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1 /* endpointId */, chip::app::Clusters::OnOff::Id,
                                                                  static_cast<void *>(data));
}

void SwitchOnOffOff(intptr_t context)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    VerifyOrReturn(data != nullptr, ChipLogError(NotSpecified, "SwitchOnOffOff -  Out of Memory of work data"));

    data->clusterId = chip::app::Clusters::OnOff::Id;
    data->commandId = chip::app::Clusters::OnOff::Commands::Off::Id;

    chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1 /* endpointId */, chip::app::Clusters::OnOff::Id,
                                                                  static_cast<void *>(data));
}

CHIP_ERROR InitBindingHandler()
{
    chip::BindingManager::GetInstance().SetAppServer(&chip::Server::GetInstance());
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);

#if defined(ENABLE_CHIP_SHELL)
    RegisterSwitchCommands();
#endif

    return CHIP_NO_ERROR;
}
