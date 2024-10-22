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
#include <app/codegen-data-model-provider/Instance.h>
#include <app/reporting/ReportSchedulerImpl.h>
#include <app/reporting/SynchronizedReportSchedulerImpl.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>
namespace {

class NullReadHandlerCallback : public chip::app::ReadHandler::ManagementCallback
{
public:
    void OnDone(chip::app::ReadHandler & apReadHandlerObj) override {}
    chip::app::ReadHandler::ApplicationCallback * GetAppCallback() override { return nullptr; }
    chip::app::InteractionModelEngine * GetInteractionModelEngine() override
    {
        return chip::app::InteractionModelEngine::GetInstance();
    }
};

} // namespace

namespace chip {
namespace app {
namespace reporting {

using ReportScheduler     = reporting::ReportScheduler;
using ReportSchedulerImpl = reporting::ReportSchedulerImpl;
using ReadHandlerNode     = reporting::ReportScheduler::ReadHandlerNode;
using Milliseconds64      = System::Clock::Milliseconds64;

static const size_t kNumMaxReadHandlers = 16;

class TestReportScheduler : public chip::Test::AppContext
{
public:
    void TestReadHandlerList();
    void TestReportTiming();
    void TestObserverCallbacks();
    void TestSynchronizedScheduler();

    /// @brief Mimicks the various operations that happen on a subscription transaction after a read handler was created so that
    /// readhandlers are in the expected state for further tests.
    /// @param readHandler
    /// @param scheduler
    static CHIP_ERROR MockReadHandlerSubscriptionTransaction(ReadHandler * readHandler, ReportScheduler * scheduler,
                                                             uint8_t min_interval_seconds, uint8_t max_interval_seconds)
    {
        ReturnErrorOnFailure(readHandler->SetMaxReportingInterval(max_interval_seconds));
        ReturnErrorOnFailure(readHandler->SetMinReportingIntervalForTests(min_interval_seconds));
        readHandler->ClearStateFlag(ReadHandler::ReadHandlerFlags::PrimingReports);
        readHandler->SetStateFlag(ReadHandler::ReadHandlerFlags::ActiveSubscription);
        scheduler->OnSubscriptionEstablished(readHandler);
        readHandler->MoveToState(ReadHandler::HandlerState::CanStartReporting);

        return CHIP_NO_ERROR;
    }

    static ReadHandler * GetReadHandlerFromPool(ReportScheduler * scheduler, uint32_t target)
    {
        uint32_t i        = 0;
        ReadHandler * ret = nullptr;

        scheduler->mNodesPool.ForEachActiveObject([target, &i, &ret](ReadHandlerNode * node) {
            if (i == target)
            {
                ret = node->GetReadHandler();
                return Loop::Break;
            }

            i++;
            return Loop::Continue;
        });

        return ret;
    }
};

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
        TimerContext * context = static_cast<TimerContext *>(aAppState);
        context->TimerFired();
        ChipLogProgress(DataManagement, "Simluating engine run for Handler: %p", aAppState);
    }
    virtual CHIP_ERROR StartTimer(TimerContext * context, System::Clock::Timeout aTimeout) override
    {
        return insertPair(static_cast<ReadHandlerNode *>(context), aTimeout + mMockSystemTimestamp);
    }
    virtual void CancelTimer(TimerContext * context) override { removePair(static_cast<ReadHandlerNode *>(context)); }
    virtual bool IsTimerActive(TimerContext * context) override
    {
        size_t position;
        NodeTimeoutPair * pair = FindPair(static_cast<ReadHandlerNode *>(context), position);
        VerifyOrReturnValue(pair != nullptr, false);

        return pair->timeout > mMockSystemTimestamp;
    }

    virtual System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return mMockSystemTimestamp; }

    void SetMockSystemTimestamp(System::Clock::Timestamp aMockTimestamp) { mMockSystemTimestamp = aMockTimestamp; }

