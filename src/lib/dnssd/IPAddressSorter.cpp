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

#include <utility>

#include <lib/support/SortUtils.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace Dnssd {
namespace IPAddressSorter {

namespace {

IpScore DefaultScore(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId)
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

IPAddressScorer * sScorer = nullptr;

} // namespace

uint8_t IPAddressScorer::ScoreIPAddress(const Inet::IPAddress & address, Inet::InterfaceId interfaceId)
{
    return to_underlying(DefaultScore(address, interfaceId));
}

IPAddressScorer * SetIPAddressScorer(IPAddressScorer * scorer)
{
    return std::exchange(sScorer, scorer);
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
    return sScorer != nullptr ? sScorer->ScoreIPAddress(ip, interfaceId) : to_underlying(DefaultScore(ip, interfaceId));
}

} // namespace IPAddressSorter
} // namespace Dnssd
} // namespace chip
