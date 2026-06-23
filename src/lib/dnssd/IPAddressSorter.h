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

#include <inet/IPAddress.h>
#include <lib/support/Span.h>

namespace chip {
namespace Dnssd {
namespace IPAddressSorter {

// IP addess "suitability"
//   - Larger value means "more suitable"
//   - Enum ordered ascending for easier read. Note however that order of
//     checks MUST match in ScoreIpAddress below.
enum class IpScore : unsigned
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

void Sort(Inet::IPAddress * addresses, size_t count, Inet::InterfaceId interfaceId);

void Sort(const Span<Inet::IPAddress> & addresses, Inet::InterfaceId interfaceId);

/**
 * Gives a score for an IP address, generally related to "how good" the address
 * is and how likely it is for it to be reachable.
 */
IpScore ScoreIpAddress(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId);

// ---- Transport-tier-aware composite scoring -------------------------------
//
// ScoreIpAddressWithTransport() returns a uint32_t with this layout:
//
//   bits [31:24]  transport tier   (Ethernet > WiFi > Thread > Unknown)
//   bits [23:16]  Thread sub-metric (0 unless callback set; only nonzero on Thread)
//   bits [15:0]   address-class score (IpScore promoted, preserves today's ordering)
//
// Because the tier dominates the upper byte, addresses on a higher-tier interface always
// outrank addresses on a lower-tier one regardless of address class. Within a single tier
// the existing IpScore ordering is preserved exactly. Platforms whose InterfaceId cannot
// report a transport type degrade to tier=Unknown (=0) for all entries, which collapses
// the composite score back to the legacy IpScore ordering.
namespace CompositeScore {
constexpr uint32_t kTierShift          = 24;
constexpr uint32_t kThreadSubShift     = 16;
constexpr uint8_t  kTierEthernet       = 3;
constexpr uint8_t  kTierWiFi           = 2;
constexpr uint8_t  kTierThread         = 1;
constexpr uint8_t  kTierUnknown        = 0;

inline uint32_t Encode(uint8_t tier, uint8_t threadSub, uint16_t classScore)
{
    return (static_cast<uint32_t>(tier) << kTierShift) |
           (static_cast<uint32_t>(threadSub) << kThreadSubShift) |
           static_cast<uint32_t>(classScore);
}
} // namespace CompositeScore

uint32_t ScoreIpAddressWithTransport(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId);

// Optional Thread sub-metric callback. The callback is invoked only when the address being
// scored sits on a Thread interface, and its 8-bit return value is packed into bits [23:16]
// of the composite score. Higher values sort earlier within the Thread tier. Apps that don't
// register a callback get a default Thread sub-metric of 0 (i.e. address class alone decides
// intra-Thread ordering). Setting cb=nullptr clears any prior registration.
using ThreadSubMetricCallback = uint8_t (*)(const Inet::IPAddress & ip, Inet::InterfaceId interfaceId);
void SetThreadSubMetricCallback(ThreadSubMetricCallback cb);

} // namespace IPAddressSorter
} // namespace Dnssd
} // namespace chip
