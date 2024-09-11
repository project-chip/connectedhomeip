/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-cluster-logic.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-delegate.h>
#include <app/clusters/valve-configuration-and-control-server/valve-configuration-and-control-matter-context.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

using namespace chip::System::Clock::Literals;
namespace chip {

namespace System {

// TODO: This might be worthwhile to generalize
class TimerAndMockClock : public Clock::Internal::MockClock, public Layer
{
public:
    // System Layer overrides
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    void Shutdown() override { Clear(); }
    void Clear()
    {
        mTimerList.Clear();
        mTimerNodes.ReleaseAll();
    }
    bool IsInitialized() const override { return true; }

    CHIP_ERROR StartTimer(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
    {
        Clock::Timestamp awakenTime = GetMonotonicMilliseconds64() + std::chrono::duration_cast<Clock::Milliseconds64>(aDelay);
        TimerList::Node * node      = mTimerNodes.Create(*this, awakenTime, aComplete, aAppState);
        mTimerList.Add(node);
        return CHIP_NO_ERROR;
    }
    void CancelTimer(TimerCompleteCallback aComplete, void * aAppState) override
    {
        TimerList::Node * cancelled = mTimerList.Remove(aComplete, aAppState);
        if (cancelled != nullptr)
        {
            mTimerNodes.Release(cancelled);
        }
    }
    CHIP_ERROR ExtendTimerTo(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) override
    {
        return mTimerList.GetRemainingTime(onComplete, appState) != Clock::Timeout(0);
    }
    Clock::Timeout GetRemainingTime(TimerCompleteCallback onComplete, void * appState) override
    {
        return mTimerList.GetRemainingTime(onComplete, appState);
    }
    CHIP_ERROR ScheduleWork(TimerCompleteCallback aComplete, void * aAppState) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // Clock overrides
    void SetMonotonic(Clock::Milliseconds64 timestamp)
    {
        MockClock::SetMonotonic(timestamp);
        // Find all the timers that fired at this time or before and invoke the callbacks
        TimerList::Node * node;
        while ((node = mTimerList.Earliest()) != nullptr && node->AwakenTime() <= timestamp)
        {
            mTimerList.PopEarliest();
            // Invoke auto-releases
            mTimerNodes.Invoke(node);
        }
    }

    void AdvanceMonotonic(Clock::Milliseconds64 increment) { SetMonotonic(GetMonotonicMilliseconds64() + increment); }

private:
    TimerPool<> mTimerNodes;
    TimerList mTimerList;
};

} // namespace System
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

namespace {
// These are globals because SetUpTestSuite is static and I'm not shaving that yak today.
System::TimerAndMockClock gSystemLayerAndClock = System::TimerAndMockClock();
System::Clock::ClockBase * gSavedClock         = nullptr;
} // namespace

class TestValveConfigurationAndControlClusterLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        gSavedClock = &System::SystemClock();
        System::Clock::Internal::SetSystemClockForTesting(&gSystemLayerAndClock);
        DeviceLayer::SetSystemLayerForTesting(&gSystemLayerAndClock);
    }
    static void TearDownTestSuite()
    {
        gSystemLayerAndClock.Shutdown();
        DeviceLayer::SetSystemLayerForTesting(nullptr);
        System::Clock::Internal::SetSystemClockForTesting(gSavedClock);
        Platform::MemoryShutdown();
    }

private:
};

class TestDelegateLevel : public LevelControlDelegate
{
public:
    TestDelegateLevel() {}
    CHIP_ERROR HandleOpenValve(const Percent targetLevel, Percent & currentLevel, BitMask<ValveFaultBitmap> & valveFault) override
    {
        lastRequestedLevel = targetLevel;
        ++numHandleOpenValveCalls;
        if (simulateAsyncOpen)
        {
            currentLevel = targetLevel / 2;
        }
        else
        {
            currentLevel = targetLevel;
        }
        level  = currentLevel;
        target = targetLevel;
        if (simulateFailure || simulateValveFaultNoFailure)
        {
            valveFault = simulatedFailureBitMask;
        }
        if (simulateFailure)
        {
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_NO_ERROR;
    }
    Percent GetCurrentValveLevel() override
    {
        // TODO: maybe want to ramp this.
        level = target;
        return level;
    }

    BitMask<ValveFaultBitmap> simulatedFailureBitMask =
        BitMask<ValveFaultBitmap>(to_underlying(ValveFaultBitmap::kBlocked) | to_underlying(ValveFaultBitmap::kGeneralFault));
    CHIP_ERROR HandleCloseValve() override { return CHIP_NO_ERROR; }
    Percent lastRequestedLevel       = 0;
    bool simulateFailure             = false;
    bool simulateValveFaultNoFailure = false;
    bool simulateAsyncOpen           = false;
    int numHandleOpenValveCalls      = 0;
    Percent level                    = 0;
    Percent target                   = 0;
};

class TestDelegateNoLevel : public NonLevelControlDelegate
{
public:
    TestDelegateNoLevel() {}
    CHIP_ERROR HandleOpenValve(ValveStateEnum & currentState, BitMask<ValveFaultBitmap> & valveFault) override
    {
        ++numHandleOpenValveCalls;
        if (simulateAsyncOpen)
        {
            currentState = ValveStateEnum::kTransitioning;
        }
        else
        {
            currentState = ValveStateEnum::kOpen;
        }
        state  = currentState;
        target = ValveStateEnum::kOpen;
        if (simulateFailure || simulateValveFaultNoFailure)
        {
            valveFault = simulatedFailureBitMask;
        }
        if (simulateFailure)
        {
            return CHIP_ERROR_INTERNAL;
        }
        return CHIP_NO_ERROR;
    }
    ValveStateEnum GetCurrentValveState() override
    {
        // Might want to have called more than one time before hitting the target.
        state = target;
        return state;
    }

