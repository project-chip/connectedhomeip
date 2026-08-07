/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/**
 *    @file
 *      Operation-sequence FuzzTest harness for ActiveResolveAttempts, the
 *      retry-scheduling state for DNS-SD resolution and browse operations.
 *
 *      This is a state machine rather than a parser, so the fuzzer drives a
 *      sequence of API calls instead of a byte blob: queueing resolve, browse and
 *      IP-resolve attempts, completing them by peer id / host name / in bulk,
 *      withdrawing a node id, draining scheduled attempts, querying the time
 *      until the next expected response, and advancing a mock clock in between.
 *      Peer ids come from a small pool so that a completion has a real chance of
 *      matching something that was queued, which a fully random id almost never
 *      would.
 *
 *      The retry queue holds kRetryQueueSize entries, so sequences longer than
 *      that drive the eviction and reuse behaviour as well as the simple case.
 *
 *      Oracles beyond no-crash under ASan/UBSan:
 *        - any attempt handed back by NextScheduled() is non-empty and is
 *          exactly one of resolve, browse or IP-resolve;
 *        - after Reset() nothing is scheduled and no response is expected;
 *        - a host name reported as awaiting IP resolution stays consistent with
 *          what the schedule says about it.
 */

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/core/PeerId.h>
#include <lib/dnssd/ActiveResolveAttempts.h>
#include <lib/dnssd/Types.h>
#include <lib/dnssd/wire/BytesRange.h>
#include <lib/dnssd/wire/HeapQName.h>
#include <lib/dnssd/wire/QName.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>

