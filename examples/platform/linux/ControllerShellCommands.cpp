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

#include <AppMain.h>
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
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

namespace chip {
namespace Shell {

using namespace chip;

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
static CHIP_ERROR ResetUDC(bool printHeader)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "udc-reset:        ");
    }

    GetDeviceCommissioner()->GetUserDirectedCommissioningServer()->ResetUDCClientProcessingStates();

    streamer_printf(sout, "done\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR PrintUDC(bool printHeader)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "udc-print:        \r\n");
    }

    GetDeviceCommissioner()->GetUserDirectedCommissioningServer()->PrintUDCClients();

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

    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);
    GetDeviceCommissioner()->DiscoverCommissionableNodes(filter);

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

    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kInstanceName, instance);
    GetDeviceCommissioner()->DiscoverCommissionableNodes(filter);

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
        const Dnssd::CommissionNodeData * next = GetDeviceCommissioner()->GetDiscoveredDevice(i);
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

static CHIP_ERROR pairOnNetwork(bool printHeader, uint32_t pincode, uint16_t disc, Transport::PeerAddress address)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "onnetwork \r\n");
    }

    CHIP_ERROR err = CommissionerPairOnNetwork(pincode, disc, address);

    streamer_printf(sout, "done\r\n");

    return err;
}

CHIP_ERROR pairUDC(bool printHeader, uint32_t pincode, size_t index)
{
    streamer_t * sout = streamer_get();

    if (printHeader)
    {
        streamer_printf(sout, "udc-commission %ld %ld\r\n", pincode, index);
    }

    CHIP_ERROR err = CommissionerPairUDC(pincode, index);
    streamer_printf(sout, "done\r\n");

    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

static CHIP_ERROR PrintAllCommands()
{
    streamer_t * sout = streamer_get();
    streamer_printf(sout, "  help                       Usage: controller <subcommand>\r\n");
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    streamer_printf(
        sout, "  udc-reset                   Clear all pending UDC sessions from this UDC server. Usage: controller udc-reset\r\n");
    streamer_printf(
        sout, "  udc-print                   Print all pending UDC sessions from this UDC server. Usage: controller udc-print\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    streamer_printf(sout, "  ux ok|cancel [<pincode>]   User input. Usage: controller ux ok 34567890\r\n");
    streamer_printf(sout,
                    "  udc-commission <pincode> <udc-entry>     Commission given udc-entry using given pincode. Usage: controller "
                    "udc-commission 34567890 0\r\n");
    streamer_printf(
        sout,
        "  discover-commissionable          Discover all commissionable nodes. Usage: controller discover-commissionable\r\n");
    streamer_printf(
        sout,
        "  discover-commissionable-instance <name>   Discover all commissionable node with given instance name. Usage: controller "
        "discover-commissionable-instance DC514873944A5CFF\r\n");
    streamer_printf(
        sout, "  discover-display           Display all discovered commissionable nodes. Usage: controller discover-display\r\n");
    streamer_printf(
        sout,
        "  commission-onnetwork <pincode> <disc> <IP> <port>   Pair given device. Usage: controller commission-onnetwork "
        "20202021 3840 127.0.0.1 5540\r\n");
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

static CHIP_ERROR ControllerHandler(int argc, char ** argv)
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
    else if (strcmp(argv[0], "udc-reset") == 0)
    {
        return ResetUDC(true);
    }
    else if (strcmp(argv[0], "udc-print") == 0)
    {
        return PrintUDC(true);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    else if (strcmp(argv[0], "discover-commissionable") == 0)
    {
        return discover(true);
    }
    else if (strcmp(argv[0], "discover-commissionable-instance") == 0)
    {
        return discover(true, argv[1]);
    }
    else if (strcmp(argv[0], "discover-display") == 0)
    {
        return display(true);
    }
    else if (strcmp(argv[0], "commission-onnetwork") == 0)
    {
        // onnetwork pincode disc IP port
        if (argc < 5)
        {
            return PrintAllCommands();
        }
        char * eptr;
        uint32_t pincode = (uint32_t) strtol(argv[1], &eptr, 10);
        uint16_t disc    = (uint16_t) strtol(argv[2], &eptr, 10);

        Inet::IPAddress address;
        Inet::IPAddress::FromString(argv[3], address);

        uint16_t port = (uint16_t) strtol(argv[4], &eptr, 10);

        return pairOnNetwork(true, pincode, disc, Transport::PeerAddress::UDP(address, port));
    }
    else if (strcmp(argv[0], "ux") == 0)
    {
        // ux ok|cancel [pincode]
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        char * response = argv[1];
        if (strcmp(response, "cancel") == 0)
        {
            GetCommissionerDiscoveryController()->Cancel();
            return CHIP_NO_ERROR;
        }
        else if (strcmp(response, "ok") == 0)
        {
            if (argc >= 3)
            {
                uint32_t passcode = (uint32_t) strtol(argv[2], &eptr, 10);
                GetCommissionerDiscoveryController()->CommissionWithPasscode(passcode);
                return CHIP_NO_ERROR;
            }
            GetCommissionerDiscoveryController()->Ok();
            return CHIP_NO_ERROR;
        }
        else
        {
            return PrintAllCommands();
        }
    }
    else if (strcmp(argv[0], "udc-commission") == 0)
    {
        // udc-commission passcode index
        if (argc < 3)
        {
            return PrintAllCommands();
        }
        char * eptr;
        uint32_t passcode = (uint32_t) strtol(argv[1], &eptr, 10);
        size_t index      = (size_t) strtol(argv[2], &eptr, 10);
        return pairUDC(true, passcode, index);
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    return CHIP_ERROR_INVALID_ARGUMENT;
}

void RegisterControllerCommands()
{
    static const shell_command_t sDeviceComand = { &ControllerHandler, "controller",
                                                   "Controller commands. Usage: controller [command_name]" };

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

} // namespace Shell
} // namespace chip
