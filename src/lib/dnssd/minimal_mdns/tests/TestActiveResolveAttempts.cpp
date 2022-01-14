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
#include <lib/dnssd/minimal_mdns/ActiveResolveAttempts.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::System::Clock::Literals;
using chip::System::Clock::Timeout;

PeerId MakePeerId(NodeId nodeId)
{
    PeerId peerId;
    return peerId.SetNodeId(nodeId).SetCompressedFabricId(123);
}

void TestSinglePeerAddRemove(nlTestSuite * inSuite, void * inContext)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(1234_ms32);

    // Starting up, no scheduled peers are expected
    NL_TEST_ASSERT(inSuite, !attempts.GetTimeUntilNextExpectedResponse().HasValue());
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // Adding a single peer should result in it being scheduled

    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(0_ms32));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // one Next schedule is called, expect to have a delay of 1000 ms
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(1000_ms32));
    mockClock.AdvanceMonotonic(500_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(500_ms32));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // past due date: timeout should be 0
    mockClock.AdvanceMonotonic(800_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(0_ms32));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // one Next schedule is called, expect to have a delay of 2000 ms
    // sincve the timeout doubles every time
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(2000_ms32));
    mockClock.AdvanceMonotonic(100_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(1900_ms32));

    // once complete, nothing to schedule
    attempts.Complete(MakePeerId(1));
    NL_TEST_ASSERT(inSuite, !attempts.GetTimeUntilNextExpectedResponse().HasValue());
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
}

void TestRescheduleSamePeerId(nlTestSuite * inSuite, void * inContext)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(112233_ms32);

    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(0_ms32));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // one Next schedule is called, expect to have a delay of 1000 ms
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(1000_ms32));

    // 2nd try goes to 2 seconds (once at least 1 second passes)
    mockClock.AdvanceMonotonic(1234_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(0_ms32));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(2000_ms32));

    // reschedule starts fresh
    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(0_ms32));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(1000_ms32));
}

void TestLRU(nlTestSuite * inSuite, void * inContext)
{
    // validates that the LRU logic is working
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(334455_ms32);

    // add a single very old peer
    attempts.MarkPending(MakePeerId(9999));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(9999)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    mockClock.AdvanceMonotonic(1000_ms32);
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(9999)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    mockClock.AdvanceMonotonic(2000_ms32);
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(9999)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // at this point, peer 9999 has a delay of 4 seconds. Fill up the rest of the table

    for (uint32_t i = 1; i < mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize; i++)
    {
        attempts.MarkPending(MakePeerId(i));
        mockClock.AdvanceMonotonic(1_ms32);

        NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(i)));
        NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    }

    // +2 because: 1 element skipped, one element is the "current" that has a delay of 1000ms
    NL_TEST_ASSERT(inSuite,
                   attempts.GetTimeUntilNextExpectedResponse() ==
                       Optional<System::Clock::Timeout>::Value(
                           System::Clock::Milliseconds32(1000 - mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize + 2)));

    // add another element - this should overwrite peer 9999
    attempts.MarkPending(MakePeerId(mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize));
    mockClock.AdvanceMonotonic(32_s16);

    for (Optional<PeerId> peerId = attempts.NextScheduledPeer(); peerId.HasValue(); peerId = attempts.NextScheduledPeer())
    {
        NL_TEST_ASSERT(inSuite, peerId.Value().GetNodeId() != 9999);
    }

    // Still have active pending items (queue is full)
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse().HasValue());

    // expire all of them. Since we double timeout every expiry, we expect a
    // few iteratios to be able to expire the entire queue
    constexpr int kMaxIterations = 10;

    int i = 0;
    for (; i < kMaxIterations; i++)
    {
        Optional<System::Clock::Timeout> ms = attempts.GetTimeUntilNextExpectedResponse();
        if (!ms.HasValue())
        {
            break;
        }

        mockClock.AdvanceMonotonic(ms.Value());

        Optional<PeerId> peerId = attempts.NextScheduledPeer();
        while (peerId.HasValue())
        {
            NL_TEST_ASSERT(inSuite, peerId.Value().GetNodeId() != 9999);
            peerId = attempts.NextScheduledPeer();
        }
    }
    NL_TEST_ASSERT(inSuite, i < kMaxIterations);
}

void TestNextPeerOrdering(nlTestSuite * inSuite, void * inContext)
{
    System::Clock::Internal::MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMonotonic(123321_ms32);

    // add a single peer that will be resolved quickly
    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(1000_ms32));
    mockClock.AdvanceMonotonic(20_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(980_ms32));

    // expect peerid to be resolve within 1 second from now
    attempts.MarkPending(MakePeerId(2));

    // mock that we are querying 2 as well
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(2)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    mockClock.AdvanceMonotonic(80_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(900_ms32));

    // Peer 1 is done, now peer2 should be pending (in 980ms)
    attempts.Complete(MakePeerId(1));
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(920_ms32));
    mockClock.AdvanceMonotonic(20_ms32);
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(900_ms32));

    // Once peer 3 is added, queue should be
    //  - 900 ms until peer id 2 is pending
    //  - 1000 ms until peer id 3 is pending
    attempts.MarkPending(MakePeerId(3));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(3)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(900_ms32));

    // After the clock advance
    //  - 400 ms until peer id 2 is pending
    //  - 500 ms until peer id 3 is pending
    mockClock.AdvanceMonotonic(500_ms32);

    NL_TEST_ASSERT(inSuite, attempts.GetTimeUntilNextExpectedResponse() == Optional<Timeout>(400_ms32));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // advancing the clock 'too long' will return both other entries, in  reverse order due to how
    // the internal cache is built
    mockClock.AdvanceMonotonic(500_ms32);
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(3)));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(2)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestSinglePeerAddRemove", TestSinglePeerAddRemove),   //
    NL_TEST_DEF("TestRescheduleSamePeerId", TestRescheduleSamePeerId), //
    NL_TEST_DEF("TestLRU", TestLRU),                                   //
    NL_TEST_DEF("TestNextPeerOrdering", TestNextPeerOrdering),         //
    NL_TEST_SENTINEL()                                                 //
};

} // namespace

int TestActiveResolveAttempts(void)
{
    nlTestSuite theSuite = { "ActiveResolveAttempts", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestActiveResolveAttempts)
