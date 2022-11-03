/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "IP.h"

#include <assert.h>

#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/records/IP.h>

namespace mdns {
namespace Minimal {

using chip::Platform::UniquePtr;

void IPv4Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                                    const ResponseConfiguration & configuration)
{
#if INET_CONFIG_ENABLE_IPV4
    chip::Inet::IPAddress addr;
    UniquePtr<IpAddressIterator> ips =
        GetAddressPolicy()->GetIpAddressesForEndpoint(source->Interface, chip::Inet::IPAddressType::kIPv4);
    VerifyOrDie(ips);

    while (ips->Next(addr))
    {
        assert(addr.IsIPv4());

        IPResourceRecord record(GetQName(), addr);
        configuration.Adjust(record);
        delegate->AddResponse(record);
    }
#endif
}

void IPv6Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                                    const ResponseConfiguration & configuration)
{
    chip::Inet::IPAddress addr;
    UniquePtr<IpAddressIterator> ips =
        GetAddressPolicy()->GetIpAddressesForEndpoint(source->Interface, chip::Inet::IPAddressType::kIPv6);
    VerifyOrDie(ips);

    while (ips->Next(addr))
    {
        assert(addr.IsIPv6());

        IPResourceRecord record(GetQName(), addr);
        configuration.Adjust(record);
        delegate->AddResponse(record);
    }
}

} // namespace Minimal
} // namespace mdns
