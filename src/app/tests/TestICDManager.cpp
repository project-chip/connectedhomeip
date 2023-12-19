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
#include <app/icd/ICDConfigurationData.h>
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

// Test Values
constexpr uint16_t kMaxTestClients      = 2;
constexpr FabricIndex kTestFabricIndex1 = 1;
constexpr FabricIndex kTestFabricIndex2 = kMaxValidFabricIndex;
constexpr uint64_t kClientNodeId11      = 0x100001;
constexpr uint64_t kClientNodeId12      = 0x100002;
constexpr uint64_t kClientNodeId21      = 0x200001;
constexpr uint64_t kClientNodeId22      = 0x200002;

constexpr uint8_t kKeyBuffer1a[] = {
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};
constexpr uint8_t kKeyBuffer1b[] = {
    0xf1, 0xe1, 0xd1, 0xc1, 0xb1, 0xa1, 0x91, 0x81, 0x71, 0x61, 0x51, 0x14, 0x31, 0x21, 0x11, 0x01
};
constexpr uint8_t kKeyBuffer2a[] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f
};
constexpr uint8_t kKeyBuffer2b[] = {
    0xf2, 0xe2, 0xd2, 0xc2, 0xb2, 0xa2, 0x92, 0x82, 0x72, 0x62, 0x52, 0x42, 0x32, 0x22, 0x12, 0x02
};

class TestICDStateObserver : public app::ICDStateObserver
{
public:
    void OnEnterActiveMode() {}
    void OnTransitionToIdle() {}
    void OnICDModeChange() {}
};

class TestContext : public chip::Test::AppContext
{
public:
    // Performs shared setup for all tests in the test suite
    CHIP_ERROR SetUpTestSuite() override
    {
        ReturnErrorOnFailure(chip::Test::AppContext::SetUpTestSuite());
        DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
        mRealClock = &chip::System::SystemClock();
        System::Clock::Internal::SetSystemClockForTesting(&mMockClock);
        return CHIP_NO_ERROR;
    }

    // Performs shared teardown for all tests in the test suite
    void TearDownTestSuite() override
    {
        System::Clock::Internal::SetSystemClockForTesting(mRealClock);
        DeviceLayer::SetSystemLayerForTesting(nullptr);
        chip::Test::AppContext::TearDownTestSuite();
    }

    // Performs setup for each individual test in the test suite
    CHIP_ERROR SetUp() override
    {
        ReturnErrorOnFailure(chip::Test::AppContext::SetUp());
        mICDManager.Init(&testStorage, &GetFabricTable(), &mKeystore, &GetExchangeManager());
        mICDManager.RegisterObserver(&mICDStateObserver);
        return CHIP_NO_ERROR;
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        mICDManager.Shutdown();
        chip::Test::AppContext::TearDown();
    }

    System::Clock::Internal::MockClock mMockClock;
    TestSessionKeystoreImpl mKeystore;
    app::ICDManager mICDManager;
    TestPersistentStorageDelegate testStorage;

private:
    System::Clock::ClockBase * mRealClock;
    TestICDStateObserver mICDStateObserver;
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
    static void AdvanceClockAndRunEventLoop(TestContext * ctx, uint64_t time_ms)
    {
        ctx->mMockClock.AdvanceMonotonic(System::Clock::Timeout(time_ms));
        ctx->GetIOContext().DriveIO();
    }

