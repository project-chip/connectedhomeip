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

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/ActiveResolveAttempts.h>

namespace {

using namespace chip;
using namespace chip::System::Clock::Literals;
using chip::System::Clock::Timeout;
using mdns::Minimal::ActiveResolveAttempts;

PeerId MakePeerId(NodeId nodeId)
{
    PeerId peerId;
    return peerId.SetNodeId(nodeId).SetCompressedFabricId(123);
}

std::optional<ActiveResolveAttempts::ScheduledAttempt> ScheduledPeer(NodeId id, bool first)
{
    return std::make_optional<ActiveResolveAttempts::ScheduledAttempt>(
        ActiveResolveAttempts::ScheduledAttempt(MakePeerId(id), first));
}

std::optional<ActiveResolveAttempts::ScheduledAttempt> ScheduledBrowse(const Dnssd::DiscoveryFilter & filter,
                                                                       const Dnssd::DiscoveryType type, bool first)
{
    return std::make_optional<ActiveResolveAttempts::ScheduledAttempt>(
        ActiveResolveAttempts::ScheduledAttempt(filter, type, first));
}

TEST(TestActiveResolveAttempts, TestSinglePeerAddRemove)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(1234_ms32);

    // Starting up, no scheduled peers are expected
    EXPECT_FALSE(attempts.GetTimeUntilNextExpectedResponse().has_value());
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // Adding a single peer should result in it being scheduled

    attempts.MarkPending(MakePeerId(1));

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // one Next schedule is called, expect to have a delay of 1000 ms
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));
    mockClock.AdvanceMonotonic(500_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(500_ms32));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // past due date: timeout should be 0
    mockClock.AdvanceMonotonic(800_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // one Next schedule is called, expect to have a delay of 2000 ms
    // sincve the timeout doubles every time
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(2000_ms32));
    mockClock.AdvanceMonotonic(100_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1900_ms32));

    // once complete, nothing to schedule
    attempts.Complete(MakePeerId(1));
    EXPECT_FALSE(attempts.GetTimeUntilNextExpectedResponse().has_value());
    EXPECT_FALSE(attempts.NextScheduled().has_value());
}

TEST(TestActiveResolveAttempts, TestSingleBrowseAddRemove)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kLongDiscriminator, 1234);
    Dnssd::DiscoveryType type = Dnssd::DiscoveryType::kCommissionableNode;

    mockClock.AdvanceMonotonic(1234_ms32);

    // Starting up, no scheduled peers are expected
    EXPECT_FALSE(attempts.GetTimeUntilNextExpectedResponse().has_value());
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // Adding a single attempt should result in it being scheduled
    attempts.MarkPending(filter, type);

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // one Next schedule is called, expect to have a delay of 1000 ms
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));
    mockClock.AdvanceMonotonic(500_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(500_ms32));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // past due date: timeout should be 0
    mockClock.AdvanceMonotonic(800_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // one Next schedule is called, expect to have a delay of 2000 ms
    // sincve the timeout doubles every time
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(2000_ms32));
    mockClock.AdvanceMonotonic(100_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1900_ms32));

    // once complete, nothing to schedule
    attempts.CompleteAllBrowses();
    EXPECT_FALSE(attempts.GetTimeUntilNextExpectedResponse().has_value());
    EXPECT_FALSE(attempts.NextScheduled().has_value());
}

TEST(TestActiveResolveAttempts, TestRescheduleSamePeerId)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(112233_ms32);

    attempts.MarkPending(MakePeerId(1));

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // one Next schedule is called, expect to have a delay of 1000 ms
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));

    // 2nd try goes to 2 seconds (once at least 1 second passes)
    mockClock.AdvanceMonotonic(1234_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(2000_ms32));

    // reschedule starts fresh
    attempts.MarkPending(MakePeerId(1));

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));
}

TEST(TestActiveResolveAttempts, TestRescheduleSameFilter)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);
    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kLongDiscriminator, 1234);
    Dnssd::DiscoveryType type = Dnssd::DiscoveryType::kCommissionableNode;

    mockClock.AdvanceMonotonic(112233_ms32);

    attempts.MarkPending(filter, type);

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // one Next schedule is called, expect to have a delay of 1000 ms
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));

    // 2nd try goes to 2 seconds (once at least 1 second passes)
    mockClock.AdvanceMonotonic(1234_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(2000_ms32));

    // reschedule starts fresh
    attempts.MarkPending(filter, type);

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));
}

