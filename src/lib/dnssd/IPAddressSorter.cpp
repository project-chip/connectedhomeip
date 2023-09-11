/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/dnssd/IPAddressSorter.h>
#include <lib/support/SortUtils.h>

namespace chip {
namespace Dnssd {
namespace IPAddressSorter {

void Sort(Inet::IPAddress * addresses, size_t count, Inet::InterfaceId interfaceId)
{
    Sorting::BubbleSort(addresses, count, [interfaceId](const Inet::IPAddress & a, const Inet::IPAddress & b) -> bool {
        auto scoreA = to_underlying(ScoreIpAddress(a, interfaceId));
        auto scoreB = to_underlying(ScoreIpAddress(b, interfaceId));
        return scoreA > scoreB;
    });
}

void Sort(const Span<Inet::IPAddress> & addresses, Inet::InterfaceId interfaceId)
{
    Sorting::BubbleSort(addresses.begin(), addresses.size(),
                        [interfaceId](const Inet::IPAddress & a, const Inet::IPAddress & b) -> bool {
                            auto scoreA = to_underlying(ScoreIpAddress(a, interfaceId));
                            auto scoreB = to_underlying(ScoreIpAddress(b, interfaceId));
                            return scoreA > scoreB;
                        });
}

IpScore ScoreIpAddress(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId)
{
    if (ip.IsIPv6())
    {
        if (ip.IsIPv6LinkLocal())
        {
            return IpScore::kLinkLocal;
        }

        if (interfaceId.MatchLocalIPv6Subnet(ip))
        {
            if (ip.IsIPv6GlobalUnicast())
            {
                return IpScore::kGlobalUnicastWithSharedPrefix;
            }
            if (ip.IsIPv6ULA())
            {
                return IpScore::kUniqueLocalWithSharedPrefix;
            }
        }
        if (ip.IsIPv6GlobalUnicast())
        {
            return IpScore::kGlobalUnicast;
        }

        if (ip.IsIPv6ULA())
        {
            return IpScore::kUniqueLocal;
        }

        return IpScore::kOtherIpv6;
    }

    return IpScore::kIpv4;
}

} // namespace IPAddressSorter
} // namespace Dnssd
} // namespace chip
