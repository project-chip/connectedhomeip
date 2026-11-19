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
#pragma once

#include <cstdint>

#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace IPAddressSorter {

// IP address "suitability", as scored when no IPAddressScorer is installed
//   - Larger value means "more suitable"
//   - Enum ordered ascending for easier read. Note however that order of
//     checks MUST match in DefaultScore in IPAddressSorter.cpp.
enum class IpScore : uint8_t
{
    kInvalid = 0, // No address available

    // "Other" IPv6 include:
    //   - invalid addresses (have seen router bugs during interop testing)
    //   - embedded IPv4 (::/80)
    kOtherIpv6                     = 1,
    kIpv4                          = 2, // Not Matter SPEC, so low priority
    kUniqueLocal                   = 3, // ULA. Thread devices use this
    kGlobalUnicast                 = 4, // Maybe routable, not local subnet
    kUniqueLocalWithSharedPrefix   = 5, // Prefix seems to match a local interface
    kGlobalUnicastWithSharedPrefix = 6, // Prefix seems to match a local interface
    kLinkLocal                     = 7, // Valid only on an interface
};

/**
 * Scores IP addresses in place of the IpScore values. IPAddressSorter::ScoreIPAddress and Sort use the installed
 * scorer, so addresses found through DNS-SD with a higher score are tried first.
 */
class IPAddressScorer
{
public:
    virtual ~IPAddressScorer() = default;

    /**
     * Returns the score of `address`, higher being more suitable. `interfaceId` is the interface the address
     * was resolved on, and may be InterfaceId::Null().
     *
     * This implementation returns the IpScore value; an override can call it to use the default score for
     * some addresses.
     */
    virtual uint8_t ScoreIPAddress(const Inet::IPAddress & address, Inet::InterfaceId interfaceId);
};

/**
 * Installs `scorer`, or restores the IpScore values when `scorer` is null, and returns the scorer it replaces
 * (null if none was installed) so that the caller can restore it later. `scorer` must remain valid until it is
 * replaced. Call this before the Matter stack starts or with the Matter stack lock held, because scoring reads
 * the installed scorer without synchronization.
 */
IPAddressScorer * SetIPAddressScorer(IPAddressScorer * scorer);

void Sort(Inet::IPAddress * addresses, size_t count, Inet::InterfaceId interfaceId);

void Sort(const Span<Inet::IPAddress> & addresses, Inet::InterfaceId interfaceId);

/**
 * Gives a score for an IP address, generally related to "how good" the address
 * is and how likely it is for it to be reachable. Higher is better. Uses the
 * installed IPAddressScorer if there is one, and the IpScore values otherwise.
 */
uint8_t ScoreIPAddress(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId);

} // namespace IPAddressSorter
} // namespace Dnssd
} // namespace chip