    BitMask<ValveFaultBitmap> simulatedFailureBitMask =
        BitMask<ValveFaultBitmap>(to_underlying(ValveFaultBitmap::kBlocked) | to_underlying(ValveFaultBitmap::kGeneralFault));
    CHIP_ERROR HandleCloseValve() override { return CHIP_NO_ERROR; }
    bool simulateFailure             = false;
    bool simulateValveFaultNoFailure = false;
    bool simulateAsyncOpen           = false;
    int numHandleOpenValveCalls      = 0;
    ValveStateEnum state             = ValveStateEnum::kUnknownEnumValue;
    ValveStateEnum target            = ValveStateEnum::kUnknownEnumValue;
};

// TODO: This also might be good to generalize, by using a common matter context for each cluster with allowed overrides
class MockedMatterContext : public MatterContext
{
public:
    MockedMatterContext(EndpointId endpoint, PersistentStorageDelegate & persistentStorageDelegate) :
        MatterContext(endpoint, persistentStorageDelegate)
    {}
    void MarkDirty(AttributeId id) override { mDirtyMarkedList.push_back(id); }
    std::vector<AttributeId> GetDirtyList() { return mDirtyMarkedList; }
    void ClearDirtyList() { mDirtyMarkedList.clear(); }
    ~MockedMatterContext() { gSystemLayerAndClock.Clear(); }

private:
    // Won't handle double-marking an attribute, so don't do that in tests
    std::vector<AttributeId> mDirtyMarkedList;
};

//=========================================================================================
// Tests for conformance
//=========================================================================================

// This test ensures that the Init function properly errors when the conformance is valid and passes otherwise.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestConformanceValid)
{
    // Nothing on, should be valid
    ClusterConformance conformance = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = false
    };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, no optional stuff, should be valid
    conformance = { .featureMap               = to_underlying(Feature::kLevel),
                    .supportsDefaultOpenLevel = false,
                    .supportsValveFault       = false,
                    .supportsLevelStep        = false };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, one optional level thing
    conformance = { .featureMap               = to_underlying(Feature::kLevel),
                    .supportsDefaultOpenLevel = true,
                    .supportsValveFault       = false,
                    .supportsLevelStep        = false };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, one optional level thing
    conformance = { .featureMap               = to_underlying(Feature::kLevel),
                    .supportsDefaultOpenLevel = false,
                    .supportsValveFault       = false,
                    .supportsLevelStep        = true };
    EXPECT_TRUE(conformance.Valid());

    // LVL on, two optional level things
    conformance = { .featureMap               = to_underlying(Feature::kLevel),
                    .supportsDefaultOpenLevel = true,
                    .supportsValveFault       = false,
                    .supportsLevelStep        = true };
    EXPECT_TRUE(conformance.Valid());

    // Fully optional thing on
    conformance = { .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = true, .supportsLevelStep = false };
    EXPECT_TRUE(conformance.Valid());

    // TS on
    conformance = { .featureMap               = to_underlying(Feature::kTimeSync),
                    .supportsDefaultOpenLevel = false,
                    .supportsValveFault       = false,
                    .supportsLevelStep        = false };
    EXPECT_TRUE(conformance.Valid());

    // Both features on
    conformance = { .featureMap               = to_underlying(Feature::kTimeSync) | to_underlying(Feature::kLevel),
                    .supportsDefaultOpenLevel = false,
                    .supportsValveFault       = false,
                    .supportsLevelStep        = false };
    EXPECT_TRUE(conformance.Valid());

    // LVL off, one optional level thing on
    conformance = { .featureMap = 0, .supportsDefaultOpenLevel = true, .supportsValveFault = false, .supportsLevelStep = false };
    EXPECT_FALSE(conformance.Valid());

    // LVL on, other optional level thing on
    conformance = { .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = true };
    EXPECT_FALSE(conformance.Valid());

    // Bad feature map
    conformance = { .featureMap = 0x04, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = true };
    EXPECT_FALSE(conformance.Valid());
}

