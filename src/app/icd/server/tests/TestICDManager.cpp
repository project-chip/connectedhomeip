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

#include <app-common/zap-generated/cluster-enums.h>
#include <pw_unit_test/framework.h>

#include <app/SubscriptionsInfoProvider.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/icd/server/DefaultICDCheckInBackOffStrategy.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDManager.h>
#include <app/icd/server/ICDMonitoringTable.h>
#include <app/icd/server/ICDNotifier.h>
#include <app/icd/server/ICDStateObserver.h>
#include <app/icd/server/tests/ICDConfigurationDataTestAccess.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/address_resolve/AddressResolve.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/TimeUtils.h>
#include <messaging/tests/MessagingContext.h>
#include <system/SystemLayerImpl.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::AddressResolve;
using namespace chip::System;
using namespace chip::System::Clock;
using namespace chip::System::Clock::Literals;

using TestSessionKeystoreImpl = Crypto::DefaultSessionKeystore;

namespace {

#if CHIP_CONFIG_ENABLE_ICD_CIP
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
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

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
    TestICDStateObserver()  = default;
    ~TestICDStateObserver() = default;

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
    bool FabricHasAtLeastOneActiveSubscription(FabricIndex aFabricIndex) { return false; };

private:
    bool mHasActiveSubscription    = false;
    bool mHasPersistedSubscription = false;
};

System::Clock::Internal::MockClock * pMockClock = nullptr;
System::Clock::ClockBase * pRealClock           = nullptr;

} // namespace

namespace chip {
namespace app {

class TestICDManager : public Test::LoopbackMessagingContext
{
public:
    /*
     * Advance the test Mock clock time by the amout passed in argument
     * and then force the SystemLayer Timer event loop. It will check for any expired timer,
     * and invoke their callbacks if there are any.
     *
     * @param time_ms: Value in milliseconds.
     */
    static void AdvanceClockAndRunEventLoop(Milliseconds64 time)
    {
        pMockClock->AdvanceMonotonic(time);
        GetIOContext().DriveIO();
    }

    // Performs shared setup for all tests in the test suite
    static void SetUpTestSuite()
    {
        if (pMockClock == nullptr)
        {
            pMockClock = new Clock::Internal::MockClock();
            ASSERT_NE(pMockClock, nullptr);
        }

        LoopbackMessagingContext::SetUpTestSuite();
        VerifyOrReturn(!HasFailure());

        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

        DeviceLayer::SetSystemLayerForTesting(&GetSystemLayer());
        pRealClock = &SystemClock();
        Clock::Internal::SetSystemClockForTesting(pMockClock);
    }

    // Performs shared teardown for all tests in the test suite
    static void TearDownTestSuite()
    {
        Clock::Internal::SetSystemClockForTesting(pRealClock);
        DeviceLayer::SetSystemLayerForTesting(nullptr);

        DeviceLayer::PlatformMgr().Shutdown();

        LoopbackMessagingContext::TearDownTestSuite();

        if (pMockClock != nullptr)
        {
            delete pMockClock;
            pMockClock = nullptr;
        }

        pRealClock = nullptr;
    }

    // Performs setup for each individual test in the test suite
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        VerifyOrReturn(!HasFailure());

        mICDStateObserver.ResetAll();
        mICDManager.RegisterObserver(&mICDStateObserver);

#if CHIP_CONFIG_ENABLE_ICD_CIP
        mICDManager.SetPersistentStorageDelegate(&testStorage)
            .SetFabricTable(&GetFabricTable())
            .SetSymmetricKeyStore(&mKeystore)
            .SetExchangeManager(&GetExchangeManager())
            .SetSubscriptionsInfoProvider(&mSubInfoProvider)
            .SetICDCheckInBackOffStrategy(&mStrategy);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
        mICDManager.Init();
    }

    // Performs teardown for each individual test in the test suite
    void TearDown() override
    {
        mICDManager.Shutdown();
        LoopbackMessagingContext::TearDown();
    }

    TestSessionKeystoreImpl mKeystore;
    ICDManager mICDManager;
    TestSubscriptionsInfoProvider mSubInfoProvider;
    TestPersistentStorageDelegate testStorage;
    TestICDStateObserver mICDStateObserver;
    DefaultICDCheckInBackOffStrategy mStrategy;
};

TEST_F(TestICDManager, TestICDModeDurations)
{
    // After the init we should be in Idle mode
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() + 1_s);
    // Idle mode Duration expired, ICDManager transitioned to the ActiveMode.
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
    // Active mode Duration expired, ICDManager transitioned to the IdleMode.
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() + 1_s);
    // Idle mode Duration expired, ICDManager transitioned to the ActiveMode.
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Events updating the Operation to Active mode can extend the current active mode time by 1 Active mode threshold.
    // Kick an active Threshold just before the end of the ActiveMode duration and validate that the active mode is extended.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeThreshold() / 2);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeThreshold());
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);
}

