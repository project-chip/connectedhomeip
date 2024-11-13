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

#include <inet/BasicPacketFilters.h>

namespace chip {
namespace Inet {

struct EndpointFilter
{
    virtual ~EndpointFilter()                                                                        = default;
    virtual EndpointQueueFilter::FilterOutcome Filter(const void * endpoint, const IPPacketInfo & pktInfo,
                                                      const System::PacketBufferHandle & pktPayload) = 0;
};

struct EndpointQueueFilterConfig
{
    size_t allowedQueuedPackets = 10; // Default value
};

struct HostNameFilter : EndpointFilter
{
    static constexpr size_t kHostNameLengthMax = 13; // 6 bytes in hex and null terminator.

    EndpointQueueFilter::FilterOutcome Filter(const void * endpoint, const IPPacketInfo & pktInfo,
                                              const System::PacketBufferHandle & pktPayload) override;

    CHIP_ERROR SetHostName(Span<const unsigned char> & name);

private:
    uint8_t mHostName[kHostNameLengthMax] = { 0 };
    static constexpr size_t kMdnsPort     = 5353;
};

namespace SilabsEndpointQueueFilter {

class EndpointQueueFilter : public Inet::EndpointQueueFilter
{
public:
    EndpointQueueFilterConfig mConfig;
    EndpointQueueFilter();
    EndpointQueueFilter(size_t maxAllowedQueuedPackets);

    FilterOutcome FilterBeforeEnqueue(const void * endpoint, const IPPacketInfo & pktInfo,
                                      const System::PacketBufferHandle & pktPayload) override;

    FilterOutcome FilterAfterDequeue(const void * endpoint, const IPPacketInfo & pktInfo,
                                     const System::PacketBufferHandle & pktPayload);

    CHIP_ERROR SetHostName(Span<const unsigned char> & addr) { return mHostNameFilter.SetHostName(addr); }

    // Method to set the configuration
    void SetConfig(const EndpointQueueFilterConfig & config) { mConfig = config; }

private:
    DropIfTooManyQueuedPacketsFilter mTooManyFilter;
    HostNameFilter mHostNameFilter;
};

} // namespace SilabsEndpointQueueFilter
} // namespace Inet
} // namespace chip