TEST(TestActiveResolveAttempts, TestLRU)
{
    // validates that the LRU logic is working
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(334455_ms32);

    // add a single very old peer
    attempts.MarkPending(MakePeerId(9999));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(9999, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    mockClock.AdvanceMonotonic(1000_ms32);
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(9999, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    mockClock.AdvanceMonotonic(2000_ms32);
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(9999, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // at this point, peer 9999 has a delay of 4 seconds. Fill up the rest of the table

    for (uint32_t i = 1; i < mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize; i++)
    {
        attempts.MarkPending(MakePeerId(i));
        mockClock.AdvanceMonotonic(1_ms32);

        EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(i, true));
        EXPECT_FALSE(attempts.NextScheduled().has_value());
    }

    // +2 because: 1 element skipped, one element is the "current" that has a delay of 1000ms
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(),
              std::make_optional<System::Clock::Timeout>(
                  System::Clock::Milliseconds32(1000 - mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize + 2)));

    // add another element - this should overwrite peer 9999
    attempts.MarkPending(MakePeerId(mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize));
    mockClock.AdvanceMonotonic(32_s16);

    for (std::optional<ActiveResolveAttempts::ScheduledAttempt> s = attempts.NextScheduled(); s.has_value();
         s                                                        = attempts.NextScheduled())
    {
        EXPECT_NE(s->ResolveData().peerId.GetNodeId(), 9999u);
    }

    // Still have active pending items (queue is full)
    EXPECT_TRUE(attempts.GetTimeUntilNextExpectedResponse().has_value());

    // expire all of them. Since we double timeout every expiry, we expect a
    // few iteratios to be able to expire the entire queue
    constexpr int kMaxIterations = 10;

    int i = 0;
    for (; i < kMaxIterations; i++)
    {
        std::optional<System::Clock::Timeout> ms = attempts.GetTimeUntilNextExpectedResponse();
        if (!ms.has_value())
        {
            break;
        }

        mockClock.AdvanceMonotonic(*ms);

        std::optional<ActiveResolveAttempts::ScheduledAttempt> s = attempts.NextScheduled();
        while (s.has_value())
        {
            // NOLINTNEXTLINE(bugprone-unchecked-optional-access): this is checked in the while loop
            EXPECT_NE(s->ResolveData().peerId.GetNodeId(), 9999u);
            s = attempts.NextScheduled();
        }
    }
    EXPECT_LT(i, kMaxIterations);
}

TEST(TestActiveResolveAttempts, TestNextPeerOrdering)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(123321_ms32);

    // add a single peer that will be resolved quickly
    attempts.MarkPending(MakePeerId(1));

    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));
    mockClock.AdvanceMonotonic(20_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(980_ms32));

    // expect peerid to be resolve within 1 second from now
    attempts.MarkPending(MakePeerId(2));

    // mock that we are querying 2 as well
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(2, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    mockClock.AdvanceMonotonic(80_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(900_ms32));

    // Peer 1 is done, now peer2 should be pending (in 980ms)
    attempts.Complete(MakePeerId(1));
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(920_ms32));
    mockClock.AdvanceMonotonic(20_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(900_ms32));

    // Once peer 3 is added, queue should be
    //  - 900 ms until peer id 2 is pending
    //  - 1000 ms until peer id 3 is pending
    attempts.MarkPending(MakePeerId(3));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(3, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(900_ms32));

    // After the clock advance
    //  - 400 ms until peer id 2 is pending
    //  - 500 ms until peer id 3 is pending
    mockClock.AdvanceMonotonic(500_ms32);

    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(400_ms32));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // advancing the clock 'too long' will return both other entries, in  reverse order due to how
    // the internal cache is built
    mockClock.AdvanceMonotonic(500_ms32);
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(3, false));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(2, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());
}

TEST(TestActiveResolveAttempts, TestCombination)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    Dnssd::DiscoveryFilter filter(Dnssd::DiscoveryFilterType::kLongDiscriminator, 1234);
    Dnssd::DiscoveryType type = Dnssd::DiscoveryType::kCommissionableNode;

    // Schedule a Resolve
    attempts.MarkPending(MakePeerId(1));
    // 20ms later, schedule a browse
    mockClock.AdvanceMonotonic(20_ms32);
    attempts.MarkPending(filter, type);

    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, true));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // At this point, both should reset, so we're back to 1000ms
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(1000_ms32));

    // We used 20 ms, so the next time for the peer and resolve should be 980 ms
    mockClock.AdvanceMonotonic(20_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(980_ms32));

    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // Add a second Peer
    mockClock.AdvanceMonotonic(20_ms32);
    attempts.MarkPending(MakePeerId(2));
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));

    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(2, true));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // Advance to the retry time of peer 1 and the resolve
    mockClock.AdvanceMonotonic(960_ms32);
    EXPECT_EQ(attempts.GetTimeUntilNextExpectedResponse(), std::make_optional<Timeout>(0_ms32));

    EXPECT_EQ(attempts.NextScheduled(), ScheduledPeer(1, false));
    EXPECT_EQ(attempts.NextScheduled(), ScheduledBrowse(filter, type, false));
    EXPECT_FALSE(attempts.NextScheduled().has_value());

    // Complete all, we should see no more scheduled.
    attempts.Complete(MakePeerId(2));
    attempts.Complete(MakePeerId(1));
    attempts.CompleteAllBrowses();

    EXPECT_FALSE(attempts.GetTimeUntilNextExpectedResponse().has_value());
    EXPECT_FALSE(attempts.NextScheduled().has_value());
}
} // namespace
