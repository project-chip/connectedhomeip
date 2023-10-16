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
#include <app/icd/ICDManagementServer.h>
#include <app/icd/ICDManager.h>
#include <app/icd/ICDNotifier.h>
#include <app/icd/ICDStateObserver.h>
#include <app/tests/AppTestContext.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemLayerImpl.h>

#include <crypto/DefaultSessionKeystore.h>

using namespace chip;
using namespace chip::app;
using namespace chip::System;

using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

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
        TestSessionKeystoreImpl keystore;
        ctx->mICDManager.Init(&ctx->testStorage, &ctx->GetFabricTable(), &mICDStateObserver, &keystore);
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
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeIntervalMs() + 1);
        // Active mode interval expired, ICDManager transitioned to the IdleMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
        AdvanceClockAndRunEventLoop(ctx, secondsToMilliseconds(ICDManagementServer::GetInstance().GetIdleModeIntervalSec()) + 1);
        // Idle mode interval expired, ICDManager transitioned to the ActiveMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Events updating the Operation to Active mode can extend the current active mode time by 1 Active mode threshold.
        // Kick an active Threshold just before the end of the Active interval and validate that the active mode is extended.
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeIntervalMs() - 1);
        ICDNotifier::GetInstance().BroadcastNetworkActivityNotification();
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeThresholdMs() / 2);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeThresholdMs());
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    static void TestKeepActivemodeRequests(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::KeepActiveFlags ActiveFlag;
        ICDNotifier notifier = ICDNotifier::GetInstance();

        // Setting a requirement will transition the ICD to active mode.
        notifier.BroadcastActiveRequestNotification(ActiveFlag::kCommissioningWindowOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // Advance time so active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeIntervalMs() + 1);
        // Requirement flag still set. We stay in active mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Remove requirement. we should directly transition to idle mode.
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kCommissioningWindowOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        notifier.BroadcastActiveRequestNotification(ActiveFlag::kFailSafeArmed);
        // Requirement will transition us to active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time, but by less than the active mode interval and remove the requirement.
        // We should stay in active mode.
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeIntervalMs() / 2);
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time again, The activemode interval is completed.
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeIntervalMs() + 1);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Set two requirements
        notifier.BroadcastActiveRequestNotification(ActiveFlag::kFailSafeArmed);
        notifier.BroadcastActiveRequestNotification(ActiveFlag::kExchangeContextOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // advance time so the active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, ICDManagementServer::GetInstance().GetActiveModeIntervalMs() + 1);
        // A requirement flag is still set. We stay in active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // remove 1 requirement. Active mode is maintained
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // remove the last requirement
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kExchangeContextOpen);
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
