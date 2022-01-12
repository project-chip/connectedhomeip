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

#include "app-common/zap-generated/attribute-id.h"
#include "app-common/zap-generated/attributes/Accessors.h"
#include "app-common/zap-generated/cluster-id.h"
#include "app-common/zap-generated/command-id.h"
#include "app/CommandSender.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "app/util/af.h"
#include "lib/core/CHIPError.h"

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
    chip::BindingManager::GetInstance().NotifyBoundClusterChanged(1, ZCL_ON_OFF_CLUSTER_ID);
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

static void BoundDeviceChangedHandler(chip::EndpointId localEndpoint, chip::EndpointId remoteEndpoint, chip::ClusterId clusterId,
                                      chip::OperationalDeviceProxy * peer_device)
{
    using namespace chip;
    using namespace chip::app;
    // Unfortunately generating both cluster server and client code is not supported.
    // We need to manually compose the packet here.
    // TODO: investigate code generation issue for binding
    if (localEndpoint == 1 && clusterId == ZCL_ON_OFF_CLUSTER_ID)
    {
        CommandId command           = sSwitchOnOffState ? Clusters::OnOff::Commands::On::Id : Clusters::OnOff::Commands::Off::Id;
        CommandPathParams cmdParams = { remoteEndpoint, /* group id */ 0, clusterId, command,
                                        (chip::app::CommandPathFlags::kEndpointIdValid) };
        CommandSender sender(nullptr, peer_device->GetExchangeManager());
        sender.PrepareCommand(cmdParams);
        sender.FinishCommand();
        peer_device->SendCommands(&sender);
    }
}

CHIP_ERROR InitBindingHandlers()
{
    chip::BindingManager::GetInstance().SetAppServer(&chip::Server::GetInstance());
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler);
#if defined(ENABLE_CHIP_SHELL)
    RegisterSwitchCommands();
#endif
    return CHIP_NO_ERROR;
}