// The cluster requires different versions of the delegate depending on the supported feature set.
// This test ensures the Init function corectly errors if the supplied delegate does not match
// the given cluster conformance.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestWrongDelegates)
{
    TestDelegateLevel delegateLevel;
    TestDelegateNoLevel delegateNoLevel;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logicLevel(delegateLevel, context);
    ClusterLogic logicNoLevel(delegateNoLevel, context);
    ClusterConformance conformanceLevel   = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                              .supportsDefaultOpenLevel = true,
                                              .supportsValveFault       = true,
                                              .supportsLevelStep        = true };
    ClusterConformance conformanceNoLevel = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = false
    };

    EXPECT_EQ(logicLevel.Init(conformanceNoLevel), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
    EXPECT_EQ(logicNoLevel.Init(conformanceLevel), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
}

//=========================================================================================
// Tests for getters
//=========================================================================================

// This test ensures that all getters are properly implemented and return valid values after successful initialization.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesAllFeatures)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MockedMatterContext context(0, storageDelegate);
    ClusterLogic logic(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;
    DataModel::Nullable<EpochUs> valEpochUsNullable;
    DataModel::Nullable<ValveStateEnum> valEnumNullable;
    DataModel::Nullable<Percent> valPercentNullable;
    Percent valPercent;
    uint8_t val8;
    BitMask<ValveFaultBitmap> valBitmap;

    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetAutoCloseTime(valEpochUsNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEpochUsNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetCurrentState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetTargetState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetCurrentLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetTargetLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetDefaultOpenLevel(valPercent), CHIP_NO_ERROR);
    EXPECT_EQ(valPercent, 100);

    EXPECT_EQ(logic.GetValveFault(valBitmap), CHIP_NO_ERROR);
    EXPECT_EQ(valBitmap.Raw(), 0);

    EXPECT_EQ(logic.GetLevelStep(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, 1);
}

// This test ensures that attributes that are not supported by the conformance properly return errors
// and attributes that are supported return values properly.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesNoFeatures)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MockedMatterContext context(0, storageDelegate);
    ClusterLogic logic(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = false
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;
    DataModel::Nullable<EpochUs> valEpochUsNullable;
    DataModel::Nullable<ValveStateEnum> valEnumNullable;
    DataModel::Nullable<Percent> valPercentNullable;
    Percent valPercent;
    uint8_t val8;
    BitMask<ValveFaultBitmap> valBitmap;

    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetAutoCloseTime(valEpochUsNullable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetCurrentState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetTargetState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetCurrentLevel(valPercentNullable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic.GetTargetLevel(valPercentNullable), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic.GetDefaultOpenLevel(valPercent), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic.GetValveFault(valBitmap), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    EXPECT_EQ(logic.GetLevelStep(val8), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

// This test ensures that all attribute getters return the given starting state values before changes.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesStartingState)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MockedMatterContext context(0, storageDelegate);
    ClusterLogic logic(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance     = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                           .supportsDefaultOpenLevel = true,
                                           .supportsValveFault       = true,
                                           .supportsLevelStep        = true };
    ClusterInitParameters initialState = {
        .currentState = DataModel::MakeNullable(ValveStateEnum::kTransitioning),
        .currentLevel = DataModel::MakeNullable(static_cast<Percent>(50u)),
        .valveFault   = BitMask<ValveFaultBitmap>(ValveFaultBitmap::kLeaking),
        .levelStep    = 2u,
    };
    EXPECT_EQ(logic.Init(conformance, initialState), CHIP_NO_ERROR);

    ClusterState state;
    state.currentState = initialState.currentState;
    state.currentLevel = initialState.currentLevel;
    state.valveFault   = initialState.valveFault;
    state.levelStep    = initialState.levelStep;

    DataModel::Nullable<ElapsedS> valElapsedSNullable;
    DataModel::Nullable<EpochUs> valEpochUsNullable;
    DataModel::Nullable<ValveStateEnum> valEnumNullable;
    DataModel::Nullable<Percent> valPercentNullable;
    Percent valPercent;
    uint8_t val8;
    BitMask<ValveFaultBitmap> valBitmap;

    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, state.openDuration);

    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, state.defaultOpenDuration);

    EXPECT_EQ(logic.GetAutoCloseTime(valEpochUsNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEpochUsNullable, state.autoCloseTime);

    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, state.remainingDuration.value());

    EXPECT_EQ(logic.GetCurrentState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, state.currentState);

    EXPECT_EQ(logic.GetTargetState(valEnumNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valEnumNullable, state.targetState);

    EXPECT_EQ(logic.GetCurrentLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable, state.currentLevel);

    EXPECT_EQ(logic.GetTargetLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable, state.targetLevel);

    EXPECT_EQ(logic.GetDefaultOpenLevel(valPercent), CHIP_NO_ERROR);
    EXPECT_EQ(valPercent, state.defaultOpenLevel);

    EXPECT_EQ(logic.GetValveFault(valBitmap), CHIP_NO_ERROR);
    EXPECT_EQ(valBitmap, state.valveFault);

    EXPECT_EQ(logic.GetLevelStep(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, state.levelStep);
}

// This test ensures that all attribute getter functions properly error on an uninitialized cluster.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesUninitialized)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MockedMatterContext context(0, storageDelegate);
    ClusterLogic logic(delegate, context);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;
    DataModel::Nullable<EpochUs> valEpochUsNullable;
    DataModel::Nullable<ValveStateEnum> valEnumNullable;
    DataModel::Nullable<Percent> valPercentNullable;
    Percent valPercent;
    uint8_t val8;
    BitMask<ValveFaultBitmap> valBitmap;

    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetAutoCloseTime(valEpochUsNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetCurrentState(valEnumNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetTargetState(valEnumNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetCurrentLevel(valPercentNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetTargetLevel(valPercentNullable), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetDefaultOpenLevel(valPercent), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetValveFault(valBitmap), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic.GetLevelStep(val8), CHIP_ERROR_INCORRECT_STATE);
}

//=========================================================================================
// Tests for setters
//=========================================================================================

// This test ensures that the Set function for DefaultOpenDuration sets the value properly including
// - constraints checks
// - value pushed through to persistent storage correctly
// - value is persisted on a per-endpoint basis
// - cluster logic loads saved values at startup
// - cluster logic operates only on the stored values for the correct endpoint
// - Set function correctly errors when the persistent storage errors
TEST_F(TestValveConfigurationAndControlClusterLogic, TestSetDefaultOpenDuration)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;

    // Setting this value before initialization should fail
    auto testVal = DataModel::MakeNullable(static_cast<ElapsedS>(5u));
    EXPECT_EQ(logic.SetDefaultOpenDuration(testVal), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    // Lowest possible value
    testVal = DataModel::MakeNullable(static_cast<ElapsedS>(1u));
    EXPECT_EQ(logic.SetDefaultOpenDuration(testVal), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, testVal);

    // Ensure the value is persisted in the test storage
    StorageKeyName keyName = DefaultStorageKeyAllocator::VCCDefaultOpenDuration(endpoint);
    uint32_t persistedValue;
    uint16_t size = static_cast<uint16_t>(sizeof(persistedValue));
    EXPECT_TRUE(storageDelegate.HasKey(keyName.KeyName()));
    EXPECT_EQ(storageDelegate.SyncGetKeyValue(keyName.KeyName(), &persistedValue, size), CHIP_NO_ERROR);
    EXPECT_EQ(persistedValue, testVal.Value());
    // Test that this doesn't exist for other endpoints. Check 1.
    EXPECT_FALSE(storageDelegate.HasKey(DefaultStorageKeyAllocator::VCCDefaultOpenDuration(1).KeyName()));

    testVal = DataModel::MakeNullable(static_cast<ElapsedS>(12u));
    EXPECT_EQ(logic.SetDefaultOpenDuration(testVal), CHIP_NO_ERROR);

    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, testVal);

    EXPECT_TRUE(storageDelegate.HasKey(keyName.KeyName()));
    EXPECT_EQ(storageDelegate.SyncGetKeyValue(keyName.KeyName(), &persistedValue, size), CHIP_NO_ERROR);
    EXPECT_EQ(persistedValue, testVal.Value());

    auto outOfRangeVal = DataModel::MakeNullable(static_cast<ElapsedS>(0u));
    EXPECT_EQ(logic.SetDefaultOpenDuration(outOfRangeVal), CHIP_ERROR_INVALID_ARGUMENT);

    // Ensure the value wasn't changed
    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, testVal);

    EXPECT_TRUE(storageDelegate.HasKey(keyName.KeyName()));
    EXPECT_EQ(storageDelegate.SyncGetKeyValue(keyName.KeyName(), &persistedValue, size), CHIP_NO_ERROR);
    EXPECT_EQ(persistedValue, testVal.Value());

    // Test that firing up a new logic cluster on the same endpoint loads the value from persisted storage
    ClusterLogic logic_same_endpoint = ClusterLogic(delegate, context);
    EXPECT_EQ(logic_same_endpoint.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic_same_endpoint.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, testVal);

    // Test that a new logic cluster on a different endpoint does not load the value
    MockedMatterContext context_ep1       = MockedMatterContext(1, storageDelegate);
    ClusterLogic logic_different_endpoint = ClusterLogic(delegate, context_ep1);
    EXPECT_EQ(logic_different_endpoint.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic_different_endpoint.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    // Test setting back to null, this should clear the persisted value
    testVal = DataModel::NullNullable;
    EXPECT_EQ(logic.SetDefaultOpenDuration(testVal), CHIP_NO_ERROR);
    EXPECT_FALSE(storageDelegate.HasKey(keyName.KeyName()));

    // Check that the value is not loaded when a new logic cluster is created
    ClusterLogic logic_same_endpoint_again = ClusterLogic(delegate, context);
    EXPECT_EQ(logic_same_endpoint_again.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic_same_endpoint_again.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    // Test that the calling function fails when write fails are on
    storageDelegate.SetRejectWrites(true);
    testVal.SetNonNull(12u);
    EXPECT_EQ(logic.SetDefaultOpenDuration(testVal), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
}

// This test ensures that the Set function for DefaultOpenDuration sets the value properly including
// - constraints checks
// - value pushed through to persistent storage correctly
// - value is persisted on a per-endpoint basis
// - cluster logic loads saved values at startup
// - cluster logic operates only on the stored values for the correct endpoint
// - Set function correctly errors when the persistent storage errors
// - Set function errors when the OpenLevel attribute is not supported
// Conformance to the LevelStep value is tested in another test
TEST_F(TestValveConfigurationAndControlClusterLogic, TestSetDefaultOpenLevel)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    uint8_t val8;

    // Setting this value before initialization should fail
    uint8_t testVal = 5u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_ERROR_INCORRECT_STATE);

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, 100u);

    // Lowest possible value
    testVal = 1u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_NO_ERROR);

    // Ensure the value is accessible via the API
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    // Ensure the value is persisted in the test storage
    StorageKeyName keyName = DefaultStorageKeyAllocator::VCCDefaultOpenLevel(endpoint);
    uint8_t persistedValue;
    uint16_t size = static_cast<uint16_t>(sizeof(persistedValue));
    EXPECT_TRUE(storageDelegate.HasKey(keyName.KeyName()));
    EXPECT_EQ(storageDelegate.SyncGetKeyValue(keyName.KeyName(), &persistedValue, size), CHIP_NO_ERROR);
    EXPECT_EQ(persistedValue, testVal);
    // Test that this doesn't exist for other endpoints. Check 1.
    EXPECT_FALSE(storageDelegate.HasKey(DefaultStorageKeyAllocator::VCCDefaultOpenLevel(1).KeyName()));

    // Highest possible value
    testVal = 100u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    EXPECT_TRUE(storageDelegate.HasKey(keyName.KeyName()));
    EXPECT_EQ(storageDelegate.SyncGetKeyValue(keyName.KeyName(), &persistedValue, size), CHIP_NO_ERROR);
    EXPECT_EQ(persistedValue, testVal);

    uint8_t outOfRangeVal = 0u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(outOfRangeVal), CHIP_ERROR_INVALID_ARGUMENT);
    outOfRangeVal = 101u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(outOfRangeVal), CHIP_ERROR_INVALID_ARGUMENT);

    // Ensure the value wasn't changed
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    EXPECT_TRUE(storageDelegate.HasKey(keyName.KeyName()));
    EXPECT_EQ(storageDelegate.SyncGetKeyValue(keyName.KeyName(), &persistedValue, size), CHIP_NO_ERROR);
    EXPECT_EQ(persistedValue, testVal);

    // Set Non-default value
    testVal = 12u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_NO_ERROR);
    // Test that firing up a new logic cluster on the same endpoint loads the value from persisted storage
    ClusterLogic logic_same_endpoint = ClusterLogic(delegate, context);
    EXPECT_EQ(logic_same_endpoint.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic_same_endpoint.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    // Test that a new logic cluster on a different endpoint does not load the value
    MockedMatterContext context_ep1       = MockedMatterContext(1, storageDelegate);
    ClusterLogic logic_different_endpoint = ClusterLogic(delegate, context_ep1);
    EXPECT_EQ(logic_different_endpoint.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic_different_endpoint.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, 100u);

    // Test that the calling function fails when write fails are on
    storageDelegate.SetRejectWrites(true);
    testVal = 15u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_ERROR_PERSISTED_STORAGE_FAILED);
    storageDelegate.SetRejectWrites(false);

    // Test that we get an error if this attribute is not supported
    ClusterLogic logic_no_level             = ClusterLogic(delegate, context);
    ClusterConformance conformance_no_level = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                                .supportsDefaultOpenLevel = false,
                                                .supportsValveFault       = true,
                                                .supportsLevelStep        = true };
    EXPECT_EQ(logic_no_level.Init(conformance_no_level), CHIP_NO_ERROR);
    EXPECT_EQ(logic_no_level.SetDefaultOpenLevel(testVal), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

// This test ensures that the SetDefaultOpenLevel function correctly assesses the set value with respect to the LevelStep.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestSetDefaultOpenLevelWithLevelStep)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    uint8_t val8;
    uint8_t testVal;

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    ClusterInitParameters state;
    state.levelStep = 45;
    EXPECT_EQ(logic.Init(conformance, state), CHIP_NO_ERROR);

    // Default is 100
    // 45, 90 and 100 should work, others should fail.
    testVal = 45u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    testVal = 90u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    testVal = 100u;
    EXPECT_EQ(logic.SetDefaultOpenLevel(testVal), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);

    // Test a value that's not in the level step and ensure the value is not changed.
    EXPECT_EQ(logic.SetDefaultOpenLevel(33u), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(logic.GetDefaultOpenLevel(val8), CHIP_NO_ERROR);
    EXPECT_EQ(val8, testVal);
}

//=========================================================================================
// Tests for Open command parameters
//=========================================================================================

// This test ensures that the OpenDuration is set correctly when Open is called. Tests:
// - Fall back to Null when omitted and DefaultOpenDuration is null
// - Fall back to DefaultOpenDuration when omitted and DefaultOpenDuration is set
// - Null and value are both accepted when the parameter is supplied in the command field.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenDuration)
{

    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;

    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(DataModel::NullNullable), std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    EXPECT_EQ(logic.GetDefaultOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    // Fall back to default
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    DataModel::Nullable<ElapsedS> defaultOpenDuration;
    defaultOpenDuration.SetNonNull(12u);
    EXPECT_EQ(logic.SetDefaultOpenDuration(defaultOpenDuration), CHIP_NO_ERROR);
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, defaultOpenDuration);

    // Set from command parameters
    DataModel::Nullable<ElapsedS> openDuration;
    openDuration.SetNull();
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    openDuration.SetNonNull(12u);
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable.ValueOr(0), 12u);
}

// This test ensures that the Open command correctly errors when the TargetLevel field is
// set for devices that do not support the LVL feature.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelFeatureUnsupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = false
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);

    // Should get an error when this is called with target level set since the feature is unsupported.
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(50u)), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
}

