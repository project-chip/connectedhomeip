/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <inet/IPPacketInfo.h>
#include <inet/IPPrefix.h>
#include <inet/InetConfig.h>
#include <lib/core/CHIPError.h>

namespace mdns {
namespace Minimal {

// RFC 6762 sections 5.5 and 11 recommend that mDNS only accept packets from the
// local link of the interface they were received on. A packet is from the local
// link if IsLinkScoped() or IsFromAttachedSubnet() returns true for it.
// IsLinkScoped() needs no interface state, so callers check it first.

/// Returns true if the addresses of the packet described by `info` show that it
/// came from the local link: it was sent to `mdnsGroup`, which routers do not
/// forward, or its source is an IPv6 link-local address.
///
/// `mdnsGroup` is the mDNS multicast group (224.0.0.251 or ff02::fb) of the
/// address family of `info.DestAddress`. An unspecified `info.DestAddress`
/// (destination not reported) does not match it.
inline bool IsLinkScoped(const chip::Inet::IPPacketInfo & info, const chip::Inet::IPAddress & mdnsGroup)
{
    return (info.DestAddress == mdnsGroup) || info.SrcAddress.IsIPv6LinkLocal();
}

/// Returns true if the source of the packet described by `info` is inside a
/// subnet that `addresses` reports for `info.Interface` (for any interface if
/// `info.Interface` is not set).
///
/// Also returns true if `addresses` reports no subnet for the source's address
/// family on that interface, because the source cannot be checked. A prefix
/// length of zero or of the full address width does not describe a subnet.
///
/// `AddressIterator` must provide the methods of chip::Inet::InterfaceAddressIterator
/// that are used below.
template <typename AddressIterator>
bool IsFromAttachedSubnet(const chip::Inet::IPPacketInfo & info, AddressIterator & addresses)
{
    bool hasSubnetToCheck = false;
    for (; addresses.HasCurrent(); addresses.Next())
    {
        chip::Inet::IPPrefix prefix;
        if ((addresses.GetAddress(prefix.IPAddr) != CHIP_NO_ERROR) || (prefix.IPAddr.Type() != info.SrcAddress.Type()))
        {
            continue;
        }

        const uint8_t prefixLength = addresses.GetPrefixLength();
        uint8_t addressBits        = 128;
#if INET_CONFIG_ENABLE_IPV4
        if (prefix.IPAddr.IsIPv4())
        {
            addressBits = 32;
        }
#endif // INET_CONFIG_ENABLE_IPV4

        if ((prefixLength == 0) || (prefixLength >= addressBits))
        {
            continue;
        }

        // Checked last because the interface lookup is the slowest step on some platforms.
        if (info.Interface.IsPresent() && (addresses.GetInterfaceId() != info.Interface))
        {
            continue;
        }
        hasSubnetToCheck = true;

        // IPAddress stores IPv4 as IPv4-mapped IPv6 (::ffff:a.b.c.d), so an IPv4
        // prefix starts after the first 96 bits.
        prefix.Length = static_cast<uint8_t>(prefixLength + (128u - addressBits));

        if (prefix.MatchAddress(info.SrcAddress))
        {
            // The source is inside a subnet of the interface.
            return true;
        }
    }

    if (hasSubnetToCheck)
    {
        // The source is outside every subnet of the interface.
        return false;
    }

    // No subnet was reported for the source's address family on the interface,
    // so the source cannot be checked. Zephyr reports no IPv4 addresses, and /128
    // for IPv6 addresses without a SLAAC prefix; rejecting would drop its mDNS traffic.
    return true;
}

} // namespace Minimal
} // namespace mdns
