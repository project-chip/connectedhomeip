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
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/ResolverProxy.h>
#include <lib/dnssd/platform/Dnssd.h>
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

class DnsShellResolverDelegate : public Dnssd::OperationalResolveDelegate, public Dnssd::CommissioningResolveDelegate
{
public:
    void OnOperationalNodeResolved(const Dnssd::ResolvedNodeData & nodeData) override
    {
        streamer_printf(streamer_get(), "DNS resolve for " ChipLogFormatX64 "-" ChipLogFormatX64 " succeeded:\r\n",
                        ChipLogValueX64(nodeData.operationalData.peerId.GetCompressedFabricId()),
                        ChipLogValueX64(nodeData.operationalData.peerId.GetNodeId()));
        streamer_printf(streamer_get(), "   Hostname: %s\r\n", nodeData.resolutionData.hostName);
        for (size_t i = 0; i < nodeData.resolutionData.numIPs; ++i)
        {
            streamer_printf(streamer_get(), "   IP address: %s\r\n", nodeData.resolutionData.ipAddress[i].ToString(ipAddressBuf));
        }
        streamer_printf(streamer_get(), "   Port: %u\r\n", nodeData.resolutionData.port);

        auto retryInterval = nodeData.resolutionData.GetMrpRetryIntervalIdle();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (idle): %" PRIu32 "ms\r\n", retryInterval.Value());

        retryInterval = nodeData.resolutionData.GetMrpRetryIntervalActive();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (active): %" PRIu32 "ms\r\n", retryInterval.Value());

        streamer_printf(streamer_get(), "   Supports TCP: %s\r\n", nodeData.resolutionData.supportsTcp ? "yes" : "no");
    }

    void OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error) override {}

    void OnNodeDiscovered(const Dnssd::DiscoveredNodeData & nodeData) override
    {
        if (!nodeData.resolutionData.IsValid())
        {
            streamer_printf(streamer_get(), "DNS browse failed - not found valid services \r\n");
            return;
        }

        char rotatingId[Dnssd::kMaxRotatingIdLen * 2 + 1];
        Encoding::BytesToUppercaseHexString(nodeData.commissionData.rotatingId, nodeData.commissionData.rotatingIdLen, rotatingId,
                                            sizeof(rotatingId));

        streamer_printf(streamer_get(), "DNS browse succeeded: \r\n");
        streamer_printf(streamer_get(), "   Hostname: %s\r\n", nodeData.resolutionData.hostName);
        streamer_printf(streamer_get(), "   Vendor ID: %u\r\n", nodeData.commissionData.vendorId);
        streamer_printf(streamer_get(), "   Product ID: %u\r\n", nodeData.commissionData.productId);
        streamer_printf(streamer_get(), "   Long discriminator: %u\r\n", nodeData.commissionData.longDiscriminator);
        streamer_printf(streamer_get(), "   Device type: %u\r\n", nodeData.commissionData.deviceType);
        streamer_printf(streamer_get(), "   Device name: %s\n", nodeData.commissionData.deviceName);
        streamer_printf(streamer_get(), "   Commissioning mode: %d\r\n",
                        static_cast<int>(nodeData.commissionData.commissioningMode));
        streamer_printf(streamer_get(), "   Pairing hint: %u\r\n", nodeData.commissionData.pairingHint);
        streamer_printf(streamer_get(), "   Pairing instruction: %s\r\n", nodeData.commissionData.pairingInstruction);
        streamer_printf(streamer_get(), "   Rotating ID %s\r\n", rotatingId);

        auto retryInterval = nodeData.resolutionData.GetMrpRetryIntervalIdle();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (idle): %" PRIu32 "ms\r\n", retryInterval.Value());

        retryInterval = nodeData.resolutionData.GetMrpRetryIntervalActive();

        if (retryInterval.HasValue())
            streamer_printf(streamer_get(), "   MRP retry interval (active): %" PRIu32 "ms\r\n", retryInterval.Value());

        streamer_printf(streamer_get(), "   Supports TCP: %s\r\n", nodeData.resolutionData.supportsTcp ? "yes" : "no");
        streamer_printf(streamer_get(), "   IP addresses:\r\n");
        for (uint8_t i = 0; i < nodeData.resolutionData.numIPs; i++)
        {
            streamer_printf(streamer_get(), "      %s\r\n", nodeData.resolutionData.ipAddress[i].ToString(ipAddressBuf));
        }
    }

private:
    char ipAddressBuf[Inet::IPAddress::kMaxStringLength];
};

DnsShellResolverDelegate sDnsShellResolverDelegate;
Dnssd::ResolverProxy sResolverProxy;

CHIP_ERROR ResolveHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 2, CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(streamer_get(), "Resolving ...\r\n");

    PeerId peerId;
    peerId.SetCompressedFabricId(strtoull(argv[0], nullptr, 10));
    peerId.SetNodeId(strtoull(argv[1], nullptr, 10));

    return sResolverProxy.ResolveNodeId(peerId, Inet::IPAddressType::kAny);
}

bool ParseSubType(int argc, char ** argv, Dnssd::DiscoveryFilter & filter)
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

    auto filterType = Dnssd::DiscoveryFilterType::kNone;

    switch (subtype[1])
    {
    case 'S':
        filterType = Dnssd::DiscoveryFilterType::kShortDiscriminator;
        break;
    case 'L':
        filterType = Dnssd::DiscoveryFilterType::kLongDiscriminator;
        break;
    case 'V':
        filterType = Dnssd::DiscoveryFilterType::kVendorId;
        break;
    case 'T':
        filterType = Dnssd::DiscoveryFilterType::kDeviceType;
        break;
    case 'C':
        filterType = Dnssd::DiscoveryFilterType::kCommissioningMode;
        break;
    default:
        return false;
    }

    uint16_t code;
    VerifyOrReturnError(ArgParser::ParseInt(subtype + 2, code), false);

    filter = Dnssd::DiscoveryFilter(filterType, code);
    return true;
}

CHIP_ERROR BrowseCommissionableHandler(int argc, char ** argv)
{
    Dnssd::DiscoveryFilter filter;

    if (!ParseSubType(argc, argv, filter))
    {
        streamer_printf(streamer_get(), "Invalid argument\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    streamer_printf(streamer_get(), "Browsing commissionable nodes...\r\n");

    return sResolverProxy.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR BrowseCommissionerHandler(int argc, char ** argv)
{
    Dnssd::DiscoveryFilter filter;

    if (!ParseSubType(argc, argv, filter))
    {
        streamer_printf(streamer_get(), "Invalid argument\r\n");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    streamer_printf(streamer_get(), "Browsing commissioners...\r\n");

    return sResolverProxy.DiscoverCommissioners(filter);
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

    sResolverProxy.Init(DeviceLayer::UDPEndPointManager());
    sResolverProxy.SetOperationalDelegate(&sDnsShellResolverDelegate);
    sResolverProxy.SetCommissioningDelegate(&sDnsShellResolverDelegate);

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
