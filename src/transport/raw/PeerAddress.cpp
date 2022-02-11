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

#include "PeerAddress.h"

#include <lib/support/StringBuilder.h>

namespace chip {
namespace Transport {

namespace {

/// Formats IP addresses into a format suitable for PeerAddress::ToString
/// Specifically it will add the first IP address, interface used and port
/// and a marker if more than one IP address is contained in the input list
///
/// Examples:
///    NONE:123                      // when addresses are empty
///    10.20.30.40:5040              // exactly one IP address in the destination list with null interface
///    10.0.0.10%wlan0:123 (+3 more) // 4 input addresses in the list
/// <ip>%<iface>:port
void AddIpInfo(StringBuilderBase & dest, uint16_t port, Span<const PeerAddress::IPDestination> destinations)
{
    if (destinations.empty())
    {
        dest.Add(":NONE:");
    }
    else
    {
        // First address information.
        //
        // In time, peer addresses should have an 'active address' to determine  which IP is
        // used to communicate with peers, in which case the output here should be the current
        // active address rather than just the first in the list.

        dest.Add(":");
        auto first = destinations.begin();

        {
            char ip_addr[Inet::IPAddress::kMaxStringLength];
            first->ipAddress.ToString(ip_addr);
            if (first->ipAddress.IsIPv6())
            {
                dest.Add("[");
            }
            dest.Add(ip_addr);
        }

        if (first->interface.IsPresent())
        {
            dest.Add("%");

            char interface[Inet::InterfaceId::kMaxIfNameLength] = {}; // +1 to prepend '%'

            if (first->interface.GetInterfaceName(interface, sizeof(interface)) != CHIP_NO_ERROR)
            {
                dest.Add("(err)");
            }
            else
            {
                dest.Add(interface);
            }
        }
        if (first->ipAddress.IsIPv6())
        {
            dest.Add("]");
        }
        dest.Add(":");
    }
    dest.Add(port);

    if (destinations.size() > 1)
    {
        dest.Add(" (+");
        dest.Add(static_cast<int>(destinations.size() - 1));
        dest.Add(" more)");
    }
}

} // namespace

void PeerAddress::ToString(char * buf, size_t bufSize) const
{
    StringBuilderBase out(buf, bufSize);

    switch (mTransportType)
    {
    case Type::kUndefined:
        out.Add("UNDEFINED");
        break;
    case Type::kUdp:
        out.Add("UDP");
        AddIpInfo(out, mPort, Span<const IPDestination>(mDestinations, mNumValidDestinations));
        break;
    case Type::kTcp:
        out.Add("TCP");
        AddIpInfo(out, mPort, Span<const IPDestination>(mDestinations, mNumValidDestinations));
        break;
    case Type::kBle:
        // Note that BLE does not currently use any specific address.
        out.Add("BLE");
        break;
    default:
        out.Add("ERROR");
        break;
    }
}

} // namespace Transport
} // namespace chip
