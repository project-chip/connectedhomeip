/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ThermostatTestCommon.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/clusters/thermostat-server/ThermostatSuggestionStructWithOwnedMembers.h>
#include <clusters/Thermostat/Metadata.h>

#include <algorithm>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::System::Clock;
using namespace chip::Testing;
using chip::app::Clusters::Globals::AtomicRequestTypeEnum;

namespace {

constexpr uint64_t kValidRealTimeMicroseconds = 1700000000ULL * 1000000ULL; // Post-2020 Unix timestamp

TEST(TestThermostatSuggestionStruct, TestStructOperations)
{
    ThermostatSuggestionStructWithOwnedMembers suggestion;
    suggestion.SetUniqueID(42);
    EXPECT_EQ(suggestion.GetUniqueID(), 42);

    uint8_t handleBytes[] = { 0x11, 0x22, 0x33, 0x44 };
    ByteSpan handle(handleBytes);
    EXPECT_EQ(suggestion.SetPresetHandle(handle), CHIP_NO_ERROR);
    EXPECT_TRUE(suggestion.GetPresetHandle().data_equal(handle));

    // Exceeding preset handle size
    uint8_t largeHandle[kThermostatSuggestionPresetHandleSize + 1];
    EXPECT_EQ(suggestion.SetPresetHandle(ByteSpan(largeHandle)), CHIP_ERROR_NO_MEMORY);

    suggestion.SetEffectiveTime(Seconds32(100));
    EXPECT_EQ(suggestion.GetEffectiveTime(), Seconds32(100));

    suggestion.SetExpirationTime(Seconds32(200));
    EXPECT_EQ(suggestion.GetExpirationTime(), Seconds32(200));

    // Copy constructor
    ThermostatSuggestionStructWithOwnedMembers copy(suggestion);
    EXPECT_EQ(copy.GetUniqueID(), 42);
    EXPECT_TRUE(copy.GetPresetHandle().data_equal(handle));
    EXPECT_EQ(copy.GetEffectiveTime(), Seconds32(100));
    EXPECT_EQ(copy.GetExpirationTime(), Seconds32(200));

    // Copy assignment
    ThermostatSuggestionStructWithOwnedMembers assigned;
    assigned = copy;
    EXPECT_EQ(assigned.GetUniqueID(), 42);
    EXPECT_TRUE(assigned.GetPresetHandle().data_equal(handle));

    // Assignment from base Struct
    Structs::ThermostatSuggestionStruct::Type baseStruct;
    baseStruct.uniqueID       = 99;
    baseStruct.presetHandle   = handle;
    baseStruct.effectiveTime  = 300;
    baseStruct.expirationTime = 400;

    assigned = baseStruct;
    EXPECT_EQ(assigned.GetUniqueID(), 99);
    EXPECT_EQ(assigned.GetEffectiveTime(), Seconds32(300));
    EXPECT_EQ(assigned.GetExpirationTime(), Seconds32(400));
}

TEST_F(ThermostatTestFixture, TestHoldAttributesReadWrite)
{
    mOptionalAttributes.TemperatureSetpointHold         = true;
    mOptionalAttributes.TemperatureSetpointHoldDuration = true;
    mOptionalAttributes.SetpointHoldExpiryTimestamp     = true;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mHoldDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read TemperatureSetpointHold
    TemperatureSetpointHoldEnum hold = TemperatureSetpointHoldEnum::kSetpointHoldOff;
    EXPECT_EQ(tester.ReadAttribute(TemperatureSetpointHold::Id, hold), Status::Success);
    EXPECT_EQ(hold, TemperatureSetpointHoldEnum::kSetpointHoldOff);

    // Write TemperatureSetpointHold
    EXPECT_EQ(tester.WriteAttribute(TemperatureSetpointHold::Id, TemperatureSetpointHoldEnum::kSetpointHoldOn), Status::Success);
    EXPECT_EQ(mHoldDelegate.mHold, TemperatureSetpointHoldEnum::kSetpointHoldOn);
    EXPECT_TRUE(tester.IsAttributeDirty(TemperatureSetpointHold::Id));

    // Invalid enum
    EXPECT_EQ(tester.WriteAttribute(TemperatureSetpointHold::Id, static_cast<TemperatureSetpointHoldEnum>(0xFF)),
              Status::InvalidValue);

    // Read & Write TemperatureSetpointHoldDuration
    DataModel::Nullable<uint16_t> duration = 0;
    EXPECT_EQ(tester.ReadAttribute(TemperatureSetpointHoldDuration::Id, duration), Status::Success);
    ASSERT_FALSE(duration.IsNull());
    EXPECT_EQ(duration.Value(), 60);

    EXPECT_EQ(tester.WriteAttribute(TemperatureSetpointHoldDuration::Id, DataModel::MakeNullable<uint16_t>(120)), Status::Success);
    EXPECT_EQ(mHoldDelegate.mDuration.Value(), 120);

    // Duration exceeding max (1440 min) -> InvalidValue
    EXPECT_EQ(tester.WriteAttribute(TemperatureSetpointHoldDuration::Id, DataModel::MakeNullable<uint16_t>(1500)),
              Status::InvalidValue);

    // Read & Write SetpointHoldExpiryTimestamp
    DataModel::Nullable<uint32_t> timestamp = 0;
    EXPECT_EQ(tester.ReadAttribute(SetpointHoldExpiryTimestamp::Id, timestamp), Status::Success);
    EXPECT_EQ(timestamp.Value(), 0u);

    EXPECT_EQ(tester.WriteAttribute(SetpointHoldExpiryTimestamp::Id, DataModel::MakeNullable<uint32_t>(12345678)), Status::Success);
    EXPECT_EQ(mHoldDelegate.mExpiryTimestamp.Value(), 12345678u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestOccupancyFeatureAndEvents)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kOccupancy, Feature::kEvents);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mOccupancyDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read Occupancy
    BitMask<OccupancyBitmap> occ = {};
    EXPECT_EQ(tester.ReadAttribute(Occupancy::Id, occ), Status::Success);
    EXPECT_TRUE(occ.Has(OccupancyBitmap::kOccupied));
    EXPECT_TRUE(cluster.IsOccupied());

