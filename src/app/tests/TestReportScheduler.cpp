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
#include <app/reporting/SynchronizedReportSchedulerImpl.h>
#include <app/tests/AppTestContext.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <nlunit-test.h>

namespace {

using TestContext = chip::Test::AppContext;

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

class TestReportScheduler
{
public:
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
                readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &sScheduler);
            sScheduler.OnSubscriptionEstablished(readHandler);
            NL_TEST_ASSERT(aSuite, nullptr != readHandler);
            VerifyOrReturn(nullptr != readHandler);
            NL_TEST_ASSERT(aSuite, nullptr != sScheduler.FindReadHandlerNode(readHandler));
        }

        NL_TEST_ASSERT(aSuite, readHandlerPool.Allocated() == kNumMaxReadHandlers);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers);
        NL_TEST_ASSERT(aSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 1);

        // Test unregister first ReadHandler
        uint32_t target                = 0;
        ReadHandler * firstReadHandler = GetReadHandlerFromPool(&sScheduler, target);

        NL_TEST_ASSERT(aSuite, nullptr != firstReadHandler);
        sScheduler.OnReadHandlerDestroyed(firstReadHandler);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers - 1);
        NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(firstReadHandler));

        // Test unregister middle ReadHandler
        target                          = static_cast<uint32_t>(sScheduler.GetNumReadHandlers() / 2);
        ReadHandler * middleReadHandler = GetReadHandlerFromPool(&sScheduler, target);

        NL_TEST_ASSERT(aSuite, nullptr != middleReadHandler);
        sScheduler.OnReadHandlerDestroyed(middleReadHandler);
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == kNumMaxReadHandlers - 2);
        NL_TEST_ASSERT(aSuite, nullptr == sScheduler.FindReadHandlerNode(middleReadHandler));

        // Test unregister last ReadHandler
        target                        = static_cast<uint32_t>(sScheduler.GetNumReadHandlers() - 1);
        ReadHandler * lastReadHandler = GetReadHandlerFromPool(&sScheduler, target);
        sScheduler.OnReadHandlerDestroyed(lastReadHandler);
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
        // Test OnReadHandler created
        ReadHandler * readHandler1 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &sScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler1, &sScheduler, 1, 2));
        readHandler1->ForceDirtyState();

        // Clean read handler, will be triggered at max interval
        ReadHandler * readHandler2 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &sScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler2, &sScheduler, 0, 3));

        // Clean read handler, will be triggered at max interval, but will be cancelled before
        ReadHandler * readHandler3 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &sScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler3, &sScheduler, 0, 3));

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

        // Clear dirty flag on readHandler1 and confirm it is still reportable by time
        readHandler1->ClearForceDirtyFlag();
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportableNow(readHandler1));

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
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &sScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler, &sScheduler, 1, 2));

        // Verifies OnSubscriptionEstablished registered the ReadHandler in the scheduler
        NL_TEST_ASSERT(aSuite, nullptr != sScheduler.FindReadHandlerNode(readHandler));

        // Should have registered the read handler in the scheduler and scheduled a report
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == 1);

        ReadHandlerNode * node = sScheduler.FindReadHandlerNode(readHandler);

        // Test OnReportingIntervalsChanged modified the intervals and re-scheduled a report
        NL_TEST_ASSERT(aSuite, node->GetMinTimestamp().count() == 1000);
        NL_TEST_ASSERT(aSuite, node->GetMaxTimestamp().count() == 2000);

        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));

        NL_TEST_ASSERT(aSuite, nullptr != node);
        VerifyOrReturn(nullptr != node);
        NL_TEST_ASSERT(aSuite, node->GetReadHandler() == readHandler);

        // Test OnBecameReportable
        readHandler->ForceDirtyState();
        // Should have changed the scheduled timeout to the handler's min interval, to check, we wait for the min interval to
        // expire
        // Simulate system clock increment
        sTestTimerDelegate.IncrementMockTimestamp(Milliseconds64(1100));

        // Check that no report is scheduled since the min interval has expired, the timer should now be stopped
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler));

        // Test OnSubscriptionReportSent
        readHandler->ClearForceDirtyFlag();
        readHandler->mObserver->OnSubscriptionReportSent(readHandler);
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

        readHandlerPool.ReleaseAll();
        exchangeCtx->Close();
        NL_TEST_ASSERT(aSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    static void TestSynchronizedScheduler(nlTestSuite * aSuite, void * aContext)
    {
        TestContext & ctx = *static_cast<TestContext *>(aContext);
        NullReadHandlerCallback nullCallback;
        // exchange context
        Messaging::ExchangeContext * exchangeCtx = ctx.NewExchangeToAlice(nullptr, false);

        // First test: ReadHandler 2 merge on ReadHandler 1 max interval
        // Read handler pool
        ObjectPool<ReadHandler, kNumMaxReadHandlers> readHandlerPool;

        // Initialize the mock system time
        sTestTimerSynchronizedDelegate.SetMockSystemTimestamp(System::Clock::Milliseconds64(0));

        ReadHandler * readHandler1 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &syncScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler1, &syncScheduler, 0, 2));
        ReadHandlerNode * node1 = syncScheduler.FindReadHandlerNode(readHandler1);

        ReadHandler * readHandler2 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &syncScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler2, &syncScheduler, 1, 3));
        ReadHandlerNode * node2 = syncScheduler.FindReadHandlerNode(readHandler2);

        // Confirm all handler are currently registered in the scheduler
        NL_TEST_ASSERT(aSuite, syncScheduler.GetNumReadHandlers() == 2);

        // Confirm that a report emission is scheduled
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportScheduled(readHandler1));

        // Validates that the lowest max is selected as the common max timestamp
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextMaxTimestamp == node1->GetMaxTimestamp());
        // Validates that the highest reportable min is selected as the common min interval (0 here)
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextMinTimestamp == node1->GetMinTimestamp());
        // Validates that the next report emission is scheduled on the common max timestamp
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == syncScheduler.mNextMaxTimestamp);

        // Simulate waiting for the max interval to expire (2s)
        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));

        // Confirm that both handlers are now reportable since the timer has expired (readHandler1 from its max and readHandler2
        // from its sync)
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        // Confirm timeout has expired and no report is scheduled, an engine run would typically happen here
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler1));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler2));

        // Simulate a report emission for readHandler1
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        // Simulate a report emission for readHandler2
        readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);

        // Validate that the max timestamp for both readhandlers got updated and that the next report emission is scheduled on
        //  the new max timestamp for readhandler1
        NL_TEST_ASSERT(aSuite, node1->GetMaxTimestamp() > sTestTimerSynchronizedDelegate.GetCurrentMonotonicTimestamp());
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());

        // Confirm behavior when a read handler becomes dirty
        readHandler2->ForceDirtyState();
        // OnBecomeReportable should have been called on ForceDirtyState because readHandler callbacks are now integrated
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        // Simulate wait enough for min timestamp of readHandler2 to be reached (1s)
        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));

        // Confirm that the next report emission is scheduled on the min timestamp of readHandler2 (now) as it is the highest
        // reportable
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node2->GetMinTimestamp());
        NL_TEST_ASSERT(aSuite, node1->CanBeSynced() == true);

        // Simulate a report emission for readHandler1
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));

        // ReadHandler 2 should still be reportable since it hasn't emitted a report yet
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        readHandler2->ClearForceDirtyFlag();
        readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        // Simulate ReadHandler 1 becoming dirty after ReadHandler 2 past min will trigger a report emission for both

        // Wait past ReadHandler 2 min
        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1100));
        // No handler should be reportable yet
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        readHandler1->ForceDirtyState();

        // Both read handlers should now be reportable since the ForceDirty should immediately trigger the timer expiration callback
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));

        // Simulate a report emission for readHandler1
        readHandler1->ClearForceDirtyFlag();
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));

        readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        // Confirm both handlers are not reportable anymore
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));

        // Validate next report scheduled on the max timestamp of readHandler1
        NL_TEST_ASSERT(aSuite, node1->GetMaxTimestamp() > sTestTimerSynchronizedDelegate.GetCurrentMonotonicTimestamp());
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());

        // Simulate readHandler1 becoming dirty after less than 1 seconds, since it is reportable now, this will Schedule an Engine
        // run immediately
        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(900));
        readHandler1->ForceDirtyState();
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        // Simulate a report emission for readHandler1
        readHandler1->ClearForceDirtyFlag();
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        // The next report should be scheduler on the max timestamp of readHandler1
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));
        // Confirm node 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler1
        NL_TEST_ASSERT(aSuite, node2->CanBeSynced() == true);

        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());

        // Simulate a new ReadHandler being added with a min timestamp that will force a conflict

        // Wait for 1 second, nothing should happen here
        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

        ReadHandler * readHandler3 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &syncScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler3, &syncScheduler, 2, 3));
        ReadHandlerNode * node3 = syncScheduler.FindReadHandlerNode(readHandler3);

        // Confirm all handler are currently registered in the scheduler
        NL_TEST_ASSERT(aSuite, syncScheduler.GetNumReadHandlers() == 3);

        // Since the min interval on readHandler3 is 2, it should be above the current max timestamp, therefore the next report
        // should still happen on the max timestamp of readHandler1 and the sync should be done on future reports
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());
        // The min timestamp should also not have changed since the min of readhandler3 is higher than the current max
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextMinTimestamp == node2->GetMinTimestamp());

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

        // Confirm that readHandler1 and readHandler 2 are now reportable, whilst readHandler3 is not
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler3));
        readHandler1->mObserver->OnBecameReportable(readHandler1);
        readHandler2->mObserver->OnBecameReportable(readHandler2);

        // Simulate a report emission for readHandler1 and readHandler2
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler2);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        // Confirm that next report is scheduled on the max timestamp of readHandler3 and other 2 readHandlers are synced
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node3->GetMaxTimestamp());

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));
        // Confirm nodes 1 and 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler1
        NL_TEST_ASSERT(aSuite, node1->CanBeSynced() == true);
        NL_TEST_ASSERT(aSuite, node2->CanBeSynced() == true);

        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler3));
        readHandler1->mObserver->OnBecameReportable(readHandler1);
        readHandler2->mObserver->OnBecameReportable(readHandler2);
        readHandler3->mObserver->OnBecameReportable(readHandler3);
        // Engine run should happen here and send all reports
        readHandler1->mObserver->OnSubscriptionReportSent(readHandler1);
        readHandler2->mObserver->OnSubscriptionReportSent(readHandler2);
        readHandler3->mObserver->OnSubscriptionReportSent(readHandler3);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler3));
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());

        // Now simulate a new readHandler being added with a max forcing a conflict
        ReadHandler * readHandler4 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe, &syncScheduler);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler4, &syncScheduler, 0, 1));
        ReadHandlerNode * node4 = syncScheduler.FindReadHandlerNode(readHandler4);

        // Confirm all handler are currently registered in the scheduler
        NL_TEST_ASSERT(aSuite, syncScheduler.GetNumReadHandlers() == 4);

        // Confirm next report is scheduled on the max timestamp of readHandler4
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node4->GetMaxTimestamp());

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1100));
        // Confirm node 1 and 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler4
        NL_TEST_ASSERT(aSuite, node1->CanBeSynced() == true);
        NL_TEST_ASSERT(aSuite, node2->CanBeSynced() == true);
        // Confirm handler 3 cannot be synched on a later timestamp since its min is higher than the max of readHandler4
        NL_TEST_ASSERT(aSuite, node3->CanBeSynced() == false);

        // Confirm readHandler1, 2 and 4 are reportable
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler4));

        // Confirm readHandler3 is not reportable
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler3));
        readHandler4->mObserver->OnBecameReportable(readHandler1);
        readHandler4->mObserver->OnBecameReportable(readHandler2);
        readHandler4->mObserver->OnBecameReportable(readHandler4);
        readHandler4->mObserver->OnSubscriptionReportSent(readHandler1);
        readHandler4->mObserver->OnSubscriptionReportSent(readHandler2);
        readHandler4->mObserver->OnSubscriptionReportSent(readHandler4);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler4));

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));

        // Confirm  readHandler3 is reportable and other handlers are synced
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler3));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler4));
        syncScheduler.OnBecameReportable(readHandler1);
        syncScheduler.OnBecameReportable(readHandler2);
        syncScheduler.OnBecameReportable(readHandler3);
        syncScheduler.OnBecameReportable(readHandler4);
        syncScheduler.OnSubscriptionReportSent(readHandler1);
        syncScheduler.OnSubscriptionReportSent(readHandler2);
        syncScheduler.OnSubscriptionReportSent(readHandler3);
        syncScheduler.OnSubscriptionReportSent(readHandler4);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler3));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler4));

        // Next emission should be scheduled on the max timestamp of readHandler4 as it is the most restrictive
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node4->GetMaxTimestamp());

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));
        // Confirm node 1 and 2 can now be synced since the scheduler timer has fired on the max timestamp of readHandler4
        NL_TEST_ASSERT(aSuite, node1->CanBeSynced() == true);
        NL_TEST_ASSERT(aSuite, node2->CanBeSynced() == true);
        // Confirm node 3 still cannot sync
        NL_TEST_ASSERT(aSuite, node3->CanBeSynced() == false);

        // Confirm  readHandler 1-2-4 are reportable
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler4));

        // Confirm readHandler3 is not reportable because of its min interval
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler3));

        syncScheduler.OnReadHandlerDestroyed(readHandler1);
        syncScheduler.OnReadHandlerDestroyed(readHandler2);
        syncScheduler.OnReadHandlerDestroyed(readHandler3);
        syncScheduler.OnReadHandlerDestroyed(readHandler4);

        // Reset all handlers
        // Test case: Scheduler 1 and 2 are reportable but min2 > max1, they should sync only when possible (min2 = 3, max1 = 2)
        NL_TEST_ASSERT(aSuite, syncScheduler.GetNumReadHandlers() == 0);

        readHandler1->MoveToState(ReadHandler::HandlerState::Idle);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler1, &syncScheduler, 0, 2));

        // Forcing the dirty flag to make the scheduler call Engine::ScheduleRun() immediately
        readHandler1->ForceDirtyState();
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));

        readHandler2->MoveToState(ReadHandler::HandlerState::Idle);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == MockReadHandlerSubscriptionTransaction(readHandler2, &syncScheduler, 3, 4));
        readHandler2->ForceDirtyState();
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        node1 = syncScheduler.FindReadHandlerNode(readHandler1);
        node2 = syncScheduler.FindReadHandlerNode(readHandler2);

        readHandler1->ClearForceDirtyFlag(); // report got emited so clear dirty flag
        syncScheduler.OnSubscriptionReportSent(readHandler1);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));

        // Confirm next report is scheduled on the max timestamp of readHandler1 and readhandler2 is not synced
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());
        // Node 2's sync timestamp should have remained unaffected since its min is higher
        NL_TEST_ASSERT(aSuite, node2->CanBeSynced() == false);

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(2000));
        // Verify handler 1 became reportable
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        syncScheduler.OnBecameReportable(readHandler1);

        // simulate run with only readhandler1 reportable
        syncScheduler.OnSubscriptionReportSent(readHandler1);
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !syncScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node2->GetMinTimestamp());

        sTestTimerSynchronizedDelegate.IncrementMockTimestamp(System::Clock::Milliseconds64(1000));
        NL_TEST_ASSERT(aSuite, node1->CanBeSynced() == true);
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, syncScheduler.IsReportableNow(readHandler2));

        readHandler2->ClearForceDirtyFlag();
        syncScheduler.OnSubscriptionReportSent(readHandler1);
        syncScheduler.OnSubscriptionReportSent(readHandler2);

        NL_TEST_ASSERT(aSuite, syncScheduler.mNextReportTimestamp == node1->GetMaxTimestamp());
        NL_TEST_ASSERT(aSuite, node2->CanBeSynced() == false);

        syncScheduler.UnregisterAllHandlers();
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
    NL_TEST_DEF("TestSynchronizedScheduler", chip::app::reporting::TestReportScheduler::TestSynchronizedScheduler),
    NL_TEST_SENTINEL(),
};

nlTestSuite sSuite = {
    "TestReportScheduler",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};

} // namespace

int TestReportScheduler()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestReportScheduler);
