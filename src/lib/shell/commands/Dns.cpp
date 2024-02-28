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
#include <lib/address_resolve/AddressResolve.h>
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
Dnssd::ResolverProxy sResolverProxy;

class DnsShellResolverDelegate : public Dnssd::CommissioningResolveDelegate, public AddressResolve::NodeListener
{
public:
    DnsShellResolverDelegate() { mSelfHandle.SetListener(this); }

    void OnNodeAddressResolved(const PeerId & peerId, const AddressResolve::ResolveResult & result) override
    {
        streamer_printf(streamer_get(), "DNS resolve for " ChipLogFormatX64 "-" ChipLogFormatX64 " succeeded:\r\n",
                        ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()));

        char addr_string[Transport::PeerAddress::kMaxToStringSize];
        result.address.ToString(addr_string);

        streamer_printf(streamer_get(), "Resolve completed: %s\r\n", addr_string);
        streamer_printf(streamer_get(), "   Supports TCP:                  %s\r\n", result.supportsTcp ? "YES" : "NO");
        streamer_printf(streamer_get(), "   MRP IDLE retransmit timeout:   %u ms\r\n",
                        result.mrpRemoteConfig.mIdleRetransTimeout.count());
        streamer_printf(streamer_get(), "   MRP ACTIVE retransmit timeout: %u ms\r\n",
                        result.mrpRemoteConfig.mActiveRetransTimeout.count());
        streamer_printf(streamer_get(), "   MRP ACTIVE Threshold time:     %u ms\r\n",
                        result.mrpRemoteConfig.mActiveThresholdTime.count());

        streamer_printf(streamer_get(), "   ICD is operating as a:         %s\r\n", result.isICDOperatingAsLIT ? "LIT" : "SIT");

        // Schedule a retry. Not called directly so we do not recurse in OnNodeAddressResolved
        DeviceLayer::SystemLayer().ScheduleLambda([this] {
            CHIP_ERROR err = AddressResolve::Resolver::Instance().TryNextResult(Handle());
            if (err != CHIP_NO_ERROR && err != CHIP_ERROR_NOT_FOUND)
            {
                ChipLogError(Discovery, "Failed to list next result: %" CHIP_ERROR_FORMAT, err.Format());
            }
        });
    }

    void OnNodeAddressResolutionFailed(const PeerId & peerId, CHIP_ERROR reason) override
    {
        streamer_printf(streamer_get(),
                        "DNS resolve for " ChipLogFormatX64 "-" ChipLogFormatX64 " failed: %" CHIP_ERROR_FORMAT "\r\n",
                        ChipLogValueX64(peerId.GetCompressedFabricId()), ChipLogValueX64(peerId.GetNodeId()), reason.Format());
    }

    AddressResolve::NodeLookupHandle & Handle() { return mSelfHandle; }

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

        if (nodeData.resolutionData.GetMrpRetryActiveThreshold().HasValue())
        {
            streamer_printf(streamer_get(), "   MRP retry active threshold time: %" PRIu32 "ms\r\n",
                            nodeData.resolutionData.GetMrpRetryActiveThreshold().Value());
        }
        streamer_printf(streamer_get(), "   Supports TCP: %s\r\n", nodeData.resolutionData.supportsTcp ? "yes" : "no");

        if (nodeData.resolutionData.isICDOperatingAsLIT.HasValue())
        {
            streamer_printf(streamer_get(), "   ICD is operating as a: %s\r\n",
                            nodeData.resolutionData.isICDOperatingAsLIT.Value() ? "LIT" : "SIT");
        }
        streamer_printf(streamer_get(), "   IP addresses:\r\n");
        for (size_t i = 0; i < nodeData.resolutionData.numIPs; i++)
        {
            streamer_printf(streamer_get(), "      %s\r\n", nodeData.resolutionData.ipAddress[i].ToString(ipAddressBuf));
        }
    }

private:
    char ipAddressBuf[Inet::IPAddress::kMaxStringLength];
    AddressResolve::NodeLookupHandle mSelfHandle;
};

DnsShellResolverDelegate sDnsShellResolverDelegate;

CHIP_ERROR ResolveHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 2, CHIP_ERROR_INVALID_ARGUMENT);

    if (sDnsShellResolverDelegate.Handle().IsActive())
    {
        streamer_printf(streamer_get(), "Cancelling previous resolve...\r\n");
        LogErrorOnFailure(AddressResolve::Resolver::Instance().CancelLookup(sDnsShellResolverDelegate.Handle(),
                                                                            AddressResolve::Resolver::FailureCallback::Call));
    }

    streamer_printf(streamer_get(), "Resolving ...\r\n");

    AddressResolve::NodeLookupRequest request(
        PeerId().SetCompressedFabricId(strtoull(argv[0], nullptr, 10)).SetNodeId(strtoull(argv[1], nullptr, 10)));

    return AddressResolve::Resolver::Instance().LookupNode(request, sDnsShellResolverDelegate.Handle());
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
    VerifyOrReturnError(ParseSubType(argc, argv, filter), CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(streamer_get(), "Browsing commissionable nodes...\r\n");

    return sResolverProxy.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR BrowseCommissionerHandler(int argc, char ** argv)
{
    Dnssd::DiscoveryFilter filter;
    VerifyOrReturnError(ParseSubType(argc, argv, filter), CHIP_ERROR_INVALID_ARGUMENT);

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

    sResolverProxy.Init(DeviceLayer::UDPEndPointManager());
    sResolverProxy.SetCommissioningDelegate(&sDnsShellResolverDelegate);

    return sShellDnsBrowseSubcommands.ExecCommand(argc, argv);
}

CHIP_ERROR DnsHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sShellDnsSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

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