// This test ensures that The Open command correctly falls back to the spec-defined default when the target level
// is omitted and the DefaultOpenLevel is not supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelNotSuppliedNoDefaultSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = false,
                                       .supportsValveFault       = false,
                                       .supportsLevelStep        = false };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 100u);
}

// This test ensures that the Open command correclty calls back to the DefaultOpenLevel when the target level
// is omitted and the DefaultOpenLevel is supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelNotSuppliedDefaultSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = false,
                                       .supportsLevelStep        = false };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 100u);

    EXPECT_EQ(logic.SetDefaultOpenLevel(50u), CHIP_NO_ERROR);
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 50u);
}

// This test ensures the Open command uses the supplied target level if it is supplied.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelSupplied)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = false,
                                       .supportsLevelStep        = true };
    ClusterInitParameters state;
    state.levelStep = 33;
    EXPECT_EQ(logic.Init(conformance, state), CHIP_NO_ERROR);

    // 33, 66, 99 and 100 should all work, nothing else should
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(33u)), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 33u);

    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(66u)), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 66u);

    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(99u)), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 99u);

    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(100u)), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 100u);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 4);

    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(32u)), CHIP_ERROR_INVALID_ARGUMENT);
    // Ensure this wasn't called again.
    EXPECT_EQ(delegate.lastRequestedLevel, 100u);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 4);
}

