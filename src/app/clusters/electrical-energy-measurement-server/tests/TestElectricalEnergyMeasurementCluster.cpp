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

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/ElectricalEnergyMeasurement/Attributes.h>
#include <clusters/ElectricalEnergyMeasurement/Events.h>
#include <clusters/ElectricalEnergyMeasurement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;
using namespace chip::Testing;

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
    TestServerClusterContext context;

    // Test 1: No features activated - should only have mandatory Accuracy attribute
    {
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = noFeatures,
            .optionalAttributes = static_cast<ElectricalEnergyMeasurement::OptionalAttributes>(0),
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster, { Attributes::Accuracy::kMetadataEntry }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 2: All features activated - optional attributes are automatically determined by features
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);

        // Optional attributes are automatically set based on feature flags in Config constructor
        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = allFeatures,
            .optionalAttributes = ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                Attributes::Accuracy::kMetadataEntry,
                                                Attributes::CumulativeEnergyImported::kMetadataEntry,
                                                Attributes::CumulativeEnergyExported::kMetadataEntry,
                                                Attributes::PeriodicEnergyImported::kMetadataEntry,
                                                Attributes::PeriodicEnergyExported::kMetadataEntry,
                                                Attributes::CumulativeEnergyReset::kMetadataEntry,
                                            }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 3: CumulativeEnergyReset requires kCumulativeEnergy feature
    {
        // Even if we request the optional CumulativeEnergyReset attribute,
        // it should not be enabled without kCumulativeEnergy feature
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = noFeatures,
            .optionalAttributes = ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        // Verify CumulativeEnergyReset is NOT present because kCumulativeEnergy feature is not enabled
        EXPECT_TRUE(IsAttributesListEqualTo(cluster, { Attributes::Accuracy::kMetadataEntry }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, GettersSettersWithFeatureValidationTest)
{
    TestServerClusterContext context;

    // Test 1: Cluster with all features enabled - snapshot methods and getters should work
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = allFeatures,
            .optionalAttributes = ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        // Set values via snapshot methods
        ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyData;
        energyData.energy = 1000;

        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalEnergyData(energyData);

        cluster.CumulativeEnergySnapshot(optionalEnergyData, optionalEnergyData);
        cluster.PeriodicEnergySnapshot(optionalEnergyData, optionalEnergyData);

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

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 2: Cluster with no features - getters should fail
    {
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = noFeatures,
            .optionalAttributes = static_cast<ElectricalEnergyMeasurement::OptionalAttributes>(0),
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        // Try to get values - should fail with feature error
        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        Optional<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> readResetValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyReset(readResetValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, FeatureAttributeTest)
{
    TestServerClusterContext context;

    // Test 1: Cluster with all features - all optional attributes should be readable
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = allFeatures,
            .optionalAttributes = ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        // Test CumulativeEnergyImported (requires kCumulativeEnergy + kImportedEnergy)
        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(CumulativeEnergyImported::Id, value).IsSuccess());
        }

        // Test CumulativeEnergyExported (requires kCumulativeEnergy + kExportedEnergy)
        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(CumulativeEnergyExported::Id, value).IsSuccess());
        }

        // Test PeriodicEnergyImported (requires kPeriodicEnergy + kImportedEnergy)
        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(PeriodicEnergyImported::Id, value).IsSuccess());
        }

        // Test PeriodicEnergyExported (requires kPeriodicEnergy + kExportedEnergy)
        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(PeriodicEnergyExported::Id, value).IsSuccess());
        }

        // Test CumulativeEnergyReset (requires kCumulativeEnergy)
        {
            DataModel::Nullable<Structs::CumulativeEnergyResetStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(CumulativeEnergyReset::Id, value).IsSuccess());
        }

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, ReadAttributeWithClusterTesterTest)
{
    TestServerClusterContext context;

    // Create a cluster with all features enabled
    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Create ClusterTester for simplified attribute reading
    ClusterTester tester(cluster);

    Structs::MeasurementAccuracyStruct::DecodableType accuracy;
    ASSERT_EQ(tester.ReadAttribute(Accuracy::Id, accuracy), CHIP_NO_ERROR);

    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyData;
    energyData.energy = 5000;
    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalEnergyData(energyData);

    cluster.CumulativeEnergySnapshot(optionalEnergyData, optionalEnergyData);
    cluster.PeriodicEnergySnapshot(optionalEnergyData, optionalEnergyData);

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

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, SnapshotsSetValuesAndGenerateEvents)
{
    TestServerClusterContext testContext;

    // Create a cluster with all features enabled
    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurement::OptionalAttributes::kOptionalAttributeCumulativeEnergyReset,
    });

    EXPECT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    auto & logOnlyEvents = testContext.EventsGenerator();

    // Prepare energy measurement data
    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyImported;
    energyImported.energy = 10000; // 10 Wh

    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyExported;
    energyExported.energy = 5000; // 5 Wh

    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalImported(energyImported);
    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalExported(energyExported);

    // Test cumulative energy snapshot - should set values and generate event
    {
        cluster.CumulativeEnergySnapshot(optionalImported, optionalExported);

        // Verify that values were set
        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 10000);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 5000);

        // Get the event from the queue
        auto event = logOnlyEvents.GetNextEvent();

        using CumulativeEventType = chip::app::Clusters::ElectricalEnergyMeasurement::Events::CumulativeEnergyMeasured::Type;
        ASSERT_TRUE(event.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(event.value().eventOptions.mPath,
                  ConcreteEventPath(kTestEndpointId, CumulativeEventType::GetClusterId(), CumulativeEventType::GetEventId()));
        chip::app::Clusters::ElectricalEnergyMeasurement::Events::CumulativeEnergyMeasured::DecodableType decodedEvent;

        // Check again for Tidy
        ASSERT_TRUE(event.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        ASSERT_EQ(event.value().GetEventData(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 10000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 5000);
    }

    // Test periodic energy snapshot - should set values and generate event
    {
        cluster.PeriodicEnergySnapshot(optionalImported, optionalExported);

        // Verify that values were set
        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 10000);

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 5000);

        // Get the event from the queue
        auto event = logOnlyEvents.GetNextEvent();

        using PeriodicEventType = chip::app::Clusters::ElectricalEnergyMeasurement::Events::PeriodicEnergyMeasured::Type;
        ASSERT_TRUE(event.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(event.value().eventOptions.mPath,
                  ConcreteEventPath(kTestEndpointId, PeriodicEventType::GetClusterId(), PeriodicEventType::GetEventId()));
        chip::app::Clusters::ElectricalEnergyMeasurement::Events::PeriodicEnergyMeasured::DecodableType decodedEvent;

        // Check again for Tidy
        ASSERT_TRUE(event.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        ASSERT_EQ(event.value().GetEventData(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 10000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 5000);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
