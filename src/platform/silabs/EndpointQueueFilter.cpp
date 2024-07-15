#include "EndpointQueueFilter.h"
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <string.h>

namespace chip {
namespace Inet {

using FilterOutcome     = EndpointQueueFilter::FilterOutcome;
using namespace chip::Logging;

namespace {

bool IsValidMdnsHostName(const chip::CharSpan & hostName)
{
    for (size_t i = 0; i < hostName.size(); ++i)
    {
        char ch_data = *(hostName.data() + i);
        if (!((ch_data >= '0' && ch_data <= '9') || (ch_data >= 'A' && ch_data <= 'F')))
        {
            return false;
        }
    }
    return true;
}

bool PayloadContains(const chip::System::PacketBufferHandle & payload, const chip::ByteSpan & byteSpan)
{
    if (payload->HasChainedBuffer() || payload->TotalLength() < byteSpan.size())
    {
        return false;
    }
    for (size_t i = 0; i <= payload->TotalLength() - byteSpan.size(); ++i)
    {
        if (memcmp(payload->Start() + i, byteSpan.data(), byteSpan.size()) == 0)
        {
            return true;
        }
    }
    return false;
}

bool PayloadContainsCaseInsensitive(const chip::System::PacketBufferHandle & payload, const ByteSpan & name)
{
    ReturnErrorCodeIf(name.size() < HostNameFilter::kHostNameLengthMax, false);

    uint8_t lower_case[HostNameFilter::kHostNameLengthMax];
    memcpy(lower_case, name.data(), name.size());
    for (size_t i = 0; i < sizeof(lower_case); ++i)
    {
        if (lower_case[i] <= 'F' && lower_case[i] >= 'A')
        {
            lower_case[i] = static_cast<uint8_t>('a' + lower_case[i] - 'A');
        }
    }
    return PayloadContains(payload, name) || PayloadContains(payload, ByteSpan(lower_case));
}

} // namespace


FilterOutcome MdnsBroadcastFilter::Filter(const void * endpoint, const IPPacketInfo & pktInfo,
                                    const chip::System::PacketBufferHandle & pktPayload)
{
    if (pktInfo.DestPort == kMdnsPort)
    {
#if INET_CONFIG_ENABLE_IPV4
        ip_addr_t mdnsIPv4BroadcastAddr = IPADDR4_INIT_BYTES(224, 0, 0, 251);
        if (pktInfo.DestAddress == chip::Inet::IPAddress(mdnsIPv4BroadcastAddr))
        {
            return FilterOutcome::kAllowPacket;
        }
#endif
        ip_addr_t mdnsIPv6BroadcastAddr = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0xFB);
        if (pktInfo.DestAddress == chip::Inet::IPAddress(mdnsIPv6BroadcastAddr))
        {
            return FilterOutcome::kAllowPacket;
        }
    }
    return FilterOutcome::kDropPacket;
}


FilterOutcome HostNameFilter::Filter(const void * endpoint, const IPPacketInfo & pktInfo,
                                    const chip::System::PacketBufferHandle & pktPayload)
{
    // Drop the mDNS packets which don't contain 'matter' or '<device-hostname>'.
    const uint8_t matterBytes[] = { 'm', 'a', 't', 't', 'e', 'r' };
    if (PayloadContains(pktPayload, ByteSpan(matterBytes)) || PayloadContainsCaseInsensitive(pktPayload, ByteSpan(mHostName)))
    {
        return FilterOutcome::kAllowPacket;
    }
    return FilterOutcome::kDropPacket;
}


CHIP_ERROR HostNameFilter::SetHostName(const chip::CharSpan & name)
{
    ReturnErrorCodeIf(name.size() != sizeof(mHostName), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(!IsValidMdnsHostName(name), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(mHostName, name.data(), name.size());
    return CHIP_NO_ERROR;
}


CHIP_ERROR HostNameFilter::SetMacAddr(const chip::ByteSpan & mac_addr)
{
    ReturnErrorCodeIf(mac_addr.size() < 6, CHIP_ERROR_INVALID_ARGUMENT);
    const uint8_t *p = mac_addr.data();
    snprintf((char*)mHostName, sizeof(mHostName), "%02x%02x%02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
    return CHIP_NO_ERROR;
}


namespace Silabs {

EndpointQueueFilter::EndpointQueueFilter() :
    mTooManyFilter(kDefaultAllowedQueuedPackets) {}

EndpointQueueFilter::EndpointQueueFilter(size_t maxAllowedQueuedPackets) :
    mTooManyFilter(maxAllowedQueuedPackets) {}


FilterOutcome EndpointQueueFilter::FilterBeforeEnqueue(const void * endpoint, const IPPacketInfo & pktInfo,
                                  const chip::System::PacketBufferHandle & pktPayload)
{
    ReturnErrorCodeIf(FilterOutcome::kDropPacket == mTooManyFilter.FilterBeforeEnqueue(endpoint, pktInfo, pktPayload), FilterOutcome::kDropPacket);
    ReturnErrorCodeIf(FilterOutcome::kDropPacket == mMdnsFilter.Filter(endpoint, pktInfo, pktPayload), FilterOutcome::kDropPacket);
    ReturnErrorCodeIf(FilterOutcome::kDropPacket == mHostNameFilter.Filter(endpoint, pktInfo, pktPayload), FilterOutcome::kDropPacket);
    return FilterOutcome::kAllowPacket;
}


FilterOutcome EndpointQueueFilter::FilterAfterDequeue(const void * endpoint, const IPPacketInfo & pktInfo,
                                          const chip::System::PacketBufferHandle & pktPayload)
{
    ReturnErrorCodeIf(FilterOutcome::kDropPacket == mTooManyFilter.FilterAfterDequeue(endpoint, pktInfo, pktPayload), FilterOutcome::kDropPacket);
    return FilterOutcome::kAllowPacket;
}

} // Silabs
} // Inet
} // chip
