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

#include <pw_unit_test/framework.h>

#include <chrono>
#include <thread>

#include <controller/python/matter/case_capture/CompletedCASEHandshakeQueue.h>

namespace {

using chip::python::CompletedCASEHandshakeQueue;

// Waits long enough to prove a blocked consumer is genuinely waiting, without making the suite
// slow. Only used where the outcome does not depend on the exact figure.
constexpr uint32_t kShortWaitMs = 50;

// A record is only identified here by its exchange id, which is enough to tell one from another.
PychipCASEHandshakeMetricsRecord RecordFor(uint16_t exchangeId)
{
    PychipCASEHandshakeMetricsRecord record{};
    record.exchangeId = exchangeId;
    return record;
}

class TestCompletedCASEHandshakeQueue : public ::testing::Test
{
protected:
    CompletedCASEHandshakeQueue mQueue;
};

TEST_F(TestCompletedCASEHandshakeQueue, PublishIsIgnoredBeforeOpen)
{
    // Nothing has asked for notifications yet, so a handshake concluding must not be retained.
    mQueue.Publish(RecordFor(1));

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    EXPECT_FALSE(mQueue.Wait(out, kShortWaitMs, dropped));
    EXPECT_EQ(dropped, 0u);
}

TEST_F(TestCompletedCASEHandshakeQueue, PublishedRecordComesBackOutOfWait)
{
    mQueue.Open(4);
    mQueue.Publish(RecordFor(7));

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    ASSERT_TRUE(mQueue.Wait(out, kShortWaitMs, dropped));
    EXPECT_EQ(out.exchangeId, 7);
    EXPECT_EQ(dropped, 0u);
}

TEST_F(TestCompletedCASEHandshakeQueue, WaitReportsNothingWhenEmpty)
{
    mQueue.Open(4);

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    EXPECT_FALSE(mQueue.Wait(out, kShortWaitMs, dropped));
}

TEST_F(TestCompletedCASEHandshakeQueue, RecordsComeBackInTheOrderPublished)
{
    mQueue.Open(4);
    mQueue.Publish(RecordFor(1));
    mQueue.Publish(RecordFor(2));
    mQueue.Publish(RecordFor(3));

    constexpr uint16_t kPublishedOrder[] = { 1, 2, 3 };
    for (uint16_t expected : kPublishedOrder)
    {
        PychipCASEHandshakeMetricsRecord out{};
        uint32_t dropped = 0;
        ASSERT_TRUE(mQueue.Wait(out, kShortWaitMs, dropped));
        EXPECT_EQ(out.exchangeId, expected);
    }
}

TEST_F(TestCompletedCASEHandshakeQueue, FullQueueDropsTheOldestAndCountsIt)
{
    // Two more than it can hold, so the two oldest have to go. Dropping the oldest rather than
    // the newest is deliberate: a consumer that has fallen behind is better served by recent
    // handshakes than by the start of the backlog.
    mQueue.Open(2);
    mQueue.Publish(RecordFor(1));
    mQueue.Publish(RecordFor(2));
    mQueue.Publish(RecordFor(3));
    mQueue.Publish(RecordFor(4));

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    ASSERT_TRUE(mQueue.Wait(out, kShortWaitMs, dropped));
    EXPECT_EQ(out.exchangeId, 3);
    EXPECT_EQ(dropped, 2u);

    ASSERT_TRUE(mQueue.Wait(out, kShortWaitMs, dropped));
    EXPECT_EQ(out.exchangeId, 4);
    EXPECT_EQ(dropped, 2u);

    // The count is a running total, so it stays put once the backlog has drained.
    EXPECT_FALSE(mQueue.Wait(out, kShortWaitMs, dropped));
    EXPECT_EQ(dropped, 2u);
}

TEST_F(TestCompletedCASEHandshakeQueue, CloseDiscardsWhatWasHeld)
{
    mQueue.Open(4);
    mQueue.Publish(RecordFor(1));
    mQueue.Close();

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    EXPECT_FALSE(mQueue.Wait(out, kShortWaitMs, dropped));
}

TEST_F(TestCompletedCASEHandshakeQueue, OpenClearsWhatTheLastRunLeft)
{
    mQueue.Open(2);
    mQueue.Publish(RecordFor(1));
    mQueue.Publish(RecordFor(2));
    mQueue.Publish(RecordFor(3)); // Drops one, so the count is non-zero.

    mQueue.Open(4);

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    EXPECT_FALSE(mQueue.Wait(out, kShortWaitMs, dropped));
    EXPECT_EQ(dropped, 0u) << "a fresh run should not inherit the previous run's dropped count";
}

TEST_F(TestCompletedCASEHandshakeQueue, WaitingConsumerIsWokenByAPublish)
{
    mQueue.Open(4);

    // Published from another thread while this one is already parked in Wait, which is how the
    // real thing works: the CHIP event loop publishes and the delivery thread is woken.
    std::thread producer([this] {
        std::this_thread::sleep_for(std::chrono::milliseconds(kShortWaitMs / 2));
        mQueue.Publish(RecordFor(9));
    });

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped = 0;
    // Generous timeout, so a slow machine cannot turn a pass into a failure.
    const bool received = mQueue.Wait(out, kShortWaitMs * 20, dropped);
    producer.join();

    ASSERT_TRUE(received);
    EXPECT_EQ(out.exchangeId, 9);
}

TEST_F(TestCompletedCASEHandshakeQueue, CloseWakesAWaitingConsumer)
{
    mQueue.Open(4);

    // Closing has to release a parked consumer, or stopping notifications would hang on the join
    // that waits for the delivery thread to finish.
    std::thread closer([this] {
        std::this_thread::sleep_for(std::chrono::milliseconds(kShortWaitMs / 2));
        mQueue.Close();
    });

    PychipCASEHandshakeMetricsRecord out{};
    uint32_t dropped    = 0;
    const bool received = mQueue.Wait(out, kShortWaitMs * 20, dropped);
    closer.join();

    EXPECT_FALSE(received);
}

TEST_F(TestCompletedCASEHandshakeQueue, ConcurrentPublishersLoseNothing)
{
    constexpr int kPublishers      = 4;
    constexpr int kPerPublisher    = 25;
    constexpr uint32_t kRoomForAll = kPublishers * kPerPublisher;
    mQueue.Open(kRoomForAll);

    std::vector<std::thread> publishers;
    for (int publisher = 0; publisher < kPublishers; publisher++)
    {
        publishers.emplace_back([this, publisher] {
            for (int i = 0; i < kPerPublisher; i++)
            {
                mQueue.Publish(RecordFor(static_cast<uint16_t>(publisher * kPerPublisher + i)));
            }
        });
    }
    for (auto & publisher : publishers)
    {
        publisher.join();
    }

    uint32_t received = 0;
    uint32_t dropped  = 0;
    PychipCASEHandshakeMetricsRecord out{};
    while (mQueue.Wait(out, kShortWaitMs, dropped))
    {
        received++;
    }
    EXPECT_EQ(received, kRoomForAll);
    EXPECT_EQ(dropped, 0u);
}

} // namespace
