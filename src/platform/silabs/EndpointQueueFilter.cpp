#include "EndpointQueueFilter.h"
#include <algorithm>
#include <cctype>
#include <lib/core/CHIPSafeCasts.h>
#include <string.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

using namespace ::chip;

namespace chip {
namespace Inet {

using FilterOutcome = EndpointQueueFilter::FilterOutcome;

namespace {

bool IsValidMdnsHostName(const Span<const unsigned char> & hostName)
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
    if (pktInfo.DestAddress == Inet::IPAddress(mdnsIPv4BroadcastAddr))
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

bool PayloadContainsCaseInsensitive(const System::PacketBufferHandle & payload, const Span<const unsigned char> & pattern)
{
    if (payload->TotalLength() == 0 || pattern.size() == 0)
    {
        return false;
    }

    if (payload->HasChainedBuffer() || payload->TotalLength() < pattern.size())
    {
        return false;
    }

    Span<const unsigned char> payloadView(payload->Start(), payload->TotalLength());

    auto toLower = [](unsigned char c) { return std::tolower(c); };

    auto it = std::search(payloadView.begin(), payloadView.end(), pattern.begin(), pattern.end(),
                          [&](unsigned char a, unsigned char b) { return toLower(a) == toLower(b); });

    return (it != payloadView.end());
}

} // namespace

FilterOutcome HostNameFilter::Filter(const void * endpoint, const IPPacketInfo & pktInfo,
                                     const System::PacketBufferHandle & pktPayload)
{
    // Drop the mDNS packets which don't contain 'matter' or '<device-hostname>'.
    const unsigned char matterBytes[] = { 'm', 'a', 't', 't', 'e', 'r' };
    if (PayloadContainsCaseInsensitive(pktPayload, Span<const unsigned char>(matterBytes)) ||
        PayloadContainsCaseInsensitive(pktPayload, Span<const unsigned char>(mHostName)))
    {
        return FilterOutcome::kAllowPacket;
    }
    return FilterOutcome::kDropPacket;
}

CHIP_ERROR HostNameFilter::SetHostName(Span<const unsigned char> & hostName)
{
    VerifyOrReturnError(IsValidMdnsHostName(hostName), CHIP_ERROR_INVALID_ARGUMENT);
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
