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
#include <app/SubscriptionsInfoProvider.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDManager.h>
#include <app/icd/server/ICDNotifier.h>
#include <app/icd/server/ICDStateObserver.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestExtendedAssertions.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemLayerImpl.h>

#include <crypto/DefaultSessionKeystore.h>

using namespace chip;
using namespace chip::app;
using namespace chip::System;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

namespace {

// Test Values
constexpr uint16_t kMaxTestClients      = 2;
constexpr FabricIndex kTestFabricIndex1 = 1;
constexpr FabricIndex kTestFabricIndex2 = kMaxValidFabricIndex;
constexpr NodeId kClientNodeId11        = 0x100001;
constexpr NodeId kClientNodeId12        = 0x100002;
constexpr NodeId kClientNodeId21        = 0x200001;
constexpr NodeId kClientNodeId22        = 0x200002;

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

// Taken from the ICDManager Implementation
enum class ICDTestEventTriggerEvent : uint64_t
{
    kAddActiveModeReq            = 0x0046'0000'00000001,
    kRemoveActiveModeReq         = 0x0046'0000'00000002,
    kInvalidateHalfCounterValues = 0x0046'0000'00000003,
    kInvalidateAllCounterValues  = 0x0046'0000'00000004,
};

class TestICDStateObserver : public app::ICDStateObserver
{
public:
    void OnEnterActiveMode() { mOnEnterActiveModeCalled = true; }
    void OnEnterIdleMode() { mOnEnterIdleModeCalled = true; }
    void OnTransitionToIdle() { mOnTransitionToIdleCalled = true; }
    void OnICDModeChange() { mOnICDModeChangeCalled = true; }

    void ResetOnEnterActiveMode() { mOnEnterActiveModeCalled = false; }
    void ResetOnEnterIdleMode() { mOnEnterIdleModeCalled = false; }
    void ResetOnTransitionToIdle() { mOnTransitionToIdleCalled = false; }
    void ResetOnICDModeChange() { mOnICDModeChangeCalled = false; }
    void ResetAll()
    {
        ResetOnEnterActiveMode();
        ResetOnEnterIdleMode();
        ResetOnTransitionToIdle();
        ResetOnICDModeChange();
    }

    bool mOnEnterActiveModeCalled  = false;
    bool mOnEnterIdleModeCalled    = false;
    bool mOnICDModeChangeCalled    = false;
    bool mOnTransitionToIdleCalled = false;
};

class TestSubscriptionsInfoProvider : public SubscriptionsInfoProvider
{
public:
    TestSubscriptionsInfoProvider() = default;
    ~TestSubscriptionsInfoProvider(){};

    void SetHasActiveSubscription(bool value) { mHasActiveSubscription = value; };
    void SetHasPersistedSubscription(bool value) { mHasPersistedSubscription = value; };

    bool SubjectHasActiveSubscription(FabricIndex aFabricIndex, NodeId subject) { return mHasActiveSubscription; };
    bool SubjectHasPersistedSubscription(FabricIndex aFabricIndex, NodeId subject) { return mHasPersistedSubscription; };

private:
    bool mHasActiveSubscription    = false;
    bool mHasPersistedSubscription = false;
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
        mICDStateObserver.ResetAll();
        mICDManager.RegisterObserver(&mICDStateObserver);
        mICDManager.Init(&testStorage, &GetFabricTable(), &mKeystore, &GetExchangeManager(), &mSubInfoProvider);
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
    TestSubscriptionsInfoProvider mSubInfoProvider;
    TestPersistentStorageDelegate testStorage;
    TestICDStateObserver mICDStateObserver;

private:
    System::Clock::ClockBase * mRealClock;
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
    static void AdvanceClockAndRunEventLoop(TestContext * ctx, Milliseconds64 time)
    {
        ctx->mMockClock.AdvanceMonotonic(time);
        ctx->GetIOContext().DriveIO();
    }

