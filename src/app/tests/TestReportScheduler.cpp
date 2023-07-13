/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/InteractionModelEngine.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <app/tests/AppTestContext.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <nlunit-test.h>

namespace {

class TestContext : public chip::Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<TestContext *>(context);

        if (ctx->mEventCounter.Init(0) != CHIP_NO_ERROR)
        {
            return FAILURE;
        }

        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        chip::app::EventManagement::DestroyEventManagement();

        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;

        return SUCCESS;
    }

private:
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

class NullReadHandlerCallback : public chip::app::ReadHandler::ManagementCallback
{
public:
    void OnDone(chip::app::ReadHandler & apReadHandlerObj) override {}
    chip::app::ReadHandler::ApplicationCallback * GetAppCallback() override { return nullptr; }
};

} // namespace

namespace chip {
namespace app {
namespace reporting {

using InteractionModelEngine = InteractionModelEngine;
using ReportScheduler        = reporting::ReportScheduler;
using ReportSchedulerImpl    = reporting::ReportSchedulerImpl;
using ReadHandlerNode        = reporting::ReportScheduler::ReadHandlerNode;
using Milliseconds64         = System::Clock::Milliseconds64;

static const size_t kNumMaxReadHandlers = 16;

class TestTimerDelegate : public ReportScheduler::TimerDelegate
{
public:
    struct NodeTimeoutPair
    {
        ReadHandlerNode * node;
        System::Clock::Timeout timeout;
    };

    NodeTimeoutPair mPairArray[kNumMaxReadHandlers];
    size_t mPairArraySize                         = 0;
    System::Clock::Timestamp mMockSystemTimestamp = System::Clock::Milliseconds64(0);

    NodeTimeoutPair * FindPair(ReadHandlerNode * node, size_t & position)
    {
        for (size_t i = 0; i < mPairArraySize; i++)
        {
            if (mPairArray[i].node == node)
            {
                position = i;
                return &mPairArray[i];
            }
        }
        return nullptr;
    }

    CHIP_ERROR insertPair(ReadHandlerNode * node, System::Clock::Timeout timeout)
    {
        VerifyOrReturnError(mPairArraySize < kNumMaxReadHandlers, CHIP_ERROR_NO_MEMORY);
        mPairArray[mPairArraySize].node    = node;
        mPairArray[mPairArraySize].timeout = timeout;
        mPairArraySize++;

        return CHIP_NO_ERROR;
    }

    void removePair(ReadHandlerNode * node)
    {
        size_t position;
        NodeTimeoutPair * pair = FindPair(node, position);
        VerifyOrReturn(pair != nullptr);

        size_t nextPos = static_cast<size_t>(position + 1);
        size_t moveNum = static_cast<size_t>(mPairArraySize - nextPos);

        // Compress array after removal, if the removed position is not the last
        if (moveNum)
        {
            memmove(&mPairArray[position], &mPairArray[nextPos], sizeof(NodeTimeoutPair) * moveNum);
        }

        mPairArraySize--;
    }

    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        // Normaly we would call the callback here, thus scheduling an engine run, but we don't need it for this test as we simulate
        // all the callbacks related to report emissions. The actual callback should look like this:
        //
        // ReadHandlerNode * node = static_cast<ReadHandlerNode *>(aAppState);
        // node->RunCallback();
        ChipLogProgress(DataManagement, "Simluating engine run for Handler: %p", aAppState);
    }
    virtual CHIP_ERROR StartTimer(void * context, System::Clock::Timeout aTimeout) override
    {
        return insertPair(static_cast<ReadHandlerNode *>(context), aTimeout + mMockSystemTimestamp);
    }
    virtual void CancelTimer(void * context) override { removePair(static_cast<ReadHandlerNode *>(context)); }
    virtual bool IsTimerActive(void * context) override
    {
        size_t position;
        NodeTimeoutPair * pair = FindPair(static_cast<ReadHandlerNode *>(context), position);
        VerifyOrReturnValue(pair != nullptr, false);

        return pair->timeout > mMockSystemTimestamp;
    }

    virtual System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return mMockSystemTimestamp; }

    void SetMockSystemTimestamp(System::Clock::Timestamp aMockTimestamp) { mMockSystemTimestamp = aMockTimestamp; }

