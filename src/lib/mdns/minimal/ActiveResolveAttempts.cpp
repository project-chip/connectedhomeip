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
    //   3 otherwise expire the one with the largest nextRetryDelaySec
    //     or if equal nextRetryDelaySec, pick the one with the oldest
    //     queryDueTimeMs

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

        // Rule 2: both choices are used (have a defined node id),
        // pick the one with most queryDueTimeMs
        if (entry->nextRetryDelaySec > entryToUse->nextRetryDelaySec)
        {
            entryToUse = entry;
        }
        else if ((entry->nextRetryDelaySec == entryToUse->nextRetryDelaySec) &&
                 (entry->queryDueTimeMs < entryToUse->queryDueTimeMs))
        {
            entryToUse = entry;
        }
    }

    if ((entryToUse->peerId.GetNodeId() != kUndefinedNodeId) && (entryToUse->peerId != peerId))
    {
        ChipLogError(Discovery, "Re-using pending resolve entry before reply was received.");
    }

    entryToUse->peerId            = peerId;
    entryToUse->queryDueTimeMs    = chip::System::Clock::GetMonotonicMilliseconds();
    entryToUse->nextRetryDelaySec = 1;
}

int ActiveResolveAttempts::GetMsUntilNextExpectedResponse() const
{
    int minDelay = kInvalidNextMs;

    chip::System::Clock::MonotonicMilliseconds nowMs = System::Clock::GetMonotonicMilliseconds();

    for (auto & entry : mRetryQueue)
    {
        if (entry.peerId.GetNodeId() == kUndefinedNodeId)
        {
            continue;
        }

        if (nowMs >= entry.queryDueTimeMs)
        {
            minDelay = 0;
            break;
        }

        minDelay = std::min(minDelay, static_cast<int>(entry.queryDueTimeMs - minDelay));
    }

    return minDelay;
}

PeerId ActiveResolveAttempts::NextScheduledPeer()
{
    chip::System::Clock::MonotonicMilliseconds nowMs = System::Clock::GetMonotonicMilliseconds();

    for (auto & entry : mRetryQueue)
    {
        if (entry.peerId.GetNodeId() == kUndefinedNodeId)
        {
            continue; // not a pending item
        }

        if (entry.queryDueTimeMs > nowMs)
        {
            continue; // not yet due
        }

        if (entry.nextRetryDelaySec > kMaxRetryDelaySec)
        {
            ChipLogError(Discovery, "Timeout waiting for mDNS resolution.");
            entry.peerId.SetNodeId(kUndefinedNodeId);
            continue;
        }

        entry.queryDueTimeMs = nowMs + entry.nextRetryDelaySec * 1000L;
        entry.nextRetryDelaySec *= 2;
        return entry.peerId;
    }
    return PeerId();
}

} // namespace Minimal
} // namespace mdns