    static void TestICDModeDurations(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // After the init we should be in Idle mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() + 1_s);
        // Idle mode Duration expired, ICDManager transitioned to the ActiveMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
        // Active mode Duration expired, ICDManager transitioned to the IdleMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() + 1_s);
        // Idle mode Duration expired, ICDManager transitioned to the ActiveMode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Events updating the Operation to Active mode can extend the current active mode time by 1 Active mode threshold.
        // Kick an active Threshold just before the end of the ActiveMode duration and validate that the active mode is extended.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThreshold() / 2);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThreshold());
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    /**
     * @brief Test verifies that the ICDManager starts its timers correctly based on if it will have any messages to send
     *        when the IdleMode timer expires
     */
    static void TestICDModeDurationsWith0ActiveModeDurationWithoutActiveSub(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::ICDManagementEvents ICDMEvent;
        ICDConfigurationData & icdConfigData = ICDConfigurationData::GetInstance();

        // Set FeatureMap - Configures CIP, UAT and LITS to 1
        ctx->mICDManager.SetTestFeatureMapValue(0x07);

        // Set that there are no matching subscriptions
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(false);

        // Set New durations for test case
        Milliseconds32 oldActiveModeDuration = icdConfigData.GetActiveModeDuration();
        icdConfigData.SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

        // Verify That ICDManager starts in Idle
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Reset IdleModeInterval since it was started before the ActiveModeDuration change
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Force the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is now 0
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeThreshold() + 1_ms16);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Add an entry to the ICDMonitoringTable
        ICDMonitoringTable table(ctx->testStorage, kTestFabricIndex1, kMaxTestClients, &(ctx->mKeystore));

        ICDMonitoringEntry entry(&(ctx->mKeystore));
        entry.checkInNodeID    = kClientNodeId11;
        entry.monitoredSubject = kClientNodeId11;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Set(0, entry));