    static void TestICDModeDurations(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // After the init we should be in Idle mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
        AdvanceClockAndRunEventLoop(ctx, SecondsToMilliseconds(ICDConfigurationData::GetInstance().GetIdleModeDurationSec()) + 1);
        // Idle mode interval expired, ICDManager transitioned to the ActiveMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDurationMs() + 1);
        // Active mode interval expired, ICDManager transitioned to the IdleMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
        AdvanceClockAndRunEventLoop(ctx, SecondsToMilliseconds(ICDConfigurationData::GetInstance().GetIdleModeDurationSec()) + 1);
        // Idle mode interval expired, ICDManager transitioned to the ActiveMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Events updating the Operation to Active mode can extend the current active mode time by 1 Active mode threshold.
        // Kick an active Threshold just before the end of the Active interval and validate that the active mode is extended.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDurationMs() - 1);
        ICDNotifier::GetInstance().BroadcastNetworkActivityNotification();
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThresholdMs() / 2);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThresholdMs());
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    static void TestKeepActivemodeRequests(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::KeepActiveFlag ActiveFlag;
        ICDNotifier notifier = ICDNotifier::GetInstance();

        // Setting a requirement will transition the ICD to active mode.
        notifier.BroadcastActiveRequestNotification(ActiveFlag::kCommissioningWindowOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // Advance time so active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDurationMs() + 1);
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
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDurationMs() / 2);
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time again, The activemode interval is completed.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDurationMs() + 1);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Set two requirements
        notifier.BroadcastActiveRequestNotification(ActiveFlag::kFailSafeArmed);
        notifier.BroadcastActiveRequestNotification(ActiveFlag::kExchangeContextOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // advance time so the active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDurationMs() + 1);
        // A requirement flag is still set. We stay in active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // remove 1 requirement. Active mode is maintained
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // remove the last requirement
        notifier.BroadcastActiveRequestWithdrawal(ActiveFlag::kExchangeContextOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    /*
     * Test that verifies that the ICDManager is the correct operating mode based on entries
     * in the ICDMonitoringTable
     */
    static void TestICDMRegisterUnregisterEvents(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::ICDManagementEvents ICDMEvent;
        ICDNotifier notifier = ICDNotifier::GetInstance();

        // Set FeatureMap
        // Configures CIP, UAT and LITS to 1
        ctx->mICDManager.SetTestFeatureMapValue(0x07);

        // Check ICDManager starts in SIT mode if no entries are present
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT);

        // Trigger a "fake" register, ICDManager shoudl remain in SIT mode
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager stayed in SIT mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT);

        // Create tables with different fabrics
        ICDMonitoringTable table1(ctx->testStorage, kTestFabricIndex1, kMaxTestClients, &(ctx->mKeystore));
        ICDMonitoringTable table2(ctx->testStorage, kTestFabricIndex2, kMaxTestClients, &(ctx->mKeystore));

        // Add first entry to the first fabric
        ICDMonitoringEntry entry1(&(ctx->mKeystore));
        entry1.checkInNodeID    = kClientNodeId11;
        entry1.monitoredSubject = kClientNodeId12;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry1.SetKey(ByteSpan(kKeyBuffer1a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Set(0, entry1));

        // Trigger register event after first entry was added
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Add second entry to the first fabric
        ICDMonitoringEntry entry2(&(ctx->mKeystore));
        entry2.checkInNodeID    = kClientNodeId12;
        entry2.monitoredSubject = kClientNodeId11;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry2.SetKey(ByteSpan(kKeyBuffer1b)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Set(1, entry2));

        // Trigger register event after first entry was added
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Add first entry to the first fabric
        ICDMonitoringEntry entry3(&(ctx->mKeystore));
        entry3.checkInNodeID    = kClientNodeId21;
        entry3.monitoredSubject = kClientNodeId22;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry3.SetKey(ByteSpan(kKeyBuffer2a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table2.Set(0, entry3));

        // Trigger register event after first entry was added
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Add second entry to the first fabric
        ICDMonitoringEntry entry4(&(ctx->mKeystore));
        entry4.checkInNodeID    = kClientNodeId22;
        entry4.monitoredSubject = kClientNodeId21;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry4.SetKey(ByteSpan(kKeyBuffer2b)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table2.Set(1, entry4));

        // Clear a fabric
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table2.RemoveAll());

        // Trigger register event after fabric was cleared
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is still in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Remove single entry from remaining fabric
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Remove(1));

        // Trigger register event after fabric was cleared
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is still in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Remove last entry from remaining fabric
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Remove(0));
        NL_TEST_ASSERT(aSuite, table1.IsEmpty());
        NL_TEST_ASSERT(aSuite, table2.IsEmpty());

        // Trigger register event after fabric was cleared
        notifier.BroadcastICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is still in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT);
    }

    static void TestICDCounter(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        uint32_t counter  = ICDConfigurationData::GetInstance().GetICDCounter();
        ctx->mICDManager.IncrementCounter();
        uint32_t counter2 = ICDConfigurationData::GetInstance().GetICDCounter();
        NL_TEST_ASSERT(aSuite, (counter + 1) == counter2);
    }
};

} // namespace app
} // namespace chip

namespace {
static const nlTest sTests[] = {
    NL_TEST_DEF("TestICDModeDurations", TestICDManager::TestICDModeDurations),
    NL_TEST_DEF("TestKeepActivemodeRequests", TestICDManager::TestKeepActivemodeRequests),
    NL_TEST_DEF("TestICDMRegisterUnregisterEvents", TestICDManager::TestICDMRegisterUnregisterEvents),
    NL_TEST_DEF("TestICDCounter", TestICDManager::TestICDCounter),
    NL_TEST_SENTINEL(),
};

nlTestSuite cmSuite = {
    "TestICDManager",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
} // namespace

int TestSuiteICDManager()
{
    return ExecuteTestsWithContext<TestContext>(&cmSuite);
}

CHIP_REGISTER_TEST_SUITE(TestSuiteICDManager)