    // Increment the mock timestamp by aTime and call callbacks for timers that have expired. Checks if the timeout expired after
    // incrementing
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

/// @brief TestTimerSynchronizedDelegate is a mock of the TimerDelegate interface that allows to control the time without dependency
/// on the system layer. This also simulates the system timer by verifying if the timeout expired when incrementing the mock
/// timestamp. only one timer can be active at a time, which is the one has the earliest timeout.
/// It is used to test the SynchronizedReportSchedulerImpl.
class TestTimerSynchronizedDelegate : public ReportScheduler::TimerDelegate
{
public:
    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        TimerContext * context = static_cast<TimerContext *>(aAppState);
        context->TimerFired();
    }
    virtual CHIP_ERROR StartTimer(TimerContext * context, System::Clock::Timeout aTimeout) override
    {
        if (nullptr == context)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        mTimerContext = context;
        mTimerTimeout = mMockSystemTimestamp + aTimeout;
        return CHIP_NO_ERROR;
    }
    virtual void CancelTimer(TimerContext * context) override
    {
        VerifyOrReturn(nullptr != mTimerContext);
        mTimerContext = nullptr;
        mTimerTimeout = System::Clock::Milliseconds64(0x7FFFFFFFFFFFFFFF);
    }
    virtual bool IsTimerActive(TimerContext * context) override
    {
        return (nullptr != mTimerContext) && (mTimerTimeout > mMockSystemTimestamp);
    }

    virtual System::Clock::Timestamp GetCurrentMonotonicTimestamp() override { return mMockSystemTimestamp; }

    void SetMockSystemTimestamp(System::Clock::Timestamp aMockTimestamp) { mMockSystemTimestamp = aMockTimestamp; }

    // Increment the mock timestamp one milisecond at a time for a total of aTime miliseconds. Checks if the timeout expired when
    // incrementing
    void IncrementMockTimestamp(System::Clock::Milliseconds64 aTime)
    {
        for (System::Clock::Milliseconds64 i = System::Clock::Milliseconds64(0); i < aTime; i++)
        {
            mMockSystemTimestamp++;
            if (mMockSystemTimestamp == mTimerTimeout)
            {
                TimerCallbackInterface(nullptr, mTimerContext);
            }
        }

        if (aTime == System::Clock::Milliseconds64(0))
        {
            if (mMockSystemTimestamp == mTimerTimeout)
            {
                TimerCallbackInterface(nullptr, mTimerContext);
            }
        }
    }

    TimerContext * mTimerContext                  = nullptr;
    System::Clock::Timeout mTimerTimeout          = System::Clock::Milliseconds64(0x7FFFFFFFFFFFFFFF);
    System::Clock::Timestamp mMockSystemTimestamp = System::Clock::Milliseconds64(0);
};

TestTimerDelegate sTestTimerDelegate;
ReportSchedulerImpl sScheduler(&sTestTimerDelegate);

TestTimerSynchronizedDelegate sTestTimerSynchronizedDelegate;
SynchronizedReportSchedulerImpl syncScheduler(&sTestTimerSynchronizedDelegate);