        // Trigger register event after first entry was added
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, icdConfigData.GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Kick an ActiveModeThreshold since a Registration can only happen from an incoming message that would transition the ICD
        // to ActiveMode
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeThreshold() + 1_ms16);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Expire IdleMode timer - Device should be in ActiveMode since it has an ICDM registration
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Remove entry from the fabric - ICDManager won't have any messages to send
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Remove(0));
        NL_TEST_ASSERT(aSuite, table.IsEmpty());

        // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeThreshold() + 1_ms16);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Reset Old durations
        icdConfigData.SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
    }

    /**
     * @brief Test verifies that the ICDManager remains in IdleMode since it will not have any messages to send
     *        when the IdleMode timer expires
     */
    static void TestICDModeDurationsWith0ActiveModeDurationWithActiveSub(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::ICDManagementEvents ICDMEvent;
        ICDConfigurationData & icdConfigData = ICDConfigurationData::GetInstance();

        // Set FeatureMap - Configures CIP, UAT and LITS to 1
        ctx->mICDManager.SetTestFeatureMapValue(0x07);

        // Set that there are not matching subscriptions
        ctx->mSubInfoProvider.SetHasActiveSubscription(true);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);

        // Set New durations for test case
        Milliseconds32 oldActiveModeDuration = icdConfigData.GetActiveModeDuration();
        icdConfigData.SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

        // Verify That ICDManager starts in Idle
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Reset IdleModeInterval since it was started before the ActiveModeDuration change
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Force the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is now 0
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeThreshold() + 1_ms16);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Add an entry to the ICDMonitoringTable
        ICDMonitoringTable table(ctx->testStorage, kTestFabricIndex1, kMaxTestClients, &(ctx->mKeystore));

        ICDMonitoringEntry entry(&(ctx->mKeystore));
        entry.checkInNodeID    = kClientNodeId11;
        entry.monitoredSubject = kClientNodeId11;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Set(0, entry));

        // Trigger register event after first entry was added
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, icdConfigData.GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Kick an ActiveModeThreshold since a Registration can only happen from an incoming message that would transition the ICD
        // to ActiveMode
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeThreshold() + 1_ms16);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Expire IdleMode timer - Device stay in IdleMode since it has an active subscription for the ICDM entry
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Remove entry from the fabric
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Remove(0));
        NL_TEST_ASSERT(aSuite, table.IsEmpty());

        // Trigger unregister event after last entry was removed
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, icdConfigData.GetICDMode() == ICDConfigurationData::ICDMode::SIT);

        // Kick an ActiveModeThreshold since a Unregistration can only happen from an incoming message that would transition the ICD
        // to ActiveMode
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeThreshold() + 1_ms16);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetIdleModeDuration() + 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Reset Old durations
        icdConfigData.SetModeDurations(MakeOptional<Milliseconds32>(oldActiveModeDuration), NullOptional);
    }

    static void TestKeepActivemodeRequests(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::KeepActiveFlag ActiveFlag;
        ICDNotifier notifier = ICDNotifier::GetInstance();

        // Setting a requirement will transition the ICD to active mode.
        notifier.NotifyActiveRequestNotification(ActiveFlag::kCommissioningWindowOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // Advance time so active mode duration expires.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
        // Requirement flag still set. We stay in active mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Remove requirement. we should directly transition to idle mode.
        notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kCommissioningWindowOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        notifier.NotifyActiveRequestNotification(ActiveFlag::kFailSafeArmed);
        // Requirement will transition us to active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time, but by less than the active mode duration and remove the requirement.
        // We should stay in active mode.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() / 2);
        notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time again, The activemode duration is completed.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Set two requirements
        notifier.NotifyActiveRequestNotification(ActiveFlag::kFailSafeArmed);
        notifier.NotifyActiveRequestNotification(ActiveFlag::kExchangeContextOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // advance time so the active mode duration expires.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
        // A requirement flag is still set. We stay in active mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // remove 1 requirement. Active mode is maintained
        notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);
        // remove the last requirement
        notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kExchangeContextOpen);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    /**
     * @brief Test that verifies that the ICDManager is in the correct operating mode based on entries
     *        in the ICDMonitoringTable
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
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

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
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Add second entry to the first fabric
        ICDMonitoringEntry entry2(&(ctx->mKeystore));
        entry2.checkInNodeID    = kClientNodeId12;
        entry2.monitoredSubject = kClientNodeId11;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry2.SetKey(ByteSpan(kKeyBuffer1b)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Set(1, entry2));

        // Trigger register event after first entry was added
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is now in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Add first entry to the first fabric
        ICDMonitoringEntry entry3(&(ctx->mKeystore));
        entry3.checkInNodeID    = kClientNodeId21;
        entry3.monitoredSubject = kClientNodeId22;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry3.SetKey(ByteSpan(kKeyBuffer2a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table2.Set(0, entry3));

        // Trigger register event after first entry was added
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

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
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is still in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Remove single entry from remaining fabric
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Remove(1));

        // Trigger register event after fabric was cleared
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is still in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::LIT);

        // Remove last entry from remaining fabric
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table1.Remove(0));
        NL_TEST_ASSERT(aSuite, table1.IsEmpty());
        NL_TEST_ASSERT(aSuite, table2.IsEmpty());

        // Trigger register event after fabric was cleared
        notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Check ICDManager is still in the LIT operating mode
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT);
    }

    static void TestICDCounter(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        uint32_t counter  = ICDConfigurationData::GetInstance().GetICDCounter().GetValue();

        // Shut down and reinit ICDManager to increment counter
        ctx->mICDManager.Shutdown();
        ctx->mICDManager.Init(&(ctx->testStorage), &(ctx->GetFabricTable()), &(ctx->mKeystore), &(ctx->GetExchangeManager()),
                              &(ctx->mSubInfoProvider));
        ctx->mICDManager.RegisterObserver(&(ctx->mICDStateObserver));

        NL_TEST_ASSERT_EQUALS(aSuite, counter + ICDConfigurationData::kICDCounterPersistenceIncrement,
                              ICDConfigurationData::GetInstance().GetICDCounter().GetValue());
    }

    static void TestOnSubscriptionReport(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx    = static_cast<TestContext *>(aContext);
        ICDNotifier notifier = ICDNotifier::GetInstance();

        // After the init we should be in Idle mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Trigger a subscription report
        notifier.NotifySubscriptionReport();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Trigger another subscription report - active time should not be increased
        notifier.NotifySubscriptionReport();

        // Advance time so active mode interval expires.
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);

        // After the init we should be in Idle mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    /**
     * @brief Test verifies the logic of the ICDManager when it receives a StayActiveRequest
     */
    static void TestICDMStayActive(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx                    = static_cast<TestContext *>(aContext);
        ICDNotifier notifier                 = ICDNotifier::GetInstance();
        ICDConfigurationData & icdConfigData = ICDConfigurationData::GetInstance();

        // Verify That ICDManager starts in Idle
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Trigger a subscription report. Put the ICD manager into active mode.
        notifier.NotifySubscriptionReport();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time just before ActiveMode timer expires
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeDuration() - 1_ms32);
        // Confirm ICD manager is in active mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        uint32_t stayActiveRequestedMs = 20000;
        // Send a stay active request for 20 seconds
        uint32_t stayActivePromisedMs = ctx->mICDManager.StayActiveRequest(stayActiveRequestedMs);
        // confirm the promised time is the same as the requested time
        NL_TEST_ASSERT(aSuite, stayActivePromisedMs == stayActiveRequestedMs);

        // Advance time by the duration of the stay stayActiveRequestedMs - 1 ms
        AdvanceClockAndRunEventLoop(ctx, Milliseconds32(stayActiveRequestedMs) - 1_ms32);
        // Confirm ICD manager is in active mode
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time by 1ms and Confirm ICD manager is in idle mode
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Trigger a subscription report Put the ICD manager into active mode
        notifier.NotifySubscriptionReport();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time by the duration of the stay active request - 1 ms
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeDuration() - 1_ms32);
        stayActiveRequestedMs = 35000;
        // Send a stay active request for 35 seconds, which is higher than the maximum stay active duration (30 seconds)
        stayActivePromisedMs = ctx->mICDManager.StayActiveRequest(stayActiveRequestedMs);
        // confirm the promised time is the maximum stay active duration (30 seconds)
        NL_TEST_ASSERT(aSuite, stayActivePromisedMs == 30000);

        // Advance time by the duration of the max stay active duration - 1 ms
        AdvanceClockAndRunEventLoop(ctx, Milliseconds32(30000) - 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time by 1ms and Confirm ICD manager is in idle mode
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Trigger a subscription report Put the ICD manager into active mode
        notifier.NotifySubscriptionReport();
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance time by the duration of the stay active request - 1 ms
        AdvanceClockAndRunEventLoop(ctx, icdConfigData.GetActiveModeDuration() - 1_ms32);
        stayActiveRequestedMs = 30000;
        // Send a stay active request for 30 seconds
        stayActivePromisedMs = ctx->mICDManager.StayActiveRequest(stayActiveRequestedMs);
        // confirm the promised time is the same as the requested time
        NL_TEST_ASSERT(aSuite, stayActivePromisedMs == 30000);

        // Advance time by the duration of the stay active request - 20000 ms
        AdvanceClockAndRunEventLoop(ctx, Milliseconds32(stayActiveRequestedMs) - 20000_ms32);
        // Confirm ICD manager is in active mode, we should have 20000 seconds left at that point
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        stayActiveRequestedMs = 10000;
        stayActivePromisedMs  = ctx->mICDManager.StayActiveRequest(stayActiveRequestedMs);
        // confirm the promised time is 20000 since the device is already planing to stay active longer than the requested time
        NL_TEST_ASSERT(aSuite, stayActivePromisedMs == 20000);
    }

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    static void TestShouldCheckInMsgsBeSentAtActiveModeFunction(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Test 1 - Has no ActiveSubscription & no persisted subscription
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(false);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

        // Test 2 - Has no active subscription & a persisted subscription
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11)));

        // Test 3 - Has an active subscription & a persisted subscription
        ctx->mSubInfoProvider.SetHasActiveSubscription(true);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11)));
    }
