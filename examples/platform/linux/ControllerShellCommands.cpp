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
 * @file Contains shell commands for for performing discovery (eg. of commissionable nodes) related to commissioning.
 */

#include <ControllerShellCommands.h>
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

using namespace ::chip::Controller;

DeviceCommissioner * gCommissioner;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
static CHIP_ERROR ResetUDC(bool printHeader)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "resetudc:        ");
    }

    gCommissioner->GetUserDirectedCommissioningServer()->ResetUDCClientProcessingStates();

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

    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kNone, (uint16_t) 0);
    gCommissioner->DiscoverCommissionableNodes(filter);

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

    Mdns::DiscoveryFilter filter(Mdns::DiscoveryFilterType::kInstanceName, instance);
    gCommissioner->DiscoverCommissionableNodes(filter);

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

    for (int i = 0; i < 10; i++)
    {
        const chip::Mdns::DiscoveredNodeData * next = gCommissioner->GetDiscoveredDevice(i);
        if (next == nullptr)
        {
            streamer_printf(sout, "  Entry %d null\r\n", i);
        }
        else
        {
            streamer_printf(sout, "  Entry %d instanceName=%s host=%s longDiscriminator=%d vendorId=%d productId=%d\r\n", i,
                            next->instanceName, next->hostName, next->longDiscriminator, next->vendorId, next->productId);
        }
    }

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                       Usage: discover <subcommand>\r\n");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    streamer_printf(
        sout, "  resetudc                   Clear all pending UDC sessions from this UDC server. Usage: commission resetudc\r\n");
    streamer_printf(sout, "  commissionable                   Discover all commissionable nodes. Usage: commission discover\r\n");
    streamer_printf(
        sout,
        "  commissionable-instance <name>   Discover all commissionable node with given instance name. Usage: commission "
        "commissionable-instance DC514873944A5CFF\r\n");
    streamer_printf(sout,
                    "  display                    Display all discovered commissionable nodes. Usage: commission display\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR DiscoverHandler(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    else if (strcmp(argv[0], "resetudc") == 0)
    {
        return error = ResetUDC(true);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    else if (strcmp(argv[0], "commissionable") == 0)
    {
        return error = discover(true);
    }
    else if (strcmp(argv[0], "commissionable-instance") == 0)
    {
        return error = discover(true, argv[1]);
    }
    else if (strcmp(argv[0], "display") == 0)
    {
        return error = display(true);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

void RegisterDiscoverCommands(chip::Controller::DeviceCommissioner * commissioner)
{
    gCommissioner                              = commissioner;
    static const shell_command_t sDeviceComand = { &DiscoverHandler, "discover",
                                                   "Discover commands. Usage: discover [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
    return;
}

} // namespace Shell
} // namespace chip
