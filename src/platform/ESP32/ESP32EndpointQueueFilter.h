/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <inet/EndpointQueueFilter.h>
#include <lwip/ip4_addr.h>

namespace chip {
namespace Inet {

class ESP32EndpointQueueFilter : public EndpointQueueFilter
{
public:
    CHIP_ERROR SetMdnsHostName(const chip::CharSpan & hostName)
    {
        ReturnErrorCodeIf(hostName.size() != sizeof(mHostNameBuffer), CHIP_ERROR_INVALID_ARGUMENT);
        ReturnErrorCodeIf(!IsValidMdnsHostName(hostName), CHIP_ERROR_INVALID_ARGUMENT);
        memcpy(mHostNameBuffer, hostName.data(), hostName.size());
        return CHIP_NO_ERROR;
    }

    FilterOutcome FilterBeforeEnqueue(const void * endpoint, const IPPacketInfo & pktInfo,
                                      const chip::System::PacketBufferHandle & pktPayload) override
    {
        if (IsMdnsBroadcastPacket(pktInfo))
        {
            // Drop the mDNS packets which don't contains 'matter' or '<device-hostname>'.
            const uint8_t matterBytes[] = { 0x6D, 0x61, 0x74, 0x74, 0x65, 0x72 }; // 'm' 'a' 't' 't' 'e' 'r'
            if (PayloadContains(pktPayload, ByteSpan(matterBytes)) || PayloadContains(pktPayload, ByteSpan(mHostNameBuffer)))
            {
                return FilterOutcome::kAllowPacket;
            }
            return FilterOutcome::kDropPacket;
        }
        return FilterOutcome::kAllowPacket;
    }

    FilterOutcome FilterAfterDequeue(const void * endpoint, const IPPacketInfo & pktInfo,
                                     const chip::System::PacketBufferHandle & pktPayload) override
    {
        return FilterOutcome::kAllowPacket;
    }

private:
    bool IsMdnsBroadcastPacket(const IPPacketInfo & pktInfo)
    {
        if (pktInfo.DestAddress.IsIPv4() && pktInfo.DestPort == 5353)
        {
            ip4_addr_t mdnsBroadcastAddr4;
            ip4addr_aton("224.0.0.251", &mdnsBroadcastAddr4);
            return pktInfo.DestAddress.ToIPv4().addr == mdnsBroadcastAddr4.addr;
        }
        if (pktInfo.DestAddress.IsIPv6() && pktInfo.DestPort == 5353)
        {
            ip6_addr_t mdnsBroadcastAddr6;
            ip6addr_aton("ff02::fb", &mdnsBroadcastAddr6);
            ip6_addr_t dstAddr6 = pktInfo.DestAddress.ToIPv6();
            return ip6_addr_cmp(&mdnsBroadcastAddr6, &dstAddr6);
        }
        return false;
    }

    bool PayloadContains(const chip::System::PacketBufferHandle & payload, const chip::ByteSpan & byteSpan)
    {
        if (payload->HasChainedBuffer())
        {
            return false;
        }
        for (size_t i = 0; i < payload->TotalLength() - byteSpan.size(); ++i)
        {
            if (memcmp(payload->Start() + i, byteSpan.data(), byteSpan.size()) == 0)
            {
                return true;
            }
        }
        return false;
    }

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

    uint8_t mHostNameBuffer[12] = { 0 };
};

} // namespace Inet
} // namespace chip
