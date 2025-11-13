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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/ClusterTester.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ElectricalEnergyMeasurement/Attributes.h>
#include <clusters/ElectricalEnergyMeasurement/Events.h>
#include <clusters/ElectricalEnergyMeasurement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::Test;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestElectricalEnergyMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

TEST_F(TestElectricalEnergyMeasurementCluster, AttributeListTest)
{
    chip::Test::TestServerClusterContext context;

    // Test 1: No features activated - should only have mandatory Accuracy attribute
    {
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
            kTestEndpointId, noFeatures, static_cast<ElectricalEnergyMeasurement::OptionalAttributes>(0)));

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

    // Test 2: All features activated - optional attributes are automatically determined by features
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);

        // Optional attributes are automatically set based on feature flags in Config constructor
        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
            kTestEndpointId, allFeatures,
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

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
        bool foundCumulativeReset    = false;

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
            else if (attr.attributeId == Attributes::CumulativeEnergyReset::Id)
                foundCumulativeReset = true;
        }

        EXPECT_TRUE(foundAccuracy);
        EXPECT_TRUE(foundCumulativeImported);
        EXPECT_TRUE(foundCumulativeExported);
        EXPECT_TRUE(foundPeriodicImported);
        EXPECT_TRUE(foundPeriodicExported);
        EXPECT_TRUE(foundCumulativeReset);

        cluster.Shutdown();
    }

    // Test 3: CumulativeEnergyReset requires kCumulativeEnergy feature
    {
        // Even if we request the optional CumulativeEnergyReset attribute,
        // it should not be enabled without kCumulativeEnergy feature
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
            kTestEndpointId, noFeatures, ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, ElectricalEnergyMeasurement::Id), attributes),
                  CHIP_NO_ERROR);

        // Verify CumulativeEnergyReset is NOT present because kCumulativeEnergy feature is not enabled
        auto attributeBuffer      = attributes.TakeBuffer();
        bool foundCumulativeReset = false;
        for (const auto & attr : attributeBuffer)
        {
            if (attr.attributeId == Attributes::CumulativeEnergyReset::Id)
            {
                foundCumulativeReset = true;
                break;
            }
        }
        EXPECT_FALSE(foundCumulativeReset);

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

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
            kTestEndpointId, allFeatures,
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

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

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
            kTestEndpointId, noFeatures, static_cast<ElectricalEnergyMeasurement::OptionalAttributes>(0)));

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

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
            kTestEndpointId, allFeatures,
            ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        Span<const ConcreteClusterPath> paths = cluster.GetPaths();
        ASSERT_EQ(paths.size(), 1u);

        // Test CumulativeEnergyImported (requires kCumulativeEnergy + kImportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyImported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_TRUE(result.IsSuccess());
        }

        // Test CumulativeEnergyExported (requires kCumulativeEnergy + kExportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyExported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_TRUE(result.IsSuccess());
        }

        // Test PeriodicEnergyImported (requires kPeriodicEnergy + kImportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, PeriodicEnergyImported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_TRUE(result.IsSuccess());
        }

        // Test PeriodicEnergyExported (requires kPeriodicEnergy + kExportedEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, PeriodicEnergyExported::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_TRUE(result.IsSuccess());
        }

        // Test CumulativeEnergyReset (requires kCumulativeEnergy)
        {
            const ConcreteDataAttributePath path = { paths[0].mEndpointId, paths[0].mClusterId, CumulativeEnergyReset::Id };
            chip::app::Testing::ReadOperation readOperation(path);
            std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();
            auto result                                    = cluster.ReadAttribute(readOperation.GetRequest(), *encoder);
            EXPECT_TRUE(result.IsSuccess());
        }

        cluster.Shutdown();
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, ReadAttributeWithClusterTesterTest)
{
    chip::Test::TestServerClusterContext context;

    // Create a cluster with all features enabled
    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
        kTestEndpointId, allFeatures, ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    chip::Test::ClusterTester tester(cluster);

    Structs::MeasurementAccuracyStruct::DecodableType accuracy;
    ASSERT_EQ(tester.ReadAttribute(Accuracy::Id, accuracy), CHIP_NO_ERROR);

    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyData;
    energyData.energy = 5000;
    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalEnergyData(energyData);

    EXPECT_EQ(cluster.SetCumulativeEnergyImported(optionalEnergyData), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetCumulativeEnergyExported(optionalEnergyData), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetPeriodicEnergyImported(optionalEnergyData), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetPeriodicEnergyExported(optionalEnergyData), CHIP_NO_ERROR);

    DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> cumulativeImported;
    ASSERT_EQ(tester.ReadAttribute(CumulativeEnergyImported::Id, cumulativeImported), CHIP_NO_ERROR);
    ASSERT_FALSE(cumulativeImported.IsNull());
    EXPECT_EQ(cumulativeImported.Value().energy, 5000);

    DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> cumulativeExported;
    ASSERT_EQ(tester.ReadAttribute(CumulativeEnergyExported::Id, cumulativeExported), CHIP_NO_ERROR);
    ASSERT_FALSE(cumulativeExported.IsNull());
    EXPECT_EQ(cumulativeExported.Value().energy, 5000);

    DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> periodicImported;
    ASSERT_EQ(tester.ReadAttribute(PeriodicEnergyImported::Id, periodicImported), CHIP_NO_ERROR);
    ASSERT_FALSE(periodicImported.IsNull());
    EXPECT_EQ(periodicImported.Value().energy, 5000);

    DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> periodicExported;
    ASSERT_EQ(tester.ReadAttribute(PeriodicEnergyExported::Id, periodicExported), CHIP_NO_ERROR);
    ASSERT_FALSE(periodicExported.IsNull());
    EXPECT_EQ(periodicExported.Value().energy, 5000);

    DataModel::Nullable<Structs::CumulativeEnergyResetStruct::DecodableType> resetValue;
    ASSERT_EQ(tester.ReadAttribute(CumulativeEnergyReset::Id, resetValue), CHIP_NO_ERROR);

    cluster.Shutdown();
}