#if CHIP_CONFIG_ENABLE_ICD_CIP
/**
 * @brief Test verifies that the ICDManager starts its timers correctly based on if it will have any messages to send
 *        when the IdleMode timer expires
 */
TEST_F(TestICDManager, TestICDModeDurationsWith0ActiveModeDurationWithoutActiveSub)
{
    typedef ICDListener::ICDManagementEvents ICDMEvent;
    ICDConfigurationData & icdConfigData = ICDConfigurationData::GetInstance();
    ICDConfigurationDataTestAccess privateIcdConfigData(&icdConfigData);

    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport).Set(Feature::kUserActiveModeTrigger).Set(Feature::kCheckInProtocolSupport);
    privateIcdConfigData.SetFeatureMap(featureMap);

    // Set that there are no matching subscriptions
    mSubInfoProvider.SetHasActiveSubscription(false);
    mSubInfoProvider.SetHasPersistedSubscription(false);

    // Set New durations for test case
    Milliseconds32 oldActiveModeDuration = icdConfigData.GetActiveModeDuration();
    mICDManager.SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

    // Verify That ICDManager starts in Idle
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Reset IdleModeInterval since it was started before the ActiveModeDuration change
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Force the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is now 0
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeThreshold() + 1_ms16);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Add an entry to the ICDMonitoringTable
    ICDMonitoringTable table(testStorage, kTestFabricIndex1, kMaxTestClients, &(mKeystore));

    ICDMonitoringEntry entry(&(mKeystore));
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table.Set(0, entry));

    // Trigger register event after first entry was added
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(icdConfigData.GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Kick an ActiveModeThreshold since a Registration can only happen from an incoming message that would transition the ICD
    // to ActiveMode
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeThreshold() + 1_ms16);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Expire IdleMode timer - Device should be in ActiveMode since it has an ICDM registration
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Remove entry from the fabric - ICDManager won't have any messages to send
    EXPECT_EQ(CHIP_NO_ERROR, table.Remove(0));
    EXPECT_TRUE(table.IsEmpty());

    // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeThreshold() + 1_ms16);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Reset Old durations
    mICDManager.SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
}

/**
 * @brief Test verifies that the ICDManager remains in IdleMode since it will not have any messages to send
 *        when the IdleMode timer expires
 */
TEST_F(TestICDManager, TestICDModeDurationsWith0ActiveModeDurationWithActiveSub)
{
    typedef ICDListener::ICDManagementEvents ICDMEvent;
    ICDConfigurationData & icdConfigData = ICDConfigurationData::GetInstance();
    ICDConfigurationDataTestAccess privateIcdConfigData(&icdConfigData);

    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport).Set(Feature::kUserActiveModeTrigger).Set(Feature::kCheckInProtocolSupport);
    privateIcdConfigData.SetFeatureMap(featureMap);

    // Set that there are not matching subscriptions
    mSubInfoProvider.SetHasActiveSubscription(true);
    mSubInfoProvider.SetHasPersistedSubscription(true);

    // Set New durations for test case
    Milliseconds32 oldActiveModeDuration = icdConfigData.GetActiveModeDuration();
    mICDManager.SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

    // Verify That ICDManager starts in Idle
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Reset IdleModeInterval since it was started before the ActiveModeDuration change
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Force the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is now 0
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeThreshold() + 1_ms16);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Add an entry to the ICDMonitoringTable
    ICDMonitoringTable table(testStorage, kTestFabricIndex1, kMaxTestClients, &(mKeystore));

    ICDMonitoringEntry entry(&(mKeystore));
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table.Set(0, entry));

    // Trigger register event after first entry was added
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(icdConfigData.GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Kick an ActiveModeThreshold since a Registration can only happen from an incoming message that would transition the ICD
    // to ActiveMode
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeThreshold() + 1_ms16);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Expire IdleMode timer - Device stay in IdleMode since it has an active subscription for the ICDM entry
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Remove entry from the fabric
    EXPECT_EQ(CHIP_NO_ERROR, table.Remove(0));
    EXPECT_TRUE(table.IsEmpty());

    // Trigger unregister event after last entry was removed
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(icdConfigData.GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Kick an ActiveModeThreshold since a Unregistration can only happen from an incoming message that would transition the ICD
    // to ActiveMode
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Return the device to return to IdleMode - Increase time by ActiveModeThreshold since ActiveModeDuration is 0
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeThreshold() + 1_ms16);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Expire Idle mode duration; ICDManager should remain in IdleMode since it has no message to send
    AdvanceClockAndRunEventLoop(icdConfigData.GetIdleModeDuration() + 1_s);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Reset Old durations
    mICDManager.SetModeDurations(MakeOptional<Milliseconds32>(oldActiveModeDuration), NullOptional);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

