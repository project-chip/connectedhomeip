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

#pragma once

#include <cstddef>
#include <cstdint>

#include <lib/core/Optional.h>
#include <lib/core/PeerId.h>
#include <system/SystemClock.h>

namespace mdns {
namespace Minimal {

/// Keeps track of active resolve attempts
///
/// Maintains a list of 'pending mdns resolve queries' and provides operations
/// for:
///    - add/remove to the list
///    - figuring out a 'next query time' for items in the list
///    - iterating through the 'schedule now' items of the list
///
class ActiveResolveAttempts
{
private:
    struct RetryEntry
    {
        // What peer id is pending resolution.
        //
        // Inactive entries are marked by having NodeId == kUndefinedNodeId
        chip::PeerId peerId;

        // First packet send is marked separately: minMDNS logic can choose
        // to first send a unicast query followed by a multicast one.
        bool firstSend = false;

        // When a reply is expected for this item
        chip::System::Clock::Timestamp queryDueTime;

        // Next expected delay for sending if reply is not reached by
        // 'queryDueTimeMs'
        //
        // Based on RFC 6762 expectations are:
        //    - the interval between the first two queries MUST be at least
        //      one second
        //    - the intervals between successive queries MUST increase by at
        //      least a factor of two
        chip::System::Clock::Timeout nextRetryDelay = chip::System::Clock::Seconds16(1);
    };

public:
    static constexpr size_t kRetryQueueSize                      = 4;
    static constexpr chip::System::Clock::Timeout kMaxRetryDelay = chip::System::Clock::Seconds16(16);

    struct ScheduledResolve
    {
        const chip::PeerId peerId;
        const bool firstSend = false;

        ScheduledResolve(const RetryEntry & entry) : peerId(entry.peerId), firstSend(entry.firstSend) {}
        ScheduledResolve(const chip::PeerId & peer, bool first) : peerId(peer), firstSend(first) {}
        ScheduledResolve(const ScheduledResolve &) = default;

        bool operator==(const ScheduledResolve & other) const { return (peerId == other.peerId) && (firstSend == other.firstSend); }
    };

    ActiveResolveAttempts(chip::System::Clock::ClockBase * clock) : mClock(clock) { Reset(); }

    /// Clear out the internal queue
    void Reset();

    /// Mark a resolution as a success, removing it from the internal list
    void Complete(const chip::PeerId & peerId);

    /// Mark that a resolution is pending, adding it to the internal list
    ///
    /// Once this complete, this peer id will be returned immediately
    /// by NextScheduledPeer (potentially with others as well)
    void MarkPending(const chip::PeerId & peerId);

    // Get minimum time until the next pending reply is required.
    //
    // Returns missing if no actively tracked elements exist.
    chip::Optional<chip::System::Clock::Timeout> GetTimeUntilNextExpectedResponse() const;

    // Get the peer Id that needs scheduling for a query
    //
    // Assumes that the resolution is being sent and will apply internal
    // query logic. This means:
    //  - internal tracking of 'next due time' will updated as 'request sent
    //    now'
    //  - there is NO sorting implied by this call. Returned value will be
    //    any peer that needs a new request sent
    chip::Optional<ScheduledResolve> NextScheduledPeer();

private:
    chip::System::Clock::ClockBase * mClock;
    RetryEntry mRetryQueue[kRetryQueueSize];
};

} // namespace Minimal
} // namespace mdns