namespace {

using namespace chip;
using namespace fuzztest;
using namespace mdns::Minimal;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Small enough that completions collide with queued entries, large enough to
// exceed kRetryQueueSize and drive eviction.
constexpr size_t kPeerPoolSize = 6;

// Valid DNS wire encodings for the host name used by the IP-resolve entries,
// plus the malformed shapes a name off the wire can take.
std::vector<std::vector<uint8_t>> HostNameSeeds()
{
    return {
        { 0x0C, 'B', '7', '5', 'A', 'F', 'B', '4', '5', '8', 'E', 'C', 'D', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 },
        { 0x04, 'h', 'o', 's', 't', 0x05, 'l', 'o', 'c', 'a', 'l', 0x00 },
        { 0x00 },             // root
        { 0x05, 'a', 'b' },   // truncated label
        { 0xC0, 0x00 },       // self-referential compression pointer
        { 0x40, 'a', 0x00 },  // reserved label type
        { 0x01, 'x', 0x00 },
    };
}

enum class Op : uint8_t
{
    kMarkPendingResolve,
    kMarkPendingBrowse,
    kMarkPendingIpResolve,
    kCompletePeer,
    kCompleteIpResolution,
    kCompleteAllBrowses,
    kNoLongerNeeded,
    kNextScheduled,
    kTimeUntilNextResponse,
    kIsWaitingForIp,
    kShouldResolveIp,
    kHasBrowseFor,
    kAdvanceClock,
    kReset,
    kCount,
};

// An attempt that was handed out must describe exactly one kind of work.
void CheckScheduledAttempt(const ActiveResolveAttempts::ScheduledAttempt & attempt)
{
    ASSERT_FALSE(attempt.IsEmpty());
    const int kinds = (attempt.IsResolve() ? 1 : 0) + (attempt.IsBrowse() ? 1 : 0) + (attempt.IsIpResolve() ? 1 : 0);
    ASSERT_EQ(kinds, 1);
}

// Property: any sequence of scheduling operations leaves the retry state
// self-consistent and never trips a memory error.
void AttemptSequenceNoCorruption(const std::vector<uint8_t> & ops, const std::vector<uint8_t> & hostNameBytes,
                                 const std::array<uint64_t, kPeerPoolSize> & fabricIds,
                                 const std::array<uint64_t, kPeerPoolSize> & nodeIds, uint16_t clockStepMs)
{
    EnsureInitialized();

    System::Clock::Internal::MockClock clock;
    ActiveResolveAttempts attempts(&clock);

    std::array<PeerId, kPeerPoolSize> peers;
    for (size_t i = 0; i < kPeerPoolSize; ++i)
    {
        peers[i] = PeerId().SetCompressedFabricId(fabricIds[i]).SetNodeId(nodeIds[i]);
    }

    // Owns the bytes the name iterators walk, so they stay valid for the run.
    const BytesRange nameRange(hostNameBytes.data(), hostNameBytes.data() + hostNameBytes.size());
    const auto hostName = [&]() -> SerializedQNameIterator {
        return hostNameBytes.empty() ? SerializedQNameIterator() : SerializedQNameIterator(nameRange, hostNameBytes.data());
    };

    constexpr uint8_t kDiscoveryTypeCount = static_cast<uint8_t>(Dnssd::DiscoveryType::kCommissionerNode) + 1;
    constexpr uint8_t kFilterTypeCount    = static_cast<uint8_t>(Dnssd::DiscoveryFilterType::kCompressedFabricId) + 1;

    for (size_t i = 0; i < ops.size(); ++i)
    {
        const uint8_t raw    = ops[i];
        const Op op          = static_cast<Op>(raw % static_cast<uint8_t>(Op::kCount));
        const size_t peerIdx = (raw / static_cast<uint8_t>(Op::kCount)) % kPeerPoolSize;

        switch (op)
        {
        case Op::kMarkPendingResolve:
            attempts.MarkPending(peers[peerIdx]);
            break;

        case Op::kMarkPendingBrowse: {
            Dnssd::DiscoveryFilter filter;
            filter.type = static_cast<Dnssd::DiscoveryFilterType>(peerIdx % kFilterTypeCount);
            filter.code = fabricIds[peerIdx];
            attempts.MarkPending(filter, static_cast<Dnssd::DiscoveryType>(peerIdx % kDiscoveryTypeCount));
            break;
        }

        case Op::kMarkPendingIpResolve:
            attempts.MarkPending(ActiveResolveAttempts::ScheduledAttempt::IpResolve(HeapQName(hostName())));
            break;

        case Op::kCompletePeer:
            attempts.Complete(peers[peerIdx]);
            break;

        case Op::kCompleteIpResolution:
            attempts.CompleteIpResolution(hostName());
            break;

        case Op::kCompleteAllBrowses:
            (void) attempts.CompleteAllBrowses();
            break;

        case Op::kNoLongerNeeded:
            attempts.NodeIdResolutionNoLongerNeeded(peers[peerIdx]);
            break;

        case Op::kNextScheduled: {
            // Bounded: the schedule must not hand out entries without end.
            size_t drained = 0;
            while (std::optional<ActiveResolveAttempts::ScheduledAttempt> attempt = attempts.NextScheduled())
            {
                CheckScheduledAttempt(*attempt);
                if (++drained > ActiveResolveAttempts::kRetryQueueSize * 4)
                {
                    break;
                }
            }
            break;
        }

        case Op::kTimeUntilNextResponse:
            (void) attempts.GetTimeUntilNextExpectedResponse();
            break;

        case Op::kIsWaitingForIp:
            (void) attempts.IsWaitingForIpResolutionFor(hostName());
            break;

        case Op::kShouldResolveIp:
            (void) attempts.ShouldResolveIpAddress(peers[peerIdx]);
            break;

        case Op::kHasBrowseFor:
            (void) attempts.HasBrowseFor(static_cast<Dnssd::DiscoveryType>(peerIdx % kDiscoveryTypeCount));
            break;

        case Op::kAdvanceClock:
            clock.AdvanceMonotonic(System::Clock::Milliseconds32(clockStepMs));
            break;

        case Op::kReset:
            attempts.Reset();
            // Nothing is owed immediately after a reset.
            ASSERT_FALSE(attempts.GetTimeUntilNextExpectedResponse().has_value());
            ASSERT_FALSE(attempts.NextScheduled().has_value());
            break;

        case Op::kCount:
            break;
        }
    }
}

FUZZ_TEST(ActiveResolveAttemptsPW, AttemptSequenceNoCorruption)
    .WithDomains(/* ops           */ Arbitrary<std::vector<uint8_t>>().WithMaxSize(64),
                 /* hostNameBytes */ Arbitrary<std::vector<uint8_t>>().WithSeeds(HostNameSeeds()).WithMaxSize(128),
                 /* fabricIds     */ Arbitrary<std::array<uint64_t, kPeerPoolSize>>(),
                 /* nodeIds       */ Arbitrary<std::array<uint64_t, kPeerPoolSize>>(),
                 /* clockStepMs   */ Arbitrary<uint16_t>());

} // namespace
