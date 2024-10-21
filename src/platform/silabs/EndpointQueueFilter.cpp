#include "EndpointQueueFilter.h"
#include <string.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <algorithm>
#include <cctype>
namespace chip {
namespace Inet {

using FilterOutcome = EndpointQueueFilter::FilterOutcome;

namespace {

bool IsValidMdnsHostName(const CharSpan & hostName)
{
    for (size_t i = 0; i < hostName.size(); ++i)
    {
        char ch_data = *(hostName.data());
        if (!((ch_data >= '0' && ch_data <= '9') || (ch_data >= 'A' && ch_data <= 'F') || (ch_data >= 'a' && ch_data <= 'f')))
        {
            return false;
        }
    }
    return true;
}

bool IsMdnsBroadcastPacket(const IPPacketInfo & pktInfo, const System::PacketBufferHandle & pktPayload)
{
    // if the packet is not a broadcast packet to mDNS port, drop it.
    VerifyOrReturnValue(pktInfo.DestPort == 5353, false);
#if INET_CONFIG_ENABLE_IPV4
    ip_addr_t mdnsIPv4BroadcastAddr = IPADDR4_INIT_BYTES(224, 0, 0, 251);
    if (pktInfo.DestAddress == Inet::IPAddress(mdnsIPv4BroadcastAddr)
    {
        return true;
    }
#endif
    ip_addr_t mdnsIPv6BroadcastAddr = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0xFB);
    if (pktInfo.DestAddress == Inet::IPAddress(mdnsIPv6BroadcastAddr))
    {
        return true;
    }
    return false;
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

    CharSpan payloadView(reinterpret_cast<const char *>(payload->Start()), payload->TotalLength());
    CharSpan patternView(reinterpret_cast<const char *>(pattern.data()), pattern.size());

    auto toLower = [](char c) { return std::tolower(static_cast<unsigned char>(c)); };

    auto it = std::search(
        payloadView.data(), payloadView.data() + payloadView.size(),
        patternView.data(), patternView.data() + patternView.size(),
        [&](char a, char b) { return toLower(a) == toLower(b); });

    return (it != payloadView.data() + payloadView.size());



    // CharSpan payloadView((const char *)payload->Start(), payload->TotalLength());
    // CharSpan patternView((const char *)pattern.data(), pattern.size());
    // std::basic_string_view<uint8_t> payloadView(payload->Start(), payload->TotalLength());
    // std::basic_string_view<uint8_t> patternView(pattern.data(), pattern.size());
    // return (payloadView.data()).find(patternView.data()) != std::basic_string_view<uint8_t>::npos;
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

CHIP_ERROR HostNameFilter::SetHostName(const CharSpan & hostName)
{
    ReturnErrorCodeIf(!IsValidMdnsHostName(hostName), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(mHostName, hostName.data(), hostName.size());
    return CHIP_NO_ERROR;
}

namespace SilabsEndpointQueueFilter {

EndpointQueueFilter::EndpointQueueFilter() : mTooManyFilter(mConfig.allowedQueuedPackets) {}

EndpointQueueFilter::EndpointQueueFilter(size_t maxAllowedQueuedPackets) : mTooManyFilter(maxAllowedQueuedPackets) {}

FilterOutcome EndpointQueueFilter::FilterBeforeEnqueue(const void * endpoint, const IPPacketInfo & pktInfo,
                                                       const System::PacketBufferHandle & pktPayload)
{
    VerifyOrReturnError(FilterOutcome::kAllowPacket == mTooManyFilter.FilterBeforeEnqueue(endpoint, pktInfo, pktPayload),
                        FilterOutcome::kDropPacket);

    if (!IsMdnsBroadcastPacket(pktInfo, pktPayload))
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
