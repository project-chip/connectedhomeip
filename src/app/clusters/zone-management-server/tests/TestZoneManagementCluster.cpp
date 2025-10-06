/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

/**
 *    @file
 *      This file implements a unit test suite for testing different zone
 *      polygons for self-intersection.
 *
 */

#include <pw_unit_test/framework.h>
#include <vector>

#include <app/AttributePathParams.h>
#include <app/AttributeValueDecoder.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

#include <clusters/ZoneManagement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/clusters/zone-management-server/zone-geometry.h>

#include <lib/support/CodeUtils.h>

// =================================
//      Unit tests
// =================================

namespace chip {
namespace app {

using namespace chip;
using namespace chip::app::Testing;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;

using chip::app::AttributeValueDecoder;
using chip::app::DataModel::AttributeEntry;
using chip::app::Testing::WriteOperation;

class MockZoneManagementDelegate : public chip::app::Clusters::ZoneManagement::Delegate
{
    chip::Protocols::InteractionModel::Status
    CreateTwoDCartesianZone(uint16_t zoneID, const chip::app::Clusters::ZoneManagement::TwoDCartesianZoneStorage & zone) override
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }
    chip::Protocols::InteractionModel::Status
    UpdateTwoDCartesianZone(uint16_t zoneID, const chip::app::Clusters::ZoneManagement::TwoDCartesianZoneStorage & zone) override
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }
    chip::Protocols::InteractionModel::Status RemoveZone(uint16_t zoneID) override
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }
    chip::Protocols::InteractionModel::Status
    CreateTrigger(const chip::app::Clusters::ZoneManagement::ZoneTriggerControlStruct & zoneTrigger) override
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }
    chip::Protocols::InteractionModel::Status
    UpdateTrigger(const chip::app::Clusters::ZoneManagement::ZoneTriggerControlStruct & zoneTrigger) override
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }
    chip::Protocols::InteractionModel::Status RemoveTrigger(uint16_t zoneID) override
    {
        return chip::Protocols::InteractionModel::Status::Success;
    }
    void OnAttributeChanged(chip::AttributeId attributeId) override {}
    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }
};

struct TestZoneManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestZoneManagementCluster, TestAttributes)
{

    MockZoneManagementDelegate delegate;

    BitFlags<Feature, uint32_t> testFeatures1(Feature::kTwoDimensionalCartesianZone, Feature::kPerZoneSensitivity,
                                              Feature::kUserDefined, Feature::kFocusZones);
    uint8_t testUserZones                      = 5;
    uint8_t testMaxZones                       = 5;
    uint8_t testSensitivityMax                 = 10;
    uint8_t testSensitivity                    = 8;
    TwoDCartesianVertexStruct twoDCartesianMax = { .x = 100, .y = 100 };