TEST_F(TestICDManager, TestKeepActivemodeRequests)
{
    typedef ICDListener::KeepActiveFlag ActiveFlag;
    ICDNotifier notifier = ICDNotifier::GetInstance();

    // Setting a requirement will transition the ICD to active mode.
    notifier.NotifyActiveRequestNotification(ActiveFlag::kCommissioningWindowOpen);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);
    // Advance time so active mode duration expires.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
    // Requirement flag still set. We stay in active mode
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Remove requirement. we should directly transition to idle mode.
    notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kCommissioningWindowOpen);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    notifier.NotifyActiveRequestNotification(ActiveFlag::kFailSafeArmed);
    // Requirement will transition us to active mode.
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time, but by less than the active mode duration and remove the requirement.
    // We should stay in active mode.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() / 2);
    notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time again, The activemode duration is completed.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Set two requirements
    notifier.NotifyActiveRequestNotification(ActiveFlag::kFailSafeArmed);
    notifier.NotifyActiveRequestNotification(ActiveFlag::kExchangeContextOpen);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);
    // advance time so the active mode duration expires.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
    // A requirement flag is still set. We stay in active mode.
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // remove 1 requirement. Active mode is maintained
    notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kFailSafeArmed);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);
    // remove the last requirement
    notifier.NotifyActiveRequestWithdrawal(ActiveFlag::kExchangeContextOpen);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);
}

#if CHIP_CONFIG_ENABLE_ICD_CIP
/**
 * @brief Test that verifies that the ICDManager is in the correct operating mode based on entries
 *        in the ICDMonitoringTable
 */
TEST_F(TestICDManager, TestICDMRegisterUnregisterEvents)
{
    typedef ICDListener::ICDManagementEvents ICDMEvent;
    ICDNotifier notifier = ICDNotifier::GetInstance();
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport).Set(Feature::kUserActiveModeTrigger).Set(Feature::kCheckInProtocolSupport);
    privateIcdConfigData.SetFeatureMap(featureMap);

    // Check ICDManager starts in SIT mode if no entries are present
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Trigger a "fake" register, ICDManager shoudl remain in SIT mode
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager stayed in SIT mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Create tables with different fabrics
    ICDMonitoringTable table1(testStorage, kTestFabricIndex1, kMaxTestClients, &(mKeystore));
    ICDMonitoringTable table2(testStorage, kTestFabricIndex2, kMaxTestClients, &(mKeystore));

    // Add first entry to the first fabric
    ICDMonitoringEntry entry1(&(mKeystore));
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_NO_ERROR, entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table1.Set(0, entry1));

    // Trigger register event after first entry was added
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Add second entry to the first fabric
    ICDMonitoringEntry entry2(&(mKeystore));
    entry2.checkInNodeID    = kClientNodeId12;
    entry2.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry2.SetKey(ByteSpan(kKeyBuffer1b)));
    EXPECT_EQ(CHIP_NO_ERROR, table1.Set(1, entry2));

    // Trigger register event after first entry was added
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Add first entry to the first fabric
    ICDMonitoringEntry entry3(&(mKeystore));
    entry3.checkInNodeID    = kClientNodeId21;
    entry3.monitoredSubject = kClientNodeId22;
    EXPECT_EQ(CHIP_NO_ERROR, entry3.SetKey(ByteSpan(kKeyBuffer2a)));
    EXPECT_EQ(CHIP_NO_ERROR, table2.Set(0, entry3));

    // Trigger register event after first entry was added
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Add second entry to the first fabric
    ICDMonitoringEntry entry4(&(mKeystore));
    entry4.checkInNodeID    = kClientNodeId22;
    entry4.monitoredSubject = kClientNodeId21;
    EXPECT_EQ(CHIP_NO_ERROR, entry4.SetKey(ByteSpan(kKeyBuffer2b)));
    EXPECT_EQ(CHIP_NO_ERROR, table2.Set(1, entry4));

    // Clear a fabric
    EXPECT_EQ(CHIP_NO_ERROR, table2.RemoveAll());

    // Trigger register event after fabric was cleared
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is still in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Remove single entry from remaining fabric
    EXPECT_EQ(CHIP_NO_ERROR, table1.Remove(1));

    // Trigger register event after fabric was cleared
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is still in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Remove last entry from remaining fabric
    EXPECT_EQ(CHIP_NO_ERROR, table1.Remove(0));
    EXPECT_TRUE(table1.IsEmpty());
    EXPECT_TRUE(table2.IsEmpty());

    // Trigger register event after fabric was cleared
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is still in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::SIT);
}

