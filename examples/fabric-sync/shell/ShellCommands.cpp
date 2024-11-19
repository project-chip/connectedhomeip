/*
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

#include "ShellCommands.h"
#include "AddBridgeCommand.h"
#include "AddDeviceCommand.h"
#include "RemoveBridgeCommand.h"
#include "RemoveDeviceCommand.h"
#include "SyncDeviceCommand.h"

#include <admin/DeviceManager.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Shell {

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                           Usage: app <subcommand>\r\n");
    streamer_printf(sout,
                    "  add-bridge       Pair remote fabric bridge to local fabric. Usage: app add-bridge node-id setup-pin-code "
                    "device-remote-ip device-remote-port\r\n");
    streamer_printf(sout, "  remove-bridge    Remove the remote fabric bridge from the local fabric. Usage: app remove-bridge\r\n");
    streamer_printf(sout,
                    "  add-device       Pair a device to local fabric. Usage: app add-device node-id setup-pin-code "
                    "device-remote-ip device-remote-port\r\n");
    streamer_printf(sout, "  remove-device    Remove a device from the local fabric. Usage: app remove-device node-id\r\n");
    streamer_printf(sout, "  sync-device      Sync a device from other ecosystem. Usage: app sync-device endpointid\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR HandleAddBridgeCommand(int argc, char ** argv)
{
    if (argc != 5)
    {
        fprintf(stderr,
                "Invalid arguments. Usage: app add-bridge <node-id> <setup-pin-code> <device-remote-ip> <device-remote-port>\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if there is already an active command
    if (commands::CommandRegistry::Instance().IsCommandActive())
    {
        fprintf(stderr, "Another command is currently active. Please wait until it completes.\n");
        return CHIP_ERROR_BUSY;
    }

    // Parse arguments
    chip::NodeId nodeId     = static_cast<chip::NodeId>(strtoull(argv[1], nullptr, 10));
    uint32_t setupPINCode   = static_cast<uint32_t>(strtoul(argv[2], nullptr, 10));
    const char * remoteAddr = argv[3];
    uint16_t remotePort     = static_cast<uint16_t>(strtoul(argv[4], nullptr, 10));

    auto command = std::make_unique<commands::AddBridgeCommand>(nodeId, setupPINCode, remoteAddr, remotePort);

    CHIP_ERROR result = command->RunCommand();
    if (result == CHIP_NO_ERROR)
    {
        commands::CommandRegistry::Instance().SetActiveCommand(std::move(command));
    }

    return result;
}

static CHIP_ERROR HandleRemoveBridgeCommand(int argc, char ** argv)
{
    if (argc != 1)
    {
        fprintf(stderr, "Invalid arguments. Usage: app remove-bridge\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if there is already an active command
    if (commands::CommandRegistry::Instance().IsCommandActive())
    {
        fprintf(stderr, "Another command is currently active. Please wait until it completes.\n");
        return CHIP_ERROR_BUSY;
    }

    auto command = std::make_unique<commands::RemoveBridgeCommand>();

    CHIP_ERROR result = command->RunCommand();
    if (result == CHIP_NO_ERROR)
    {
        commands::CommandRegistry::Instance().SetActiveCommand(std::move(command));
    }

    return result;
}

static CHIP_ERROR HandleAddDeviceCommand(int argc, char ** argv)
{
    if (argc != 5)
    {
        fprintf(stderr,
                "Invalid arguments. Usage: app add-device <node-id> <setup-pin-code> <device-remote-ip> <device-remote-port>\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if there is already an active command
    if (commands::CommandRegistry::Instance().IsCommandActive())
    {
        fprintf(stderr, "Another command is currently active. Please wait until it completes.\n");
        return CHIP_ERROR_BUSY;
    }

    // Parse arguments
    chip::NodeId nodeId     = static_cast<chip::NodeId>(strtoull(argv[1], nullptr, 10));
    uint32_t payload        = static_cast<uint32_t>(strtoul(argv[2], nullptr, 10));
    const char * remoteAddr = argv[3];
    uint16_t remotePort     = static_cast<uint16_t>(strtoul(argv[4], nullptr, 10));

    auto command = std::make_unique<commands::AddDeviceCommand>(nodeId, payload, remoteAddr, remotePort);

    CHIP_ERROR result = command->RunCommand();
    if (result == CHIP_NO_ERROR)
    {
        commands::CommandRegistry::Instance().SetActiveCommand(std::move(command));
    }

    return result;
}

static CHIP_ERROR HandleRemoveDeviceCommand(int argc, char ** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid arguments. Usage: app remove-device <node-id>\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if there is already an active command
    if (commands::CommandRegistry::Instance().IsCommandActive())
    {
        fprintf(stderr, "Another command is currently active. Please wait until it completes.\n");
        return CHIP_ERROR_BUSY;
    }

    // Parse arguments
    chip::NodeId nodeId = static_cast<chip::NodeId>(strtoull(argv[1], nullptr, 10));

    auto command = std::make_unique<commands::RemoveDeviceCommand>(nodeId);

    CHIP_ERROR result = command->RunCommand();
    if (result == CHIP_NO_ERROR)
    {
        commands::CommandRegistry::Instance().SetActiveCommand(std::move(command));
    }

    return result;
}

static CHIP_ERROR HandleSyncDeviceCommand(int argc, char ** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid arguments. Usage: app sync-device\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Check if there is already an active command
    if (commands::CommandRegistry::Instance().IsCommandActive())
    {
        fprintf(stderr, "Another command is currently active. Please wait until it completes.\n");
        return CHIP_ERROR_BUSY;
    }

    // Parse arguments
    chip::EndpointId endpointId = static_cast<chip::EndpointId>(strtoul(argv[1], nullptr, 10));

    auto command = std::make_unique<commands::SyncDeviceCommand>(endpointId);

    CHIP_ERROR result = command->RunCommand();
    if (result == CHIP_NO_ERROR)
    {
        commands::CommandRegistry::Instance().SetActiveCommand(std::move(command));
    }

    return result;
}

static CHIP_ERROR AppPlatformHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
    else if (argc == 0 || strcmp(argv[0], "?") == 0)
    {
        return PrintAllCommands();
    }
    else if (strcmp(argv[0], "add-bridge") == 0)
    {
        return HandleAddBridgeCommand(argc, argv);
    }
    else if (strcmp(argv[0], "remove-bridge") == 0)
    {
        return HandleRemoveBridgeCommand(argc, argv);
    }
    else if (strcmp(argv[0], "add-device") == 0)
    {
        return HandleAddDeviceCommand(argc, argv);
    }
    else if (strcmp(argv[0], "remove-device") == 0)
    {
        return HandleRemoveDeviceCommand(argc, argv);
    }
    else if (strcmp(argv[0], "sync-device") == 0)
    {
        return HandleSyncDeviceCommand(argc, argv);
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterCommands()
{

    static const shell_command_t sDeviceComand = { &AppPlatformHandler, "app", "App commands. Usage: app [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

} // namespace Shell
} // namespace chip
