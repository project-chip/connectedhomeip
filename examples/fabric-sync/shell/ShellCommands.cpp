/*
 *
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
#include "RemoveBridgeCommand.h"

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

// Global pointer to hold the AddBridgeCommand instance
static std::unique_ptr<AddBridgeCommand> gAddBridgeCommand;

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                           Usage: app <subcommand>\r\n");
    streamer_printf(sout,
                    "  add-bridge       Pair remote fabric bridge to local fabric. Usage: app add-bridge node-id setup-pin-code "
                    "device-remote-ip device-remote-port\r\n");
    streamer_printf(sout, "  remove-bridge    Remove the remote fabric bridge from the local fabric. Usage: app remove-bridge\r\n");
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

    // Parse arguments
    chip::NodeId nodeId     = static_cast<chip::NodeId>(strtoull(argv[1], nullptr, 10));
    uint32_t setupPINCode   = static_cast<uint32_t>(strtoul(argv[2], nullptr, 10));
    const char * remoteAddr = argv[3];
    uint16_t remotePort     = static_cast<uint16_t>(strtoul(argv[4], nullptr, 10));

    gAddBridgeCmd.reset();
    gAddBridgeCmd = std::make_unique<AddBridgeCommand>(nodeId, setupPINCode, remoteAddr, remotePort);

    return gAddBridgeCmd->RunCommand();
}

static CHIP_ERROR HandleRemoveBridgeCommand(int argc, char ** argv)
{
    if (argc != 1)
    {
        fprintf(stderr, "Invalid arguments. Usage: app remove-bridge\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    gRemoveBridgeCmd.reset();
    gRemoveBridgeCmd = std::make_unique<RemoveBridgeCommand>();

    return gRemoveBridgeCmd->RunCommand();
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
    else if (strcmp(argv[0], "sync-device") == 0)
    {
        // TODO
        return CHIP_NO_ERROR;
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
    return;
}

} // namespace Shell
} // namespace chip
