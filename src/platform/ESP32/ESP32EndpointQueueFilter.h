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
#include <inet/IPAddress.h>
#include <lwip/ip_addr.h>

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
        if (!IsMdnsBroadcastPacket(pktInfo))
        {
            return FilterOutcome::kAllowPacket;
        }
        // Drop the mDNS packets which don't contain 'matter' or '<device-hostname>'.
        const uint8_t matterBytes[] = { 'm', 'a', 't', 't', 'e', 'r' };
        if (PayloadContains(pktPayload, ByteSpan(matterBytes)) || PayloadContainsHostNameCaseInsensitive(pktPayload))
        {
            return FilterOutcome::kAllowPacket;
        }
        return FilterOutcome::kDropPacket;
    }

    FilterOutcome FilterAfterDequeue(const void * endpoint, const IPPacketInfo & pktInfo,
                                     const chip::System::PacketBufferHandle & pktPayload) override
    {
        return FilterOutcome::kAllowPacket;
    }

private:
    // TODO: Add unit tests for these static functions
    static bool IsMdnsBroadcastPacket(const IPPacketInfo & pktInfo)
    {
        if (pktInfo.DestPort == 5353)
        {
#if INET_CONFIG_ENABLE_IPV4
            ip_addr_t mdnsIPv4BroadcastAddr = IPADDR4_INIT_BYTES(224, 0, 0, 251);
            if (pktInfo.DestAddress == chip::Inet::IPAddress(mdnsIPv4BroadcastAddr))
            {
                return true;
            }
#endif
            ip_addr_t mdnsIPv6BroadcastAddr = IPADDR6_INIT_HOST(0xFF020000, 0, 0, 0xFB);
            if (pktInfo.DestAddress == chip::Inet::IPAddress(mdnsIPv6BroadcastAddr))
            {
                return true;
            }
        }
        return false;
    }

    static bool PayloadContains(const chip::System::PacketBufferHandle & payload, const chip::ByteSpan & byteSpan)
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

    bool PayloadContainsHostNameCaseInsensitive(const chip::System::PacketBufferHandle & payload)
    {
        uint8_t hostNameLowerCase[12];
        memcpy(hostNameLowerCase, mHostNameBuffer, sizeof(mHostNameBuffer));
        for (size_t i = 0; i < sizeof(hostNameLowerCase); ++i)
        {
            if (hostNameLowerCase[i] <= 'F' && hostNameLowerCase[i] >= 'A')
            {
                hostNameLowerCase[i] = static_cast<uint8_t>('a' + hostNameLowerCase[i] - 'A');
            }
        }
        return PayloadContains(payload, ByteSpan(mHostNameBuffer)) || PayloadContains(payload, ByteSpan(hostNameLowerCase));
    }

    static bool IsValidMdnsHostName(const chip::CharSpan & hostName)
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