TEST_F(TestElectricalEnergyMeasurementCluster, EventGeneratedOnSnapshots)
{
    chip::Test::TestServerClusterContext testContext;

    // Create a cluster with all features enabled
    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config(
        kTestEndpointId, allFeatures, ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

    EXPECT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    auto & logOnlyEvents = testContext.EventsGenerator();

    // Prepare energy measurement data
    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyImported;
    energyImported.energy = 10000; // 10 Wh

    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyExported;
    energyExported.energy = 5000; // 5 Wh

    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalImported(energyImported);
    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalExported(energyExported);

    // Test cumulative energy snapshot event generation
    {
        EventNumber initialCount = logOnlyEvents.CurrentEventNumber();
        auto eventNumber         = cluster.CumulativeEnergySnapshot(optionalImported, optionalExported);

        ASSERT_TRUE(eventNumber.has_value());
        EXPECT_EQ(eventNumber.value(), logOnlyEvents.CurrentEventNumber());
        EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), initialCount + 1);

        // Verify event path
        using CumulativeEventType = chip::app::Clusters::ElectricalEnergyMeasurement::Events::CumulativeEnergyMeasured::Type;
        EXPECT_EQ(logOnlyEvents.LastOptions().mPath,
                  ConcreteEventPath(kTestEndpointId, CumulativeEventType::GetClusterId(), CumulativeEventType::GetEventId()));

        // Decode and verify event payload
        chip::app::Clusters::ElectricalEnergyMeasurement::Events::CumulativeEnergyMeasured::DecodableType decodedEvent;
        ASSERT_EQ(logOnlyEvents.DecodeLastEvent(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 10000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 5000);
    }

    // Test periodic energy snapshot event generation
    {
        EventNumber initialCount = logOnlyEvents.CurrentEventNumber();

        auto eventNumber = cluster.PeriodicEnergySnapshot(optionalImported, optionalExported);

        ASSERT_TRUE(eventNumber.has_value());
        EXPECT_EQ(eventNumber.value(), logOnlyEvents.CurrentEventNumber());
        EXPECT_EQ(logOnlyEvents.CurrentEventNumber(), initialCount + 1);

        // Verify event path
        using PeriodicEventType = chip::app::Clusters::ElectricalEnergyMeasurement::Events::PeriodicEnergyMeasured::Type;
        EXPECT_EQ(logOnlyEvents.LastOptions().mPath,
                  ConcreteEventPath(kTestEndpointId, PeriodicEventType::GetClusterId(), PeriodicEventType::GetEventId()));

        // Decode and verify event payload
        chip::app::Clusters::ElectricalEnergyMeasurement::Events::PeriodicEnergyMeasured::DecodableType decodedEvent;
        ASSERT_EQ(logOnlyEvents.DecodeLastEvent(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 10000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 5000);
    }

    cluster.Shutdown();
}

} // namespace
