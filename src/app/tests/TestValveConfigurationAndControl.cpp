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
#include <pw_unit_test/framework.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ValveConfigurationAndControl {

class TestValveConfigurationAndControlClusterLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesAllFeatures)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MatterContext context(0, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesNoFeatures)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MatterContext context(0, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesStartingState)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MatterContext context(0, storageDelegate);
    ClusterLogic logic(delegate, context);

    // Everything on, all should return values
    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    ClusterState state             = {
                    .openDuration        = DataModel::MakeNullable(static_cast<uint32_t>(12u)),
                    .defaultOpenDuration = DataModel::MakeNullable(static_cast<uint32_t>(10u)),
                    .autoCloseTime       = DataModel::MakeNullable(static_cast<uint64_t>(2500u)),
                    .remainingDuration   = DataModel::MakeNullable(static_cast<uint32_t>(1u)),
                    .currentState        = DataModel::MakeNullable(ValveStateEnum::kTransitioning),
                    .targetState         = DataModel::MakeNullable(ValveStateEnum::kOpen),
                    .currentLevel        = DataModel::MakeNullable(static_cast<uint8_t>(50u)),
                    .targetLevel         = DataModel::MakeNullable(static_cast<uint8_t>(75u)),
                    .defaultOpenLevel    = 90u,
                    .valveFault          = BitMask<ValveFaultBitmap>(ValveFaultBitmap::kLeaking),
                    .levelStep           = 2u,
    };
    EXPECT_EQ(logic.Init(conformance, state), CHIP_NO_ERROR);

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
    EXPECT_EQ(valElapsedSNullable, state.remainingDuration);

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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestGetAttributesUninitialized)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    MatterContext context(0, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestSetDefaultOpenDuration)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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
    MatterContext context_ep1             = MatterContext(1, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestSetDefaultOpenLevel)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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
    MatterContext context_ep1             = MatterContext(1, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestSetDefaultOpenLevelWithLevelStep)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    uint8_t val8;
    uint8_t testVal;

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    ClusterState state             = ClusterState();
    state.levelStep                = 45;
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestWrongDelegates)
{
    TestDelegateLevel delegateLevel;
    TestDelegateNoLevel delegateNoLevel;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
    ClusterLogic logicLevel(delegateLevel, context);
    ClusterLogic logicNoLevel(delegateNoLevel, context);
    ClusterConformance conformanceLevel   = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                              .supportsDefaultOpenLevel = true,
                                              .supportsValveFault       = true,
                                              .supportsLevelStep        = true };
    ClusterConformance conformanceNoLevel = {
        .featureMap = 0, .supportsDefaultOpenLevel = false, .supportsValveFault = false, .supportsLevelStep = false
    };

    EXPECT_EQ(logicLevel.Init(conformanceNoLevel), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(logicNoLevel.Init(conformanceLevel), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenDuration)
{

    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap = to_underlying(Feature::kLevel) | to_underlying(Feature::kTimeSync),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = true,
                                       .supportsLevelStep        = true };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> valElapsedSNullable;

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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelFeatureUnsupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelNotSuppliedNoDefaultSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = false,
                                       .supportsValveFault       = false,
                                       .supportsLevelStep        = false };
    EXPECT_EQ(logic.Init(conformance), CHIP_NO_ERROR);
    EXPECT_EQ(logic.HandleOpenCommand(std::nullopt, std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(delegate.lastRequestedLevel, 100u);
}

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelNotSuppliedDefaultSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenTargetLevelSupplied)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
    ClusterLogic logic(delegate, context);

    ClusterConformance conformance = { .featureMap               = to_underlying(Feature::kLevel),
                                       .supportsDefaultOpenLevel = true,
                                       .supportsValveFault       = false,
                                       .supportsLevelStep        = true };
    ClusterState state             = ClusterState();
    state.levelStep                = 33;
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenImmediateLevelSupported)
{
    // Testing that current level, target level, target state and current state are set correctly
    // If the delegate is able to open the valve fully during the handler call
    // then the current level and current state should indicate open and the appropriate level.
    // TargetLevel and TargetState should be NULL.
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenImmediateLevelNotSupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenAsyncLevelSupported)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenAsyncLevelNotSupported)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedNoErrorLevel)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedNoErrorNoLevel)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedErrorLevel)
{
    TestDelegateLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint = 0;
    MatterContext context(endpoint, storageDelegate);
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

TEST_F(TestValveConfigurationAndControlClusterLogic, TestHandleOpenFaultReturnedErrorNoLevel)
{
    TestDelegateNoLevel delegate;
    TestPersistentStorageDelegate storageDelegate;
    EndpointId endpoint   = 0;
    MatterContext context = MatterContext(endpoint, storageDelegate);
    ClusterLogic logic    = ClusterLogic(delegate, context);

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

// Clock::ClockBase * const savedClock = &SystemClock();
// Clock::Internal::MockClock mockClock;
// Clock::Internal::SetSystemClockForTesting(&mockClock);

// Test that the cluster logic doesn't balk if error is returned, but valve fault isn't supported
// Test that the auto close time is set appropriately for open duration - add in prior test?
// Test setter for valve fault
// Test attribute change notifications are sent out and not sent out when the attribute is change do the same value - add in prior
// tests? Test events are generated

// Init providing the wrong delegate type

// TODO: Should the cluster logic query the current level and curent state from the driver at startup?

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
