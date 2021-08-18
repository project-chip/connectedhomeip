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

/**
 * @file Contains shell commands for a commissionee (eg. end device) related to commissioning.
 */

#include <CommissioneeShellCommands.h>
#include <app/server/Mdns.h>
#include <app/server/Server.h>
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

namespace chip {
namespace Shell {

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
static CHIP_ERROR SendUDC(bool printHeader, chip::Transport::PeerAddress commissioner)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "SendUDC:        ");
    }

    SendUserDirectedCommissioningRequest(commissioner);

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                       Usage: commissionee <subcommand>\r\n");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    streamer_printf(sout,
                    "  sendudc <address> <port>   Send UDC message to address. Usage: commissionee sendudc 127.0.0.1 5543\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    streamer_printf(sout,
                    "  restartmdns <commissioningMode> (disabled|enabled_basic|enabled_enhanced)   Start Mdns with given "
                    "settings. Usage: commissionee "
                    "restartmdns enabled_basic\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR CommissioneeHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    else if (strcmp(argv[0], "sendudc") == 0)
    {
        char * eptr;
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        uint16_t port = (uint16_t) strtol(argv[2], &eptr, 10);
        return error  = SendUDC(true, chip::Transport::PeerAddress::UDP(commissioner, port));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    else if (strcmp(argv[0], "restartmdns") == 0)
    {
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        if (strcmp(argv[1], "disabled") == 0)
        {
            chip::app::Mdns::StartServer(chip::app::Mdns::CommissioningMode::kDisabled);
            return CHIP_NO_ERROR;
        }
        if (strcmp(argv[1], "enabled_basic") == 0)
        {
            chip::app::Mdns::StartServer(chip::app::Mdns::CommissioningMode::kEnabledBasic);
            return CHIP_NO_ERROR;
        }
        else if (strcmp(argv[1], "enabled_enhanced") == 0)
        {
            chip::app::Mdns::StartServer(chip::app::Mdns::CommissioningMode::kEnabledEnhanced);
            return CHIP_NO_ERROR;
        }
        return PrintAllCommands();
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterCommissioneeCommands()
{

    static const shell_command_t sDeviceComand = { &CommissioneeHandler, "commissionee",
                                                   "Commissionee commands. Usage: commissionee [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