    ZoneManagementCluster cluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                  testSensitivity, twoDCartesianMax);
    chip::Test::TestServerClusterContext context;
    cluster.Init();
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<AttributeEntry> builder;
    ASSERT_EQ(cluster.Attributes({ kRootEndpointId, ZoneManagement::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

    ASSERT_EQ(expectedBuilder.AppendElements({
                  Attributes::MaxUserDefinedZones::kMetadataEntry,
                  Attributes::MaxZones::kMetadataEntry,
                  Attributes::Zones::kMetadataEntry,
                  Attributes::Triggers::kMetadataEntry,
                  Attributes::SensitivityMax::kMetadataEntry,
                  // testFeatures1 defines PerZoneSensitivity, so there should be no global Sensitivity attribute
                  Attributes::TwoDCartesianMax::kMetadataEntry,
              }),
              CHIP_NO_ERROR);
    ASSERT_TRUE(::chip::Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

TEST_F(TestZoneManagementCluster, TestGeneratedCommands)
{

    MockZoneManagementDelegate delegate;

    BitFlags<Feature, uint32_t> testFeatures1(Feature::kTwoDimensionalCartesianZone, Feature::kPerZoneSensitivity,
                                              Feature::kUserDefined, Feature::kFocusZones);
    uint8_t testUserZones                      = 5;
    uint8_t testMaxZones                       = 5;
    uint8_t testSensitivityMax                 = 10;
    uint8_t testSensitivity                    = 8;
    TwoDCartesianVertexStruct twoDCartesianMax = { .x = 100, .y = 100 };

    ZoneManagementCluster cluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                  testSensitivity, twoDCartesianMax);
    chip::Test::TestServerClusterContext context;
    cluster.Init();
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<CommandId> builder;
    ASSERT_EQ(cluster.GeneratedCommands({ kRootEndpointId, ZoneManagement::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<CommandId> expectedBuilder;

    ASSERT_EQ(expectedBuilder.AppendElements({
                  Commands::CreateTwoDCartesianZoneResponse::Id,
              }),
              CHIP_NO_ERROR);
    // TODO: expand chip::Testing macros to allow for comparisons of not only attribute sets but also of command sets
    // ASSERT_TRUE(::chip::Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

TEST_F(TestZoneManagementCluster, TestAcceptedCommands)
{

    MockZoneManagementDelegate delegate;

    BitFlags<Feature, uint32_t> testFeatures1(Feature::kTwoDimensionalCartesianZone, Feature::kPerZoneSensitivity,
                                              Feature::kUserDefined, Feature::kFocusZones);
    uint8_t testUserZones                      = 5;
    uint8_t testMaxZones                       = 5;
    uint8_t testSensitivityMax                 = 10;
    uint8_t testSensitivity                    = 8;
    TwoDCartesianVertexStruct twoDCartesianMax = { .x = 100, .y = 100 };

    ZoneManagementCluster cluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                  testSensitivity, twoDCartesianMax);
    chip::Test::TestServerClusterContext context;
    cluster.Init();
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    ASSERT_EQ(cluster.AcceptedCommands({ kRootEndpointId, ZoneManagement::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedBuilder;

    ASSERT_EQ(expectedBuilder.AppendElements({
                  Commands::CreateTwoDCartesianZone::kMetadataEntry,
                  Commands::UpdateTwoDCartesianZone::kMetadataEntry,
                  Commands::RemoveZone::kMetadataEntry,
                  Commands::CreateOrUpdateTrigger::kMetadataEntry,
                  Commands::RemoveTrigger::kMetadataEntry,
              }),
              CHIP_NO_ERROR);
    ASSERT_TRUE(::chip::Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

TEST_F(TestZoneManagementCluster, TestZonePersistence)
{
    MockZoneManagementDelegate delegate;

    BitFlags<Feature, uint32_t> testFeatures1(Feature::kTwoDimensionalCartesianZone, Feature::kPerZoneSensitivity,
                                              Feature::kUserDefined, Feature::kFocusZones);
    uint8_t testUserZones                      = 5;
    uint8_t testMaxZones                       = 5;
    uint8_t testSensitivityMax                 = 10;
    uint8_t testSensitivity                    = 8;
    TwoDCartesianVertexStruct twoDCartesianMax = { .x = 100, .y = 100 };

    ZoneManagementCluster cluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                  testSensitivity, twoDCartesianMax);
    chip::Test::TestServerClusterContext context;
    cluster.Init();
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    // Add a zone
    {
        ZoneInformationStorage zone;
        TwoDCartesianZoneStorage twoDCartZone;
        std::vector<TwoDCartesianVertexStruct> vertices;
        vertices.push_back({ .x = 1, .y = 1 });
        vertices.push_back({ .x = 2, .y = 2 });
        vertices.push_back({ .x = 3, .y = 3 });
        twoDCartZone.Set("TestZone"_span, ZoneUseEnum::kMotion, vertices, NullOptional);
        zone.Set(1, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZone));
        EXPECT_EQ(cluster.AddZone(zone), CHIP_NO_ERROR);
    }

    // Verify the zone is there
    EXPECT_EQ(cluster.GetZones().size(), 1u);

    // Create a new server instance and check if it loads the persisted zone
    ZoneManagementCluster newCluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                     testSensitivity, twoDCartesianMax);
    newCluster.Init();
    ASSERT_EQ(newCluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(newCluster.GetZones().size(), 1u);
    EXPECT_EQ(newCluster.GetZones()[0].zoneID, 1u);
    EXPECT_TRUE(newCluster.GetZones()[0].twoDCartZoneStorage.HasValue());
    EXPECT_TRUE(newCluster.GetZones()[0].twoDCartZoneStorage.Value().name.data_equal("TestZone"_span));
}

TEST_F(TestZoneManagementCluster, TestTriggerPersistence)
{
    MockZoneManagementDelegate delegate;

    BitFlags<Feature, uint32_t> testFeatures1(Feature::kTwoDimensionalCartesianZone, Feature::kPerZoneSensitivity,
                                              Feature::kUserDefined, Feature::kFocusZones);
    uint8_t testUserZones                      = 5;
    uint8_t testMaxZones                       = 5;
    uint8_t testSensitivityMax                 = 10;
    uint8_t testSensitivity                    = 8;
    TwoDCartesianVertexStruct twoDCartesianMax = { .x = 100, .y = 100 };

    ZoneManagementCluster cluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                  testSensitivity, twoDCartesianMax);
    chip::Test::TestServerClusterContext context;
    cluster.Init();
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    // Add a zone
    {
        ZoneInformationStorage zone;
        TwoDCartesianZoneStorage twoDCartZone;
        std::vector<TwoDCartesianVertexStruct> vertices;
        vertices.push_back({ .x = 1, .y = 1 });
        vertices.push_back({ .x = 2, .y = 2 });
        vertices.push_back({ .x = 3, .y = 3 });
        twoDCartZone.Set("TestZone"_span, ZoneUseEnum::kMotion, vertices, NullOptional);
        zone.Set(1, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZone));
        EXPECT_EQ(cluster.AddZone(zone), CHIP_NO_ERROR);
    }

    // Add a trigger
    {
        ZoneTriggerControlStruct trigger;
        trigger.zoneID = 1;
        EXPECT_EQ(cluster.AddOrUpdateTrigger(trigger), Protocols::InteractionModel::Status::Success);
    }

    // Verify the trigger is there
    EXPECT_EQ(cluster.GetTriggers().size(), 1u);

    // Create a new server instance and check if it loads the persisted trigger
    ZoneManagementCluster newCluster(kRootEndpointId, delegate, testFeatures1, testUserZones, testMaxZones, testSensitivityMax,
                                     testSensitivity, twoDCartesianMax);
    newCluster.Init();
    ASSERT_EQ(newCluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(newCluster.GetTriggers().size(), 1u);
    EXPECT_EQ(newCluster.GetTriggers()[0].zoneID, 1u);
}

} // namespace app
} // namespace chip
