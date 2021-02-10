/* See Project CHIP LICENSE file for licensing information. */

#include "IP.h"

#include <mdns/minimal/records/IP.h>

namespace mdns {
namespace Minimal {

void IPv4Responder::AddAllResponses(const chip::Inet::IPPacketInfo * source, ResponderDelegate * delegate)
{
    for (chip::Inet::InterfaceAddressIterator it; it.HasCurrent(); it.Next())
    {
        if (it.GetInterfaceId() != source->Interface)
        {
            continue;
        }

        chip::Inet::IPAddress addr = it.GetAddress();
        if (!addr.IsIPv4())
        {
            continue;
        }
        delegate->AddResponse(IPResourceRecord(GetQName(), addr));
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

        chip::Inet::IPAddress addr = it.GetAddress();
        if (!addr.IsIPv6())
        {
            continue;
        }

        delegate->AddResponse(IPResourceRecord(GetQName(), addr));
    }
}

} // namespace Minimal
} // namespace mdns