    // Update Occupancy via cluster SetOccupancy
    BitMask<OccupancyBitmap> unocc;
    EXPECT_EQ(cluster.SetOccupancy(unocc), Status::Success);
    EXPECT_FALSE(cluster.IsOccupied());
    EXPECT_TRUE(tester.IsAttributeDirty(Occupancy::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSuggestionsAttributesAndAddRemoveCommands)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    PresetStructWithOwnedMembers preset;
    preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handle[4] = { 1, 2, 3, 4 };
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
    mPresetsDelegate.mPresets.push_back(preset);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Set up mock clock with 0 timestamp (unsynced Matter epoch)
    System::Clock::Internal::RAIIMockClock mockClock;

    // Read MaxThermostatSuggestions
    uint8_t maxSuggestions = 0;
    EXPECT_EQ(tester.ReadAttribute(MaxThermostatSuggestions::Id, maxSuggestions), Status::Success);
    EXPECT_EQ(maxSuggestions, 5);

    // 1. Trying to add suggestion without synced clock -> InvalidInState
    Commands::AddThermostatSuggestion::Type addCmd;
    addCmd.presetHandle        = ByteSpan(handle);
    addCmd.expirationInMinutes = 60;
    addCmd.effectiveTime       = DataModel::NullNullable;

    auto result = tester.Invoke(addCmd);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidInState));

    // 2. Set synced mock clock
    EXPECT_EQ(mockClock.SetClock_RealTime(Microseconds64(kValidRealTimeMicroseconds)), CHIP_NO_ERROR);

    // 3. Expiration out of bounds (< 30 or > 1440) -> ConstraintError
    addCmd.expirationInMinutes = 10;
    result                     = tester.Invoke(addCmd);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

    addCmd.expirationInMinutes = 2000;
    result                     = tester.Invoke(addCmd);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));

    // 4. Non-existent preset handle -> NotFound
    uint8_t unknownHandle[4]   = { 9, 9, 9, 9 };
    addCmd.presetHandle        = ByteSpan(unknownHandle);
    addCmd.expirationInMinutes = 60;
    result                     = tester.Invoke(addCmd);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::NotFound));

    // 5. Valid suggestion add -> Success
    addCmd.presetHandle = ByteSpan(handle);
    result              = tester.Invoke(addCmd);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    uint8_t uniqueID = 0;
    if (result.response.has_value())
    {
        uniqueID = result.response.value().uniqueID;
        EXPECT_GT(uniqueID, 0);
    }

    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_TRUE(mSuggestionsDelegate.mReEvaluateCalled);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::ThermostatSuggestions::Id));

    // 6. Remove suggestion with matching uniqueID -> Success
    Commands::RemoveThermostatSuggestion::Type removeCmd;
    removeCmd.uniqueID = uniqueID;
    auto removeResult  = tester.Invoke(removeCmd);
    EXPECT_TRUE(removeResult.IsSuccess());
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 0u);

    // 7. Remove non-existent suggestion -> NotFound
    removeCmd.uniqueID = 0xFE;
    removeResult       = tester.Invoke(removeCmd);
    EXPECT_EQ(removeResult.GetStatusCode(), ClusterStatusCode(Status::NotFound));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestPresetRemovalCascadesToStaleSuggestion)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    // Two presets: A survives the atomic write below, B does not. Both are pre-existing (committed) presets, so
    // BuiltIn must be non-null on both them and their replacements below, per AppendPendingPreset's built-in
    // consistency check.
    PresetStructWithOwnedMembers presetA;
    presetA.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleA[4] = { 1, 1, 1, 1 };
    EXPECT_EQ(presetA.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleA))), CHIP_NO_ERROR);
    presetA.SetBuiltIn(DataModel::MakeNullable(true));
    mPresetsDelegate.mPresets.push_back(presetA);

    PresetStructWithOwnedMembers presetB;
    presetB.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleB[4] = { 2, 2, 2, 2 };
    EXPECT_EQ(presetB.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleB))), CHIP_NO_ERROR);
    presetB.SetBuiltIn(DataModel::MakeNullable(false));
    mPresetsDelegate.mPresets.push_back(presetB);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // A suggestion referencing preset B, and it is the current suggestion.
    ThermostatSuggestionStructWithOwnedMembers suggestion;
    suggestion.SetUniqueID(7);
    EXPECT_EQ(suggestion.SetPresetHandle(ByteSpan(handleB)), CHIP_NO_ERROR);
    suggestion.SetEffectiveTime(Seconds32(0));
    suggestion.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(suggestion);
    mSuggestionsDelegate.mCurrentSuggestion.SetNonNull(suggestion);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Remove preset B by committing a Presets atomic write that only re-lists preset A.
    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Attributes::Presets::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    ASSERT_TRUE(tester.Invoke(beginReq).IsSuccess());

    Structs::PresetStruct::Type keepA;
    keepA.presetScenario                  = PresetScenarioEnum::kOccupied;
    keepA.presetHandle                    = DataModel::MakeNullable(ByteSpan(handleA));
    keepA.builtIn                         = DataModel::MakeNullable(true);
    Structs::PresetStruct::Type newList[] = { keepA };
    auto listPayload                      = DataModel::List<const Structs::PresetStruct::Type>(newList, 1);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    ASSERT_TRUE(tester.Invoke(commitReq).IsSuccess());

    // The stale suggestion referencing the removed preset B is gone, and CurrentThermostatSuggestion is null
    // (there is nothing left to pick as a replacement), with both attributes reported dirty.
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 0u);
    EXPECT_TRUE(mSuggestionsDelegate.mCurrentSuggestion.IsNull());
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::ThermostatSuggestions::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(CurrentThermostatSuggestion::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestPresetRemovalLeavesSurvivingPresetSuggestionAlone)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    PresetStructWithOwnedMembers presetA;
    presetA.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleA[4] = { 1, 1, 1, 1 };
    EXPECT_EQ(presetA.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleA))), CHIP_NO_ERROR);
    presetA.SetBuiltIn(DataModel::MakeNullable(true));
    mPresetsDelegate.mPresets.push_back(presetA);

    PresetStructWithOwnedMembers presetB;
    presetB.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleB[4] = { 2, 2, 2, 2 };
    EXPECT_EQ(presetB.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleB))), CHIP_NO_ERROR);
    presetB.SetBuiltIn(DataModel::MakeNullable(false));
    mPresetsDelegate.mPresets.push_back(presetB);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // The only suggestion references the surviving preset A.
    ThermostatSuggestionStructWithOwnedMembers suggestion;
    suggestion.SetUniqueID(9);
    EXPECT_EQ(suggestion.SetPresetHandle(ByteSpan(handleA)), CHIP_NO_ERROR);
    suggestion.SetEffectiveTime(Seconds32(0));
    suggestion.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(suggestion);
    mSuggestionsDelegate.mCurrentSuggestion.SetNonNull(suggestion);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Remove preset B, leaving A (and its suggestion) untouched.
    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Attributes::Presets::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    ASSERT_TRUE(tester.Invoke(beginReq).IsSuccess());

    Structs::PresetStruct::Type keepA;
    keepA.presetScenario                  = PresetScenarioEnum::kOccupied;
    keepA.presetHandle                    = DataModel::MakeNullable(ByteSpan(handleA));
    keepA.builtIn                         = DataModel::MakeNullable(true);
    Structs::PresetStruct::Type newList[] = { keepA };
    auto listPayload                      = DataModel::List<const Structs::PresetStruct::Type>(newList, 1);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    ASSERT_TRUE(tester.Invoke(commitReq).IsSuccess());

    ASSERT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions[0].GetUniqueID(), 9);
    ASSERT_FALSE(mSuggestionsDelegate.mCurrentSuggestion.IsNull());
    EXPECT_EQ(mSuggestionsDelegate.mCurrentSuggestion.Value().GetUniqueID(), 9);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestPresetRemovalCascadeAbortsOnEnumerationFailure)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    PresetStructWithOwnedMembers presetA;
    presetA.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleA[4] = { 1, 1, 1, 1 };
    EXPECT_EQ(presetA.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleA))), CHIP_NO_ERROR);
    presetA.SetBuiltIn(DataModel::MakeNullable(true));
    mPresetsDelegate.mPresets.push_back(presetA);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // A suggestion referencing a preset that will no longer exist after the atomic write below.
    uint8_t handleB[4] = { 2, 2, 2, 2 };
    ThermostatSuggestionStructWithOwnedMembers suggestion;
    suggestion.SetUniqueID(3);
    EXPECT_EQ(suggestion.SetPresetHandle(ByteSpan(handleB)), CHIP_NO_ERROR);
    suggestion.SetEffectiveTime(Seconds32(0));
    suggestion.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(suggestion);
    mSuggestionsDelegate.mCurrentSuggestion.SetNonNull(suggestion);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Attributes::Presets::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    ASSERT_TRUE(tester.Invoke(beginReq).IsSuccess());

    Structs::PresetStruct::Type keepA;
    keepA.presetScenario                  = PresetScenarioEnum::kOccupied;
    keepA.presetHandle                    = DataModel::MakeNullable(ByteSpan(handleA));
    keepA.builtIn                         = DataModel::MakeNullable(true);
    Structs::PresetStruct::Type newList[] = { keepA };
    auto listPayload                      = DataModel::List<const Structs::PresetStruct::Type>(newList, 1);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    // The Presets commit itself will enumerate the (now single-entry) committed list cleanly; only fail the
    // first call the cascade's own scan makes afterwards, to exercise its abort-on-error path specifically
    // rather than an unrelated failure earlier in PrecommitPresets' own enumeration.
    mPresetsDelegate.mFailGetPresetAtIndexOnCall = 4;

    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    ASSERT_TRUE(tester.Invoke(commitReq).IsSuccess());

    // The Presets commit itself succeeded; the cascade aborted on the enumeration failure without removing
    // anything, rather than treating the failure as "preset not found" and dropping the suggestion.
    ASSERT_EQ(mPresetsDelegate.mPresets.size(), 1u);
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_FALSE(mSuggestionsDelegate.mCurrentSuggestion.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestPresetRemovalCascadeAbortsOnSuggestionLookupFailure)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    PresetStructWithOwnedMembers presetA;
    presetA.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleA[4] = { 1, 1, 1, 1 };
    EXPECT_EQ(presetA.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleA))), CHIP_NO_ERROR);
    presetA.SetBuiltIn(DataModel::MakeNullable(true));
    mPresetsDelegate.mPresets.push_back(presetA);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // A suggestion referencing a preset that will no longer exist after the atomic write below.
    uint8_t handleB[4] = { 2, 2, 2, 2 };
    ThermostatSuggestionStructWithOwnedMembers suggestion;
    suggestion.SetUniqueID(3);
    EXPECT_EQ(suggestion.SetPresetHandle(ByteSpan(handleB)), CHIP_NO_ERROR);
    suggestion.SetEffectiveTime(Seconds32(0));
    suggestion.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(suggestion);
    mSuggestionsDelegate.mCurrentSuggestion.SetNonNull(suggestion);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Attributes::Presets::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    ASSERT_TRUE(tester.Invoke(beginReq).IsSuccess());

    Structs::PresetStruct::Type keepA;
    keepA.presetScenario                  = PresetScenarioEnum::kOccupied;
    keepA.presetHandle                    = DataModel::MakeNullable(ByteSpan(handleA));
    keepA.builtIn                         = DataModel::MakeNullable(true);
    Structs::PresetStruct::Type newList[] = { keepA };
    auto listPayload                      = DataModel::List<const Structs::PresetStruct::Type>(newList, 1);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    // Fail the cascade's own first (lookup) pass over ThermostatSuggestions itself, rather than its scan of
    // Presets: this exercises GetThermostatSuggestionAtIndex()'s error path (as opposed to
    // TestPresetRemovalCascadeAbortsOnEnumerationFailure, which exercises PresetHandleStillExists()'s).
    mSuggestionsDelegate.mFailGetThermostatSuggestionAtIndexOnCall = 1;

    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    ASSERT_TRUE(tester.Invoke(commitReq).IsSuccess());

    // The Presets commit itself succeeded; the cascade aborted on the lookup failure without removing anything or
    // re-evaluating the current suggestion.
    ASSERT_EQ(mPresetsDelegate.mPresets.size(), 1u);
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_FALSE(mSuggestionsDelegate.mCurrentSuggestion.IsNull());
    EXPECT_FALSE(mSuggestionsDelegate.mReEvaluateCalled);
    EXPECT_FALSE(tester.IsAttributeDirty(Attributes::ThermostatSuggestions::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestPresetRemovalCascadeAbortsOnRemovalFailure)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // No presets survive the atomic write below, so both suggestions below become stale.
    uint8_t handleB[4] = { 2, 2, 2, 2 };
    uint8_t handleC[4] = { 3, 3, 3, 3 };

    ThermostatSuggestionStructWithOwnedMembers suggestion1;
    suggestion1.SetUniqueID(11);
    EXPECT_EQ(suggestion1.SetPresetHandle(ByteSpan(handleB)), CHIP_NO_ERROR);
    suggestion1.SetEffectiveTime(Seconds32(0));
    suggestion1.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(suggestion1);

    ThermostatSuggestionStructWithOwnedMembers suggestion2;
    suggestion2.SetUniqueID(12);
    EXPECT_EQ(suggestion2.SetPresetHandle(ByteSpan(handleC)), CHIP_NO_ERROR);
    suggestion2.SetEffectiveTime(Seconds32(0));
    suggestion2.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(suggestion2);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Attributes::Presets::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    ASSERT_TRUE(tester.Invoke(beginReq).IsSuccess());

    // Remove every preset, so both suggestions above become stale.
    DataModel::List<const Structs::PresetStruct::Type> emptyListPayload;
    ASSERT_EQ(tester.WriteAttribute(Attributes::Presets::Id, emptyListPayload, ListWritingPattern::ReplaceAll), Status::Success);

    // Fail the cascade's second-pass removal of the lower-index stale entry, after it has already removed the
    // higher-index one (the backward walk visits index 1 first, then index 0).
    mSuggestionsDelegate.mFailRemoveFromThermostatSuggestionsListOnCall = 2;

    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    ASSERT_TRUE(tester.Invoke(commitReq).IsSuccess());

    // The higher-index entry was removed before the failure, but the lower-index one was not: the cascade is
    // partial, so it must report incomplete and OnPresetsCommitted() must skip re-evaluating the current
    // suggestion rather than treat the partial cleanup as done.
    ASSERT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions[0].GetUniqueID(), 11);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::ThermostatSuggestions::Id));
    EXPECT_FALSE(mSuggestionsDelegate.mReEvaluateCalled);
    EXPECT_FALSE(tester.IsAttributeDirty(CurrentThermostatSuggestion::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestPresetRemovalCascadeDoesNotDoubleNotifyCurrentSuggestion)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kThermostatSuggestions);

    PresetStructWithOwnedMembers presetA;
    presetA.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleA[4] = { 1, 1, 1, 1 };
    EXPECT_EQ(presetA.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleA))), CHIP_NO_ERROR);
    presetA.SetBuiltIn(DataModel::MakeNullable(true));
    mPresetsDelegate.mPresets.push_back(presetA);

    PresetStructWithOwnedMembers presetB;
    presetB.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handleB[4] = { 2, 2, 2, 2 };
    EXPECT_EQ(presetB.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handleB))), CHIP_NO_ERROR);
    presetB.SetBuiltIn(DataModel::MakeNullable(false));
    mPresetsDelegate.mPresets.push_back(presetB);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // A surviving suggestion referencing preset A, and a stale one referencing preset B that is the current
    // suggestion. Removing the current suggestion's preset must let the surviving suggestion become the new
    // current one, reported changed exactly once: ReEvaluateCurrentSuggestion() already notifies that internally,
    // so OnPresetsCommitted()'s own before/after diff must not notify it a second time.
    ThermostatSuggestionStructWithOwnedMembers survivingSuggestion;
    survivingSuggestion.SetUniqueID(21);
    EXPECT_EQ(survivingSuggestion.SetPresetHandle(ByteSpan(handleA)), CHIP_NO_ERROR);
    survivingSuggestion.SetEffectiveTime(Seconds32(0));
    survivingSuggestion.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(survivingSuggestion);

    ThermostatSuggestionStructWithOwnedMembers staleCurrentSuggestion;
    staleCurrentSuggestion.SetUniqueID(22);
    EXPECT_EQ(staleCurrentSuggestion.SetPresetHandle(ByteSpan(handleB)), CHIP_NO_ERROR);
    staleCurrentSuggestion.SetEffectiveTime(Seconds32(0));
    staleCurrentSuggestion.SetExpirationTime(Seconds32(1000000));
    mSuggestionsDelegate.mSuggestions.push_back(staleCurrentSuggestion);
    mSuggestionsDelegate.mCurrentSuggestion.SetNonNull(staleCurrentSuggestion);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate, mSuggestionsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType        = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Attributes::Presets::Id };
    beginReq.attributeRequests  = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout            = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));
    ASSERT_TRUE(tester.Invoke(beginReq).IsSuccess());

    Structs::PresetStruct::Type keepA;
    keepA.presetScenario                  = PresetScenarioEnum::kOccupied;
    keepA.presetHandle                    = DataModel::MakeNullable(ByteSpan(handleA));
    keepA.builtIn                         = DataModel::MakeNullable(true);
    Structs::PresetStruct::Type newList[] = { keepA };
    auto listPayload                      = DataModel::List<const Structs::PresetStruct::Type>(newList, 1);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    ASSERT_TRUE(tester.Invoke(commitReq).IsSuccess());

    ASSERT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions[0].GetUniqueID(), 21);
    ASSERT_FALSE(mSuggestionsDelegate.mCurrentSuggestion.IsNull());
    EXPECT_EQ(mSuggestionsDelegate.mCurrentSuggestion.Value().GetUniqueID(), 21);

    auto & dirtyList = tester.GetDirtyList();
    auto currentSuggestionPath =
        ConcreteAttributePath(cluster.GetPaths()[0].mEndpointId, cluster.GetPaths()[0].mClusterId, CurrentThermostatSuggestion::Id);
    EXPECT_EQ(std::count(dirtyList.begin(), dirtyList.end(), currentSuggestionPath), 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
