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

#include <app/server/Mdns.h>
#include <controller/CHIPDeviceController.h>

using chip::DeviceLayer::ConfigurationMgr;

namespace chip {
namespace Shell {

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
static CHIP_ERROR SendUDC(bool printHeader, chip::Inet::IPAddress commissioner)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "SendUDC:        ");
    }

    // SendUserDirectedCommissioningRequest(commissioner, CHIP_PORT + 2);
    SendUserDirectedCommissioningRequest(commissioner, CHIP_PORT);

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
static CHIP_ERROR ResetUDC(bool printHeader)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "SendUDC:        ");
    }

    ResetUDCStates();

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR discover(bool printHeader)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "Discover:        ");
    }

    DiscoverCommissionableNodes();

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR discover(bool printHeader, char * instance)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "Discover Instance:        ");
    }

    DiscoverCommissionableNodes(instance);

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR display(bool printHeader)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "Display:\r\n");
    }

    DisplayCommissionableNodes();

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}
#endif

static int PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                       Usage: commission <subcommand>\r\n");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    streamer_printf(sout, "  sendudc <address>          Send UDC message to address. Usage: commission sendudc 127.0.0.1\r\n");
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    streamer_printf(
        sout, "  resetudc                   Clear all pending UDC sessions from this UDC server. Usage: commission resetudc\r\n");
    streamer_printf(sout, "  discover                   Discover all commissionable nodes. Usage: commission discover\r\n");
    streamer_printf(sout,
                    "  discover-instance <name>   Discover all commissionable node with given instance name. Usage: commission "
                    "discover-instance DC514873944A5CFF\r\n");
    streamer_printf(sout,
                    "  display                    Display all discovered commissionable nodes. Usage: commission display\r\n");
#endif
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static int CommissioningHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    else if (strcmp(argv[0], "sendudc") == 0)
    {
        printf("sendudc ip=%s\n", argv[1]);
        chip::Inet::IPAddress commissioner;
        chip::Inet::IPAddress::FromString(argv[1], commissioner);
        return error = SendUDC(true, commissioner);
    }
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    else if (strcmp(argv[0], "resetudc") == 0)
    {
        printf("resetudc\n");
        return error = ResetUDC(true);
    }
    else if (strcmp(argv[0], "discover") == 0)
    {
        printf("discover\n");
        return error = discover(true);
    }
    else if (strcmp(argv[0], "discover-instance") == 0)
    {
        printf("discover instance=%s\n", argv[1]);
        return error = discover(true, argv[1]);
    }
    else if (strcmp(argv[0], "display") == 0)
    {
        printf("display\n");
        return error = display(true);
    }
#endif
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void RegisterCommissioningCommands()
{

    static const shell_command_t sDeviceComand = { &CommissioningHandler, "commission",
                                                   "Commission commands. Usage: commission [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
