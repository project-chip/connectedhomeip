/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>


using namespace chip;
using namespace chip::app;


#if defined(ENABLE_CHIP_SHELL)
#include <lib/shell/Engine.h> // nogncheck

using chip::Shell::Engine;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;
#endif // defined(ENABLE_CHIP_SHELL)

static bool sSwitchOnOffState = false;
#if defined(ENABLE_CHIP_SHELL)
static void ToggleSwitchOnOff(bool newState)
{
    sSwitchOnOffState = newState;
    chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1, chip::app::Clusters::OnOff::Id, nullptr);
}

static CHIP_ERROR SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 1 && strcmp(argv[0], "on") == 0)
    {
        ToggleSwitchOnOff(true);
        return CHIP_NO_ERROR;
    }
    if (argc == 1 && strcmp(argv[0], "off") == 0)
    {
        ToggleSwitchOnOff(false);
        return CHIP_NO_ERROR;
    }
    streamer_printf(streamer_get(), "Usage: switch [on|off]");
    return CHIP_NO_ERROR;
}

static void RegisterSwitchCommands()
{
    static const shell_command_t sSwitchCommand = { SwitchCommandHandler, "switch", "Switch commands. Usage: switch [on|off]" };
    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
    return;
}
#endif // defined(ENABLE_CHIP_SHELL)

static void BoundDeviceChangedHandler(const EmberBindingTableEntry & binding, chip::OperationalDeviceProxy * peer_device,
                                      void * context)
{
    using namespace chip;
    using namespace chip::app;

    if (binding.type == MATTER_MULTICAST_BINDING)
    {
        ChipLogError(NotSpecified, "Group binding is not supported now");
        return;
    }

    if (binding.type == MATTER_UNICAST_BINDING && binding.local == 1 &&
        binding.clusterId.value_or(Clusters::OnOff::Id) == Clusters::OnOff::Id)
    {
        auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
            ChipLogProgress(NotSpecified, "OnOff command succeeds");
        };
        auto onFailure = [](CHIP_ERROR error) {
            ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
        };

        VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
        if (sSwitchOnOffState)
        {
            Clusters::OnOff::Commands::On::Type onCommand;
            Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                             binding.remote, onCommand, onSuccess, onFailure);
        }
        else
        {
            Clusters::OnOff::Commands::Off::Type offCommand;
            Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                             binding.remote, offCommand, onSuccess, onFailure);
        }
    }
}

static void BoundDeviceContextReleaseHandler(void * context)
{
    (void) context;
}


static void ProcessOnOffUnicastBindingCommand(chip::CommandId commandId, const EmberBindingTableEntry & binding,
                                       Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
        //board_led_on_off(LED_D2, true);
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
        //board_led_on_off(LED_D2, false);
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

static void ProcessLevelControlUnicastBindingCommand(chip::CommandId commandId, const EmberBindingTableEntry & binding,
                                       Messaging::ExchangeManager * exchangeMgr, const SessionHandle & sessionHandle, void * context)
{
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "LevelControl command succeeds");
        //board_led_on_off(LED_D2, true);
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "LevelControl command failed: %" CHIP_ERROR_FORMAT, error.Format());
        //board_led_on_off(LED_D2, false);
    };

    switch (commandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id:
        {
            Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
            moveToLevelCommand.level = data->value;
            Controller::InvokeCommandRequest(exchangeMgr, sessionHandle, binding.remote, moveToLevelCommand, onSuccess, onFailure);
        }
        break;

    default:
        ChipLogError(NotSpecified, "Invalid binding unicast command data - commandId is not supported");
        break;
    }
}

static void LightSwitchBoundDeviceChangedHandler(const EmberBindingTableEntry & binding, chip::OperationalDeviceProxy * peer_device,
                                      void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    ChipLogProgress(NotSpecified, "BoundDeviceChangedHandler Enter");

    if (binding.type == MATTER_MULTICAST_BINDING)
    {
        ChipLogError(NotSpecified, "Group binding is not supported now");
        return;
    }

    if (binding.type == MATTER_UNICAST_BINDING)
    {
       switch (data->clusterId)
       {
       case Clusters::OnOff::Id:
           VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
           ProcessOnOffUnicastBindingCommand(data->commandId, binding, peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value());
           break;
       case Clusters::LevelControl::Id:
           VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
           ProcessLevelControlUnicastBindingCommand(data->commandId, binding, peer_device->GetExchangeManager(), peer_device->GetSecureSession().        Value(), context);
           break;
       default:
           ChipLogError(NotSpecified, "Invalid binding unicast command data");
           break;
       }
    }
}

static void LightSwitchBoundDeviceContextReleaseHandler(void * context)
{
    (void) context;
}



static void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });

    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchBoundDeviceChangedHandler);
    chip::BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchBoundDeviceContextReleaseHandler);

}

CHIP_ERROR InitBindingHandlers()
{
    // The initialization of binding manager will try establishing connection with unicast peers
    // so it requires the Server instance to be correctly initialized. Post the init function to
    // the event queue so that everything is ready when initialization is conducted.
    // TODO: Fix initialization order issue in Matter server.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
#if defined(ENABLE_CHIP_SHELL)
    RegisterSwitchCommands();
#endif
    return CHIP_NO_ERROR;
}