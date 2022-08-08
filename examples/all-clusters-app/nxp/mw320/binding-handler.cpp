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

#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "lib/core/CHIPError.h"
#include "platform/CHIPDeviceLayer.h"

#if defined(ENABLE_CHIP_SHELL)
#include "lib/shell/Engine.h"

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

    if (binding.type == EMBER_MULTICAST_BINDING)
    {
        ChipLogError(NotSpecified, "Group binding is not supported now");
        return;
    }

    if (binding.type == EMBER_UNICAST_BINDING && binding.local == 1 &&
        (!binding.clusterId.HasValue() || binding.clusterId.Value() == Clusters::OnOff::Id))
    {
        auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
            ChipLogProgress(NotSpecified, "OnOff command succeeds");
        };
        auto onFailure = [](CHIP_ERROR error) {
            ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
        };
        // Note: Need to change using toggle command since sSwitchOnOffState won't be changed if not triggerred from the switch
        // command (SwitchCommandHandler)
        {
            Clusters::OnOff::Commands::Toggle::Type toggleCommand;
            VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());
            Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                             binding.remote, toggleCommand, onSuccess, onFailure);
        }
    }
}

static void BoundDeviceContextReleaseHandler(void * context)
{
    (void) context;
}

static void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler);
    chip::BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(BoundDeviceContextReleaseHandler);
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