//=========================================================================================
// Tests for Open Command handlers - current and target values
//=========================================================================================

// This test ensures the target and current values are set correcly when the delegate is
// able to open the value immediately. Cluster supports LVL feature
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenImmediateLevelSupported)
{
    // Testing that current level, target level, target state and current state are set correctly
    // If the delegate is able to open the valve fully during the handler call
    // then the current level and current state should indicate open and the appropriate level.
    // TargetLevel and TargetState should be NULL.
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    delegate.simulateFailure   = false;
    delegate.simulateAsyncOpen = false;

    DataModel::Nullable<uint8_t> level;
    DataModel::Nullable<ValveStateEnum> state;
    uint8_t targetLevel;

    targetLevel = 100u;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(targetLevel)), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetCurrentLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level.ValueOr(0), targetLevel);
    EXPECT_EQ(logic.GetTargetLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level, DataModel::NullNullable);
    EXPECT_EQ(logic.GetCurrentState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kOpen);
    EXPECT_EQ(logic.GetTargetState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, DataModel::NullNullable);

    targetLevel = 50u;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(targetLevel)), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetCurrentLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level.ValueOr(0), targetLevel);
    EXPECT_EQ(logic.GetTargetLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level, DataModel::NullNullable);
    EXPECT_EQ(logic.GetCurrentState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kOpen);
    EXPECT_EQ(logic.GetTargetState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, DataModel::NullNullable);

    EXPECT_EQ(delegate.numHandleOpenValveCalls, 2);
}