    // Increment the mock timestamp one milisecond at a time for a total of aTime miliseconds. Checks if
    void IncrementMockTimestamp(System::Clock::Milliseconds64 aTime)
    {
        mMockSystemTimestamp = mMockSystemTimestamp + aTime;
        for (size_t i = 0; i < mPairArraySize; i++)
        {
            if (mPairArray[i].timeout <= mMockSystemTimestamp)
            {
                TimerCallbackInterface(nullptr, mPairArray[i].node);
            }
        }
    }
};

TestTimerDelegate sTestTimerDelegate;
ReportSchedulerImpl sScheduler(&sTestTimerDelegate);

class TestReportScheduler
{
public:
    static void TestReadHandlerList(nlTestSuite * aSuite, void * aContext)
    {
        TestContext & ctx = *static_cast<TestContext *>(aContext);
        NullReadHandlerCallback nullCallback;
        // exchange context
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

        // Read handler pool
        ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

        // Initialize mock timestamp
        sTestTimerDelegate.SetMockSystemTimestamp(Milliseconds64(0));

        for (size_t i = 0; i < kNumMaxReadHandlers; i++)
        {
            ReadHandler * readHandler =
                readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
            NL_TEST_ASSERT(aSuite, nullptr != readHandler);
            VerifyOrReturn(nullptr != readHandler);
            NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler));
            NL_TEST_ASSERT(aSuite, nullptr != sScheduler.FindReadHandlerNode(readHandler));
        }

