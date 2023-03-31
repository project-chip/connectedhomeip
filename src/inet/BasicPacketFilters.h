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
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Inet {

/**
 * @brief Basic filter that counts how many pending (not yet dequeued) packets
 *        are accumulated that match a predicate function, and drops those that
 *        would cause crossing of the threshold.
 */
class DropIfTooManyQueuedPacketsFilter : public chip::Inet::EndpointQueueFilter
{
public:
    typedef bool (*PacketMatchPredicateFunc)(void * context, const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                             const chip::System::PacketBufferHandle & pktPayload);

    /**
     * @brief Initialize the packet filter with a starting limit
     *
     * @param maxAllowedQueuedPackets - max number of pending-in-queue not yet processed predicate-matching packets
     */
    DropIfTooManyQueuedPacketsFilter(size_t maxAllowedQueuedPackets) : mMaxAllowedQueuedPackets(maxAllowedQueuedPackets) {}

    /**
     * @brief Set the predicate to use for filtering
     *
     * @warning DO NOT modify at runtime while the filter is being called. If you do so, the queue accounting could
     *          get out of sync, and cause the filtering to fail to properly work.
     *
     * @param predicateFunc - Predicate function to apply. If nullptr, no filtering will take place
     * @param context - Pointer to predicate-specific context that will be provided to predicate at every call. May be nullptr.
     */
    void SetPredicate(PacketMatchPredicateFunc predicateFunc, void * context)
    {
        mPredicate = predicateFunc;
        mContext   = context;
    }

    /**
     * @brief Set the ceiling for max allowed packets queued up that matched the predicate.
     *
     * @note Changing this at runtime while packets are coming only affects future dropping, and
     *       does not remove packets from the queue if the limit is lowered below the currently-in-queue
     *       count.
     *
     * @param maxAllowedQueuedPackets - number of packets currently pending allowed.
     */
    void SetMaxQueuedPacketsLimit(size_t maxAllowedQueuedPackets) { mMaxAllowedQueuedPackets.store(maxAllowedQueuedPackets); }

    /**
     * @return the total number of packets dropped so far by the filter
     */
    size_t GetNumDroppedPackets() const { return mNumDroppedPackets.load(); }

    /**
     * @brief Reset the counter of dropped packets.
     */
    void ClearNumDroppedPackets() { mNumDroppedPackets.store(0); }

    /**
     * @brief Method called when a packet is dropped due to high watermark getting reached, based on predicate.
     *
     * Subclasses may use this to implement additional behavior or diagnostics.
     *
     * This is called once for every dropped packet. If there is no filter predicate, this is not called.
     *
     * @param endpoint - pointer to endpoint instance (platform-dependent, which is why it's void)
     * @param pktInfo - info about source/dest of packet
     * @param pktPayload - payload content of packet
     */
    virtual void OnDropped(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                           const chip::System::PacketBufferHandle & pktPayload)
    {}

    /**
     * @brief Method called whenever queue of accumulated packets is now empty, based on predicate.
     *
     * Subclasses may use this to implement additional behavior or diagnostics.
     *
     * This is possibly called repeatedly in a row, if the queue actually never gets above one.
     *
     * This is only called for packets that had matched the filtering rule, where they had
     * been explicitly allowed in the past. If there is no filter predicate, this is not called.
     *
     * @param endpoint - pointer to endpoint instance (platform-dependent, which is why it's void)
     * @param pktInfo - info about source/dest of packet
     * @param pktPayload - payload content of packet
     */
    virtual void OnLastMatchDequeued(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                     const chip::System::PacketBufferHandle & pktPayload)
    {}

    /**
     * @brief Implementation of filtering before queueing that applies the predicate.
     *
     * See base class for arguments
     */
    FilterOutcome FilterBeforeEnqueue(const void * endpoint, const chip::Inet::IPPacketInfo & pktInfo,
                                      const chip::System::PacketBufferHandle & pktPayload) override
    {
        // WARNING: This is likely called in a different context than `FilterAfterDequeue`. We use an atomic for the counter.

        // Non-matching is never accounted, always allowed. Lack of predicate is equivalent to non-matching.
        if ((mPredicate == nullptr) || !mPredicate(mContext, endpoint, pktInfo, pktPayload))
        {
            return FilterOutcome::kAllowPacket;
        }

        if (mNumQueuedPackets.load() >= mMaxAllowedQueuedPackets)
        {
            ++mNumDroppedPackets;
            OnDropped(endpoint, pktInfo, pktPayload);
            return FilterOutcome::kDropPacket;
        }

        ++mNumQueuedPackets;

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
        // WARNING: This is likely called in a different context than `FilterBeforeEnqueue`. We use an atomic for the counter.
        // NOTE: This is always called from Matter platform event loop

        // Non-matching is never accounted, always allowed. Lack of predicate is equivalent to non-matching.
        if ((mPredicate == nullptr) || !mPredicate(mContext, endpoint, pktInfo, pktPayload))
        {
            return FilterOutcome::kAllowPacket;
        }

        // If we ever go negative, we have mismatch ingress/egress filter via predicate and
        // device may eventually starve.
        VerifyOrDie(mNumQueuedPackets != 0);

        --mNumQueuedPackets;
        size_t numQueuedPackets = mNumQueuedPackets.load();
        if (numQueuedPackets == 0)
        {
            OnLastMatchDequeued(endpoint, pktInfo, pktPayload);
        }

        // We always allow the packet and just do accounting, since all dropping is prior to queue entry.
        return FilterOutcome::kAllowPacket;
    }

protected:
    PacketMatchPredicateFunc mPredicate = nullptr;
    void * mContext                     = nullptr;
    std::atomic_size_t mNumQueuedPackets{ 0 };
    std::atomic_size_t mMaxAllowedQueuedPackets{ 0 };
    std::atomic_size_t mNumDroppedPackets{ 0u };
};

} // namespace Inet
} // namespace chip
