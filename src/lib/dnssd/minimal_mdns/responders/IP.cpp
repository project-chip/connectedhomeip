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

#include <lib/dnssd/minimal_mdns/records/IP.h>

namespace mdns {
namespace Minimal {

void IPv4Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate)
{
    chip::Inet::IPAddress addr;
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        if ((it.GetInterfaceId() == source->Interface) && (it.GetAddress(addr) == CHIP_NO_ERROR) && addr.IsIPv4())
        {
            delegate->AddResponse(IPResourceRecord(GetQName(), addr));
        }
    }
}

void IPv6Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate)
{
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        if (it.GetInterfaceId() != source->Interface)
        {
            continue;
        }

        chip::Inet::IPAddress addr;
        if ((it.GetInterfaceId() == source->Interface) && (it.GetAddress(addr) == CHIP_NO_ERROR) && addr.IsIPv6())
        {
            delegate->AddResponse(IPResourceRecord(GetQName(), addr));
        }
    }
}

} // namespace Minimal
} // namespace mdns
