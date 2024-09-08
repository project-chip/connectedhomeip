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

class TestDelegate : public Delegate
{
public:
    TestDelegate() {}
    DataModel::Nullable<chip::Percent> HandleOpenValve(DataModel::Nullable<chip::Percent> level) override
    {
        return DataModel::NullNullable;
    }
    CHIP_ERROR HandleCloseValve() override { return CHIP_NO_ERROR; }
    void HandleRemainingDurationTick(uint32_t duration) override {}
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
    TestDelegate delegate;
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
    TestDelegate delegate;
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
    TestDelegate delegate;
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
    TestDelegate delegate;
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
    TestDelegate delegate;
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
    TestDelegate delegate;
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
    TestDelegate delegate;
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

} // namespace ValveConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