// This test ensures the target and current values are set correcly when the delegate is
// able to open the value immediately. Cluster does not support LVL feature
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenImmediateLevelNotSupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = true, .supportsLevelStep = false
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    delegate.simulateFailure   = false;
    delegate.simulateAsyncOpen = false;

    DataModel::Nullable<ValveStateEnum> state;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetCurrentState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kOpen);
    EXPECT_EQ(logic.GetTargetState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, DataModel::NullNullable);

    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
}

// This test ensures the target and current values are set correcly when the delegate is
// not able to open the value immediately. Cluster supports LVL feature
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenAsyncLevelSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    delegate.simulateFailure   = false;
    delegate.simulateAsyncOpen = true;

    DataModel::Nullable<uint8_t> level;
    DataModel::Nullable<ValveStateEnum> state;
    uint8_t targetLevel;

    targetLevel = 100u;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(targetLevel)), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetCurrentLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level.ValueOr(0), targetLevel / 2);
    EXPECT_EQ(logic.GetTargetLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level.ValueOr(0), targetLevel);
    EXPECT_EQ(logic.GetCurrentState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kTransitioning);
    EXPECT_EQ(logic.GetTargetState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kOpen);

    targetLevel = 50u;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::make_optional(targetLevel)), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetCurrentLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level.ValueOr(0), targetLevel / 2);
    EXPECT_EQ(logic.GetTargetLevel(level), CHIP_NO_ERROR);
    EXPECT_EQ(level.ValueOr(0), targetLevel);
    EXPECT_EQ(logic.GetCurrentState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kTransitioning);
    EXPECT_EQ(logic.GetTargetState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kOpen);

    EXPECT_EQ(delegate.numHandleOpenValveCalls, 2);
}

// This test ensures the target and current values are set correcly when the delegate is
// not able to open the value immediately. Cluster does not support LVL feature
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenAsyncLevelNotSupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = {
        .featureMap               = 0,
        .supportsDefaultOpenLevel = false,
        .supportsValveFault       = false,
        .supportsLevelStep        = false,
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    delegate.simulateFailure   = false;
    delegate.simulateAsyncOpen = true;

    DataModel::Nullable<ValveStateEnum> state;

    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetCurrentState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kTransitioning);
    EXPECT_EQ(logic.GetTargetState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state.ValueOr(ValveStateEnum::kUnknownEnumValue), ValveStateEnum::kOpen);

    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
}

//=========================================================================================
// Tests for Open Command handlers - valve faults
//=========================================================================================

