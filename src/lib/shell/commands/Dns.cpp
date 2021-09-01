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

#include <inet/IPAddress.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/PeerId.h>
#include <lib/mdns/Advertiser.h>
#include <lib/mdns/Resolver.h>
#include <lib/mdns/platform/Mdns.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace Shell {

namespace {

Shell::Engine sShellDnsBrowseSubcommands;
Shell::Engine sShellDnsSubcommands;

class DnsShellResolverDelegate : public Mdns::ResolverDelegate
{
public:
    void OnNodeIdResolved(const Mdns::ResolvedNodeData & nodeData) override
    {
        streamer_printf(streamer_get(), "DNS resolve for " ChipLogFormatX64 "-" ChipLogFormatX64 " succeeded:\n",
                        ChipLogValueX64(nodeData.mPeerId.GetCompressedFabricId()), ChipLogValueX64(nodeData.mPeerId.GetNodeId()));
        streamer_printf(streamer_get(), "   Hostname: %s\n", nodeData.mHostName);
        streamer_printf(streamer_get(), "   IP address: %s\n", nodeData.mAddress.ToString(ipAddressBuf));
        streamer_printf(streamer_get(), "   Port: %" PRIu16 "\n", nodeData.mPort);

        auto retryInterval = nodeData.GetMrpRetryIntervalIdle();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (idle): %" PRIu32 "ms\n", retryInterval.Value());

        retryInterval = nodeData.GetMrpRetryIntervalActive();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (active): %" PRIu32 "ms\n", retryInterval.Value());

        streamer_printf(streamer_get(), "   Supports TCP: %s\n", nodeData.mSupportsTcp ? "yes" : "no");
    }

    void OnNodeIdResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override {}

    void OnNodeDiscoveryComplete(const Mdns::DiscoveredNodeData & nodeData) override
    {
        if (!nodeData.IsValid())
        {
            streamer_printf(streamer_get(), "DNS browse failed - not found valid services \n");
            return;
        }

        char rotatingId[Mdns::kMaxRotatingIdLen * 2 + 1];
        Encoding::BytesToUppercaseHexString(nodeData.rotatingId, nodeData.rotatingIdLen, rotatingId, sizeof(rotatingId));

        streamer_printf(streamer_get(), "DNS browse succeeded: \n");
        streamer_printf(streamer_get(), "   Hostname: %s\n", nodeData.hostName);
        streamer_printf(streamer_get(), "   Vendor ID: %" PRIu16 "\n", nodeData.vendorId);
        streamer_printf(streamer_get(), "   Product ID: %" PRIu16 "\n", nodeData.productId);
        streamer_printf(streamer_get(), "   Long discriminator: %" PRIu16 "\n", nodeData.longDiscriminator);
        streamer_printf(streamer_get(), "   Device type: %" PRIu16 "\n", nodeData.deviceType);
        streamer_printf(streamer_get(), "   Device name: %s\n", nodeData.deviceName);
        streamer_printf(streamer_get(), "   Commissioning mode: %d\n", static_cast<int>(nodeData.commissioningMode));
        streamer_printf(streamer_get(), "   Additional pairing: %d\n", static_cast<int>(nodeData.additionalPairing));
        streamer_printf(streamer_get(), "   Pairing hint: %" PRIu16 "\n", nodeData.pairingHint);
        streamer_printf(streamer_get(), "   Pairing instruction: %s\n", nodeData.pairingInstruction);
        streamer_printf(streamer_get(), "   Rotating ID %s\n", rotatingId);

        auto retryInterval = nodeData.GetMrpRetryIntervalIdle();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (idle): %" PRIu32 "ms\n", retryInterval.Value());

        retryInterval = nodeData.GetMrpRetryIntervalActive();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (active): %" PRIu32 "ms\n", retryInterval.Value());

        streamer_printf(streamer_get(), "   Supports TCP: %s\n", nodeData.supportsTcp ? "yes" : "no");
        streamer_printf(streamer_get(), "   IP addresses:\n");
        for (uint8_t i = 0; i < nodeData.kMaxIPAddresses; i++)
        {
            if (nodeData.ipAddress[i] != Inet::IPAddress::Any)
                streamer_printf(streamer_get(), "      %s\n", nodeData.ipAddress[i].ToString(ipAddressBuf));
        }
    }

private:
    char ipAddressBuf[Inet::kMaxIPAddressStringLength];
};

DnsShellResolverDelegate sDnsShellResolverDelegate;

CHIP_ERROR ResolveHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 2, CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(streamer_get(), "Resolving ...\n");

