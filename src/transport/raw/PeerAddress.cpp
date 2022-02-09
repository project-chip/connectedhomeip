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
///    NONE%eth0:5040                // addresses are empty but interface is set to something
///    10.20.30.40:5040              // exactly one IP address in the address list, interface is empty
///    10.0.0.10%wlan0:123 (+3 more) // 4 input addresses in the list
/// <ip>%<iface>:port
void AddIpInfo(StringBuilderBase & dest, const char * interface, uint16_t port, Span<const Inet::IPAddress> addresses)
{
    if (addresses.empty())
    {
        dest.Add("NONE:");
    }
    else
    {
        // First address information.
        //
        // In time, peer addresses should have an 'active address' to determine  which IP is
        // used to communicate with peers, in which case the output here should be the current
        // active address rather than just the first in the list.
        dest.Add(":");
        char ip_addr[Inet::IPAddress::kMaxStringLength];
        auto first = addresses.begin();
        first->ToString(ip_addr);
        if (first->IsIPv6())
        {
            dest.Add("[");
        }
        dest.Add(ip_addr);
        dest.Add(interface);
        if (first->IsIPv6())
        {
            dest.Add("]");
        }
        dest.Add(":");
        dest.Add(port);
    }

    if (addresses.size() > 1)
    {
        dest.Add(" (+");
        dest.Add(static_cast<int>(addresses.size() - 1));
        dest.Add(" more)");
    }
}

} // namespace

void PeerAddress::ToString(char * buf, size_t bufSize) const
{

    char interface[Inet::InterfaceId::kMaxIfNameLength + 1] = {}; // +1 to prepend '%'
    if (mInterface.IsPresent())
    {
        interface[0]   = '%';
        interface[1]   = 0;
        CHIP_ERROR err = mInterface.GetInterfaceName(interface + 1, sizeof(interface) - 1);
        if (err != CHIP_NO_ERROR)
        {
            Platform::CopyString(interface, sizeof(interface), "%(err)");
        }
    }

    StringBuilderBase out(buf, bufSize);

    switch (mTransportType)
    {
    case Type::kUndefined:
        out.Add("UNDEFINED");
        break;
    case Type::kUdp:
        out.Add("UDP:");
        AddIpInfo(out, interface, mPort, Span<const Inet::IPAddress>(mIPAddresses, mNumValidIPAddresses));
        break;
    case Type::kTcp:
        out.Add("TCP:");
        AddIpInfo(out, interface, mPort, Span<const Inet::IPAddress>(mIPAddresses, mNumValidIPAddresses));
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