// Test ensures valve faults are handled correctly when the valve can still be opened. LVL feature supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedNoErrorLevel)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    BitMask<ValveFaultBitmap> valveFault;

    delegate.simulateValveFaultNoFailure = true;
    delegate.simulatedFailureBitMask     = BitMask<ValveFaultBitmap>(to_underlying(ValveFaultBitmap::kLeaking));
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
    EXPECT_EQ(logic.GetValveFault(valveFault), CHIP_NO_ERROR);
    EXPECT_EQ(valveFault, delegate.simulatedFailureBitMask);
}

// Test ensures valve faults are handled correctly when the valve can still be opened. LVL feature not supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedNoErrorNoLevel)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = {
        .featureMap               = 0,
        .supportsDefaultOpenLevel = false,
        .supportsValveFault       = true,
        .supportsLevelStep        = false,
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    BitMask<ValveFaultBitmap> valveFault;

    delegate.simulateValveFaultNoFailure = true;
    delegate.simulatedFailureBitMask     = BitMask<ValveFaultBitmap>(to_underlying(ValveFaultBitmap::kLeaking));
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
    EXPECT_EQ(logic.GetValveFault(valveFault), CHIP_NO_ERROR);
    EXPECT_EQ(valveFault, delegate.simulatedFailureBitMask);
}

// Test ensures valve faults are handled correctly when the valve cannot be opened. LVL feature supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedErrorLevel)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    BitMask<ValveFaultBitmap> valveFault;

    delegate.simulateFailure = true;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
    EXPECT_EQ(logic.GetValveFault(valveFault), CHIP_NO_ERROR);
    EXPECT_EQ(valveFault, delegate.simulatedFailureBitMask);
}

// Test ensures valve faults are handled correctly when the valve cannot be opened. LVL feature not supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedErrorNoLevel)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = {
        .featureMap               = 0,
        .supportsDefaultOpenLevel = false,
        .supportsValveFault       = true,
        .supportsLevelStep        = false,
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    BitMask<ValveFaultBitmap> valveFault;

    delegate.simulateFailure = true;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);
    EXPECT_EQ(logic.GetValveFault(valveFault), CHIP_NO_ERROR);
    EXPECT_EQ(valveFault, delegate.simulatedFailureBitMask);
}

// Test ensures returned valve faults do not cause cluster problems if returned from the delegate when the valve feature is not
// supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedErrorLevelFaultNotSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = false,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    delegate.simulateFailure = true;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);

    delegate.simulateFailure             = false;
    delegate.simulateValveFaultNoFailure = true;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 2);
}

// Test ensures returned valve faults do not cause cluster problems if returned from the delegate when the valve feature is not
// supported.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedErrorNoLevelFaultNotSupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = {
        .featureMap               = 0,
        .supportsDefaultOpenLevel = false,
        .supportsValveFault       = false,
        .supportsLevelStep        = false,
    };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    delegate.simulateFailure = true;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 1);

    delegate.simulateFailure             = false;
    delegate.simulateValveFaultNoFailure = true;
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.numHandleOpenValveCalls, 2);
}

bool HasAttributeChanges(std::vector<AttributeId> changes, AttributeId id)
{
    return std::find(changes.begin(), changes.end(), id) != changes.end();
}
//=========================================================================================
// Tests for timing for Attribute updates and RemainingDuration Q
//=========================================================================================
// Tests that remaining duration is updated when openLevel is set to a value or to to null
// Tests that attribute reports are sent out for attributes changed on Open call
// Tests that attribute reports are sent for RemainingDuration at Q times.
TEST_F(TestValveConfigurationAndControlClusterLogic, TestAttributeUpdates)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;
    DataModel::Nullable<Percent> valPercentNullable;

    // When null, RemainingDuration should also be null. No updates are expected.
    context.ClearDirtyList();
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(DataModel::NullNullable), std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);
    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);
    // We should see the currentLevel and currentState marked as dirty, and the targets should not be
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));

    // When non-null RemainingDuration should initially be set to OpenDuration and this should trigger
    // an attribute change callback. Attribute change callbacks should happen no more than once per second
    // and at the end.
    // This test tests attribute callbacks on an open duration that runs to the end. The OpenDuration is
    // in units of seconds, so the close call will come at units of seconds.
    gSystemLayerAndClock.SetMonotonic(0_ms64);
    context.ClearDirtyList();

    DataModel::Nullable<ElapsedS> openDuration;
    openDuration.SetNonNull(12u);
    Percent requestedLevel = 50u;
    // Test also that we get an attribute update
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::make_optional(requestedLevel)), CHIP_NO_ERROR);
    EXPECT_EQ(logic.GetOpenDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable.ValueOr(0), openDuration.Value());
    EXPECT_EQ(logic.GetCurrentLevel(valPercentNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valPercentNullable.ValueOr(0), requestedLevel);
    // We should see the following attributes marked as dirty: currentLevel, remainingDuration, openDuration
    // The targetLevel and targetState should be null, and thus we should see no update.
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetState::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));

    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(500_ms64);
    // No new reports should be happening in this time.
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    // Even if we read the remaining duration to force an update, we shouldn't expect an attribute change report
    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));

    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    // At 1s, the system should generate a report.
    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(500_ms64);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable.ValueOr(0), openDuration.Value() - 1);

    // Nothing else should have changed
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    // At 1.7s, the system should not generate a report even if we read the value to force and update.
    // The new duration should be reflected (rounded to nearest s)
    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(700_ms64);
    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable.ValueOr(0), openDuration.Value() - 2);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));

    // At 2s, we should get an attribute change report even if we don't read.
    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(300_ms64);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));

    // Only that one attribute should be reporting a change.
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::TargetState::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    // Clear out all the attribute reports at 11.5 seconds. This should trigger the prior timer.
    gSystemLayerAndClock.SetMonotonic(11500_ms64);
    context.ClearDirtyList();

    // Ensure we get a report at 12s and that the value goes back to Null
    gSystemLayerAndClock.AdvanceMonotonic(500_ms64);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));

    // Everything else should also be reporting changes as they flip back to their defaults.
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    context.ClearDirtyList();
    // Ensure we don't get a further report generated by reading
    EXPECT_EQ(logic.GetRemainingDuration(valElapsedSNullable), CHIP_NO_ERROR);
    EXPECT_EQ(valElapsedSNullable, DataModel::NullNullable);

    // Test increasing the open duration on a non-second order to see that we get an attribute change callback for the change
    // up.
    gSystemLayerAndClock.SetMonotonic(0_ms64);
    context.ClearDirtyList();

    openDuration.SetNonNull(12u);
    requestedLevel = 50u;
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::make_optional(requestedLevel)), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));

    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(500_ms64);
    // At 0.5s, we wouldn't normally expect a report for duration, but if we send a new open to increase the time, we should.
    openDuration.SetNonNull(15u);
    requestedLevel = 50u;
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::make_optional(requestedLevel)), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));

    // TODO: Clarify, should we get a report if we use open to set the remaining duration down? I think so.
    // TODO: Add such tests here. I don't think the underlying layer handles that properly right now.

    // Just before the next report should go out, we set this to NULL. We should get a report immediately.
    gSystemLayerAndClock.AdvanceMonotonic(400_ms64);

    openDuration.SetNull();
    requestedLevel = 100u;
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::make_optional(requestedLevel)), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentLevel::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::CurrentState::Id));

    context.ClearDirtyList();

    // Check that we DON'T get a report from the previous Open call, which had a remaining duration timer on it.
    gSystemLayerAndClock.AdvanceMonotonic(100_ms64);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
}