    PeerId peerId;
    peerId.SetCompressedFabricId(strtoull(argv[0], NULL, 10));
    peerId.SetNodeId(strtoull(argv[1], NULL, 10));

    return Mdns::Resolver::Instance().ResolveNodeId(peerId, Inet::kIPAddressType_Any);
}

bool ParseSubType(int argc, char ** argv, Mdns::DiscoveryFilter & filter)
{
    if (argc == 0)
    {
        // No filtering by subtype.
        return true;
    }

    VerifyOrReturnError(argc == 1, false);

    const char * subtype = argv[0];
    // All supported subtypes are:_<S><dd...>, where the number of digits is greater than 0.
    VerifyOrReturnError(strlen(subtype) >= 3, false);
    VerifyOrReturnError(subtype[0] == '_', false);

    auto filterType = Mdns::DiscoveryFilterType::kNone;

    switch (subtype[1])
    {
    case 'S':
        filterType = Mdns::DiscoveryFilterType::kShort;
        break;
    case 'L':
        filterType = Mdns::DiscoveryFilterType::kLong;
        break;
    case 'V':
        filterType = Mdns::DiscoveryFilterType::kVendor;
        break;
    case 'T':
        filterType = Mdns::DiscoveryFilterType::kDeviceType;
        break;
    case 'C':
        filterType = Mdns::DiscoveryFilterType::kCommissioningMode;
        break;
    case 'A':
        filterType = Mdns::DiscoveryFilterType::kCommissioningModeFromCommand;
        break;
    default:
        return false;
    }

    uint16_t code;
    VerifyOrReturnError(ArgParser::ParseInt(subtype + 2, code), false);

    filter = Mdns::DiscoveryFilter(filterType, code);
    return true;
}

CHIP_ERROR BrowseCommissionableHandler(int argc, char ** argv)
{
    Mdns::DiscoveryFilter filter;

    if (!ParseSubType(argc, argv, filter))
    {
        streamer_printf(streamer_get(), "Invalid argument\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    streamer_printf(streamer_get(), "Browsing commissionable nodes...\n");

    return Mdns::Resolver::Instance().FindCommissionableNodes(filter);
}

CHIP_ERROR BrowseCommissionerHandler(int argc, char ** argv)
{
    Mdns::DiscoveryFilter filter;

    if (!ParseSubType(argc, argv, filter))
    {
        streamer_printf(streamer_get(), "Invalid argument\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    streamer_printf(streamer_get(), "Browsing commissioners...\n");

    return Mdns::Resolver::Instance().FindCommissioners(filter);
}

CHIP_ERROR BrowseHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sShellDnsBrowseSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    return sShellDnsBrowseSubcommands.ExecCommand(argc, argv);
}

CHIP_ERROR DnsHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sShellDnsSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    Mdns::Resolver::Instance().StartResolver(&DeviceLayer::InetLayer, Mdns::kMdnsPort);
    Mdns::Resolver::Instance().SetResolverDelegate(&sDnsShellResolverDelegate);

    return sShellDnsSubcommands.ExecCommand(argc, argv);
}

} // namespace

void RegisterDnsCommands()
{
    static const shell_command_t sDnsBrowseSubCommands[] = {
        { &BrowseCommissionableHandler, "commissionable",
          "Browse Matter commissionable nodes. Usage: dns browse commissionable [subtype]" },
        { &BrowseCommissionerHandler, "commissioner",
          "Browse Matter commissioner nodes. Usage: dns browse commissioner [subtype]" },
    };

    static const shell_command_t sDnsSubCommands[] = {
        { &ResolveHandler, "resolve",
          "Resolve the DNS service. Usage: dns resolve <fabric-id> <node-id> (e.g. dns resolve 5544332211 1)" },
        { &BrowseHandler, "browse",
          "Browse DNS services published by Matter nodes. Usage: dns browse <commissionable|commissioner>" },
    };

    static const shell_command_t sDnsCommand = { &DnsHandler, "dns", "Dns client commands" };

    // Register `dns browse` subcommands
    sShellDnsBrowseSubcommands.RegisterCommands(sDnsBrowseSubCommands, ArraySize(sDnsBrowseSubCommands));

    // Register `dns` subcommands with the local shell dispatcher.
    sShellDnsSubcommands.RegisterCommands(sDnsSubCommands, ArraySize(sDnsSubCommands));

    // Register the root `dns` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDnsCommand, 1);
}

} // namespace Shell
} // namespace chip
