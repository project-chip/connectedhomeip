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
#include <app/EventManagement.h>
#include <app/tests/AppTestContext.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemLayerImpl.h>

#include <app/icd/ICDManager.h>
#include <app/icd/ICDStateObserver.h>
#include <app/icd/IcdManagementServer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::System;

namespace {

class TestICDStateObserver : public app::ICDStateObserver
{
public:
    void OnEnterActiveMode() {}
    void OnTransitionToIdle() {}
};

TestICDStateObserver mICDStateObserver;
static Clock::Internal::MockClock gMockClock;
static Clock::ClockBase * gRealClock;

class TestContext : public Test::AppContext
{
public:
    static int Initialize(void * context)
    {
        if (AppContext::Initialize(context) != SUCCESS)
            return FAILURE;

        auto * ctx = static_cast<TestContext *>(context);
        DeviceLayer::SetSystemLayerForTesting(&ctx->GetSystemLayer());

        gRealClock = &SystemClock();
        Clock::Internal::SetSystemClockForTesting(&gMockClock);

        if (ctx->mEventCounter.Init(0) != CHIP_NO_ERROR)
        {
            return FAILURE;
        }

        ctx->mICDManager.Init(&ctx->testStorage, &ctx->GetFabricTable(), &mICDStateObserver);
        return SUCCESS;
    }

    static int Finalize(void * context)
    {
        auto * ctx = static_cast<TestContext *>(context);
        ctx->mICDManager.Shutdown();
        app::EventManagement::DestroyEventManagement();
        System::Clock::Internal::SetSystemClockForTesting(gRealClock);
        DeviceLayer::SetSystemLayerForTesting(nullptr);

        if (AppContext::Finalize(context) != SUCCESS)
            return FAILURE;

        return SUCCESS;
    }

    app::ICDManager mICDManager;

private:
    TestPersistentStorageDelegate testStorage;
    MonotonicallyIncreasingCounter<EventNumber> mEventCounter;
};

} // namespace

namespace chip {
namespace app {
class TestICDManager
{
public:
    /*
     * Advance the test Mock clock time by the amout passed in argument
     * and then force the SystemLayer Timer event loop. It will check for any expired timer,
     * and invoke their callbacks if there are any.
     *
     * @param time_ms: Value in milliseconds.
     */
    static void AdvanceClockAndRunEventLoop(TestContext * ctx, uint32_t time_ms)
    {
        gMockClock.AdvanceMonotonic(System::Clock::Timeout(time_ms));
        ctx->GetIOContext().DriveIO();
    }

