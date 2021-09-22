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

const nlTest sTests[] = {
    NL_TEST_DEF("TestSinglePeerAddRemove", TestSinglePeerAddRemove),   //
    NL_TEST_DEF("TestRescheduleSamePeerId", TestRescheduleSamePeerId), //
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