TEST_F(TestICDManager, TestICDCounter)
{
    uint32_t counter = ICDConfigurationData::GetInstance().GetICDCounter().GetValue();

    // Shut down and reinit ICDManager to increment counter
    mICDManager.Shutdown();
#if CHIP_CONFIG_ENABLE_ICD_CIP
    mICDManager.SetPersistentStorageDelegate(&testStorage)
        .SetFabricTable(&GetFabricTable())
        .SetSymmetricKeyStore(&mKeystore)
        .SetExchangeManager(&GetExchangeManager())
        .SetSubscriptionsInfoProvider(&mSubInfoProvider)
        .SetICDCheckInBackOffStrategy(&mStrategy);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
    mICDManager.Init();

    mICDManager.RegisterObserver(&(mICDStateObserver));

    EXPECT_EQ(counter + ICDConfigurationData::kICDCounterPersistenceIncrement,
              ICDConfigurationData::GetInstance().GetICDCounter().GetValue());
}

TEST_F(TestICDManager, TestOnSubscriptionReport)
{
    ICDNotifier notifier = ICDNotifier::GetInstance();

    // After the init we should be in Idle mode
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Trigger a subscription report
    notifier.NotifySubscriptionReport();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Trigger another subscription report - active time should not be increased
    notifier.NotifySubscriptionReport();

    // Advance time so active mode interval expires.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);

    // After the init we should be in Idle mode
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

/**
 * @brief Test verifies the logic of the ICDManager when it receives a StayActiveRequest
 */
TEST_F(TestICDManager, TestICDMStayActive)
{
    ICDNotifier notifier                 = ICDNotifier::GetInstance();
    ICDConfigurationData & icdConfigData = ICDConfigurationData::GetInstance();

    // Verify That ICDManager starts in Idle
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Trigger a subscription report. Put the ICD manager into active mode.
    notifier.NotifySubscriptionReport();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time just before ActiveMode timer expires
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeDuration() - 1_ms32);
    // Confirm ICD manager is in active mode
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    uint32_t stayActiveRequestedMs = 20000;
    // Send a stay active request for 20 seconds
    uint32_t stayActivePromisedMs = mICDManager.StayActiveRequest(stayActiveRequestedMs);
    // confirm the promised time is the same as the requested time
    EXPECT_EQ(stayActivePromisedMs, stayActiveRequestedMs);

    // Advance time by the duration of the stay stayActiveRequestedMs - 1 ms
    AdvanceClockAndRunEventLoop(Milliseconds32(stayActiveRequestedMs) - 1_ms32);
    // Confirm ICD manager is in active mode
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time by 1ms and Confirm ICD manager is in idle mode
    AdvanceClockAndRunEventLoop(1_ms32);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Trigger a subscription report Put the ICD manager into active mode
    notifier.NotifySubscriptionReport();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time by the duration of the stay active request - 1 ms
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeDuration() - 1_ms32);
    stayActiveRequestedMs = 35000;
    // Send a stay active request for 35 seconds, which is higher than the maximum stay active duration (30 seconds)
    stayActivePromisedMs = mICDManager.StayActiveRequest(stayActiveRequestedMs);
    // confirm the promised time is the maximum stay active duration (30 seconds)
    EXPECT_EQ(stayActivePromisedMs, 30000UL);

    // Advance time by the duration of the max stay active duration - 1 ms
    AdvanceClockAndRunEventLoop(Milliseconds32(30000) - 1_ms32);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time by 1ms and Confirm ICD manager is in idle mode
    AdvanceClockAndRunEventLoop(1_ms32);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Trigger a subscription report Put the ICD manager into active mode
    notifier.NotifySubscriptionReport();
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance time by the duration of the stay active request - 1 ms
    AdvanceClockAndRunEventLoop(icdConfigData.GetActiveModeDuration() - 1_ms32);
    stayActiveRequestedMs = 30000;
    // Send a stay active request for 30 seconds
    stayActivePromisedMs = mICDManager.StayActiveRequest(stayActiveRequestedMs);
    // confirm the promised time is the same as the requested time
    EXPECT_EQ(stayActivePromisedMs, 30000UL);

    // Advance time by the duration of the stay active request - 20000 ms
    AdvanceClockAndRunEventLoop(Milliseconds32(stayActiveRequestedMs) - 20000_ms32);
    // Confirm ICD manager is in active mode, we should have 20000 seconds left at that point
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    stayActiveRequestedMs = 10000;
    stayActivePromisedMs  = mICDManager.StayActiveRequest(stayActiveRequestedMs);
    // confirm the promised time is 20000 since the device is already planing to stay active longer than the requested time
    EXPECT_EQ(stayActivePromisedMs, 20000UL);
}