#else
    static void TestShouldCheckInMsgsBeSentAtActiveModeFunction(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Test 1 - Has no active subscription and no persisted subscription at boot up
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(false);
        ctx->mICDManager.mIsBootUpResumeSubscriptionExecuted = false;
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

        // Test 2 - Has no active subscription and a persisted subscription at boot up
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);
        ctx->mICDManager.mIsBootUpResumeSubscriptionExecuted = false;
        NL_TEST_ASSERT(aSuite, !(ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11)));

        // Test 3 - Has an active subscription and a persisted subscription during normal operations
        ctx->mSubInfoProvider.SetHasActiveSubscription(true);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);
        ctx->mICDManager.mIsBootUpResumeSubscriptionExecuted = true;
        NL_TEST_ASSERT(aSuite, !(ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11)));

        // Test 4 - Has no active subscription and a persisted subscription during normal operations
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);
        ctx->mICDManager.mIsBootUpResumeSubscriptionExecuted = true;
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));
    }
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#else
    static void TestShouldCheckInMsgsBeSentAtActiveModeFunction(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Test 1 - Has an active subscription
        ctx->mSubInfoProvider.SetHasActiveSubscription(true);
        NL_TEST_ASSERT(aSuite,
                       ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11) == false);

        // Test 2 - Has no active subscription
        ctx->mSubInfoProvider.SetHasActiveSubscription(false);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

        // Test 3 - Make sure that the persisted subscription has no impact
        ctx->mSubInfoProvider.SetHasPersistedSubscription(true);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));
    }
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

    static void TestHandleTestEventTriggerActiveModeReq(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Verify That ICDManager starts in Idle
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);

        // Add ActiveMode req for the Test event trigger event
        ctx->mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kAddActiveModeReq));
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Advance clock by the ActiveModeDuration and check that the device is still in ActiveMode
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::ActiveMode);

        // Remove req and device should go to IdleMode
        ctx->mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kRemoveActiveModeReq));
        NL_TEST_ASSERT(aSuite, ctx->mICDManager.mOperationalState == ICDManager::OperationalState::IdleMode);
    }

    static void TestHandleTestEventTriggerInvalidateHalfCounterValues(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        constexpr uint32_t startValue    = 1;
        constexpr uint32_t expectedValue = 2147483648;

        // Set starting value for test
        ICDConfigurationData::GetInstance().GetICDCounter().SetValue(startValue);

        // Trigger ICD kInvalidateHalfCounterValues event
        ctx->mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kInvalidateHalfCounterValues));

        // Validate counter has the expected value
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDCounter().GetValue() == expectedValue);
    }

    static void TestHandleTestEventTriggerInvalidateAllCounterValues(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        constexpr uint32_t startValue    = 105;
        constexpr uint32_t expectedValue = 104;

        // Set starting value for test
        ICDConfigurationData::GetInstance().GetICDCounter().SetValue(startValue);

        // Trigger ICD kInvalidateHalfCounterValues event
        ctx->mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kInvalidateAllCounterValues));

        // Validate counter has the expected value
        NL_TEST_ASSERT(aSuite, ICDConfigurationData::GetInstance().GetICDCounter().GetValue() == expectedValue);
    }

    /**
     * @brief Test verifies when OnEnterIdleMode is called during normal operations.
     *        Without the ActiveMode timer being extended
     */
    static void TestICDStateObserverOnEnterIdleModeActiveModeDuration(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Verify that ICDManager starts in IdleMode and calls OnEnterIdleMode
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnEnterIdleModeCalled);
        ctx->mICDStateObserver.ResetOnEnterIdleMode();

        // Advance clock just before IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Expire IdleModeInterval
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Advance clock Just before ActiveMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Expire ActiveMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnEnterIdleModeCalled);
    }

    /**
     * @brief Test verifies when OnEnterIdleMode is called with the ActiveMode timer gets extended
     */
    static void TestICDStateObserverOnEnterIdleModeActiveModeThreshold(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Verify that ICDManager starts in IdleMode and calls OnEnterIdleMode
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnEnterIdleModeCalled);
        ctx->mICDStateObserver.ResetOnEnterIdleMode();

        // Advance clock just before the IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Expire IdleMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Advance clock Just before ActiveMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Increase ActiveMode timer by one ActiveModeThreshold
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Advance clock Just before ActiveMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThreshold() - 1_ms32);
        NL_TEST_ASSERT(aSuite, !ctx->mICDStateObserver.mOnEnterIdleModeCalled);

        // Expire ActiveMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnEnterIdleModeCalled);
    }

    static void TestICDStateObserverOnEnterActiveMode(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Verify OnEnterActiveMode wasn't called at Init
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Advance clock just before IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Expire IdleMode timer and check wether OnEnterActiveMode was called
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnEnterActiveModeCalled);
        ctx->mICDStateObserver.ResetOnEnterActiveMode();

        // Advance clock just before the ActiveMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);

        // Verify OnEnterActiveMde wasn't called
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Increase ActiveMode timer by one ActiveModeThreshold
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();

        // Verify OnEnterActiveMde wasn't called
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Advance clock just before ActiveMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThreshold() - 1_ms32);

        // Verify OnEnterActiveMde wasn't called
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Expire ActiveMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);

        // Verify OnEnterActiveMde wasn't called
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Advance clock just before IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnEnterActiveModeCalled));

        // Expire IdleMode timer and check OnEnterActiveMode was called
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnEnterActiveModeCalled);
    }

    static void TestICDStateObserverOnICDModeChange(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);
        typedef ICDListener::ICDManagementEvents ICDMEvent;

        // Since we don't have a registration, we stay in SIT mode. No changes
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnICDModeChangeCalled));

        // Trigger register event to force ICDManager to re-evaluate OperatingMode
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Since we don't have a registration, we stay in SIT mode. No changes
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnICDModeChangeCalled));

        // Add an entry to the ICDMonitoringTable
        ICDMonitoringTable table(ctx->testStorage, kTestFabricIndex1, kMaxTestClients, &(ctx->mKeystore));

        ICDMonitoringEntry entry(&(ctx->mKeystore));
        entry.checkInNodeID    = kClientNodeId11;
        entry.monitoredSubject = kClientNodeId11;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Set(0, entry));

        // Trigger register event after first entry was added
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // We have a registration. Transition to LIT mode
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnICDModeChangeCalled);
        ctx->mICDStateObserver.ResetOnICDModeChange();

        // Trigger register event to force ICDManager to re-evaluate OperatingMode
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // We have a registration. We stay in LIT mode. No changes.
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnICDModeChangeCalled));

        // Remove entry from the ICDMonitoringTable
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Remove(0));
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

        // Since we don't have a registration anymore. Transition to SIT mode.
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnICDModeChangeCalled);
        ctx->mICDStateObserver.ResetOnICDModeChange();
    }

    static void TestICDStateObserverOnICDModeChangeOnInit(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        ICDMonitoringTable table(ctx->testStorage, kTestFabricIndex1, kMaxTestClients, &(ctx->mKeystore));

        // Since we don't have a registration, we stay in SIT mode. No changes
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnICDModeChangeCalled));

        // Add an entry to the ICDMonitoringTable
        ICDMonitoringEntry entry(&(ctx->mKeystore));
        entry.checkInNodeID    = kClientNodeId11;
        entry.monitoredSubject = kClientNodeId11;
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == entry.SetKey(ByteSpan(kKeyBuffer1a)));
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Set(0, entry));

        // Shut down and reinit ICDManager - We should go to LIT mode since we have a registration
        ctx->mICDManager.Shutdown();
        ctx->mICDManager.RegisterObserver(&(ctx->mICDStateObserver));
        ctx->mICDManager.Init(&(ctx->testStorage), &(ctx->GetFabricTable()), &(ctx->mKeystore), &(ctx->GetExchangeManager()),
                              &(ctx->mSubInfoProvider));

        // We have a registration, transition to LIT mode
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnICDModeChangeCalled);
        ctx->mICDStateObserver.ResetOnICDModeChange();

        // Remove entry from the ICDMonitoringTable
        NL_TEST_ASSERT(aSuite, CHIP_NO_ERROR == table.Remove(0));
    }

    /**
     * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is greater than the
     *        ICD_ACTIVE_TIME_JITTER_MS
     */
    static void TestICDStateObserverOnTransitionToIdleModeGreaterActiveModeDuration(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Set New durations for test case - ActiveModeDuration must be longuer than ICD_ACTIVE_TIME_JITTER_MS
        Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
        ICDConfigurationData::GetInstance().SetModeDurations(
            MakeOptional<Milliseconds32>(Milliseconds32(200) + Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS)), NullOptional);

        // Advance clock just before IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Expire IdleMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Advance time just before OnTransitionToIdleMode is called
        AdvanceClockAndRunEventLoop(
            ctx, ICDConfigurationData::GetInstance().GetActiveModeDuration() - Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS) - 1_ms32);

        // Check mOnTransitionToIdleCalled has not been called
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Increase ActiveMode timer by one ActiveModeThreshold
        ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Advance time just before OnTransitionToIdleMode is called
        AdvanceClockAndRunEventLoop(
            ctx, ICDConfigurationData::GetInstance().GetActiveModeThreshold() - Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS) - 1_ms32);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Expire OnTransitionToIdleMode
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        // Check mOnTransitionToIdleCalled has been called
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnTransitionToIdleCalled);
        ctx->mICDStateObserver.ResetOnTransitionToIdle();

        // Expire ActiveMode timer
        AdvanceClockAndRunEventLoop(ctx, Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS));
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Reset Old durations
        ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
    }

    /**
     * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is equal to the
     *        ICD_ACTIVE_TIME_JITTER_MS.
     */
    static void TestICDStateObserverOnTransitionToIdleModeEqualActiveModeDuration(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Set New durations for test case - ActiveModeDuration must be equal to ICD_ACTIVE_TIME_JITTER_MS
        Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
        ICDConfigurationData::GetInstance().SetModeDurations(
            MakeOptional<Milliseconds32>(Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS)), NullOptional);

        // Advance clock just before IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Expire IdleMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Expire OnTransitionToIdleMode
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnTransitionToIdleCalled);

        // Reset Old durations
        ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
    }

    /**
     * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is 0 and without an ActiveMode req
     */
    static void TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWithoutReq(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Set New durations for test case - ActiveModeDuration equal 0
        Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
        ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

        // Advance clock just before IdleMode timer expires
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Expire IdleMode timer
        AdvanceClockAndRunEventLoop(ctx, 1_s);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Increase time by 1 - Should not trigger OnTransitionToIdle.
        // Timer length is one ActiveModeThreshold
        AdvanceClockAndRunEventLoop(ctx, 1_ms32);
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Expire ActiveModeThreshold
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetActiveModeThreshold());
        NL_TEST_ASSERT(aSuite, ctx->mICDStateObserver.mOnTransitionToIdleCalled);

        // Reset Old durations
        ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
    }

    /**
     * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is 0 with an ActiveMode req
     */
    static void TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWittReq(nlTestSuite * aSuite, void * aContext)
    {
        TestContext * ctx = static_cast<TestContext *>(aContext);

        // Set New durations for test case - ActiveModeDuration equal 0
        Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
        ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

        // Add ActiveMode req for the Test event trigger event
        ctx->mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kAddActiveModeReq));

        // Expire IdleMode timer
        AdvanceClockAndRunEventLoop(ctx, ICDConfigurationData::GetInstance().GetIdleModeDuration());
        NL_TEST_ASSERT(aSuite, !(ctx->mICDStateObserver.mOnTransitionToIdleCalled));

        // Reset Old durations
        ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
    }
};

} // namespace app
} // namespace chip

