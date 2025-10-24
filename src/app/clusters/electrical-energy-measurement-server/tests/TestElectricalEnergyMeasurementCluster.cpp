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

#include "ClusterActions.h"
#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <pw_unit_test/framework.h>

#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ElectricalEnergyMeasurement/Attributes.h>
#include <clusters/ElectricalEnergyMeasurement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestElectricalEnergyMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        // Reset any global state
    }
};

TEST_F(TestElectricalEnergyMeasurementCluster, AttributeListTest)
{
    chip::Test::TestServerClusterContext context;

    // Test 1: No features activated - should only have mandatory Accuracy attribute
    {
        BitMask<Feature> noFeatures;
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;

        ElectricalEnergyMeasurementCluster cluster(
            ElectricalEnergyMeasurementCluster::Config(kTestEndpointId, noFeatures, optionalAttributes));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, ElectricalEnergyMeasurement::Id), attributes),
                  CHIP_NO_ERROR);

        // Verify Accuracy attribute is present
        auto attributeBuffer = attributes.TakeBuffer();
        bool foundAccuracy   = false;
        for (const auto & attr : attributeBuffer)
        {
            if (attr.attributeId == Attributes::Accuracy::Id)
            {
                foundAccuracy = true;
                break;
            }
        }
        EXPECT_TRUE(foundAccuracy);

        cluster.Shutdown();
    }

    // Test 2: All features activated - should have all feature-dependent attributes
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;

        // Set up optional attributes based on enabled features
        optionalAttributes.Set<Attributes::CumulativeEnergyImported::Id>();
        optionalAttributes.Set<Attributes::CumulativeEnergyExported::Id>();
        optionalAttributes.Set<Attributes::PeriodicEnergyImported::Id>();
        optionalAttributes.Set<Attributes::PeriodicEnergyExported::Id>();
        optionalAttributes.Set<Attributes::CumulativeEnergyReset::Id>();

        ElectricalEnergyMeasurementCluster cluster(
            ElectricalEnergyMeasurementCluster::Config(kTestEndpointId, allFeatures, optionalAttributes));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, ElectricalEnergyMeasurement::Id), attributes),
                  CHIP_NO_ERROR);

        // Verify all expected attributes are present
        auto attributeBuffer         = attributes.TakeBuffer();
        bool foundAccuracy           = false;
        bool foundCumulativeImported = false;
        bool foundCumulativeExported = false;
        bool foundPeriodicImported   = false;
        bool foundPeriodicExported   = false;

        for (const auto & attr : attributeBuffer)
        {
            if (attr.attributeId == Attributes::Accuracy::Id)
                foundAccuracy = true;
            else if (attr.attributeId == Attributes::CumulativeEnergyImported::Id)
                foundCumulativeImported = true;
            else if (attr.attributeId == Attributes::CumulativeEnergyExported::Id)
                foundCumulativeExported = true;
            else if (attr.attributeId == Attributes::PeriodicEnergyImported::Id)
                foundPeriodicImported = true;
            else if (attr.attributeId == Attributes::PeriodicEnergyExported::Id)
                foundPeriodicExported = true;
        }

        EXPECT_TRUE(foundAccuracy);
        EXPECT_TRUE(foundCumulativeImported);
        EXPECT_TRUE(foundCumulativeExported);
        EXPECT_TRUE(foundPeriodicImported);
        EXPECT_TRUE(foundPeriodicExported);

        cluster.Shutdown();
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, GettersSettersWithFeatureValidationTest)
{
    chip::Test::TestServerClusterContext context;

    // Test 1: Cluster with all features enabled - setters and getters should work
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;

        ElectricalEnergyMeasurementCluster cluster(
            ElectricalEnergyMeasurementCluster::Config(kTestEndpointId, allFeatures, optionalAttributes));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        // Set values via setters
        ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyData;
        energyData.energy = 1000;

        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalEnergyData(energyData);

        EXPECT_EQ(cluster.SetCumulativeEnergyImported(optionalEnergyData), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetCumulativeEnergyExported(optionalEnergyData), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetPeriodicEnergyImported(optionalEnergyData), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.SetPeriodicEnergyExported(optionalEnergyData), CHIP_NO_ERROR);

        ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct resetData;
        Optional<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> optionalResetData(resetData);
        EXPECT_EQ(cluster.SetCumulativeEnergyReset(optionalResetData), CHIP_NO_ERROR);

        // Read back via getters and verify values
        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;

        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 1000);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 1000);

        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());

        Optional<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> readResetValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyReset(readResetValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readResetValue.HasValue());

        cluster.Shutdown();
    }

    // Test 2: Cluster with no features - setters and getters should fail
    {
        BitMask<Feature> noFeatures;
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;

        ElectricalEnergyMeasurementCluster cluster(
            ElectricalEnergyMeasurementCluster::Config(kTestEndpointId, noFeatures, optionalAttributes));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        // Try to set values - should fail with feature error
        ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyData;
        energyData.energy = 1000;
        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalEnergyData(energyData);

        EXPECT_EQ(cluster.SetCumulativeEnergyImported(optionalEnergyData), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.SetCumulativeEnergyExported(optionalEnergyData), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.SetPeriodicEnergyImported(optionalEnergyData), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.SetPeriodicEnergyExported(optionalEnergyData), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct resetData;
        Optional<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> optionalResetData(resetData);
        EXPECT_EQ(cluster.SetCumulativeEnergyReset(optionalResetData), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        // Try to get values - should also fail with feature error
        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        Optional<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> readResetValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyReset(readResetValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        cluster.Shutdown();
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, FeatureAttributeTest)
{
    chip::Test::TestServerClusterContext context;

    // Test 1: Cluster with all features - all optional attributes should be readable
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;

        // Enable all optional attributes
        optionalAttributes.Set<Attributes::CumulativeEnergyImported::Id>();
        optionalAttributes.Set<Attributes::CumulativeEnergyExported::Id>();
        optionalAttributes.Set<Attributes::PeriodicEnergyImported::Id>();
        optionalAttributes.Set<Attributes::PeriodicEnergyExported::Id>();
        optionalAttributes.Set<Attributes::CumulativeEnergyReset::Id>();

        ElectricalEnergyMeasurementCluster cluster(
            ElectricalEnergyMeasurementCluster::Config(kTestEndpointId, allFeatures, optionalAttributes));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        Span<const ConcreteClusterPath> paths = cluster.GetPaths();
        ASSERT_EQ(paths.size(), 1u);

        // Test CumulativeEnergyImported (requires kCumulativeEnergy + kImportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyImported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::Success);
        }

        // Test CumulativeEnergyExported (requires kCumulativeEnergy + kExportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyExported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::Success);
        }

        // Test PeriodicEnergyImported (requires kPeriodicEnergy + kImportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, PeriodicEnergyImported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::Success);
        }

        // Test PeriodicEnergyExported (requires kPeriodicEnergy + kExportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, PeriodicEnergyExported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::Success);
        }

        // Test CumulativeEnergyReset (requires kCumulativeEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyReset::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::Success);
        }

        cluster.Shutdown();
    }

    // Test 2: Cluster with no features - all optional attributes should be unsupported
    {
        BitMask<Feature> noFeatures;
        ElectricalEnergyMeasurementCluster::OptionalAttributesSet optionalAttributes;
        // Do not enable any optional attributes since features are not present

        ElectricalEnergyMeasurementCluster cluster(
            ElectricalEnergyMeasurementCluster::Config(kTestEndpointId, noFeatures, optionalAttributes));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        Span<const ConcreteClusterPath> paths = cluster.GetPaths();
        ASSERT_EQ(paths.size(), 1u);

        // Test CumulativeEnergyImported - should be unsupported
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyImported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::UnsupportedAttribute);
        }

        // Test CumulativeEnergyExported - should be unsupported
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyExported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::UnsupportedAttribute);
        }

        // Test PeriodicEnergyImported - should be unsupported
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, PeriodicEnergyImported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::UnsupportedAttribute);
        }

        // Test PeriodicEnergyExported - should be unsupported
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, PeriodicEnergyExported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::UnsupportedAttribute);
        }

        // Test CumulativeEnergyReset - should be unsupported
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyReset::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_EQ(result, Protocols::InteractionModel::Status::UnsupportedAttribute);
        }

        cluster.Shutdown();
    }
}

} // namespace