#if CHIP_CONFIG_ENABLE_ICD_DSLS
/**
 * @brief Test verifies the logic of the ICDManager related to DSLS (Dynamic SIT LIT Support)
 */
TEST_F(TestICDManager, TestICDMDSLS)
{
    typedef ICDListener::ICDManagementEvents ICDMEvent;
    ICDNotifier notifier = ICDNotifier::GetInstance();
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport)
        .Set(Feature::kUserActiveModeTrigger)
        .Set(Feature::kCheckInProtocolSupport)
        .Set(Feature::kDynamicSitLitSupport);
    privateIcdConfigData.SetFeatureMap(featureMap);

    // Check ICDManager starts in SIT mode if no entries are present
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Create table with one fabric
    ICDMonitoringTable table1(testStorage, kTestFabricIndex1, kMaxTestClients, &(mKeystore));

    // Add an entry to the fabric
    ICDMonitoringEntry entry1(&(mKeystore));
    entry1.checkInNodeID    = kClientNodeId11;
    entry1.monitoredSubject = kClientNodeId12;
    EXPECT_EQ(CHIP_NO_ERROR, entry1.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table1.Set(0, entry1));

    // Trigger register event after first entry was added
    notifier.NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Simulate SIT Mode Request - device must switch to SIT mode even if there is a client registered
    notifier.NotifySITModeRequestNotification();

    // Check ICDManager is now in the SIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Advance time so active mode interval expires.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);

    // Check ICDManager is still in the SIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::SIT);

    // Withdraw SIT mode
    notifier.NotifySITModeRequestWithdrawal();

    // Check ICDManager is now in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);

    // Advance time so active mode interval expires.
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);

    // Check ICDManager is still in the LIT operating mode
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDMode(), ICDConfigurationData::ICDMode::LIT);
}
#endif // CHIP_CONFIG_ENABLE_ICD_DSLS

#if CHIP_CONFIG_ENABLE_ICD_CIP
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
TEST_F(TestICDManager, TestShouldCheckInMsgsBeSentAtActiveModeFunction)
{
    // Test 1 - Has no ActiveSubscription & no persisted subscription
    mSubInfoProvider.SetHasActiveSubscription(false);
    mSubInfoProvider.SetHasPersistedSubscription(false);
    EXPECT_TRUE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    // Test 2 - Has no active subscription & a persisted subscription
    mSubInfoProvider.SetHasActiveSubscription(false);
    mSubInfoProvider.SetHasPersistedSubscription(true);
    EXPECT_FALSE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    // Test 3 - Has an active subscription & a persisted subscription
    mSubInfoProvider.SetHasActiveSubscription(true);
    mSubInfoProvider.SetHasPersistedSubscription(true);
    EXPECT_FALSE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));
}
#else
TEST_F(TestICDManager, TestShouldCheckInMsgsBeSentAtActiveModeFunction)
{
    // Test 1 - Has no active subscription and no persisted subscription at boot up
    mSubInfoProvider.SetHasActiveSubscription(false);
    mSubInfoProvider.SetHasPersistedSubscription(false);
    EXPECT_TRUE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    // Test 2 - Has no active subscription and a persisted subscription at boot up
    mSubInfoProvider.SetHasActiveSubscription(false);
    mSubInfoProvider.SetHasPersistedSubscription(true);
    EXPECT_FALSE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    mICDManager.SetBootUpResumeSubscriptionExecuted();

    // Test 3 - Has an active subscription and a persisted subscription during normal operations
    mSubInfoProvider.SetHasActiveSubscription(true);
    mSubInfoProvider.SetHasPersistedSubscription(true);
    EXPECT_FALSE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    // Test 4 - Has no active subscription and a persisted subscription during normal operations
    mSubInfoProvider.SetHasActiveSubscription(false);
    mSubInfoProvider.SetHasPersistedSubscription(true);
    EXPECT_TRUE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));
}
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#else
TEST_F(TestICDManager, TestShouldCheckInMsgsBeSentAtActiveModeFunction)
{
    // Test 1 - Has an active subscription
    mSubInfoProvider.SetHasActiveSubscription(true);
    EXPECT_FALSE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    // Test 2 - Has no active subscription
    mSubInfoProvider.SetHasActiveSubscription(false);
    EXPECT_TRUE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));

    // Test 3 - Make sure that the persisted subscription has no impact
    mSubInfoProvider.SetHasPersistedSubscription(true);
    EXPECT_TRUE(mICDManager.ShouldCheckInMsgsBeSentAtActiveModeFunction(kTestFabricIndex1, kClientNodeId11));
}
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

