/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "IP.h"

#include <lib/dnssd/minimal_mdns/AddressPolicy.h>
#include <lib/dnssd/minimal_mdns/records/IP.h>

namespace mdns {
namespace Minimal {

using chip::Platform::UniquePtr;

void IPv4Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                                    const ResponseConfiguration & configuration)
{
    chip::Inet::IPAddress addr;
    UniquePtr<IpAddressIterator> ips = Policies::GetIpAddressesForEndpoint(source->Interface);
    VerifyOrDie(ips);

    while (ips->Next(addr))
    {
        if (addr.IsIPv4())
        {
            IPResourceRecord record(GetQName(), addr);
            configuration.Adjust(record);
            delegate->AddResponse(record);
        }
    }
}

void IPv6Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate,
                                    const ResponseConfiguration & configuration)
{
    chip::Inet::IPAddress addr;
    UniquePtr<IpAddressIterator> ips = Policies::GetIpAddressesForEndpoint(source->Interface);

    VerifyOrDie(ips);

    while (ips->Next(addr))
    {
        if (addr.IsIPv6())
        {
            IPResourceRecord record(GetQName(), addr);
            configuration.Adjust(record);
            delegate->AddResponse(record);
        }
    }
}

} // namespace Minimal
} // namespace mdns
