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
#include <app/clusters/thermostat-server/PresetStructWithOwnedMembers.h>
#include <clusters/Thermostat/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;
using chip::app::Clusters::Globals::AtomicRequestTypeEnum;

namespace {

TEST(TestPresetStructWithOwnedMembers, TestBasicOperations)
{
    PresetStructWithOwnedMembers preset;
    preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    EXPECT_EQ(preset.GetPresetScenario(), PresetScenarioEnum::kOccupied);

    uint8_t handle[4] = { 1, 2, 3, 4 };
    ByteSpan handleSpan(handle);
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(handleSpan)), CHIP_NO_ERROR);
    EXPECT_FALSE(preset.GetPresetHandle().IsNull());
    EXPECT_TRUE(preset.GetPresetHandle().Value().data_equal(handleSpan));

    // Exceed max handle size
    uint8_t largeHandle[kPresetHandleSize + 1];
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(largeHandle))), CHIP_ERROR_NO_MEMORY);

    CharSpan nameSpan = "Home"_span;
    EXPECT_EQ(preset.SetName(MakeOptional(DataModel::MakeNullable(nameSpan))), CHIP_NO_ERROR);
    EXPECT_TRUE(preset.GetName().HasValue());
    EXPECT_FALSE(preset.GetName().Value().IsNull());
    EXPECT_TRUE(preset.GetName().Value().Value().data_equal(nameSpan));

    // Exceed max name size
    char largeName[kPresetNameSize + 1];
    memset(largeName, 'A', sizeof(largeName));
    EXPECT_EQ(preset.SetName(MakeOptional(DataModel::MakeNullable(CharSpan(largeName, sizeof(largeName))))),
              CHIP_ERROR_NO_MEMORY);

    // Setpoints
    preset.SetHeatingSetpoint(MakeOptional<int16_t>(static_cast<int16_t>(2000)));
    EXPECT_TRUE(preset.GetHeatingSetpoint().HasValue());
    EXPECT_EQ(preset.GetHeatingSetpoint().Value(), 2000);

    preset.SetCoolingSetpoint(MakeOptional<int16_t>(static_cast<int16_t>(2500)));
    EXPECT_TRUE(preset.GetCoolingSetpoint().HasValue());
    EXPECT_EQ(preset.GetCoolingSetpoint().Value(), 2500);

    preset.SetBuiltIn(DataModel::MakeNullable(true));
    EXPECT_TRUE(preset.GetBuiltIn().ValueOr(false));

    // Copy constructor
    PresetStructWithOwnedMembers copy(preset);
    EXPECT_EQ(copy.GetPresetScenario(), PresetScenarioEnum::kOccupied);
    EXPECT_TRUE(copy.GetPresetHandle().Value().data_equal(handleSpan));
    EXPECT_EQ(copy.GetHeatingSetpoint().Value(), 2000);

    // Move constructor
    PresetStructWithOwnedMembers moved(std::move(copy));
    EXPECT_EQ(moved.GetPresetScenario(), PresetScenarioEnum::kOccupied);
    EXPECT_TRUE(moved.GetPresetHandle().Value().data_equal(handleSpan));

    // Copy assignment
    PresetStructWithOwnedMembers assigned;
    assigned = moved;
    EXPECT_EQ(assigned.GetPresetScenario(), PresetScenarioEnum::kOccupied);
    EXPECT_TRUE(assigned.GetPresetHandle().Value().data_equal(handleSpan));

    // Move assignment
    PresetStructWithOwnedMembers moveAssigned;
    moveAssigned = std::move(assigned);
    EXPECT_EQ(moveAssigned.GetPresetScenario(), PresetScenarioEnum::kOccupied);
}

