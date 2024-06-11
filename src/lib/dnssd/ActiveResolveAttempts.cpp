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

using namespace chip;

namespace mdns {
namespace Minimal {

constexpr chip::System::Clock::Timeout ActiveResolveAttempts::kMaxRetryDelay;

void ActiveResolveAttempts::Reset()

{
    for (auto & item : mRetryQueue)
    {
        item.attempt.Clear();
    }
}

void ActiveResolveAttempts::Complete(const PeerId & peerId)
{
    for (auto & item : mRetryQueue)
    {
        if (item.attempt.Matches(peerId))
        {
            item.attempt.Clear();
            return;
        }
    }

#if CHIP_MINMDNS_HIGH_VERBOSITY
    // This may happen during boot time adverisements: nodes come online
    // and advertise their IP without any explicit queries for them
    ChipLogProgress(Discovery, "Discovered node without a pending query");
#endif
}

bool ActiveResolveAttempts::HasBrowseFor(chip::Dnssd::DiscoveryType type) const
{
    for (auto & item : mRetryQueue)
    {
        if (!item.attempt.IsBrowse())
        {
            continue;
        }

        if (item.attempt.BrowseData().type == type)
        {
            return true;
        }
    }

    return false;
}

void ActiveResolveAttempts::CompleteIpResolution(SerializedQNameIterator targetHostName)
{
    for (auto & item : mRetryQueue)
    {
        if (item.attempt.MatchesIpResolve(targetHostName))
        {
            item.attempt.Clear();
            return;
        }
    }
}

CHIP_ERROR ActiveResolveAttempts::CompleteAllBrowses()
{
    for (auto & item : mRetryQueue)
    {
        if (item.attempt.IsBrowse())
        {
            item.attempt.Clear();
        }
    }

    return CHIP_NO_ERROR;
}

void ActiveResolveAttempts::NodeIdResolutionNoLongerNeeded(const PeerId & peerId)
{
    for (auto & item : mRetryQueue)
    {
        if (item.attempt.Matches(peerId))
        {
            item.attempt.ConsumerRemoved();
            return;
        }
    }
}

void ActiveResolveAttempts::MarkPending(const chip::PeerId & peerId)
{
    MarkPending(ScheduledAttempt(peerId, /* firstSend */ true));
}

void ActiveResolveAttempts::MarkPending(const chip::Dnssd::DiscoveryFilter & filter, const chip::Dnssd::DiscoveryType type)
{
    MarkPending(ScheduledAttempt(filter, type, /* firstSend */ true));
}

void ActiveResolveAttempts::MarkPending(ScheduledAttempt::IpResolve && resolve)
{
    MarkPending(ScheduledAttempt(std::move(resolve), /* firstSend */ true));
}

void ActiveResolveAttempts::MarkPending(ScheduledAttempt && attempt)
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
        if (entryToUse->attempt.Matches(attempt))
        {
            break; // best match possible
        }

        RetryEntry * entry = mRetryQueue + i;

        // Rule 1: attempt match always matches
        if (entry->attempt.Matches(attempt))
        {
            entryToUse = entry;
            continue;
        }

        // Rule 2: select unused entries
        if (!entryToUse->attempt.IsEmpty() && entry->attempt.IsEmpty())
        {
            entryToUse = entry;
            continue;
        }
        if (entryToUse->attempt.IsEmpty())
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

    if ((!entryToUse->attempt.IsEmpty()) && (!entryToUse->attempt.Matches(attempt)))
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

    attempt.WillCoalesceWith(entryToUse->attempt);
    entryToUse->attempt        = attempt;
    entryToUse->queryDueTime   = mClock->GetMonotonicTimestamp();
    entryToUse->nextRetryDelay = System::Clock::Seconds16(1);
}

std::optional<System::Clock::Timeout> ActiveResolveAttempts::GetTimeUntilNextExpectedResponse() const
{
    std::optional<System::Clock::Timeout> minDelay = std::nullopt;

    chip::System::Clock::Timestamp now = mClock->GetMonotonicTimestamp();

    for (auto & entry : mRetryQueue)
    {
        if (entry.attempt.IsEmpty())
        {
            continue;
        }

        if (now >= entry.queryDueTime)
        {
            // found an entry that needs processing right now
            return std::make_optional<System::Clock::Timeout>(0);
        }

        System::Clock::Timeout entryDelay = entry.queryDueTime - now;
        if (!minDelay.has_value() || (*minDelay > entryDelay))
        {
            minDelay.emplace(entryDelay);
        }
    }

    return minDelay;
}

std::optional<ActiveResolveAttempts::ScheduledAttempt> ActiveResolveAttempts::NextScheduled()
{
    chip::System::Clock::Timestamp now = mClock->GetMonotonicTimestamp();

    for (auto & entry : mRetryQueue)
    {
        if (entry.attempt.IsEmpty())
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
            entry.attempt.Clear();
            continue;
        }

        entry.queryDueTime = now + entry.nextRetryDelay;
        entry.nextRetryDelay *= 2;

        std::optional<ScheduledAttempt> attempt = std::make_optional(entry.attempt);
        entry.attempt.firstSend                 = false;

        return attempt;
    }

    return std::nullopt;
}

bool ActiveResolveAttempts::ShouldResolveIpAddress(PeerId peerId) const
{
    for (auto & item : mRetryQueue)
    {
        if (item.attempt.IsEmpty())
        {
            continue;
        }
        if (item.attempt.IsBrowse())
        {
            return true;
        }

        if (item.attempt.IsResolve())
        {
            auto & data = item.attempt.ResolveData();
            if (data.peerId == peerId)
            {
                return true;
            }
        }
    }

    return false;
}

bool ActiveResolveAttempts::IsWaitingForIpResolutionFor(SerializedQNameIterator hostName) const
{
    for (auto & entry : mRetryQueue)
    {
        if (entry.attempt.IsEmpty())
        {
            continue; // not a pending item
        }

        if (!entry.attempt.IsIpResolve())
        {
            continue;
        }

        if (hostName == entry.attempt.IpResolveData().hostName.Content())
        {
            return true;
        }
    }

    return false;
}

} // namespace Minimal
} // namespace mdns
