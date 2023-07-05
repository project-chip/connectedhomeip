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
#include <nlunit-test.h>

namespace {

uint8_t gDebugEventBuffer[128];
uint8_t gInfoEventBuffer[128];
uint8_t gCritEventBuffer[128];
chip::app::CircularEventBuffer gCircularEventBuffer[3];

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

        chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        chip::app::EventManagement::CreateEventManagement(&ctx->GetExchangeManager(), ArraySize(logStorageResources),
                                                          gCircularEventBuffer, logStorageResources, &ctx->mEventCounter);

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

using InteractionModelEngine = InteractionModelEngine;
using ReportScheduler        = reporting::ReportScheduler;
using ReportSchedulerImpl    = reporting::ReportSchedulerImpl;
using ReadHandlerNode        = reporting::ReportScheduler::ReadHandlerNode;

class TestTimerDelegate : public ReportScheduler::TimerDelegate
{
public:
    static void TimerCallbackInterface(System::Layer * aLayer, void * aAppState)
    {
        ReadHandlerNode * node = static_cast<ReadHandlerNode *>(aAppState);
        node->RunCallback();
    }
    virtual CHIP_ERROR StartTimer(void * context, System::Clock::Timeout aTimeout) override
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->StartTimer(
            aTimeout, TimerCallbackInterface, context);
    }
    virtual void CancelTimer(void * context) override
    {
        InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->CancelTimer(
            TimerCallbackInterface, context);
    }
    virtual bool IsTimerActive(void * context) override
    {
        return InteractionModelEngine::GetInstance()->GetExchangeManager()->GetSessionManager()->SystemLayer()->IsTimerActive(
            TimerCallbackInterface, context);
    }
};

static const size_t kNumMaxReadHandlers = 16;

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

        // Dirty read handler, will be triggered at min interval
        ReadHandler * readHandler1 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler1->SetMaxReportingIntervals(2));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler1->SetMinReportingIntervals(1));
        readHandler1->ForceDirtyState();
        readHandler1->MoveToState(ReadHandler::HandlerState::GeneratingReports);

        // Clean read handler, will be triggered at max interval
        ReadHandler * readHandler2 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler2->SetMaxReportingIntervals(3));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler2->SetMinReportingIntervals(0));
        readHandler2->MoveToState(ReadHandler::HandlerState::GeneratingReports);

        // Clean read handler, will be triggered at max interval, but will be cancelled before
        ReadHandler * readHandler3 =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler3->SetMaxReportingIntervals(3));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler3->SetMinReportingIntervals(0));
        readHandler3->MoveToState(ReadHandler::HandlerState::GeneratingReports);

        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler1));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler2));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == sScheduler.RegisterReadHandler(readHandler3));

        // Confirms that none of the ReadHandlers are currently reportable
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler3));

        ctx.GetIOContext().DriveIOUntil(System::Clock::Milliseconds32(1100),
                                        [&]() -> bool { return sScheduler.IsReportableNow(readHandler1); });
        // Checks that the first ReadHandler is reportable after 1 second since it is dirty and min interval has expired
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportableNow(readHandler1));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler2));
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportableNow(readHandler3));

        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler3));
        sScheduler.CancelReport(readHandler3);
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler3));

        // Wait another 3 seconds to let the second ReadHandler become reportable
        ctx.GetIOContext().DriveIOUntil(System::Clock::Milliseconds32(3100),
                                        [&]() -> bool { return sScheduler.IsReportableNow(readHandler2); });

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

        ReadHandler * readHandler =
            readHandlerPool.CreateObject(nullCallback, exchangeCtx, ReadHandler::InteractionType::Subscribe);
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler->SetMaxReportingIntervals(2));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == readHandler->SetMinReportingIntervals(1));
        readHandler->MoveToState(ReadHandler::HandlerState::GeneratingReports);
        readHandler->SetObserver(&sScheduler);

        // Test OnReadHandlerAdded
        readHandler->mObserver->OnReadHandlerAdded(readHandler);
        // Should have registered the read handler in the scheduler and scheduled a report
        NL_TEST_ASSERT(aSuite, sScheduler.GetNumReadHandlers() == 1);
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));
        ReadHandlerNode * node = sScheduler.FindReadHandlerNode(readHandler);
        NL_TEST_ASSERT(aSuite, nullptr != node);
        VerifyOrReturn(nullptr != node);
        NL_TEST_ASSERT(aSuite, node->GetReadHandler() == readHandler);

        // Test OnBecameReportable
        readHandler->ForceDirtyState();
        readHandler->mObserver->OnBecameReportable(readHandler);
        // Should have changed the scheduled timeout to the handlers min interval, to check, we wait for the min interval to
        // expire
        ctx.GetIOContext().DriveIOUntil(System::Clock::Milliseconds32(1100),
                                        [&]() -> bool { return sScheduler.IsReportableNow(readHandler); });
        // Check that no report is scheduled since the min interval has expired, the timer should now be stopped
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler));

        // Test OnReportSent
        readHandler->ClearForceDirtyFlag();
        readHandler->mObserver->OnReportSent(readHandler);
        // Should have changed the scheduled timeout to the handlers max interval, to check, we wait for the min interval to
        // confirm it is not expired yet so the report should still be scheduled
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));
        ctx.GetIOContext().DriveIOUntil(System::Clock::Milliseconds32(1100),
                                        [&]() -> bool { return sScheduler.IsReportableNow(readHandler); });
        // Check that the report is still scheduled as the max interval has not expired yet and the dirty flag was cleared
        NL_TEST_ASSERT(aSuite, sScheduler.IsReportScheduled(readHandler));
        ctx.GetIOContext().DriveIOUntil(System::Clock::Milliseconds32(2100),
                                        [&]() -> bool { return sScheduler.IsReportableNow(readHandler); });
        // Check that no report is scheduled since the max interval should have expired, the timer should now be stopped
        NL_TEST_ASSERT(aSuite, !sScheduler.IsReportScheduled(readHandler));

        // Test OnReadHandlerRemoved
        readHandler->mObserver->OnReadHandlerRemoved(readHandler);
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

} // namespace app
} // namespace chip

namespace {

/**
 *   Test Suite. It lists all the test functions.
 */

static nlTest sTests[] = {
    NL_TEST_DEF("TestReadHandlerList", chip::app::TestReportScheduler::TestReadHandlerList),
    NL_TEST_DEF("TestReportTiming", chip::app::TestReportScheduler::TestReportTiming),
    NL_TEST_DEF("TestObserverCallbacks", chip::app::TestReportScheduler::TestObserverCallbacks),
    NL_TEST_SENTINEL(),
};

nlTestSuite sSuite = { "TestReportScheduler", &sTests[0], TestContext::Initialize, TestContext::Finalize };

} // namespace

int TestReportScheduler()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestReportScheduler);
