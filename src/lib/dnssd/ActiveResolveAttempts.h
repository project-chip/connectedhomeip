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
#include <optional>

#include <lib/core/PeerId.h>
#include <lib/dnssd/Resolver.h>
#include <lib/dnssd/minimal_mdns/core/HeapQName.h>
#include <lib/support/Variant.h>
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
        struct Browse
        {
            Browse(const chip::Dnssd::DiscoveryFilter discoveryFilter, const chip::Dnssd::DiscoveryType discoveryType) :
                filter(discoveryFilter), type(discoveryType)
            {}
            chip::Dnssd::DiscoveryFilter filter;
            chip::Dnssd::DiscoveryType type;
        };

        struct Resolve
        {
            chip::PeerId peerId;
            uint32_t consumerCount = 0;

            Resolve(chip::PeerId id) : peerId(id) {}
        };

        struct IpResolve
        {
            HeapQName hostName;
            IpResolve(HeapQName && host) : hostName(std::move(host)) {}
        };

        ScheduledAttempt()
        {
            static_assert(sizeof(Resolve) <= sizeof(Browse) || sizeof(Resolve) <= sizeof(HeapQName),
                          "Figure out where to put the Resolve counter so that Resolve is not making ScheduledAttempt bigger than "
                          "it has to be anyway to handle the other attempt types.");
        }
        ScheduledAttempt(const chip::PeerId & peer, bool first) :
            resolveData(chip::InPlaceTemplateType<Resolve>(), peer), firstSend(first)
        {}
        ScheduledAttempt(const chip::Dnssd::DiscoveryFilter discoveryFilter, const chip::Dnssd::DiscoveryType type, bool first) :
            resolveData(chip::InPlaceTemplateType<Browse>(), discoveryFilter, type), firstSend(first)
        {}

        ScheduledAttempt(IpResolve && ipResolve, bool first) :
            resolveData(chip::InPlaceTemplateType<IpResolve>(), ipResolve), firstSend(first)
        {}

        bool operator==(const ScheduledAttempt & other) const { return Matches(other) && other.firstSend == firstSend; }
        bool Matches(const ScheduledAttempt & other) const
        {
            if (!resolveData.Valid())
            {
                return !other.resolveData.Valid();
            }

            if (resolveData.Is<Browse>())
            {
                if (!other.resolveData.Is<Browse>())
                {
                    return false;
                }

                auto & a = resolveData.Get<Browse>();
                auto & b = other.resolveData.Get<Browse>();
                return (a.filter == b.filter && a.type == b.type);
            }

            if (resolveData.Is<Resolve>())
            {
                if (!other.resolveData.Is<Resolve>())
                {
                    return false;
                }
                auto & a = resolveData.Get<Resolve>();
                auto & b = other.resolveData.Get<Resolve>();

                return a.peerId == b.peerId;
            }

            if (resolveData.Is<IpResolve>())
            {
                if (!other.resolveData.Is<IpResolve>())
                {
                    return false;
                }
                auto & a = resolveData.Get<IpResolve>();
                auto & b = other.resolveData.Get<IpResolve>();

                return a.hostName == b.hostName;
            }
            return false;
        }

        bool MatchesIpResolve(SerializedQNameIterator hostName) const
        {
            return resolveData.Is<IpResolve>() && (hostName == resolveData.Get<IpResolve>().hostName.Content());
        }
        bool Matches(const chip::PeerId & peer) const
        {
            return resolveData.Is<Resolve>() && (resolveData.Get<Resolve>().peerId == peer);
        }
        bool Matches(const chip::Dnssd::DiscoveredNodeData & data, chip::Dnssd::DiscoveryType type) const
        {
            if (!resolveData.Is<Browse>())
            {
                return false;
            }

            auto & browse = resolveData.Get<Browse>();

            if (browse.type != type)
            {
                return false;
            }
            auto & nodeData = data.Get<chip::Dnssd::CommissionNodeData>();

            switch (browse.filter.type)
            {
            case chip::Dnssd::DiscoveryFilterType::kNone:
                return true;
            case chip::Dnssd::DiscoveryFilterType::kShortDiscriminator:
                return browse.filter.code == static_cast<uint64_t>((nodeData.longDiscriminator >> 8) & 0x0F);
            case chip::Dnssd::DiscoveryFilterType::kLongDiscriminator:
                return browse.filter.code == nodeData.longDiscriminator;
            case chip::Dnssd::DiscoveryFilterType::kVendorId:
                return browse.filter.code == nodeData.vendorId;
            case chip::Dnssd::DiscoveryFilterType::kDeviceType:
                return browse.filter.code == nodeData.deviceType;
            case chip::Dnssd::DiscoveryFilterType::kCommissioningMode:
                return browse.filter.code == nodeData.commissioningMode;
            case chip::Dnssd::DiscoveryFilterType::kInstanceName:
                return strncmp(browse.filter.instanceName, nodeData.instanceName,
                               chip::Dnssd::Commission::kInstanceNameMaxLength + 1) == 0;
            case chip::Dnssd::DiscoveryFilterType::kCommissioner:
            case chip::Dnssd::DiscoveryFilterType::kCompressedFabricId:
            default:
                // These are for other discovery types.
                return false;
            }
        }

        bool IsEmpty() const { return !resolveData.Valid(); }
        bool IsResolve() const { return resolveData.Is<Resolve>(); }
        bool IsBrowse() const { return resolveData.Is<Browse>(); }
        bool IsIpResolve() const { return resolveData.Is<IpResolve>(); }
        void Clear() { resolveData = DataType(); }

        // Called when this scheduled attempt will replace an existing scheduled
        // attempt, because either they match or we have run out of attempt
        // slots.  When this happens, we want to propagate the consumer count
        // from the existing attempt to the new one, if they're matching resolve
        // attempts.
        void WillCoalesceWith(const ScheduledAttempt & existing)
        {
            if (!IsResolve())
            {
                // Consumer count is only tracked for resolve requests
                return;
            }

            if (!existing.Matches(*this))
            {
                // Out of attempt slots, so just dropping the existing attempt.
                return;
            }

            // Adding another consumer to the same query. Propagate along the
            // consumer count to our new attempt, which will replace the
            // existing one.
            resolveData.Get<Resolve>().consumerCount = existing.resolveData.Get<Resolve>().consumerCount + 1;
        }

        void ConsumerRemoved()
        {
            if (!IsResolve())
            {
                return;
            }

            auto & count = resolveData.Get<Resolve>().consumerCount;
            if (count > 0)
            {
                --count;
            }

            if (count == 0)
            {
                Clear();
            }
        }

        const Browse & BrowseData() const { return resolveData.Get<Browse>(); }
        const Resolve & ResolveData() const { return resolveData.Get<Resolve>(); }
        const IpResolve & IpResolveData() const { return resolveData.Get<IpResolve>(); }

        using DataType = chip::Variant<Browse, Resolve, IpResolve>;

        DataType resolveData;

        // First packet send is marked separately: minMDNS logic can choose
        // to first send a unicast query followed by a multicast one.
        bool firstSend = false;
    };

    ActiveResolveAttempts(chip::System::Clock::ClockBase * clock) : mClock(clock) { Reset(); }

    /// Clear out the internal queue
    void Reset();

    /// Mark a resolution as a success, removing it from the internal list
    void Complete(const chip::PeerId & peerId);
    void CompleteIpResolution(SerializedQNameIterator targetHostName);

    /// Mark all browse-type scheduled attemptes as a success, removing them
    /// from the internal list.
    CHIP_ERROR CompleteAllBrowses();

    /// Note that resolve attempts for the given peer id now have one fewer
    /// consumer.
    void NodeIdResolutionNoLongerNeeded(const chip::PeerId & peerId);

    /// Mark that a resolution is pending, adding it to the internal list
    ///
    /// Once this complete, this peer id will be returned immediately
    /// by NextScheduled (potentially with others as well)
    void MarkPending(const chip::PeerId & peerId);
    void MarkPending(const chip::Dnssd::DiscoveryFilter & filter, const chip::Dnssd::DiscoveryType type);
    void MarkPending(ScheduledAttempt::IpResolve && resolve);

    // Get minimum time until the next pending reply is required.
    //
    // Returns missing if no actively tracked elements exist.
    std::optional<chip::System::Clock::Timeout> GetTimeUntilNextExpectedResponse() const;

    // Get the peer Id that needs scheduling for a query
    //
    // Assumes that the resolution is being sent and will apply internal
    // query logic. This means:
    //  - internal tracking of 'next due time' will updated as 'request sent
    //    now'
    //  - there is NO sorting implied by this call. Returned value will be
    //    any peer that needs a new request sent
    std::optional<ScheduledAttempt> NextScheduled();

    /// Check if any of the pending queries are for the given host name for
    /// IP resolution.
    bool IsWaitingForIpResolutionFor(SerializedQNameIterator hostName) const;

    /// Determines if address resolution for the given peer ID is required
    ///
    /// IP Addresses are required for active operational discovery of specific peers
    /// or if an active browse is being performed.
    bool ShouldResolveIpAddress(chip::PeerId peerId) const;

    /// Check if a browse operation is active for the given discovery type
    bool HasBrowseFor(chip::Dnssd::DiscoveryType type) const;

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
    void MarkPending(ScheduledAttempt && attempt);
    chip::System::Clock::ClockBase * mClock;
    RetryEntry mRetryQueue[kRetryQueueSize];
};

} // namespace Minimal
} // namespace mdns