namespace {
static const nlTest sTests[] = {
    NL_TEST_DEF("TestICDModeDurations", TestICDManager::TestICDModeDurations),
    NL_TEST_DEF("TestOnSubscriptionReport", TestICDManager::TestOnSubscriptionReport),
    NL_TEST_DEF("TestICDModeDurationsWith0ActiveModeDurationWithoutActiveSub",
                TestICDManager::TestICDModeDurationsWith0ActiveModeDurationWithoutActiveSub),
    NL_TEST_DEF("TestICDModeDurationsWith0ActiveModeDurationWithActiveSub",
                TestICDManager::TestICDModeDurationsWith0ActiveModeDurationWithActiveSub),
    NL_TEST_DEF("TestKeepActivemodeRequests", TestICDManager::TestKeepActivemodeRequests),
    NL_TEST_DEF("TestICDMRegisterUnregisterEvents", TestICDManager::TestICDMRegisterUnregisterEvents),
    NL_TEST_DEF("TestICDCounter", TestICDManager::TestICDCounter),
    NL_TEST_DEF("TestICDStayActive", TestICDManager::TestICDMStayActive),
    NL_TEST_DEF("TestShouldCheckInMsgsBeSentAtActiveModeFunction", TestICDManager::TestShouldCheckInMsgsBeSentAtActiveModeFunction),
    NL_TEST_DEF("TestHandleTestEventTriggerActiveModeReq", TestICDManager::TestHandleTestEventTriggerActiveModeReq),
    NL_TEST_DEF("TestHandleTestEventTriggerInvalidateHalfCounterValues",
                TestICDManager::TestHandleTestEventTriggerInvalidateHalfCounterValues),
    NL_TEST_DEF("TestHandleTestEventTriggerInvalidateAllCounterValues",
                TestICDManager::TestHandleTestEventTriggerInvalidateAllCounterValues),
    NL_TEST_DEF("TestICDStateObserverOnEnterIdleModeActiveModeDuration",
                TestICDManager::TestICDStateObserverOnEnterIdleModeActiveModeDuration),
    NL_TEST_DEF("TestICDStateObserverOnEnterIdleModeActiveModeThreshold",
                TestICDManager::TestICDStateObserverOnEnterIdleModeActiveModeThreshold),
    NL_TEST_DEF("TestICDStateObserverOnEnterActiveMode", TestICDManager::TestICDStateObserverOnEnterActiveMode),
    NL_TEST_DEF("TestICDStateObserverOnICDModeChange", TestICDManager::TestICDStateObserverOnICDModeChange),
    NL_TEST_DEF("TestICDStateObserverOnICDModeChangeOnInit", TestICDManager::TestICDStateObserverOnICDModeChangeOnInit),
    NL_TEST_DEF("TestICDStateObserverOnTransitionToIdleModeGreaterActiveModeDuration",
                TestICDManager::TestICDStateObserverOnTransitionToIdleModeGreaterActiveModeDuration),
    NL_TEST_DEF("TestICDStateObserverOnTransitionToIdleModeEqualActiveModeDuration",
                TestICDManager::TestICDStateObserverOnTransitionToIdleModeEqualActiveModeDuration),
    NL_TEST_DEF("TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWithoutReq",
                TestICDManager::TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWithoutReq),
    // TODO(#33074): When the OnTransitionToIdle edge is fixed, we can enable this test
    // NL_TEST_DEF("TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWittReq",
    //             TestICDManager::TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWittReq),
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
