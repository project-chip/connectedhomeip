/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "ActiveResolveAttempts.h"

#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

using namespace chip;

namespace mdns {
namespace Minimal {

constexpr chip::System::Clock::Timeout ActiveResolveAttempts::kMaxRetryDelay;

void ActiveResolveAttempts::Reset()

{
    for (auto & item : mRetryQueue)
    {
        item.peerId.SetNodeId(kUndefinedNodeId);
    }
}

void ActiveResolveAttempts::Complete(const PeerId & peerId)
{
    for (auto & item : mRetryQueue)
    {
        if (item.peerId == peerId)
        {
            item.peerId.SetNodeId(kUndefinedNodeId);
            return;
        }
    }

    // This may happen during boot time adverisements: nodes come online
    // and advertise their IP without any explicit queries for them
    ChipLogProgress(Discovery, "Discovered node without a pending query");
}

void ActiveResolveAttempts::MarkPending(const PeerId & peerId)
{
    // Strategy when picking the peer id to use:
    //   1 if a matching peer id is already found, use that one
    //   2 if an 'unused' entry is found, use that
    //   3 otherwise expire the one with the largest nextRetryDelay
    //     or if equal nextRetryDelay, pick the one with the oldest
    //     queryDueTime

    RetryEntry * entryToUse = &mRetryQueue[0];

    for (size_t i = 1; i < kRetryQueueSize; i++)
    {
        if (entryToUse->peerId == peerId)
        {
            break; // best match possible
        }

        RetryEntry * entry = mRetryQueue + i;

        // Rule 1: peer id match always matches
        if (entry->peerId == peerId)
        {
            entryToUse = entry;
            continue;
        }

        // Rule 2: select unused entries
        if ((entryToUse->peerId.GetNodeId() != kUndefinedNodeId) && (entry->peerId.GetNodeId() == kUndefinedNodeId))
        {
            entryToUse = entry;
            continue;
        }
        else if (entryToUse->peerId.GetNodeId() == kUndefinedNodeId)
        {
            continue;
        }

        // Rule 3: both choices are used (have a defined node id):
        //    - try to find the one with the largest next delay (oldest request)
        //    - on same delay, use queryDueTime to determine the oldest request
        //      (the one with the smallest  due time was issued the longest time
        //       ago)
        if (entry->nextRetryDelay > entryToUse->nextRetryDelay)
        {
            entryToUse = entry;
        }
        else if ((entry->nextRetryDelay == entryToUse->nextRetryDelay) && (entry->queryDueTime < entryToUse->queryDueTime))
        {
            entryToUse = entry;
        }
    }

    if ((entryToUse->peerId.GetNodeId() != kUndefinedNodeId) && (entryToUse->peerId != peerId))
    {
        // TODO: node was evicted here, if/when resolution failures are
        // supported this could be a place for error callbacks
        //
        // Note however that this is NOT an actual 'timeout' it is showing
        // a burst of lookups for which we cannot maintain state. A reply may
        // still be received for this peer id (query was already sent on the
        // network)
        ChipLogError(Discovery, "Re-using pending resolve entry before reply was received.");
    }

    entryToUse->peerId         = peerId;
    entryToUse->queryDueTime   = mClock->GetMonotonicTimestamp();
    entryToUse->nextRetryDelay = System::Clock::Seconds16(1);
}

Optional<System::Clock::Timeout> ActiveResolveAttempts::GetTimeUntilNextExpectedResponse() const
{
    Optional<System::Clock::Timeout> minDelay = Optional<System::Clock::Timeout>::Missing();

    chip::System::Clock::Timestamp now = mClock->GetMonotonicTimestamp();

    for (auto & entry : mRetryQueue)
    {
        if (entry.peerId.GetNodeId() == kUndefinedNodeId)
        {
            continue;
        }

        if (now >= entry.queryDueTime)
        {
            // found an entry that needs processing right now
            return Optional<System::Clock::Timeout>::Value(0);
        }

        System::Clock::Timeout entryDelay = entry.queryDueTime - now;
        if (!minDelay.HasValue() || (minDelay.Value() > entryDelay))
        {
            minDelay.SetValue(entryDelay);
        }
    }

    return minDelay;
}

Optional<PeerId> ActiveResolveAttempts::NextScheduledPeer()
{
    chip::System::Clock::Timestamp now = mClock->GetMonotonicTimestamp();

    for (auto & entry : mRetryQueue)
    {
        if (entry.peerId.GetNodeId() == kUndefinedNodeId)
        {
            continue; // not a pending item
        }

        if (entry.queryDueTime > now)
        {
            continue; // not yet due
        }

        if (entry.nextRetryDelay > kMaxRetryDelay)
        {
            ChipLogError(Discovery, "Timeout waiting for mDNS resolution.");
            entry.peerId.SetNodeId(kUndefinedNodeId);
            continue;
        }

        entry.queryDueTime = now + entry.nextRetryDelay;
        entry.nextRetryDelay *= 2;

        return Optional<PeerId>::Value(entry.peerId);
    }

    return Optional<PeerId>::Missing();
}

} // namespace Minimal
} // namespace mdns
