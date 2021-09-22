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

#include <lib/core/PeerId.h>
#include <system/SystemClock.h>

namespace mdns {
namespace Minimal {

/// Keeps track of active resolve attempts
class ActiveResolveAttempts
{
public:
    static constexpr int kInvalidNextMs = 1;

    ActiveResolveAttempts() { Reset(); }

    void Reset();

    void Complete(const chip::PeerId & peerId);

    void MarkPending(const chip::PeerId & peerId);

    // Get minimum milliseconds until the next pending reply is required.
    //
    // Returns -1 if no active resolve entries exist, >= 0 if a delay is expected
    int GetMsUntilNextExpectedResponse() const;

    // Get the peer Id that needs scheduling for a query
    //
    // Assumes that the resolution is being sent and will apply internal
    // query logic.
    //
    // Returns kUndefinedNodeId if no peer scheduled.
    chip::PeerId NextScheduledPeer();

private:
    static constexpr uint32_t kMaxRetryDelaySec = 16;
    static constexpr size_t kRetryQueueSize     = 4;

    struct RetryEntry
    {
        // What peer id is pending discovery.
        //
        // Inactive entries are marked by having NodeId == kUndefinedNodeId
        chip::PeerId peerId;

        // When a reply is expected for this item
        chip::System::Clock::MonotonicMilliseconds queryDueTimeMs;

        // Next expected delay for sending if reply is not reached by
        // 'queryDueTimeMs'
        //
        // Based on RFC 6762 expectations are:
        //    - the interval between the first two queries MUST be at least
        //      one second
        //    - the intervals between successive queries MUST increase by at
        //      least a factor of two
        uint32_t nextRetryDelaySec = 1;
    };

    RetryEntry mRetryQueue[kRetryQueueSize];
};

} // namespace Minimal
} // namespace mdns