TEST_F(TestICDManager, TestHandleTestEventTriggerActiveModeReq)
{
    // Verify That ICDManager starts in Idle
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);

    // Add ActiveMode req for the Test event trigger event
    mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kAddActiveModeReq));
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Advance clock by the ActiveModeDuration and check that the device is still in ActiveMode
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() + 1_ms32);
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::ActiveMode);

    // Remove req and device should go to IdleMode
    mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kRemoveActiveModeReq));
    EXPECT_EQ(mICDManager.GetOperaionalState(), ICDManager::OperationalState::IdleMode);
}

#if CHIP_CONFIG_ENABLE_ICD_CIP
TEST_F(TestICDManager, TestHandleTestEventTriggerInvalidateHalfCounterValues)
{
    constexpr uint32_t startValue    = 1;
    constexpr uint32_t expectedValue = 2147483648;

    // Set starting value
    uint32_t currentValue = ICDConfigurationData::GetInstance().GetICDCounter().GetValue();
    uint32_t delta        = startValue - currentValue;

    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDCounter().AdvanceBy(delta), CHIP_NO_ERROR);
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDCounter().GetValue(), startValue);

    // Trigger ICD kInvalidateHalfCounterValues event
    mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kInvalidateHalfCounterValues));

    // Validate counter has the expected value
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDCounter().GetValue(), expectedValue);
}

TEST_F(TestICDManager, TestHandleTestEventTriggerInvalidateAllCounterValues)
{
    constexpr uint32_t startValue    = 105;
    constexpr uint32_t expectedValue = 104;

    // Set starting value
    uint32_t currentValue = ICDConfigurationData::GetInstance().GetICDCounter().GetValue();
    uint32_t delta        = startValue - currentValue;

    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDCounter().AdvanceBy(delta), CHIP_NO_ERROR);
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDCounter().GetValue(), startValue);

    // Trigger ICD kInvalidateAllCounterValues event
    mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kInvalidateAllCounterValues));

    // Validate counter has the expected value
    EXPECT_EQ(ICDConfigurationData::GetInstance().GetICDCounter().GetValue(), expectedValue);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

/**
 * @brief Test verifies when OnEnterIdleMode is called during normal operations.
 *        Without the ActiveMode timer being extended
 */
TEST_F(TestICDManager, TestICDStateObserverOnEnterIdleModeActiveModeDuration)
{
    // Verify that ICDManager starts in IdleMode and calls OnEnterIdleMode
    EXPECT_TRUE(mICDStateObserver.mOnEnterIdleModeCalled);
    mICDStateObserver.ResetOnEnterIdleMode();

    // Advance clock just before IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Expire IdleModeInterval
    AdvanceClockAndRunEventLoop(1_s);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Advance clock Just before ActiveMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Expire ActiveMode timer
    AdvanceClockAndRunEventLoop(1_ms32);
    EXPECT_TRUE(mICDStateObserver.mOnEnterIdleModeCalled);
}

/**
 * @brief Test verifies when OnEnterIdleMode is called with the ActiveMode timer gets extended
 */
TEST_F(TestICDManager, TestICDStateObserverOnEnterIdleModeActiveModeThreshold)
{
    // Verify that ICDManager starts in IdleMode and calls OnEnterIdleMode
    EXPECT_TRUE(mICDStateObserver.mOnEnterIdleModeCalled);
    mICDStateObserver.ResetOnEnterIdleMode();

    // Advance clock just before the IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Expire IdleMode timer
    AdvanceClockAndRunEventLoop(1_s);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Advance clock Just before ActiveMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Increase ActiveMode timer by one ActiveModeThreshold
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Advance clock Just before ActiveMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeThreshold() - 1_ms32);
    EXPECT_FALSE(mICDStateObserver.mOnEnterIdleModeCalled);

    // Expire ActiveMode timer
    AdvanceClockAndRunEventLoop(1_ms32);
    EXPECT_TRUE(mICDStateObserver.mOnEnterIdleModeCalled);
}

TEST_F(TestICDManager, TestICDStateObserverOnEnterActiveMode)
{
    // Verify OnEnterActiveMode wasn't called at Init
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Advance clock just before IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Expire IdleMode timer and check wether OnEnterActiveMode was called
    AdvanceClockAndRunEventLoop(1_s);
    EXPECT_TRUE(mICDStateObserver.mOnEnterActiveModeCalled);
    mICDStateObserver.ResetOnEnterActiveMode();

    // Advance clock just before the ActiveMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() - 1_ms32);

    // Verify OnEnterActiveMde wasn't called
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Increase ActiveMode timer by one ActiveModeThreshold
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();

    // Verify OnEnterActiveMde wasn't called
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Advance clock just before ActiveMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeThreshold() - 1_ms32);

    // Verify OnEnterActiveMde wasn't called
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Expire ActiveMode timer
    AdvanceClockAndRunEventLoop(1_ms32);

    // Verify OnEnterActiveMde wasn't called
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Advance clock just before IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnEnterActiveModeCalled);

    // Expire IdleMode timer and check OnEnterActiveMode was called
    AdvanceClockAndRunEventLoop(1_s);
    EXPECT_TRUE(mICDStateObserver.mOnEnterActiveModeCalled);
}

