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

/// Returns true if the packet described by `info` came from the local link of the
/// interface it was received on, as RFC 6762 section 11 requires for mDNS.
///
/// A packet is accepted when any of these hold:
///   - it was sent to a multicast group (link-scoped mDNS groups are never routed)
///   - its source is an IPv6 link-local address
///   - its source is inside a subnet configured on `info.Interface`
///     (on any interface if `info.Interface` is not set)
///
/// `AddressIterator` must provide the methods of chip::Inet::InterfaceAddressIterator
/// that are used below. It is only advanced when the subnet check is needed.
template <typename AddressIterator>
bool IsFromAttachedSubnet(const chip::Inet::IPPacketInfo & info, AddressIterator & addresses)
{
    if (info.DestAddress.IsMulticast() || info.SrcAddress.IsIPv6LinkLocal())
    {
        return true;
    }

    for (; addresses.HasCurrent(); addresses.Next())
    {
        if (info.Interface.IsPresent() && (addresses.GetInterfaceId() != info.Interface))
        {
            continue;
        }

        chip::Inet::IPPrefix prefix;
        if ((addresses.GetAddress(prefix.IPAddr) != CHIP_NO_ERROR) || (prefix.IPAddr.Type() != info.SrcAddress.Type()))
        {
            continue;
        }

        prefix.Length = addresses.GetPrefixLength();
        if (prefix.Length == 0)
        {
            // A zero-length prefix would match every address.
            continue;
        }

#if INET_CONFIG_ENABLE_IPV4
        if (prefix.IPAddr.IsIPv4())
        {
            // IPAddress stores IPv4 as IPv4-mapped IPv6 (::ffff:a.b.c.d), so the
            // IPv4 prefix starts after the first 96 bits.
            prefix.Length = static_cast<uint8_t>(prefix.Length + 96u);
        }
#endif // INET_CONFIG_ENABLE_IPV4

        if (prefix.MatchAddress(info.SrcAddress))
        {
            return true;
        }
    }

    return false;
}

} // namespace Minimal
} // namespace mdns