        NL_TEST_ASSERT(aSuite, readHandlerPool.Allocated() == kNumMaxReadHandlers);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers);
        NL_TEST_ASSERT(aSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);

        // Test unregister first ReadHandler
        ReadHandler * firstReadHandler = sScheduler.mReadHandlerList.begin()->GetReadHandler();
        sScheduler.UnregisterReadHandler(firstReadHandler);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers - 1);
        NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(firstReadHandler));

        // Test unregister middle ReadHandler
        auto iter = sScheduler.mReadHandlerList.begin();
        for (size_t i = 0; i < static_cast<size_t>(kNumMaxReadHandlers / 2); i++)
        {
            iter++;
        }
        ReadHandler * middleReadHandler = iter->GetReadHandler();
        sScheduler.UnregisterReadHandler(middleReadHandler);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers - 2);
        NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(middleReadHandler));

        // Test unregister last ReadHandler
        iter = sScheduler.mReadHandlerList.end();
        iter--;
        ReadHandler * lastReadHandler = iter->GetReadHandler();
        sScheduler.UnregisterReadHandler(lastReadHandler);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers - 3);
        NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(lastReadHandler));

        sScheduler.UnregisterAllHandlers();
        // Confirm all ReadHandlers are unregistered from the scheduler
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == 0);
        readHandlerPool.ForEachActiveObject([&](ReadHandler * handler) {
            NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(handler));
            return Loop::Continue;
        });

        readHandlerPool.ReleaseAll();
        exchangeCtx->Close();
        NL_TEST_ASSERT(aSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    static void TestReportTiming(nlTestSuite * aSuite, void * aContext)
    {
        TestContext & ctx = *static_cast<TestContext *>(aContext);
        NullReadHandlerCallback nullCallback;
        // exchange context
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

        // Read handler pool
        ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

        // Initialize mock timestamp
        sTestTimerDelegate.SetMockSystemTimestamp(Milliseconds64(0));

        // Dirty read handler, will be triggered at min interval
        ReadHandler * readHandler1 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler1->SetMaxReportingInterval(2));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler1->SetMinReportingIntervalForTests(1));
        // Do those manually to avoid scheduling an engine run
        readHandler1->mFlags.Set(ReadHandler::ReadHandlerFlags::ForceDirty, true);
        readHandler1->mState = ReadHandler::HandlerState::GeneratingReports;

        // Clean read handler, will be triggered at max interval
        ReadHandler * readHandler2 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler2->SetMaxReportingInterval(3));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler2->SetMinReportingIntervalForTests(0));
        // Do those manually to avoid scheduling an engine run
        readHandler2->mState = ReadHandler::HandlerState::GeneratingReports;

        // Clean read handler, will be triggered at max interval, but will be cancelled before
        ReadHandler * readHandler3 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler3->SetMaxReportingInterval(3));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler3->SetMinReportingIntervalForTests(0));
        // Do those manually to avoid scheduling an engine run
        readHandler3->mState = ReadHandler::HandlerState::GeneratingReports;

        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler1));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler2));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler3));

        // Confirms that none of the ReadHandlers are currently reportable
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler3));

        // Simulate system clock increment
        sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

        // Checks that the first ReadHandler is reportable after 1 second since it is dirty and min interval has expired
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler3));

        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler3));
        sScheduler.CancelReport(readHandler3);
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler3));

        // Simulate system clock increment
        sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(2000));

        // Checks that all ReadHandlers are reportable
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportableNow(readHandler2));
        // Even if its timer got cancelled, readHandler3 should still be considered reportable as the max interval has expired
        // and it is in generating report state
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportableNow(readHandler3));

        sScheduler.UnregisterAllHandlers();
        readHandlerPool.ReleaseAll();
        exchangeCtx->Close();
        NL_TEST_ASSERT(aSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    static void TestObserverCallbacks(nlTestSuite * aSuite, void * aContext)
    {
        TestContext & ctx = *static_cast<TestContext *>(aContext);
        NullReadHandlerCallback nullCallback;
        // exchange context
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

        // Read handler pool
        ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

        // Initialize mock timestamp
        sTestTimerDelegate.SetMockSystemTimestamp(Milliseconds64(0));

        ReadHandler * readHandler =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler->SetMaxReportingInterval(2));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler->SetMinReportingIntervalForTests(1));
        // Do those manually to avoid scheduling an engine run
        readHandler->mState = ReadHandler::HandlerState::GeneratingReports;
        readHandler->SetObserver(&sScheduler);

        // Test OnReadHandlerCreated
        readHandler->mObserver->OnReadHandlerCreated(readHandler);
        // Should have registered the read handler in the scheduler and scheduled a report
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == 1);
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));
        ReadHandlerNode * node = sScheduler.FindReadHandlerNode(readHandler);
        NL_TEST_ASSERT(aSuite, nullptr != node);
        VerifyOrReturn(nullptr != node);
        NL_TEST_ASSERT(aSuite, node->GetReadHandler() == readHandler);

        // Test OnBecameReportable
        readHandler->mFlags.Set(ReadHandler::ReadHandlerFlags::ForceDirty, true);
        readHandler->mObserver->OnBecameReportable(readHandler);
        // Should have changed the scheduled timeout to the handler's min interval, to check, we wait for the min interval to
        // expire
        // Simulate system clock increment
        sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

        // Check that no report is scheduled since the min interval has expired, the timer should now be stopped
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler));

        // Test OnSubscriptionAction
        readHandler->mFlags.Set(ReadHandler::ReadHandlerFlags::ForceDirty, false);
        readHandler->mObserver->OnSubscriptionAction(readHandler);
        // Should have changed the scheduled timeout to the handlers max interval, to check, we wait for the min interval to
        // confirm it is not expired yet so the report should still be scheduled

        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));
        // Simulate system clock increment
        sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

        // Check that the report is still scheduled as the max interval has not expired yet and the dirty flag was cleared
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));
        // Simulate system clock increment
        sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(2100));

        // Check that no report is scheduled since the max interval should have expired, the timer should now be stopped
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler));

        // Test OnReadHandlerDestroyed
        readHandler->mObserver->OnReadHandlerDestroyed(readHandler);
        // Should have unregistered the read handler in the scheduler and cancelled the report
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler));
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == 0);
        NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(readHandler));

        sScheduler.UnregisterReadHandler(readHandler);
        readHandlerPool.ReleaseAll();
        exchangeCtx->Close();
        NL_TEST_ASSERT(aSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }
};

} // namespace reporting
} // namespace app
} // namespace chip

namespace {

/**
 *   Test Suite. It lists all the test functions.
 */

static nlTest sTests[] = {
    NL_TEST_DEF("TestReadHandlerList", chip::app::reporting::TestReportScheduler::TestReadHandlerList),
    NL_TEST_DEF("TestReportTiming", chip::app::reporting::TestReportScheduler::TestReportTiming),
    NL_TEST_DEF("TestObserverCallbacks", chip::app::reporting::TestReportScheduler::TestObserverCallbacks),
    NL_TEST_SENTINEL(),
};

nlTestSuite sSuite = { "TestReportScheduler", &sTests[0], TestContext::Initialize, TestContext::Finalize };

} // namespace

int TestReportScheduler()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestReportScheduler);