    static void TestICDModeIntervals(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // After the init we should be in active mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeInterval() + 1);
        // Active mode interval expired, ICDManager transitioned to the IdleMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetIdleModeInterval() + 1);
        // Idle mode interval expired, ICDManager transitioned to the ActiveMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Events updating the Operation to Active mode can extend the current active mode time by 1 Active mode threshold.
        // Kick an active Threshold just before the end of the Active interval and validate that the active mode is extended.
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeInterval() - 1);
        ctx->mICDManager.UpdateOperationState(ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeThreshold() / 2);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, (IcdManagementServer::GetInstance().GetActiveModeThreshold() / 2) + 1);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    static void TestKeepActivemodeRequests(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Setting a requirement will transition the ICD to active mode.
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kCommissioningWindowOpen, true);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // Advance time so active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeInterval() + 1);
        // Requirement flag still set. We stay in active mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Remove requirement. we should directly transition to idle mode.
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kCommissioningWindowOpen, false);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kFailSafeArmed, true);
        // Requirement will transition us to active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time, but by less than the active mode interval and remove the requirement.
        // We should stay in active mode.
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeInterval() / 2);
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kFailSafeArmed, false);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time again, The activemode interval is completed.
        AdvanceClockAndRunEventLoop(ctx, (IcdManagementServer::GetInstance().GetActiveModeInterval() / 2) + 1);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Set two requirements
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kExpectingMsgResponse, true);
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kAwaitingMsgAck, true);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // advance time so the active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeInterval() + 1);
        // A requirement flag is still set. We stay in active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // remove 1 requirement. Active mode is maintained
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kExpectingMsgResponse, false);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // remove the last requirement
        ctx->mICDManager.SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kAwaitingMsgAck, false);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    static void TestIdleTransition(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        /// TEST 1: Test that the Transition to Idle is called before going back to Idle Mode
        // Advance time so that we begin the test at the beginning of the active mode interval
        while (ctx->mICDManager.mOperationalState != ICDManager::OperationalState::ActiveMode)
        {
            AdvanceClockAndRunEventLoop(ctx, 1);
        }
        // Confirm we are in Active Mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == false);
        // Advance time enough to trigger OnTransitionToIdle before going back to Idle Mode
        uint32_t timeToAdvance = (IcdManagementServer::GetInstance().GetActiveModeInterval() - ICD_ACTIVE_TIME_JITTER_MS) + 1;

        AdvanceClockAndRunEventLoop(ctx, timeToAdvance);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == true);
        // Confirm that we are still in Active Mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // Advance the clock enough the rest of the way to trigger the transition back to Idle Mode
        timeToAdvance = ICD_ACTIVE_TIME_JITTER_MS + 1;

        AdvanceClockAndRunEventLoop(ctx, timeToAdvance);
        // Confirm we are back in Idle Mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        /// TEST 2: Test that extending the Active Mode interval will cause the call to OnTransitionToIdle to be postponed.
        while (ctx->mICDManager.mOperationalState != ICDManager::OperationalState::ActiveMode)
        {
            AdvanceClockAndRunEventLoop(ctx, 1);
        }
        // Confirm we are in Active Mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == false);
        // Advance time until we are close to the OnTransitionToIdle call
        timeToAdvance = (IcdManagementServer::GetInstance().GetActiveModeInterval() - ICD_ACTIVE_TIME_JITTER_MS) - 1;

        AdvanceClockAndRunEventLoop(ctx, timeToAdvance);
        // Extend the Active Mode interval by active mode threshold
        ctx->mICDManager.UpdateOperationState(ICDManager::OperationalState::ActiveMode);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == false);

        // Advance time to pass the point of emission without the extension
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeThreshold() / 2);
        // Confirm we are still in Active Mode and that a OnTransitionToIdle wasn't called
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == false);

        AdvanceClockAndRunEventLoop(ctx, (IcdManagementServer::GetInstance().GetActiveModeThreshold() / 2) + 1);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == true);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        /// TEST 3: Test that extending the Active Mode interval after the OnTransitionToIdle call point will not cause another call
        while (ctx->mICDManager.mOperationalState != ICDManager::OperationalState::ActiveMode)
        {
            AdvanceClockAndRunEventLoop(ctx, 1);
        }
        // Confirm we are in Active Mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == false);

        // Advance time enough to trigger emission of a report
        timeToAdvance = (IcdManagementServer::GetInstance().GetActiveModeInterval() - ICD_ACTIVE_TIME_JITTER_MS) + 1;
        AdvanceClockAndRunEventLoop(ctx, timeToAdvance);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == true);
        // Extend the Active Mode interval by active mode threshold
        ctx->mICDManager.UpdateOperationState(ICDManager::OperationalState::ActiveMode);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mTransitionToIdleCalled == true);

        // Advance time to pass the point of emission with the extension
        AdvanceClockAndRunEventLoop(ctx, IcdManagementServer::GetInstance().GetActiveModeThreshold() + 1);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }
};

} // namespace app
} // namespace chip

namespace {
/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestICDModeIntervals",         TestICDManager::TestICDModeIntervals),
    NL_TEST_DEF("TestKeepActivemodeRequests",   TestICDManager::TestKeepActivemodeRequests),
    NL_TEST_DEF("TestIdleTransition",           TestICDManager::TestIdleTransition),
    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite cmSuite =
{
    "TestICDManager",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on
} // namespace

int TestSuiteICDManager()
{
    return ExecuteTestsWithContext<TestContext>(&cmSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSuiteICDManager)
