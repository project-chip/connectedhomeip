/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/dnssd/IPAddressSorter.h>

#include <lib/support/SortUtils.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace Dnssd {
namespace IPAddressSorter {

namespace {

uint8_t DefaultScore(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId)
{
    if (ip.IsIPv6())
    {
        if (ip.IsIPv6LinkLocal())
        {
            return to_underlying(IpScore::kLinkLocal);
        }

        if (interfaceId.MatchLocalIPv6Subnet(ip))
        {
            if (ip.IsIPv6GlobalUnicast())
            {
                return to_underlying(IpScore::kGlobalUnicastWithSharedPrefix);
            }
            if (ip.IsIPv6ULA())
            {
                return to_underlying(IpScore::kUniqueLocalWithSharedPrefix);
            }
        }
        if (ip.IsIPv6GlobalUnicast())
        {
            return to_underlying(IpScore::kGlobalUnicast);
        }

        if (ip.IsIPv6ULA())
        {
            return to_underlying(IpScore::kUniqueLocal);
        }

        return to_underlying(IpScore::kOtherIpv6);
    }
    return to_underlying(IpScore::kIpv4);
}

IPAddressScorer * sScorer = nullptr;

} // namespace

uint8_t IPAddressScorer::ScoreIPAddress(const Inet::IPAddress & address, Inet::InterfaceId interfaceId)
{
    return DefaultScore(address, interfaceId);
}

void SetIPAddressScorer(IPAddressScorer * scorer)
{
    sScorer = scorer;
}

void Sort(Inet::IPAddress * addresses, size_t count, Inet::InterfaceId interfaceId)
{
    Sorting::InsertionSort(addresses, count, [interfaceId](const Inet::IPAddress & a, const Inet::IPAddress & b) -> bool {
        return ScoreIPAddress(a, interfaceId) > ScoreIPAddress(b, interfaceId);
    });
}

void Sort(const Span<Inet::IPAddress> & addresses, Inet::InterfaceId interfaceId)
{
    Sort(addresses.data(), addresses.size(), interfaceId);
}

uint8_t ScoreIPAddress(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId)
{
    return sScorer != nullptr ? sScorer->ScoreIPAddress(ip, interfaceId) : DefaultScore(ip, interfaceId);
}

} // namespace IPAddressSorter
} // namespace Dnssd
} // namespace chip
