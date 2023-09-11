/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

} // namespace IPAddressSorter
} // namespace Dnssd
} // namespace chip
