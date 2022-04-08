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
#include <lib/dnssd/Resolver.h>
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
public:
    static constexpr size_t kRetryQueueSize                      = 4;
    static constexpr chip::System::Clock::Timeout kMaxRetryDelay = chip::System::Clock::Seconds16(16);

    struct ScheduledAttempt
    {
        enum AttemptType
        {
            kInvalid,
            kResolve,
            kBrowse,
        };

        ScheduledAttempt() : attemptType(kInvalid) {}
        ScheduledAttempt(const chip::PeerId & peer, bool first) : attemptType(kResolve), peerId(peer), firstSend(first) {}
        ScheduledAttempt(const chip::Dnssd::DiscoveryFilter discoveryFilter, const chip::Dnssd::DiscoveryType type, bool first) :
            attemptType(kBrowse), browse(discoveryFilter, type), firstSend(first)
        {}
        bool operator==(const ScheduledAttempt & other) const { return Matches(other) && other.firstSend == firstSend; }
        bool Matches(const ScheduledAttempt & other) const
        {
            if (other.attemptType != attemptType)
            {
                return false;
            }
            switch (attemptType)
            {
            case kInvalid:
                return true;
            case kBrowse:
                return (other.browse.filter == browse.filter && other.browse.type == browse.type);
            case kResolve:
                return other.peerId == peerId;
            default:
                return false;
            }
        }
        bool Matches(const chip::PeerId & peer) const { return (attemptType == kResolve) && (peerId == peer); }
        bool Matches(const chip::Dnssd::DiscoveredNodeData & data) const
        {
            if (attemptType != kBrowse)
            {
                return false;
            }
            // TODO: we should mark returned node data based on the query
            if (browse.type != chip::Dnssd::DiscoveryType::kCommissionableNode)
            {
                // We don't currently have markers in the returned DiscoveredNodeData to differentiate these, so assume all returned
                // packets match
                return true;
            }
            switch (browse.filter.type)
            {
            case chip::Dnssd::DiscoveryFilterType::kNone:
                return true;
            case chip::Dnssd::DiscoveryFilterType::kShortDiscriminator:
                return browse.filter.code == static_cast<uint64_t>((data.longDiscriminator >> 8) & 0x0F);
            case chip::Dnssd::DiscoveryFilterType::kLongDiscriminator:
                return browse.filter.code == data.longDiscriminator;
            case chip::Dnssd::DiscoveryFilterType::kVendorId:
                return browse.filter.code == data.vendorId;
            case chip::Dnssd::DiscoveryFilterType::kDeviceType:
                return browse.filter.code == data.deviceType;
            case chip::Dnssd::DiscoveryFilterType::kCommissioningMode:
                return browse.filter.code == data.commissioningMode;
            case chip::Dnssd::DiscoveryFilterType::kInstanceName:
                return strncmp(browse.filter.instanceName, data.instanceName,
                               chip::Dnssd::Commission::kInstanceNameMaxLength + 1) == 0;
            case chip::Dnssd::DiscoveryFilterType::kCommissioner:
            case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
            default:
                // These are for other discovery types.
                return false;
            }
        }
        bool IsEmpty() const { return attemptType == kInvalid; }
        bool IsResolve() const { return attemptType == kResolve; }
        bool IsBrowse() const { return attemptType == kBrowse; }
        void Clear() { attemptType = kInvalid; }

        // Not using Variant because it assumes a heap impl
        AttemptType attemptType;
        struct Browse
        {
            Browse(const chip::Dnssd::DiscoveryFilter discoveryFilter, const chip::Dnssd::DiscoveryType discoveryType) :
                filter(discoveryFilter), type(discoveryType)
            {}
            chip::Dnssd::DiscoveryFilter filter;
            chip::Dnssd::DiscoveryType type;
        };
        union
        {
            chip::PeerId peerId; // Peer id for resolve attempts
            Browse browse;
        };
        // First packet send is marked separately: minMDNS logic can choose
        // to first send a unicast query followed by a multicast one.
        bool firstSend = false;
    };

    ActiveResolveAttempts(chip::System::Clock::ClockBase * clock) : mClock(clock) { Reset(); }

    /// Clear out the internal queue
    void Reset();

    /// Mark a resolution as a success, removing it from the internal list
    void Complete(const chip::PeerId & peerId);
    void Complete(const chip::Dnssd::DiscoveredNodeData & data);

    /// Mark that a resolution is pending, adding it to the internal list
    ///
    /// Once this complete, this peer id will be returned immediately
    /// by NextScheduled (potentially with others as well)
    void MarkPending(const chip::PeerId & peerId);
    void MarkPending(const chip::Dnssd::DiscoveryFilter & filter, const chip::Dnssd::DiscoveryType type);

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
    chip::Optional<ScheduledAttempt> NextScheduled();

private:
    struct RetryEntry
    {
        ScheduledAttempt attempt;
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
    void MarkPending(const ScheduledAttempt & attempt);
    chip::System::Clock::ClockBase * mClock;
    RetryEntry mRetryQueue[kRetryQueueSize];
};

} // namespace Minimal
} // namespace mdns