// Tests that RemainingDuration gets updated correctly even if the timer doesn't fire on the exact ms (it doesn't).
TEST_F(TestValveConfigurationAndControlClusterLogic, TestAttributeUpdatesNonExactClock)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MockedMatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    gSystemLayerAndClock.SetMonotonic(0_ms64);
    gSystemLayerAndClock.Clear();
    DataModel::Nullable<ElapsedS> openDuration;
    openDuration.SetNonNull(2u);
    EXPECT_EQ(logic.HandleOpenCommand(std::make_optional(openDuration), std::nullopt), CHIP_NO_ERROR);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(550_ms64);
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    context.ClearDirtyList();
    gSystemLayerAndClock.AdvanceMonotonic(550_ms64);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_FALSE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));

    context.ClearDirtyList();
    gSystemLayerAndClock.SetMonotonic(2100_ms64);
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::RemainingDuration::Id));
    EXPECT_TRUE(HasAttributeChanges(context.GetDirtyList(), Attributes::OpenDuration::Id));
}

//=========================================================================================
// Tests for automatically calling close from the cluster logic
//=========================================================================================
// ensures close is called at open duration and not before

//=========================================================================================
// Tests for handling close commands
//=========================================================================================
// while remaining duration is null and valve is open
// while remaining duration is not null and valve is open
// while valve is closed

//=========================================================================================
// Tests for timing for async read updates to current / target level and state
//=========================================================================================
// TODO: Should the cluster logic be responsible for reaching out to the delegate at the Q update period to get updates?
// TODO: What about for target state? Should this be drive by the cluster logic or the delegate? Maybe both?
// Pros for cluster logic:
// - delegate has fewer responsibilities, doesn't have to do things like timers for updating level
// Pros for delegate:
// - hardware might have built-in mechanisms to alert for level changes and state changes

// Tests that GetCurrentValveLevel is called until the level hits the target
// Tests that the attribute update is not more than
// TODO: should reads in the middle of the update period go out and get the current value? Probably, but it's annoying.

// Timing tests for async read
// Tests to ensure that we get calls to update the level and state until the target state / level is reached.
// Tests to ensure that we don't get updates on more than an X basis or at the end and the beginning

//=========================================================================================
// Tests for attribute callbacks from delegates
//=========================================================================================
// TODO: Should the delegate call the cluster logic class direclty, or should this be piped through the delegate class so the app
// layer ONLY has to interact with the delegate?
// Test setter for valve fault Test attribute change notifications are sent out and not
// sent out when the attribute is change do the same value - add in prior

//=========================================================================================
// Tests for attribute callbacks from delegates
//=========================================================================================
// Tests for events
// Test events are generated

//=========================================================================================
// Tests for attribute callbacks from delegates
//=========================================================================================
// Tests for auto-close
// Test that the auto close time is set appropriately for open duration - add in prior test?
// should work both when the time is available at the time of the call and when the time is later set

// TODO: Should the cluster logic query the current level and curent state from the driver at startup?

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