TEST_F_FROM_FIXTURE(TestReportScheduler, TestReadHandlerList)
{

    NullReadHandlerCallback nullCallback;
    // exchange context
    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    // Read handler pool
    ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

    // Initialize mock timestamp
    sTestTimerDelegate.SetMockSystemTimestamp(Milliseconds64(0));

    for (size_t i = 0; i < kNumMaxReadHandlers; i++)
    {
        ReadHandler * readHandler = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                                 &sScheduler, CodegenDataModelProviderInstance());
        sScheduler.OnSubscriptionEstablished(readHandler);
        ASSERT_NE(nullptr, readHandler);
        ASSERT_NE(nullptr, sScheduler.FindReadHandlerNode(readHandler));
    }

    EXPECT_EQ(readHandlerPool.Allocated(), kNumMaxReadHandlers);
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), kNumMaxReadHandlers);
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 1u);

    // Test unregister first ReadHandler
    uint32_t target                = 0;
    ReadHandler * firstReadHandler = GetReadHandlerFromPool(&sScheduler, target);

    ASSERT_NE(nullptr, firstReadHandler);
    sScheduler.OnReadHandlerDestroyed(firstReadHandler);
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), kNumMaxReadHandlers - 1);
    EXPECT_EQ(nullptr, sScheduler.FindReadHandlerNode(firstReadHandler));

    // Test unregister middle ReadHandler
    target                          = static_cast<uint32_t>(sScheduler.GetNumReadHandlers() / 2);
    ReadHandler * middleReadHandler = GetReadHandlerFromPool(&sScheduler, target);

    ASSERT_NE(nullptr, middleReadHandler);
    sScheduler.OnReadHandlerDestroyed(middleReadHandler);
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), kNumMaxReadHandlers - 2);
    EXPECT_EQ(nullptr, sScheduler.FindReadHandlerNode(middleReadHandler));

    // Test unregister last ReadHandler
    target                        = static_cast<uint32_t>(sScheduler.GetNumReadHandlers() - 1);
    ReadHandler * lastReadHandler = GetReadHandlerFromPool(&sScheduler, target);
    sScheduler.OnReadHandlerDestroyed(lastReadHandler);
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), kNumMaxReadHandlers - 3);
    EXPECT_EQ(nullptr, sScheduler.FindReadHandlerNode(lastReadHandler));

    sScheduler.UnregisterAllHandlers();
    // Confirm all ReadHandlers are unregistered from the scheduler
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), 0u);
    readHandlerPool.ForEachActiveObject([&](ReadHandler * handler) {
        EXPECT_EQ(nullptr, sScheduler.FindReadHandlerNode(handler));
        return Loop::Continue;
    });

    readHandlerPool.ReleaseAll();
    exchangeCtx->Close();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE(TestReportScheduler, TestReportTiming)
{

    NullReadHandlerCallback nullCallback;
    // exchange context
    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    // Read handler pool
    ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

    // Initialize mock timestamp
    sTestTimerDelegate.SetMockSystemTimestamp(Milliseconds64(0));

    // Dirty read handler, will be triggered at min interval
    // Test OnReadHandler created
    ReadHandler * readHandler1 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &sScheduler, CodegenDataModelProviderInstance());

    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler1, &sScheduler, 1, 2));
    readHandler1->ForceDirtyState();

    // Clean read handler, will be triggered at max interval
    ReadHandler * readHandler2 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &sScheduler, CodegenDataModelProviderInstance());
    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler2, &sScheduler, 0, 3));

    // Clean read handler, will be triggered at max interval, but will be cancelled before
    ReadHandler * readHandler3 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &sScheduler, CodegenDataModelProviderInstance());
    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler3, &sScheduler, 0, 3));

    // Confirms that none of the ReadHandlers are currently reportable
    EXPECT_FALSE(sScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(sScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(sScheduler.IsReportableNow(readHandler3));

    // Simulate system clock increment
    sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

    // Checks that the first ReadHandler is reportable after 1 second since it is dirty and min interval has expired
    EXPECT_TRUE(sScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(sScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(sScheduler.IsReportableNow(readHandler3));

    EXPECT_TRUE(sScheduler.IsReportScheduled(readHandler3));
    sScheduler.CancelReport(readHandler3);
    EXPECT_FALSE(sScheduler.IsReportScheduled(readHandler3));

    // Simulate system clock increment
    sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(2000));

    // Checks that all ReadHandlers are reportable
    EXPECT_TRUE(sScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(sScheduler.IsReportableNow(readHandler2));
    // Even if its timer got cancelled, readHandler3 should still be considered reportable as the max interval has expired
    // and it is in generating report state
    EXPECT_TRUE(sScheduler.IsReportableNow(readHandler3));

    // Clear dirty flag on readHandler1 and confirm it is still reportable by time
    readHandler1->ClearForceDirtyFlag();
    EXPECT_TRUE(sScheduler.IsReportableNow(readHandler1));

    sScheduler.UnregisterAllHandlers();
    readHandlerPool.ReleaseAll();
    exchangeCtx->Close();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE(TestReportScheduler, TestObserverCallbacks)
{

    NullReadHandlerCallback nullCallback;
    // exchange context
    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    // Read handler pool
    ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

    // Initialize mock timestamp
    sTestTimerDelegate.SetMockSystemTimestamp(Milliseconds64(0));

    ReadHandler * readHandler = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                             &sScheduler, CodegenDataModelProviderInstance());

    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler, &sScheduler, 1, 2));

    // Verifies OnSubscriptionEstablished registered the ReadHandler in the scheduler
    ASSERT_NE(nullptr, sScheduler.FindReadHandlerNode(readHandler));

    // Should have registered the read handler in the scheduler and scheduled a report
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), 1u);

    ReadHandlerNode * node = sScheduler.FindReadHandlerNode(readHandler);

    // Test OnReportingIntervalsChanged modified the intervals and re-scheduled a report
    EXPECT_EQ(node->GetMinTimestamp().count(), 1000u);
    EXPECT_EQ(node->GetMaxTimestamp().count(), 2000u);

    EXPECT_TRUE(sScheduler.IsReportScheduled(readHandler));

    ASSERT_NE(nullptr, node);
    EXPECT_EQ(node->GetReadHandler(), readHandler);

    // Test OnBecameReportable
    readHandler->ForceDirtyState();
    // Should have changed the scheduled timeout to the handler's min interval, to check, we wait for the min interval to
    // expire
    // Simulate system clock increment
    sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

    // Check that no report is scheduled since the min interval has expired, the timer should now be stopped
    EXPECT_FALSE(sScheduler.IsReportScheduled(readHandler));

    // Test OnSubscriptionReportSent
    readHandler->ClearForceDirtyFlag();
    readHandler->mObserver->OnSubscriptionReportSent(readHandler);
    // Should have changed the scheduled timeout to the handlers max interval, to check, we wait for the min interval to
    // confirm it is not expired yet so the report should still be scheduled

    EXPECT_TRUE(sScheduler.IsReportScheduled(readHandler));
    // Simulate system clock increment
    sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

    // Check that the report is still scheduled as the max interval has not expired yet and the dirty flag was cleared
    EXPECT_TRUE(sScheduler.IsReportScheduled(readHandler));
    // Simulate system clock increment
    sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(2100));

    // Check that no report is scheduled since the max interval should have expired, the timer should now be stopped
    EXPECT_FALSE(sScheduler.IsReportScheduled(readHandler));

    // Test OnReadHandlerDestroyed
    readHandler->mObserver->OnReadHandlerDestroyed(readHandler);
    // Should have unregistered the read handler in the scheduler and cancelled the report
    EXPECT_FALSE(sScheduler.IsReportScheduled(readHandler));
    EXPECT_EQ(sScheduler.GetNumReadHandlers(), 0u);
    EXPECT_EQ(nullptr, sScheduler.FindReadHandlerNode(readHandler));

    readHandlerPool.ReleaseAll();
    exchangeCtx->Close();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F_FROM_FIXTURE(TestReportScheduler, TestSynchronizedScheduler)
{

    NullReadHandlerCallback nullCallback;
    // exchange context
    Messaging::ExchangeContext * exchangeCtx = NewExchangeToAlice(nullptr, false);

    // First test: ReadHandler 2 merge on ReadHandler 1 max interval
    // Read handler pool
    ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

    // Initialize the mock system time
    sTestTimerSynchronizedDelegate.SetMockSystemTimestamp(System::Clock::Milliseconds64(0));

    ReadHandler * readHandler1 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &syncScheduler, CodegenDataModelProviderInstance());

    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler1, &syncScheduler, 0, 2));
    ReadHandlerNode * node1 = syncScheduler.FindReadHandlerNode(readHandler1);

    ReadHandler * readHandler2 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &syncScheduler, CodegenDataModelProviderInstance());
    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler2, &syncScheduler, 1, 3));

    ReadHandlerNode * node2 = syncScheduler.FindReadHandlerNode(readHandler2);

    // Confirm all handler are currently registered in the scheduler
    EXPECT_EQ(syncScheduler.GetNumReadHandlers(), 2u);

    // Confirm that a report emission is scheduled
    EXPECT_TRUE(syncScheduler.IsReportScheduled(readHandler1));

    // Validates that the lowest max is selected as the common max timestamp
    EXPECT_EQ(syncScheduler.mNextMaxTimestamp, node1->GetMaxTimestamp());
    // Validates that the highest reportable min is selected as the common min interval (0 here)
    EXPECT_EQ(syncScheduler.mNextMinTimestamp, node1->GetMinTimestamp());
    // Validates that the next report emission is scheduled on the common max timestamp
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, syncScheduler.mNextMaxTimestamp);

    // Simulate waiting for the max interval to expire (2s)
    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));

    // Confirm that both handlers are now reportable since the timer has expired (readHandler1 from its max and readHandler2
    // from its sync)
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    // Confirm timeout has expired and no report is scheduled, an engine run would typically happen here
    EXPECT_FALSE(sScheduler.IsReportScheduled(readHandler1));
    EXPECT_FALSE(sScheduler.IsReportScheduled(readHandler2));

    // Simulate a report emission for readHandler1
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    // Simulate a report emission for readHandler2
    readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);

    // Validate that the max timestamp for both readhandlers got updated and that the next report emission is scheduled on
    //  the new max timestamp for readhandler1
    EXPECT_GT(node1->GetMaxTimestamp(), sTestTimerSynchronizedDelegate.GetCurrentMonotonicTimestamp());
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());

    // Confirm behavior when a read handler becomes dirty
    readHandler2->ForceDirtyState();
    // OnBecomeReportable should have been called on ForceDirtyState because readHandler callbacks are now integrated
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    // Simulate wait enough for min timestamp of readHandler2 to be reached (1s)
    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));

    // Confirm that the next report emission is scheduled on the min timestamp of readHandler2 (now) as it is the highest
    // reportable
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node2->GetMinTimestamp());
    EXPECT_TRUE(node1->CanBeSynced());

    // Simulate a report emission for readHandler1
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));

    // ReadHandler 2 should still be reportable since it hasn't emitted a report yet
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    readHandler2->ClearForceDirtyFlag();
    readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    // Simulate ReadHandler 1 becoming dirty after ReadHandler 2 past min will trigger a report emission for both

    // Wait past ReadHandler 2 min
    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1100));
    // No handler should be reportable yet
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    readHandler1->ForceDirtyState();

    // Both read handlers should now be reportable since the ForceDirty should immediately trigger the timer expiration callback
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));

    // Simulate a report emission for readHandler1
    readHandler1->ClearForceDirtyFlag();
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));

    readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    // Confirm both handlers are not reportable anymore
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));

    // Validate next report scheduled on the max timestamp of readHandler1
    EXPECT_GT(node1->GetMaxTimestamp(), sTestTimerSynchronizedDelegate.GetCurrentMonotonicTimestamp());
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());

    // Simulate readHandler1 becoming dirty after less than 1 seconds, since it is reportable now, this will Schedule an Engine
    // run immediately
    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(900));
    readHandler1->ForceDirtyState();
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    // Simulate a report emission for readHandler1
    readHandler1->ClearForceDirtyFlag();
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    // The next report should be scheduler on the max timestamp of readHandler1
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));
    // Confirm node 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler1
    EXPECT_TRUE(node2->CanBeSynced());

    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());

    // Simulate a new ReadHandler being added with a min timestamp that will force a conflict

    // Wait for 1 second, nothing should happen here
    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

    ReadHandler * readHandler3 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &syncScheduler, CodegenDataModelProviderInstance());

    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler3, &syncScheduler, 2, 3));
    ReadHandlerNode * node3 = syncScheduler.FindReadHandlerNode(readHandler3);

    // Confirm all handler are currently registered in the scheduler
    EXPECT_EQ(syncScheduler.GetNumReadHandlers(), 3u);

    // Since the min interval on readHandler3 is 2, it should be above the current max timestamp, therefore the next report
    // should still happen on the max timestamp of readHandler1 and the sync should be done on future reports
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());
    // The min timestamp should also not have changed since the min of readhandler3 is higher than the current max
    EXPECT_EQ(syncScheduler.mNextMinTimestamp, node2->GetMinTimestamp());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

    // Confirm that readHandler1 and readHandler 2 are now reportable, whilst readHandler3 is not
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler3));
    readHandler1->mObserver->OnBecameReportable(readHandler1);
    readHandler2->mObserver->OnBecameReportable(readHandler2);

    // Simulate a report emission for readHandler1 and readHandler2
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler2);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    // Confirm that next report is scheduled on the max timestamp of readHandler3 and other 2 readHandlers are synced
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node3->GetMaxTimestamp());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));
    // Confirm nodes 1 and 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler1
    EXPECT_TRUE(node1->CanBeSynced());
    EXPECT_TRUE(node2->CanBeSynced());

    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler3));
    readHandler1->mObserver->OnBecameReportable(readHandler1);
    readHandler2->mObserver->OnBecameReportable(readHandler2);
    readHandler3->mObserver->OnBecameReportable(readHandler3);
    // Engine run should happen here and send all reports
    readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
    readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
    readHandler3->mObserver->OnSubscriptionReportSent(readHandler3);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler3));
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());

    // Now simulate a new readHandler being added with a max forcing a conflict
    ReadHandler * readHandler4 = readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe,
                                                              &syncScheduler, CodegenDataModelProviderInstance());

    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler4, &syncScheduler, 0, 1));
    ReadHandlerNode * node4 = syncScheduler.FindReadHandlerNode(readHandler4);

    // Confirm all handler are currently registered in the scheduler
    EXPECT_EQ(syncScheduler.GetNumReadHandlers(), 4u);

    // Confirm next report is scheduled on the max timestamp of readHandler4
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node4->GetMaxTimestamp());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1100));
    // Confirm node 1 and 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler4
    EXPECT_TRUE(node1->CanBeSynced());
    EXPECT_TRUE(node2->CanBeSynced());
    // Confirm handler 3 cannot be synched on a later timestamp since its min is higher than the max of readHandler4
    EXPECT_FALSE(node3->CanBeSynced());

    // Confirm readHandler1, 2 and 4 are reportable
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler4));

    // Confirm readHandler3 is not reportable
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler3));
    readHandler4->mObserver->OnBecameReportable(readHandler1);
    readHandler4->mObserver->OnBecameReportable(readHandler2);
    readHandler4->mObserver->OnBecameReportable(readHandler4);
    readHandler4->mObserver->OnSubscriptionReportSent(readHandler1);
    readHandler4->mObserver->OnSubscriptionReportSent(readHandler2);
    readHandler4->mObserver->OnSubscriptionReportSent(readHandler4);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler4));

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

    // Confirm  readHandler3 is reportable and other handlers are synced
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler3));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler4));
    syncScheduler.OnBecameReportable(readHandler1);
    syncScheduler.OnBecameReportable(readHandler2);
    syncScheduler.OnBecameReportable(readHandler3);
    syncScheduler.OnBecameReportable(readHandler4);
    syncScheduler.OnSubscriptionReportSent(readHandler1);
    syncScheduler.OnSubscriptionReportSent(readHandler2);
    syncScheduler.OnSubscriptionReportSent(readHandler3);
    syncScheduler.OnSubscriptionReportSent(readHandler4);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler3));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler4));

    // Next emission should be scheduled on the max timestamp of readHandler4 as it is the most restrictive
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node4->GetMaxTimestamp());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));
    // Confirm node 1 and 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler4
    EXPECT_TRUE(node1->CanBeSynced());
    EXPECT_TRUE(node2->CanBeSynced());
    // Confirm node 3 still cannot sync
    EXPECT_FALSE(node3->CanBeSynced());

    // Confirm  readHandler 1-2-4 are reportable
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler4));

    // Confirm readHandler3 is not reportable because of its min interval
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler3));

    syncScheduler.OnReadHandlerDestroyed(readHandler1);
    syncScheduler.OnReadHandlerDestroyed(readHandler2);
    syncScheduler.OnReadHandlerDestroyed(readHandler3);
    syncScheduler.OnReadHandlerDestroyed(readHandler4);

    // Reset all handlers
    // Test case: Scheduler 1 and 2 are reportable but min2 > max1, they should sync only when possible (min2 = 3, max1 = 2)
    EXPECT_EQ(syncScheduler.GetNumReadHandlers(), 0u);

    readHandler1->MoveToState(ReadHandler::HandlerState::Idle);
    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler1, &syncScheduler, 0, 2));

    // Forcing the dirty flag to make the scheduler call Engine::ScheduleRun() immediately
    readHandler1->ForceDirtyState();
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));

    readHandler2->MoveToState(ReadHandler::HandlerState::Idle);
    EXPECT_EQ(CHIP_NO_ERROR, MockReadHandlerSubscriptionTransaction(readHandler2, &syncScheduler, 3, 4));
    readHandler2->ForceDirtyState();
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    node1 = syncScheduler.FindReadHandlerNode(readHandler1);
    node2 = syncScheduler.FindReadHandlerNode(readHandler2);

    readHandler1->ClearForceDirtyFlag(); // report got emited so clear dirty flag
    syncScheduler.OnSubscriptionReportSent(readHandler1);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));

    // Confirm next report is scheduled on the max timestamp of readHandler1 and readhandler2 is not synced
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());
    // Node 2's sync timestamp should have remained unaffected since its min is higher
    EXPECT_FALSE(node2->CanBeSynced());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));
    // Verify handler 1 became reportable
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    syncScheduler.OnBecameReportable(readHandler1);

    // simulate run with only readhandler1 reportable
    syncScheduler.OnSubscriptionReportSent(readHandler1);
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_FALSE(syncScheduler.IsReportableNow(readHandler2));
    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node2->GetMinTimestamp());

    sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));
    EXPECT_TRUE(node1->CanBeSynced());
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler1));
    EXPECT_TRUE(syncScheduler.IsReportableNow(readHandler2));

    readHandler2->ClearForceDirtyFlag();
    syncScheduler.OnSubscriptionReportSent(readHandler1);
    syncScheduler.OnSubscriptionReportSent(readHandler2);

    EXPECT_EQ(syncScheduler.mNextReportTimestamp, node1->GetMaxTimestamp());
    EXPECT_FALSE(node2->CanBeSynced());

    syncScheduler.UnregisterAllHandlers();
    readHandlerPool.ReleaseAll();
    exchangeCtx->Close();
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

} // namespace reporting
} // namespace app
} // namespace chip
