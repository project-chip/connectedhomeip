/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <lib/dnssd/minimal_mdns/ListenIterator.h>
#include <lib/dnssd/minimal_mdns/ServerIPAddresses.h>
#include <lib/support/CHIPMem.h>

namespace mdns {
namespace Minimal {

/// Describes a policy of endpoints and IP addresses to use for
/// MinMDNS listening and advertisement.
class AddressPolicy
{
public:
    virtual ~AddressPolicy() = default;

    /// Get all endpoints on which minimal MDNS should listen on.
    virtual chip::Platform::UniquePtr<ListenIterator> GetListenEndpoints() = 0;

    /// Fetch all the IP addresses for the given interface which are valid
    /// for DNSSD server advertisement.
    ///
    /// Generally this should skip invalid addresses even if reported by the
    /// underlying operating system (e.g. linux could skip deprecated/temporary/dad-failed ones).
    virtual chip::Platform::UniquePtr<IpAddressIterator> GetIpAddressesForEndpoint(chip::Inet::InterfaceId interfaceId,
                                                                                   chip::Inet::IPAddressType type) = 0;
};

/// Fetch the globally used MinMDNS address policy
AddressPolicy * GetAddressPolicy();

/// Update the global address policy.
///
/// MUST be called before any minmdns functionality is used (e.g. server
/// startup)
void SetAddressPolicy(AddressPolicy * policy);

} // namespace Minimal
} // namespace mdns
