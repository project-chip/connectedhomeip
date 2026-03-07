/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ZoneManagement/Metadata.h>
#include <lib/support/CHIPMem.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;
using chip::Testing::EqualAcceptedCommandSets;
using chip::Testing::EqualGeneratedCommandSets;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId   = 1;
constexpr uint8_t kMaxUserDefinedZones = 5;
constexpr uint8_t kMaxZones            = 8;
constexpr uint8_t kSensitivityMax      = 4;
const TwoDCartesianVertexStruct kTwoDMaxPoint{ 640, 480 };

class MockDelegate : public Delegate
{
public:
    Protocols::InteractionModel::Status CreateTwoDCartesianZone(const TwoDCartesianZoneStorage &, uint16_t &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status UpdateTwoDCartesianZone(uint16_t, const TwoDCartesianZoneStorage &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status RemoveZone(uint16_t) override { return Protocols::InteractionModel::Status::Success; }

    Protocols::InteractionModel::Status CreateTrigger(const ZoneTriggerControlStruct &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status UpdateTrigger(const ZoneTriggerControlStruct &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status RemoveTrigger(uint16_t) override { return Protocols::InteractionModel::Status::Success; }

    void OnAttributeChanged(AttributeId) override {}

    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }

    CHIP_ERROR LoadZones(std::vector<ZoneInformationStorage> &) override { return CHIP_NO_ERROR; }

    CHIP_ERROR LoadTriggers(std::vector<ZoneTriggerControlStruct> &) override { return CHIP_NO_ERROR; }
};

class TestZoneManagementCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

protected:
    ZoneManagementCluster CreateCluster(BitFlags<Feature> features)
    {
        return ZoneManagementCluster(ZoneManagementCluster::Context{
            .delegate   = mDelegate,
            .endpointId = kTestEndpointId,
            .features   = features,
            .config     = {
                .maxUserDefinedZones = kMaxUserDefinedZones,
                .maxZones            = kMaxZones,
                .sensitivityMax      = kSensitivityMax,
                .twoDCartesianMax    = kTwoDMaxPoint,
            },
        });
    }

    MockDelegate mDelegate;
};

TEST_F(TestZoneManagementCluster, AcceptedCommandsWithoutPreallocationForAllRelevantFeatures)
{
    ZoneManagementCluster cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));
    ConcreteClusterPath path(kTestEndpointId, ZoneManagement::Id);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    ASSERT_EQ(cluster.AcceptedCommands(path, builder), CHIP_NO_ERROR);

    static constexpr DataModel::AcceptedCommandEntry kExpected[] = {
        Commands::CreateTwoDCartesianZone::kMetadataEntry,
        Commands::UpdateTwoDCartesianZone::kMetadataEntry,
        Commands::RemoveZone::kMetadataEntry,
        Commands::CreateOrUpdateTrigger::kMetadataEntry,
        Commands::RemoveTrigger::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(kExpected), CHIP_NO_ERROR);
    ASSERT_TRUE(EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster, { kExpected[0], kExpected[1], kExpected[2], kExpected[3], kExpected[4] }));
}

TEST_F(TestZoneManagementCluster, AcceptedCommandsFollowFeatureGates)
{
    ZoneManagementCluster userDefinedOnly = CreateCluster(BitFlags<Feature>(Feature::kUserDefined));
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(userDefinedOnly,
                                              {
                                                  Commands::RemoveZone::kMetadataEntry,
                                                  Commands::CreateOrUpdateTrigger::kMetadataEntry,
                                                  Commands::RemoveTrigger::kMetadataEntry,
                                              }));

    ZoneManagementCluster noOptionalFeatures = CreateCluster(BitFlags<Feature>());
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(noOptionalFeatures,
                                              {
                                                  Commands::CreateOrUpdateTrigger::kMetadataEntry,
                                                  Commands::RemoveTrigger::kMetadataEntry,
                                              }));
}

TEST_F(TestZoneManagementCluster, GeneratedCommandsWithoutPreallocationForCreateResponse)
{
    ZoneManagementCluster cluster = CreateCluster(BitFlags<Feature>(Feature::kUserDefined, Feature::kTwoDimensionalCartesianZone));
    ConcreteClusterPath path(kTestEndpointId, ZoneManagement::Id);

    ReadOnlyBufferBuilder<CommandId> builder;
    ASSERT_EQ(cluster.GeneratedCommands(path, builder), CHIP_NO_ERROR);

    static constexpr CommandId kExpected[] = {
        Commands::CreateTwoDCartesianZoneResponse::Id,
    };

    ReadOnlyBufferBuilder<CommandId> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(kExpected), CHIP_NO_ERROR);
    ASSERT_TRUE(EqualGeneratedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(cluster, { kExpected[0] }));
}

TEST_F(TestZoneManagementCluster, GeneratedCommandsEmptyWhenCreateIsNotSupported)
{
    ZoneManagementCluster userDefinedOnly = CreateCluster(BitFlags<Feature>(Feature::kUserDefined));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(userDefinedOnly, {}));

    ZoneManagementCluster twoDOnly = CreateCluster(BitFlags<Feature>(Feature::kTwoDimensionalCartesianZone));
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(twoDOnly, {}));
}

} // namespace
