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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::System::Clock;
using namespace chip::Testing;

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
    TemperatureSetpointHoldEnum hold;
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
    DataModel::Nullable<uint16_t> duration;
    EXPECT_EQ(tester.ReadAttribute(TemperatureSetpointHoldDuration::Id, duration), Status::Success);
    ASSERT_FALSE(duration.IsNull());
    EXPECT_EQ(duration.Value(), 60);

    EXPECT_EQ(tester.WriteAttribute(TemperatureSetpointHoldDuration::Id, DataModel::MakeNullable<uint16_t>(120)), Status::Success);
    EXPECT_EQ(mHoldDelegate.mDuration.Value(), 120);

    // Duration exceeding max (1440 min) -> InvalidValue
    EXPECT_EQ(tester.WriteAttribute(TemperatureSetpointHoldDuration::Id, DataModel::MakeNullable<uint16_t>(1500)),
              Status::InvalidValue);

    // Read & Write SetpointHoldExpiryTimestamp
    DataModel::Nullable<uint32_t> timestamp;
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
    BitMask<OccupancyBitmap> occ;
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
    EXPECT_TRUE(result.status.has_value());
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::InvalidInState);

    // 2. Set synced mock clock
    EXPECT_EQ(mockClock.SetClock_RealTime(Microseconds64(kValidRealTimeMicroseconds)), CHIP_NO_ERROR);

    // 3. Expiration out of bounds (< 30 or > 1440) -> ConstraintError
    addCmd.expirationInMinutes = 10;
    result                     = tester.Invoke(addCmd);
    EXPECT_TRUE(result.status.has_value());
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::ConstraintError);

    addCmd.expirationInMinutes = 2000;
    result                     = tester.Invoke(addCmd);
    EXPECT_TRUE(result.status.has_value());
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::ConstraintError);

    // 4. Non-existent preset handle -> NotFound
    uint8_t unknownHandle[4]   = { 9, 9, 9, 9 };
    addCmd.presetHandle        = ByteSpan(unknownHandle);
    addCmd.expirationInMinutes = 60;
    result                     = tester.Invoke(addCmd);
    EXPECT_TRUE(result.status.has_value());
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::NotFound);

    // 5. Valid suggestion add -> Success
    addCmd.presetHandle = ByteSpan(handle);
    result              = tester.Invoke(addCmd);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    ASSERT_TRUE(result.response.has_value());
    uint8_t uniqueID = result.response->uniqueID;
    EXPECT_GT(uniqueID, 0);

    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 1u);
    EXPECT_TRUE(mSuggestionsDelegate.mReEvaluateCalled);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::ThermostatSuggestions::Id));

    // 6. Remove suggestion with matching uniqueID -> Success
    Commands::RemoveThermostatSuggestion::Type removeCmd;
    removeCmd.uniqueID = uniqueID;
    auto removeResult  = tester.Invoke(removeCmd);
    EXPECT_TRUE(removeResult.status.has_value());
    EXPECT_EQ(removeResult.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(mSuggestionsDelegate.mSuggestions.size(), 0u);

    // 7. Remove non-existent suggestion -> NotFound
    removeCmd.uniqueID = 0xFE;
    removeResult       = tester.Invoke(removeCmd);
    EXPECT_TRUE(removeResult.status.has_value());
    ASSERT_TRUE(removeResult.GetStatusCode().has_value());
    EXPECT_EQ(removeResult.GetStatusCode()->GetStatus(), Status::NotFound);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