#if CHIP_CONFIG_ENABLE_ICD_CIP
TEST_F(TestICDManager, TestICDStateObserverOnICDModeChange)
{
    typedef ICDListener::ICDManagementEvents ICDMEvent;
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport).Set(Feature::kUserActiveModeTrigger).Set(Feature::kCheckInProtocolSupport);
    privateIcdConfigData.SetFeatureMap(featureMap);

    // Since we don't have a registration, we stay in SIT mode. No changes
    EXPECT_FALSE(mICDStateObserver.mOnICDModeChangeCalled);

    // Trigger register event to force ICDManager to re-evaluate OperatingMode
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Since we don't have a registration, we stay in SIT mode. No changes
    EXPECT_FALSE(mICDStateObserver.mOnICDModeChangeCalled);

    // Add an entry to the ICDMonitoringTable
    ICDMonitoringTable table(testStorage, kTestFabricIndex1, kMaxTestClients, &(mKeystore));

    ICDMonitoringEntry entry(&(mKeystore));
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table.Set(0, entry));

    // Trigger register event after first entry was added
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // We have a registration. Transition to LIT mode
    EXPECT_TRUE(mICDStateObserver.mOnICDModeChangeCalled);
    mICDStateObserver.ResetOnICDModeChange();

    // Trigger register event to force ICDManager to re-evaluate OperatingMode
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // We have a registration. We stay in LIT mode. No changes.
    EXPECT_FALSE(mICDStateObserver.mOnICDModeChangeCalled);

    // Remove entry from the ICDMonitoringTable
    EXPECT_EQ(CHIP_NO_ERROR, table.Remove(0));
    ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDMEvent::kTableUpdated);

    // Since we don't have a registration anymore. Transition to SIT mode.
    EXPECT_TRUE(mICDStateObserver.mOnICDModeChangeCalled);
    mICDStateObserver.ResetOnICDModeChange();
}

TEST_F(TestICDManager, TestICDStateObserverOnICDModeChangeOnInit)
{
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    using Feature = Clusters::IcdManagement::Feature;
    BitFlags<Feature> featureMap;
    featureMap.Set(Feature::kLongIdleTimeSupport).Set(Feature::kUserActiveModeTrigger).Set(Feature::kCheckInProtocolSupport);
    privateIcdConfigData.SetFeatureMap(featureMap);

    ICDMonitoringTable table(testStorage, kTestFabricIndex1, kMaxTestClients, &(mKeystore));

    // Since we don't have a registration, we stay in SIT mode. No changes
    EXPECT_FALSE(mICDStateObserver.mOnICDModeChangeCalled);

    // Add an entry to the ICDMonitoringTable
    ICDMonitoringEntry entry(&(mKeystore));
    entry.checkInNodeID    = kClientNodeId11;
    entry.monitoredSubject = kClientNodeId11;
    EXPECT_EQ(CHIP_NO_ERROR, entry.SetKey(ByteSpan(kKeyBuffer1a)));
    EXPECT_EQ(CHIP_NO_ERROR, table.Set(0, entry));

    // Shut down and reinit ICDManager - We should go to LIT mode since we have a registration
    mICDManager.Shutdown();
    mICDManager.RegisterObserver(&(mICDStateObserver));
#if CHIP_CONFIG_ENABLE_ICD_CIP
    mICDManager.SetPersistentStorageDelegate(&testStorage)
        .SetFabricTable(&GetFabricTable())
        .SetSymmetricKeyStore(&mKeystore)
        .SetExchangeManager(&GetExchangeManager())
        .SetSubscriptionsInfoProvider(&mSubInfoProvider)
        .SetICDCheckInBackOffStrategy(&mStrategy);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
    mICDManager.Init();

    // We have a registration, transition to LIT mode
    EXPECT_TRUE(mICDStateObserver.mOnICDModeChangeCalled);
    mICDStateObserver.ResetOnICDModeChange();

    // Remove entry from the ICDMonitoringTable
    EXPECT_EQ(CHIP_NO_ERROR, table.Remove(0));
}
#endif

