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

#include <core/CHIPCore.h>
#include <core/PeerId.h>
#include <inet/IPAddress.h>
#include <lib/mdns/Advertiser.h>
#include <lib/mdns/Resolver.h>
#include <lib/mdns/platform/Mdns.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>

namespace chip {
namespace Shell {

static chip::Shell::Engine sShellDnsSubcommands;
static bool isResolverStarted;

class DnsShellResolverDelegate : public chip::Mdns::ResolverDelegate
{
public:
    void OnNodeIdResolved(const chip::Mdns::ResolvedNodeData & nodeData) override
    {
        streamer_printf(streamer_get(), "DNS resolve for " ChipLogFormatX64 "-" ChipLogFormatX64 " succeeded:\n",
                        ChipLogValueX64(nodeData.mPeerId.GetFabricId()), ChipLogValueX64(nodeData.mPeerId.GetNodeId()));
        streamer_printf(streamer_get(), "   IP address: %s\n", nodeData.mAddress.ToString(ipAddressBuf));
        streamer_printf(streamer_get(), "   Port: %d\n", nodeData.mPort);
    }

    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override {}

    void OnNodeDiscoveryComplete(const chip::Mdns::DiscoveredNodeData & nodeData) override
    {
        if (!nodeData.IsValid())
        {
            streamer_printf(streamer_get(), "DNS browse failed - not found valid services \n");
            return;
        }
        streamer_printf(streamer_get(), "DNS browse succeeded: \n");
        streamer_printf(streamer_get(), "   Hostname: %s\n", nodeData.hostName);
        streamer_printf(streamer_get(), "   Vendor ID: %d\n", nodeData.vendorId);
        streamer_printf(streamer_get(), "   Product ID: %d\n", nodeData.productId);
        streamer_printf(streamer_get(), "   Long discriminator: %d\n", nodeData.longDiscriminator);
        streamer_printf(streamer_get(), "   Device type: %d\n", nodeData.deviceType);
        streamer_printf(streamer_get(), "   Device name: %s\n", nodeData.deviceName);
        streamer_printf(streamer_get(), "   Commissioning mode: %d\n", nodeData.commissioningMode);
        streamer_printf(streamer_get(), "   IP addresses:\n");
        for (uint8_t i = 0; i < nodeData.kMaxIPAddresses; i++)
        {
            if (nodeData.ipAddress[i] != chip::Inet::IPAddress::Any)
                streamer_printf(streamer_get(), "      %s\n", nodeData.ipAddress[i].ToString(ipAddressBuf));
        }
    }

private:
    char ipAddressBuf[chip::Inet::kMaxIPAddressStringLength];
};

static DnsShellResolverDelegate sDnsShellResolverDelegate;

CHIP_ERROR DnsHelpHandler(int argc, char ** argv)
{
    sShellDnsSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ResolveHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Resolving ...\r\n");

    VerifyOrReturnError(argc == 2, CHIP_ERROR_INVALID_ARGUMENT);

    PeerId peerId;
    peerId.SetFabricId(strtoull(argv[0], NULL, 10));
    peerId.SetNodeId(strtoull(argv[1], NULL, 10));

    return chip::Mdns::Resolver::Instance().ResolveNodeId(peerId, Inet::kIPAddressType_Any);
}

static CHIP_ERROR BrowseHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Browsing ...\r\n");

    return chip::Mdns::Resolver::Instance().FindCommissionableNodes();
}

static CHIP_ERROR DnsHandler(int argc, char ** argv)
{
    if (!isResolverStarted)
    {
        chip::Mdns::Resolver::Instance().StartResolver(&chip::DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
        chip::Mdns::Resolver::Instance().SetResolverDelegate(&sDnsShellResolverDelegate);

        isResolverStarted = true;
    }

    if (argc == 0)
    {
        DnsHelpHandler(argc, argv);
        return CHIP_NO_ERROR;
    }
    return sShellDnsSubcommands.ExecCommand(argc, argv);
}

void RegisterDnsCommands()
{
    static const shell_command_t sDnsSubCommands[] = {
        { &ResolveHandler, "resolve",
          "Resolve the DNS service. Usage: dns resolve <fabric-id> <node-id> (e.g. dns resolve 5544332211 1)" },
        { &BrowseHandler, "browse", "Browse the services published by mdns. Usage: dns browse" },
    };

    static const shell_command_t sDnsCommand = { &DnsHandler, "dns", "Dns client commands" };

    // Register `dns` subcommands with the local shell dispatcher.
    sShellDnsSubcommands.RegisterCommands(sDnsSubCommands, ArraySize(sDnsSubCommands));

    // Register the root `dns` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDnsCommand, 1);
}

} // namespace Shell
} // namespace chip
