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

#pragma once

#include <atomic>
#include <inet/EndpointQueueFilter.h>
#include <inet/IPPacketInfo.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

namespace mdns {
namespace Minimal {

/**
 * @brief Basic filter that counts how many pending (not yet dequeued) mDNS packets
 *        are accumulated, and drops those that would cause crossing of the threshold.
 *
 */
class DropIfTooManyQueuedMdnsPacketFilter : public chip::Inet::EndpointQueueFilter
{
    // Repeated here since there are no accessible headers with this
    static constexpr uint16_t kMdnsPort = 5353u;

public:
    /**
     * @brief Initialize the packet filter with a starting limit
     *
     * @param maxQueuedPackets - max number of pending-in-queue mDNS packets not yet processed
     */
    DropIfTooManyQueuedMdnsPacketFilter(size_t maxQueuedPackets) : mMaxQueuedPackets(maxQueuedPackets) {}

    FilterOutcome FilterBeforeEnqueue(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                      const chip::System::PacketBufferHandle & pktPayload) override
    {
        // WARNING: This is likely called in a different context than `FilterAfterDequeue`. We use an atomic for the counter.

        // Drop all IPv4 Mdns packets
        VerifyOrReturnValue((pktInfo.DestAddress.IsIPv4() && pktInfo.DestPort == kMdnsPort), FilterOutcome::kDropPacket);

        // Other mDNS packets are allowed
        VerifyOrReturnValue(pktInfo.DestPort == kMdnsPort, FilterOutcome::kAllowPacket);

        if (mNumQueuedMdnsPackets.load() >= mMaxQueuedPackets && !mStopLogging)
        {
            ChipLogError(Discovery, "Started dropping mDNS from storm.");
            mStopLogging = true;
            return FilterOutcome::kDropPacket;
        }

        ++mNumQueuedMdnsPackets;

        return FilterOutcome::kAllowPacket;
    }

    FilterOutcome FilterAfterDequeue(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                     const chip::System::PacketBufferHandle & pktPayload) override
    {
        // WARNING: This is likely called in a different context than `FilterBeforeEnqueue`. We use an atomic for the counter.

        // Non-mDNS is ignored
        VerifyOrReturnValue(pktInfo.DestPort == kMdnsPort, FilterOutcome::kAllowPacket);

        --mNumQueuedMdnsPackets;
        if (mNumQueuedMdnsPackets.load() == 0 && mStopLogging)
        {
            mStopLogging = false;
            ChipLogError(Discovery, "Resolved mDNS storm.");
        }

        // We always allow the packet and just do accounting, since all dropping is prior to queue entry.
        return FilterOutcome::kAllowPacket;
    }

protected:
    std::atomic_size_t mNumQueuedMdnsPackets = { 0 };
    const size_t mMaxQueuedPackets           = 0;
    bool mStopLogging                        = false;
};

} // namespace Minimal
} // namespace mdns