/**
 * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is greater than the
 *        ICD_ACTIVE_TIME_JITTER_MS
 */
TEST_F(TestICDManager, TestICDStateObserverOnTransitionToIdleModeGreaterActiveModeDuration)
{
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    // Set New durations for test case - ActiveModeDuration must be longuer than ICD_ACTIVE_TIME_JITTER_MS
    Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
    mICDManager.SetModeDurations(MakeOptional<Milliseconds32>(Milliseconds32(200) + Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS)),
                                 NullOptional);

    // Advance clock just before IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Expire IdleMode timer
    AdvanceClockAndRunEventLoop(1_s);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Advance time just before OnTransitionToIdleMode is called
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeDuration() -
                                Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS) - 1_ms32);

    // Check mOnTransitionToIdleCalled has not been called
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Increase ActiveMode timer by one ActiveModeThreshold
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Advance time just before OnTransitionToIdleMode is called
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeThreshold() -
                                Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS) - 1_ms32);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Expire OnTransitionToIdleMode
    AdvanceClockAndRunEventLoop(1_ms32);
    // Check mOnTransitionToIdleCalled has been called
    EXPECT_TRUE(mICDStateObserver.mOnTransitionToIdleCalled);
    mICDStateObserver.ResetOnTransitionToIdle();

    // Expire ActiveMode timer
    AdvanceClockAndRunEventLoop(Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS));
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Reset Old durations
    mICDManager.SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
}

/**
 * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is equal to the
 *        ICD_ACTIVE_TIME_JITTER_MS.
 */
TEST_F(TestICDManager, TestICDStateObserverOnTransitionToIdleModeEqualActiveModeDuration)
{
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    // Set New durations for test case - ActiveModeDuration must be equal to ICD_ACTIVE_TIME_JITTER_MS
    Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
    mICDManager.SetModeDurations(MakeOptional<Milliseconds32>(Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS)), NullOptional);

    // Advance clock just before IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Expire IdleMode timer
    AdvanceClockAndRunEventLoop(1_s);
    // In this scenario, The ICD state machine kicked a OnTransitionToIdle timer with a duration of 0 seconds.
    // The freeRTOS systemlayer timer calls a 0s timer's callback instantly while on posix it take and 1 addition event loop.
    // Thefore, the expect result diverges here based on the systemlayer implementation. Skip this check.
    // https://github.com/project-chip/connectedhomeip/issues/33441
    // EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Expire OnTransitionToIdleMode
    AdvanceClockAndRunEventLoop(1_ms32);
    // All systems should have called the OnTransitionToIdle callback by now.
    EXPECT_TRUE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Reset Old durations
    mICDManager.SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
}

/**
 * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is 0 and without an ActiveMode req
 */
TEST_F(TestICDManager, TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWithoutReq)
{
    ICDConfigurationDataTestAccess privateIcdConfigData(&ICDConfigurationData::GetInstance());

    // Set New durations for test case - ActiveModeDuration equal 0
    Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
    mICDManager.SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

    // Advance clock just before IdleMode timer expires
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration() - 1_s);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Expire IdleMode timer
    AdvanceClockAndRunEventLoop(1_s);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Increase time by 1 - Should not trigger OnTransitionToIdle.
    // Timer length is one ActiveModeThreshold
    AdvanceClockAndRunEventLoop(1_ms32);
    EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Expire ActiveModeThreshold
    AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetActiveModeThreshold());
    EXPECT_TRUE(mICDStateObserver.mOnTransitionToIdleCalled);

    // Reset Old durations
    mICDManager.SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
}

/**
 * @brief Test verifies the OnTransitionToIdleMode event when the ActiveModeDuration is 0 with an ActiveMode req
 *        TODO(#33074): When the OnTransitionToIdle edge is fixed, we can enable this test
 */
// TEST_F(TestICDManager, TestICDStateObserverOnTransitionToIdleMode0ActiveModeDurationWithReq)
// {
//     // Set New durations for test case - ActiveModeDuration equal 0
//     Milliseconds32 oldActiveModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();
//     ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional<Milliseconds32>(0), NullOptional);

//     // Add ActiveMode req for the Test event trigger event
//     mICDManager.HandleEventTrigger(static_cast<uint64_t>(ICDTestEventTriggerEvent::kAddActiveModeReq));

//     // Expire IdleMode timer
//     AdvanceClockAndRunEventLoop(ICDConfigurationData::GetInstance().GetIdleModeDuration());
//     EXPECT_FALSE(mICDStateObserver.mOnTransitionToIdleCalled);

//     // Reset Old durations
//     ICDConfigurationData::GetInstance().SetModeDurations(MakeOptional(oldActiveModeDuration), NullOptional);
// }

} // namespace app
} // namespace chip
