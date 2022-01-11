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

#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include <platform/OTARequestorInterface.h>

using namespace chip::DeviceLayer;

namespace chip {

// TODO: Remove weak functions when ESP32 all-clusters-app includes code from src/app/clusters/ota-requestor
// The code requires OTA Provider client cluster which interferes with OTA Provider server cluster,
// already enabled in the all-clusters-app.
__attribute__((weak)) void SetRequestorInstance(OTARequestorInterface * instance) {}
__attribute__((weak)) OTARequestorInterface * GetRequestorInstance()
{
    return nullptr;
}

namespace Shell {
namespace {

Shell::Engine sSubShell;

CHIP_ERROR QueryImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 3, CHIP_ERROR_INVALID_ARGUMENT);

    const FabricIndex fabricIndex       = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
    const NodeId providerNodeId         = static_cast<NodeId>(strtoull(argv[1], nullptr, 10));
    const EndpointId providerEndpointId = static_cast<EndpointId>(strtoul(argv[2], nullptr, 10));

    GetRequestorInstance()->TestModeSetProviderParameters(providerNodeId, fabricIndex, providerEndpointId);
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->TriggerImmediateQuery(); });
    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplyImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(GetRequestorInstance() != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(argc == 3, CHIP_ERROR_INVALID_ARGUMENT);

    const FabricIndex fabricIndex       = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
    const NodeId providerNodeId         = static_cast<NodeId>(strtoull(argv[1], nullptr, 10));
    const EndpointId providerEndpointId = static_cast<EndpointId>(strtoul(argv[2], nullptr, 10));

    GetRequestorInstance()->TestModeSetProviderParameters(providerNodeId, fabricIndex, providerEndpointId);
    PlatformMgr().ScheduleWork([](intptr_t) { GetRequestorInstance()->ApplyUpdate(); });
    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
    }

    return error;
}
} // namespace

void RegisterOtaCommands()
{
    // Register subcommands of the `ota` commands.
    static const shell_command_t subCommands[] = {
        { &QueryImageHandler, "query", "Query for a new image. Usage: ota query <fabric-index> <provider-node-id> <endpoint-id>" },
        { &ApplyImageHandler, "apply",
          "Apply the current update. Usage ota apply <fabric-index> <provider-node-id> <endpoint-id>" },
    };

    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    // Register the root `ota` command in the top-level shell.
    static const shell_command_t otaCommand = { &OtaHandler, "ota", "OTA commands" };

    Engine::Root().RegisterCommands(&otaCommand, 1);
}

} // namespace Shell
} // namespace chip
