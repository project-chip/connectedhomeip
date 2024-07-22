#include "EndpointQueueFilter.h"
#include <string.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <string_view>

namespace chip {
namespace Inet {

using FilterOutcome = EndpointQueueFilter::FilterOutcome;

namespace {

bool IsValidMdnsHostName(uint8_t * hostName)
{
    for (size_t i = 0; i < sizeof(hostName); ++i)
    {
        char ch_data = hostName[i];
        if (!((ch_data >= '0' && ch_data <= '9') || (ch_data >= 'A' && ch_data <= 'F')))
        {
            return false;
        }
    }
    return true;
}

bool IsMdnsBroadcastPacket(const void * endpoint, const IPPacketInfo & pktInfo, const System::PacketBufferHandle & pktPayload)
{
    // if the packet is not a broadcast packet to mDNS port, drop it.
    VerifyOrReturnValue(pktInfo.DestPort == 5353, false);
#if INET_CONFIG_ENABLE_IPV4
    ip_addr_t mdnsIPv4BroadcastAddr = IPADDR4_INIT_BYTES(224, 0, 0, 251);
    VerifyOrReturnValue(pktInfo.DestAddress == Inet::IPAddress(mdnsIPv4BroadcastAddr), false);
#endif
    ip_addr_t mdnsIPv6BroadcastAddr = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0xFB);
    VerifyOrReturnValue(pktInfo.DestAddress == Inet::IPAddress(mdnsIPv6BroadcastAddr), false);
    return true;
}

bool PayloadContainsCaseInsensitive(const System::PacketBufferHandle & payload, const ByteSpan & pattern)
{
    if (payload->TotalLength() == 0 || pattern.size() == 0)
    {
        return false;
    }

    if (payload->HasChainedBuffer() || payload->TotalLength() < pattern.size())
    {
        return false;
    }

    std::basic_string_view<uint8_t> payloadView(payload->Start(), payload->TotalLength());
    std::basic_string_view<uint8_t> patternView(pattern.data(), pattern.size());
    return payloadView.find(patternView) != std::basic_string_view<uint8_t>::npos;
}

} // namespace

FilterOutcome HostNameFilter::Filter(const void * endpoint, const IPPacketInfo & pktInfo,
                                     const System::PacketBufferHandle & pktPayload)
{
    // Drop the mDNS packets which don't contain 'matter' or '<device-hostname>'.
    const uint8_t matterBytes[] = { 'm', 'a', 't', 't', 'e', 'r' };
    if (PayloadContainsCaseInsensitive(pktPayload, ByteSpan(matterBytes)) ||
        PayloadContainsCaseInsensitive(pktPayload, ByteSpan(mHostName)))
    {
        return FilterOutcome::kAllowPacket;
    }

    return FilterOutcome::kDropPacket;
}

CHIP_ERROR HostNameFilter::SetHostName(const ByteSpan & hostName)
{
    const uint8_t * p = hostName.data();
    snprintf((char *) mHostName, sizeof(mHostName), "%02X%02X%02X%02X%02X%02X", p[0], p[1], p[2], p[3], p[4], p[5]);
    ReturnErrorCodeIf(!IsValidMdnsHostName(mHostName), CHIP_ERROR_INVALID_ARGUMENT);
    return CHIP_NO_ERROR;
}

namespace SilabsEndpointQueueFilter {

EndpointQueueFilter::EndpointQueueFilter() : mTooManyFilter(kDefaultAllowedQueuedPackets) {}

EndpointQueueFilter::EndpointQueueFilter(size_t maxAllowedQueuedPackets) : mTooManyFilter(maxAllowedQueuedPackets) {}

FilterOutcome EndpointQueueFilter::FilterBeforeEnqueue(const void * endpoint, const IPPacketInfo & pktInfo,
                                                       const System::PacketBufferHandle & pktPayload)
{
    VerifyOrReturnError(FilterOutcome::kAllowPacket == mTooManyFilter.FilterBeforeEnqueue(endpoint, pktInfo, pktPayload),
                        FilterOutcome::kDropPacket);

    if (!IsMdnsBroadcastPacket(endpoint, pktInfo, pktPayload))
    {
        return FilterOutcome::kAllowPacket;
    }
    return mHostNameFilter.Filter(endpoint, pktInfo, pktPayload);
}

FilterOutcome EndpointQueueFilter::FilterAfterDequeue(const void * endpoint, const IPPacketInfo & pktInfo,
                                                      const System::PacketBufferHandle & pktPayload)
{
    return mTooManyFilter.FilterAfterDequeue(endpoint, pktInfo, pktPayload);
}

} // namespace SilabsEndpointQueueFilter
} // namespace Inet
} // namespace chip
