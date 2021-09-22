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
#include <lib/mdns/minimal/ActiveResolveAttempts.h>

#include <lib/support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

class MockClock : public System::ClockBase
{
public:
    MonotonicMicroseconds GetMonotonicMicroseconds() override { return mUsec; }
    MonotonicMilliseconds GetMonotonicMilliseconds() override { return mUsec / 1000; }

    void AdvanceMs(MonotonicMilliseconds ms) { mUsec += ms * 1000L; }
    void AdvanceSec(uint32_t s) { AdvanceMs(s * 1000); }

private:
    MonotonicMicroseconds mUsec = 0;
};

PeerId MakePeerId(NodeId nodeId)
{
    PeerId peerId;
    return peerId.SetNodeId(nodeId).SetCompressedFabricId(123);
}

void TestSinglePeerAddRemove(nlTestSuite * inSuite, void * inContext)
{
    MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMs(1234);

    // Starting up, no scheduled peers are expected
    NL_TEST_ASSERT(inSuite, !attempts.GetMsUntilNextExpectedResponse().HasValue());
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // Adding a single peer should result in it being scheduled

    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(0));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // one Next schedule is called, expect to have a delay of 1000 ms
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(1000));
    mockClock.AdvanceMs(500);
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(500));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // past due date: timeout should be 0
    mockClock.AdvanceMs(800);
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(0));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // one Next schedule is called, expect to have a delay of 2000 ms
    // sincve the timeout doubles every time
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(2000));
    mockClock.AdvanceMs(100);
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(1900));

    // once complete, nothing to schedule
    attempts.Complete(MakePeerId(1));
    NL_TEST_ASSERT(inSuite, !attempts.GetMsUntilNextExpectedResponse().HasValue());
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
}

void TestRescheduleSamePeerId(nlTestSuite * inSuite, void * inContext)
{
    MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMs(112233);

    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(0));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // one Next schedule is called, expect to have a delay of 1000 ms
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(1000));

    // 2nd try goes to 2 seconds (once at least 1 second passes)
    mockClock.AdvanceMs(1234);
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(0));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(2000));

    // reschedule starts fresh
    attempts.MarkPending(MakePeerId(1));

    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(0));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(1)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse() == Optional<uint32_t>::Value(1000));
}

void TestLRU(nlTestSuite * inSuite, void * inContext)
{
    // validates that the LRU logic is working
    MockClock mockClock;
    mdns::Minimal::ActiveResolveAttempts attempts(&mockClock);

    mockClock.AdvanceMs(334455);

    // add a single very old peer
    attempts.MarkPending(MakePeerId(9999));
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(9999)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    mockClock.AdvanceMs(1000);
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(9999)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    mockClock.AdvanceMs(2000);
    NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(9999)));
    NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());

    // at this point, peer 9999 has a delay of 4 seconds. Fill up the rest of the table

    for (uint32_t i = 1; i < mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize; i++)
    {
        attempts.MarkPending(MakePeerId(i));
        mockClock.AdvanceMs(1);

        NL_TEST_ASSERT(inSuite, attempts.NextScheduledPeer() == Optional<PeerId>::Value(MakePeerId(i)));
        NL_TEST_ASSERT(inSuite, !attempts.NextScheduledPeer().HasValue());
    }

    // +2 because: 1 element skipped, one element is the "current" that has a delay of 1000ms
    NL_TEST_ASSERT(
        inSuite,
        attempts.GetMsUntilNextExpectedResponse() ==
            Optional<uint32_t>::Value(static_cast<uint32_t>(1000 - mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize + 2)));

    // add another element - this should overwrite peer 9999
    attempts.MarkPending(MakePeerId(mdns::Minimal::ActiveResolveAttempts::kRetryQueueSize));
    mockClock.AdvanceSec(32);

    for (Optional<PeerId> peerId = attempts.NextScheduledPeer(); peerId.HasValue(); peerId = attempts.NextScheduledPeer())
    {
        NL_TEST_ASSERT(inSuite, peerId.Value().GetNodeId() != 9999);
    }

    // Still have active pending items (queue is full)
    NL_TEST_ASSERT(inSuite, attempts.GetMsUntilNextExpectedResponse().HasValue());

    // expire all of them. Since we double timeout every expiry, we expect a
    // few iteratios to be able to expire the entire queue
    constexpr int kMaxIterations = 10;

    int i = 0;
    for (; i < kMaxIterations; i++)
    {
        Optional<uint32_t> ms = attempts.GetMsUntilNextExpectedResponse();
        if (!ms.HasValue())
        {
            break;
        }

        mockClock.AdvanceMs(ms.Value());

        Optional<PeerId> peerId = attempts.NextScheduledPeer();
        while (peerId.HasValue())
        {
            NL_TEST_ASSERT(inSuite, peerId.Value().GetNodeId() != 9999);
            peerId = attempts.NextScheduledPeer();
        }
    }
    NL_TEST_ASSERT(inSuite, i < kMaxIterations);
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestSinglePeerAddRemove", TestSinglePeerAddRemove),   //
    NL_TEST_DEF("TestRescheduleSamePeerId", TestRescheduleSamePeerId), //
    NL_TEST_DEF("TestLRU", TestLRU),                                   //
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