TEST_F(ThermostatTestFixture, TestPresetsAttributesReading)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets);

    // Populate preset types
    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario       = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets      = 2;
    ptype.presetTypeFeatures.Set(PresetTypeFeaturesBitmap::kSupportsNames);
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    // Populate presets
    PresetStructWithOwnedMembers preset;
    preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handle[4] = { 0xAA, 0xBB, 0xCC, 0xDD };
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
    preset.SetBuiltIn(DataModel::MakeNullable(true));
    preset.SetHeatingSetpoint(MakeOptional<int16_t>(static_cast<int16_t>(2000)));
    preset.SetCoolingSetpoint(MakeOptional<int16_t>(static_cast<int16_t>(2500)));
    mPresetsDelegate.mPresets.push_back(preset);

    // Active preset handle
    EXPECT_EQ(mPresetsDelegate.SetActivePresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read NumberOfPresets
    uint8_t numPresets = 0;
    EXPECT_EQ(tester.ReadAttribute(NumberOfPresets::Id, numPresets), Status::Success);
    EXPECT_EQ(numPresets, 10);

    // Read ActivePresetHandle
    DataModel::Nullable<ByteSpan> activeHandle;
    EXPECT_EQ(tester.ReadAttribute(ActivePresetHandle::Id, activeHandle), Status::Success);
    ASSERT_FALSE(activeHandle.IsNull());
    EXPECT_TRUE(activeHandle.Value().data_equal(ByteSpan(handle)));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestAtomicWriteSessionFullLifecycle)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario       = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets      = 5;
    ptype.presetTypeFeatures.Set(PresetTypeFeaturesBitmap::kSupportsNames);
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    PresetStructWithOwnedMembers preset;
    preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handle[4] = { 1, 2, 3, 4 };
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
    preset.SetBuiltIn(DataModel::MakeNullable(true));
    preset.SetHeatingSetpoint(MakeOptional<int16_t>(static_cast<int16_t>(2000)));
    preset.SetCoolingSetpoint(MakeOptional<int16_t>(static_cast<int16_t>(2600)));
    mPresetsDelegate.mPresets.push_back(preset);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // 1. Trying to write Presets outside an atomic write session -> InvalidInState
    std::vector<Structs::PresetStruct::Type> newPresetsList;
    auto listPayload = DataModel::List<const Structs::PresetStruct::Type>(newPresetsList.data(), newPresetsList.size());
    EXPECT_EQ(tester.WriteAttribute(Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::InvalidInState);

    // 2. BeginAtomicWrite
    Commands::AtomicRequest::Type beginReq;
    beginReq.requestType = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Presets::Id };
    beginReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);
    beginReq.timeout           = MakeOptional<uint16_t>(static_cast<uint16_t>(5000)); // 5 seconds

    auto result = tester.Invoke(beginReq);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->statusCode, to_underlying(Status::Success));

    // 3. During atomic write session: writing non-atomic attribute returns InvalidInState
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::InvalidInState);

    // 4. Writing presets list during atomic session succeeds
    Structs::PresetStruct::Type updateEntry;
    updateEntry.presetScenario  = PresetScenarioEnum::kOccupied;
    updateEntry.presetHandle    = DataModel::MakeNullable(ByteSpan(handle));
    updateEntry.builtIn         = DataModel::MakeNullable(true);
    updateEntry.heatingSetpoint = MakeOptional<int16_t>(static_cast<int16_t>(2100));
    updateEntry.coolingSetpoint = MakeOptional<int16_t>(static_cast<int16_t>(2500));
    newPresetsList.push_back(updateEntry);

    listPayload = DataModel::List<const Structs::PresetStruct::Type>(newPresetsList.data(), newPresetsList.size());
    EXPECT_EQ(tester.WriteAttribute(Presets::Id, listPayload, ListWritingPattern::ReplaceAll), Status::Success);

    // Pending presets should now reflect the update
    ASSERT_EQ(mPresetsDelegate.mPendingPresets.size(), 1u);
    EXPECT_EQ(mPresetsDelegate.mPendingPresets[0].GetHeatingSetpoint().Value(), 2100);

    // 5. CommitAtomicWrite
    Commands::AtomicRequest::Type commitReq;
    commitReq.requestType       = AtomicRequestTypeEnum::kCommitWrite;
    commitReq.attributeRequests = DataModel::List<const chip::AttributeId>(attrIds, 1);

    result = tester.Invoke(commitReq);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(result.response->statusCode, to_underlying(Status::Success));

    // Presets should be committed
    ASSERT_EQ(mPresetsDelegate.mPresets.size(), 1u);
    EXPECT_EQ(mPresetsDelegate.mPresets[0].GetHeatingSetpoint().Value(), 2100);
    EXPECT_TRUE(tester.IsAttributeDirty(Presets::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestAtomicWriteRollback)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario       = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets      = 5;
    ptype.presetTypeFeatures.Set(PresetTypeFeaturesBitmap::kSupportsNames);
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // BeginAtomicWrite
    Commands::AtomicRequest::Type req;
    req.requestType             = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Presets::Id };
    req.attributeRequests       = DataModel::List<const chip::AttributeId>(attrIds, 1);
    req.timeout                 = MakeOptional<uint16_t>(static_cast<uint16_t>(5000));

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.response->statusCode, to_underlying(Status::Success));

    // RollbackAtomicWrite
    req.requestType = AtomicRequestTypeEnum::kRollbackWrite;
    result          = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.response->statusCode, to_underlying(Status::Success));

    // Verify session closed by checking writing a non-atomic attribute succeeds again
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::Success);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestAtomicWriteTimerExpiration)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets);

    Structs::PresetTypeStruct::Type ptype;
    ptype.presetScenario  = PresetScenarioEnum::kOccupied;
    ptype.numberOfPresets = 5;
    mPresetsDelegate.mPresetTypes.push_back(ptype);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // BeginAtomicWrite with 2000 ms
    Commands::AtomicRequest::Type req;
    req.requestType             = AtomicRequestTypeEnum::kBeginWrite;
    chip::AttributeId attrIds[] = { Presets::Id };
    req.attributeRequests       = DataModel::List<const chip::AttributeId>(attrIds, 1);
    req.timeout                 = MakeOptional<uint16_t>(static_cast<uint16_t>(2000));

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.response->statusCode, to_underlying(Status::Success));

    // While timer is active, writing non-atomic attribute is blocked
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::InvalidInState);

    // Trigger timer expiration via mock timer delegate
    mMockTimerDelegate.AdvanceClock(chip::System::Clock::Milliseconds64(3000));

    // Now session should have been reset by timeout handler -> non-atomic write allowed again
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::Success);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetActivePresetRequestCommand)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets, Feature::kEvents);

    PresetStructWithOwnedMembers preset;
    preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handle[4] = { 'P', 'R', 'S', 'T' };
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
    mPresetsDelegate.mPresets.push_back(preset);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Invoke SetActivePresetRequest with valid handle
    Commands::SetActivePresetRequest::Type cmd;
    cmd.presetHandle = DataModel::MakeNullable(ByteSpan(handle));

    auto result = tester.Invoke(cmd);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.IsAttributeDirty(ActivePresetHandle::Id));

    // Verify ActivePresetChange event
    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath.mEventId, Events::ActivePresetChange::Id);

    // Invoke with non-existent handle -> InvalidCommand
    uint8_t unknownHandle[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    cmd.presetHandle         = DataModel::MakeNullable(ByteSpan(unknownHandle));
    result                   = tester.Invoke(cmd);
    EXPECT_TRUE(result.status.has_value());
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::InvalidCommand);

    // Invoke with null handle -> clears active preset
    cmd.presetHandle = DataModel::NullNullable;
    result           = tester.Invoke(cmd);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_TRUE(mPresetsDelegate.mActivePresetHandle.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestManualSetpointChangeClearsActivePreset)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kPresets);

    PresetStructWithOwnedMembers preset;
    preset.SetPresetScenario(PresetScenarioEnum::kOccupied);
    uint8_t handle[4] = { 1, 2, 3, 4 };
    EXPECT_EQ(preset.SetPresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);
    mPresetsDelegate.mPresets.push_back(preset);
    EXPECT_EQ(mPresetsDelegate.SetActivePresetHandle(DataModel::MakeNullable(ByteSpan(handle))), CHIP_NO_ERROR);

    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mPresetsDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Active preset is set
    ASSERT_FALSE(mPresetsDelegate.mActivePresetHandle.IsNull());

    // When occupied, writing OccupiedHeatingSetpoint is an active setpoint change
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2100)), Status::Success);

    // Per Matter spec, manually updating the active setpoint clears the active preset handle to null
    EXPECT_TRUE(mPresetsDelegate.mActivePresetHandle.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
