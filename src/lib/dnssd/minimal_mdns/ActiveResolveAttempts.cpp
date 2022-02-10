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

ActiveResolveAttempts::RetryEntry * ActiveResolveAttempts::FindEntry(const chip::PeerId & peerId)
{
    RetryEntry * result = nullptr;
    mRetryQueue.ForEachActiveObject([&](RetryEntry * item) {
        if (item->peerId == peerId)
        {
            result = item;
            return Loop::Break;
        }
        return Loop::Continue;
    });
    return result;
}

void ActiveResolveAttempts::Complete(const PeerId & peerId)
{
    RetryEntry * match = FindEntry(peerId);
    if (match != nullptr)
    {
        mRetryQueue.ReleaseObject(match);
    }
    else
    {
        ChipLogProgress(Discovery, "Discovered node without a pending query");
    }
}

void ActiveResolveAttempts::MarkPending(const PeerId & peerId)
{
    // Strategy when picking the peer to use:
    //   1 if a matching peer is already found, use that one
    //   2 if an 'unused' entry is found, use that
    //   3 otherwise expire the one with the largest nextRetryDelay
    //     or if equal nextRetryDelay, pick the one with the oldest
    //     queryDueTime

    // Rule 1: peer id match always matches
    RetryEntry * entryToUse = FindEntry(peerId);
    if (entryToUse != nullptr)
    {
        MarkPending(entryToUse);
        return;
    }

    // Rule 2: select unused entries
    if (mRetryQueue.Allocated() < kRetryQueueSize)
    {
        entryToUse = mRetryQueue.CreateObject(peerId);
        if (entryToUse != nullptr)
        {
            MarkPending(entryToUse);
            return;
        }
    }

    // Rule 3: both choices are used (have a defined node id):
    //    - try to find the one with the largest next delay (oldest request)
    //    - on same delay, use queryDueTime to determine the oldest request
    //      (the one with the smallest  due time was issued the longest time
    //       ago)
    mRetryQueue.ForEachActiveObject([&](RetryEntry * entry) {
        if (entryToUse == nullptr)
        {
            entryToUse = entry;
        }
        else if (entry->nextRetryDelay > entryToUse->nextRetryDelay)
        {
            entryToUse = entry;
        }
        else if ((entry->nextRetryDelay == entryToUse->nextRetryDelay) && (entry->queryDueTime < entryToUse->queryDueTime))
        {
            entryToUse = entry;
        }

        return Loop::Continue;
    });

    if (entryToUse != nullptr)
    {
        mRetryQueue.ReleaseObject(entryToUse);
        entryToUse = mRetryQueue.CreateObject(peerId);
        VerifyOrDie(entryToUse != nullptr);

        // TODO: node was evicted here, if/when resolution failures are
        // supported this could be a place for error callbacks
        //
        // Note however that this is NOT an actual 'timeout' it is showing
        // a burst of lookups for which we cannot maintain state. A reply may
        // still be received for this peer id (query was already sent on the
        // network)
        ChipLogError(Discovery, "Re-using pending resolve entry before reply was received.");
        MarkPending(entryToUse);
        return;
    }
}

void ActiveResolveAttempts::MarkPending(RetryEntry * entryToUse)
{
    entryToUse->queryDueTime   = mClock->GetMonotonicTimestamp();
    entryToUse->nextRetryDelay = System::Clock::Seconds16(1);
}

Optional<System::Clock::Timeout> ActiveResolveAttempts::GetTimeUntilNextExpectedResponse() const
{
    Optional<System::Clock::Timeout> minDelay = Optional<System::Clock::Timeout>::Missing();

    chip::System::Clock::Timestamp now = mClock->GetMonotonicTimestamp();

    mRetryQueue.ForEachActiveObject([&](const RetryEntry * entry) {
        if (now >= entry->queryDueTime)
        {
            // found an entry that needs processing right now
            minDelay = Optional<System::Clock::Timeout>::Value(0);
            return Loop::Break;
        }

        System::Clock::Timeout entryDelay = entry->queryDueTime - now;
        if (!minDelay.HasValue() || (minDelay.Value() > entryDelay))
        {
            minDelay.SetValue(entryDelay);
        }

        return Loop::Continue;
    });

    return minDelay;
}

Optional<PeerId> ActiveResolveAttempts::NextScheduledPeer()
{
    chip::System::Clock::Timestamp now = mClock->GetMonotonicTimestamp();

    RetryEntry * result = nullptr;
    mRetryQueue.ForEachActiveObject([&](RetryEntry * entry) {
        if (entry->queryDueTime > now)
        {
            return Loop::Continue; // not yet due
        }

        if (entry->nextRetryDelay > kMaxRetryDelay)
        {
            ChipLogError(Discovery, "Timeout waiting for mDNS resolution.");
            mRetryQueue.ReleaseObject(entry);
            return Loop::Continue;
        }

        if (result == nullptr || entry->queryDueTime < result->queryDueTime)
        {
            result = entry;
        }
        return Loop::Continue;
    });

    if (result != nullptr)
    {
        result->queryDueTime = now + result->nextRetryDelay;
        result->nextRetryDelay *= 2;
        return Optional<PeerId>::Value(result->peerId);
    }
    else
    {
        return Optional<PeerId>::Missing();
    }
}

} // namespace Minimal
} // namespace mdns
