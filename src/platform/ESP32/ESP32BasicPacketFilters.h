/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <inet/EndpointQueueFilter.h>
#include <stdio.h>
#include <string.h>

#include <inet/IPPacketInfo.h>
#include <inet/InetInterface.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

namespace {

using namespace chip;
using namespace chip::Inet;

class ESP32PacketFilter : public chip::Inet::EndpointQueueFilter
{
public:
    /**
     * @brief Initialize the packet filter with a starting limit
     *
     * @param maxAllowedQueuedPackets - max number of pending-in-queue not yet processed predicate-matching packets
     */
    ESP32PacketFilter() {}

    void Init()
    {
        /* Packets which are causing Packet Buffer Pool to get empty
         *  Destination IP fe80::0000:1111:2222:3333 indicates packets shared locally on device to device communication, not needed
         * in matter Destination IP ff02::fb : multicast mdns packets */

        IPAddress::FromString("fe80::0000:1111:2222:3333", fakeDest);
        IPAddress::FromString("ff02::fb", fakeMdnsDest);
    }

    FilterOutcome FilterBeforeEnqueue(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                      const chip::System::PacketBufferHandle & pktPayload) override
    {
        // WARNING: This is likely called in a different context than `FilterAfterDequeue`. We use an atomic for the counter.
        // Non-matching is never accounted, always allowed. Lack of predicate is equivalent to non-matching.

        if (pktInfo.DestAddress == fakeDest || pktInfo.DestAddress == fakeMdnsDest)
        {
            return FilterOutcome::kDropPacket;
        }

        return FilterOutcome::kAllowPacket;
    }

    /**
     * @brief Implementation of filtering after dequeueing that applies the predicate.
     *
     * See base class for arguments
     */
    FilterOutcome FilterAfterDequeue(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                     const chip::System::PacketBufferHandle & pktPayload) override
    {
        return FilterOutcome::kAllowPacket;
    }

protected:
    IPAddress fakeDest;
    IPAddress fakeMdnsDest;
};

} // namespace
