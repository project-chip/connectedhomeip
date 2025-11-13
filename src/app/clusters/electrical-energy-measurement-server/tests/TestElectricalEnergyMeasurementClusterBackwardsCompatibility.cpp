/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/EventManagement.h>
#include <app/clusters/electrical-energy-measurement-server/CodegenIntegration.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ElectricalEnergyMeasurement/Events.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CHIPCounter.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement;
using namespace chip::app::Clusters::ElectricalEnergyMeasurement::Attributes;

// Mock function for linking
void InitDataModelHandler() {}

namespace {

static uint8_t gDebugEventBuffer[128];
static uint8_t gInfoEventBuffer[128];
static uint8_t gCritEventBuffer[128];
static chip::app::CircularEventBuffer gCircularEventBuffer[3];

constexpr EndpointId kTestEndpointId = 1;

struct TestElectricalEnergyMeasurementClusterBackwardsCompatibility : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        const chip::app::LogStorageResources logStorageResources[] = {
            { &gDebugEventBuffer[0], sizeof(gDebugEventBuffer), chip::app::PriorityLevel::Debug },
            { &gInfoEventBuffer[0], sizeof(gInfoEventBuffer), chip::app::PriorityLevel::Info },
            { &gCritEventBuffer[0], sizeof(gCritEventBuffer), chip::app::PriorityLevel::Critical },
        };

        ASSERT_EQ(mEventCounter.Init(0), CHIP_NO_ERROR);
        chip::app::EventManagement::CreateEventManagement(nullptr, MATTER_ARRAY_SIZE(logStorageResources), gCircularEventBuffer,
                                                          logStorageResources, &mEventCounter);
    }

    void TearDown() override { chip::app::EventManagement::DestroyEventManagement(); }

    chip::Test::TestServerClusterContext mContext;
    chip::MonotonicallyIncreasingCounter<chip::EventNumber> mEventCounter;
};

TEST_F(TestElectricalEnergyMeasurementClusterBackwardsCompatibility, TestAttrAccessLifecycle)
{
    BitMask<Feature> features(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                              Feature::kPeriodicEnergy);
    BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeCumulativeEnergyReset);

    ElectricalEnergyMeasurementAttrAccess attrAccess(features, optionalAttrs, kTestEndpointId);

    // Test initialization
    EXPECT_EQ(attrAccess.Init(), CHIP_NO_ERROR);

    // Verify cluster is registered in the registry
    ElectricalEnergyMeasurementCluster * registeredCluster = FindElectricalEnergyMeasurementClusterOnEndpoint(kTestEndpointId);
    ASSERT_NE(registeredCluster, nullptr);

    // Test feature checking methods
    EXPECT_TRUE(attrAccess.HasFeature(Feature::kImportedEnergy));
    EXPECT_TRUE(attrAccess.HasFeature(Feature::kExportedEnergy));
    EXPECT_TRUE(attrAccess.HasFeature(Feature::kCumulativeEnergy));
    EXPECT_TRUE(attrAccess.HasFeature(Feature::kPeriodicEnergy));

    // Test with a feature that's not enabled
    BitMask<Feature> minimalFeatures(Feature::kImportedEnergy);
    BitMask<OptionalAttributes> noOptionalAttrs;
    ElectricalEnergyMeasurementAttrAccess minimalAttrAccess(minimalFeatures, noOptionalAttrs, kTestEndpointId + 1);
    EXPECT_EQ(minimalAttrAccess.Init(), CHIP_NO_ERROR);
    EXPECT_TRUE(minimalAttrAccess.HasFeature(Feature::kImportedEnergy));
    EXPECT_FALSE(minimalAttrAccess.HasFeature(Feature::kExportedEnergy));
    EXPECT_FALSE(minimalAttrAccess.HasFeature(Feature::kCumulativeEnergy));
    EXPECT_FALSE(minimalAttrAccess.HasFeature(Feature::kPeriodicEnergy));
    minimalAttrAccess.Shutdown();

    // Test optional attribute checking methods
    EXPECT_TRUE(attrAccess.SupportsOptAttr(OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));

    // Test with no optional attributes
    ElectricalEnergyMeasurementAttrAccess noOptAttrAccess(features, noOptionalAttrs, kTestEndpointId + 2);
    EXPECT_EQ(noOptAttrAccess.Init(), CHIP_NO_ERROR);
    EXPECT_FALSE(noOptAttrAccess.SupportsOptAttr(OptionalAttributes::kOptionalAttributeCumulativeEnergyReset));
    noOptAttrAccess.Shutdown();

    // Test shutdown
    attrAccess.Shutdown();

    // Verify cluster is unregistered from the registry
    EXPECT_EQ(FindElectricalEnergyMeasurementClusterOnEndpoint(kTestEndpointId), nullptr);
}

TEST_F(TestElectricalEnergyMeasurementClusterBackwardsCompatibility, TestCodegenIntegrationFunctions)
{
    // Setup: Create and register a cluster for testing
    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);
    BitMask<OptionalAttributes> optionalAttrs(OptionalAttributes::kOptionalAttributeCumulativeEnergyReset);

    ElectricalEnergyMeasurementAttrAccess attrAccess(allFeatures, optionalAttrs, kTestEndpointId);
    EXPECT_EQ(attrAccess.Init(), CHIP_NO_ERROR);

    // Verify cluster can be found via FindElectricalEnergyMeasurementClusterOnEndpoint
    ElectricalEnergyMeasurementCluster * cluster = FindElectricalEnergyMeasurementClusterOnEndpoint(kTestEndpointId);
    ASSERT_NE(cluster, nullptr);

    // Initialize the cluster with test context for event logging
    EXPECT_EQ(cluster->Startup(mContext.Get()), CHIP_NO_ERROR);

    // Test SetMeasurementAccuracy
    {
        Structs::MeasurementAccuracyStruct::Type accuracy;
        accuracy.measurementType  = MeasurementTypeEnum::kApparentEnergy;
        accuracy.measured         = true;
        accuracy.minMeasuredValue = 0;
        accuracy.maxMeasuredValue = 1000000;
        accuracy.accuracyRanges   = DataModel::List<Structs::MeasurementAccuracyRangeStruct::Type>();

        EXPECT_EQ(SetMeasurementAccuracy(kTestEndpointId, accuracy), CHIP_NO_ERROR);

        // Verify the value was set
        Structs::MeasurementAccuracyStruct::Type readAccuracy;
        cluster->GetMeasurementAccuracy(readAccuracy);
        EXPECT_EQ(readAccuracy.measurementType, MeasurementTypeEnum::kApparentEnergy);
        EXPECT_TRUE(readAccuracy.measured);
    }

    // Test SetCumulativeReset
    {
        Structs::CumulativeEnergyResetStruct::Type resetData;
        Optional<Structs::CumulativeEnergyResetStruct::Type> optionalReset(resetData);

        EXPECT_EQ(SetCumulativeReset(kTestEndpointId, optionalReset), CHIP_NO_ERROR);

        // Verify the value was set
        Optional<Structs::CumulativeEnergyResetStruct::Type> readReset;
        EXPECT_EQ(cluster->GetCumulativeEnergyReset(readReset), CHIP_NO_ERROR);
        EXPECT_TRUE(readReset.HasValue());
    }

    // Test NotifyCumulativeEnergyMeasured
    {
        auto & logOnlyEvents = mContext.EventsGenerator();

        Structs::EnergyMeasurementStruct::Type energyData;
        energyData.energy = 5000;
        Optional<Structs::EnergyMeasurementStruct::Type> energyImported(energyData);

        energyData.energy = 2000;
        Optional<Structs::EnergyMeasurementStruct::Type> energyExported(energyData);

        EXPECT_TRUE(NotifyCumulativeEnergyMeasured(kTestEndpointId, energyImported, energyExported));

        // Verify the values were set
        Optional<Structs::EnergyMeasurementStruct::Type> readValue;
        EXPECT_EQ(cluster->GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 5000);

        EXPECT_EQ(cluster->GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 2000);

        // Verify event was generated
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());

        using CumulativeEventType = chip::app::Clusters::ElectricalEnergyMeasurement::Events::CumulativeEnergyMeasured::Type;
        EXPECT_EQ(event->eventOptions.mPath,
                  ConcreteEventPath(kTestEndpointId, CumulativeEventType::GetClusterId(), CumulativeEventType::GetEventId()));

        chip::app::Clusters::ElectricalEnergyMeasurement::Events::CumulativeEnergyMeasured::DecodableType decodedEvent;
        ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 5000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 2000);
    }

    // Test NotifyPeriodicEnergyMeasured
    {
        auto & logOnlyEvents = mContext.EventsGenerator();

        Structs::EnergyMeasurementStruct::Type energyData;
        energyData.energy = 1500;
        Optional<Structs::EnergyMeasurementStruct::Type> energyImported(energyData);

        energyData.energy = 800;
        Optional<Structs::EnergyMeasurementStruct::Type> energyExported(energyData);

        EXPECT_TRUE(NotifyPeriodicEnergyMeasured(kTestEndpointId, energyImported, energyExported));

        // Verify the values were set
        Optional<Structs::EnergyMeasurementStruct::Type> readValue;
        EXPECT_EQ(cluster->GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 1500);

        EXPECT_EQ(cluster->GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        EXPECT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 800);

        // Verify event was generated
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());

        using PeriodicEventType = chip::app::Clusters::ElectricalEnergyMeasurement::Events::PeriodicEnergyMeasured::Type;
        EXPECT_EQ(event->eventOptions.mPath,
                  ConcreteEventPath(kTestEndpointId, PeriodicEventType::GetClusterId(), PeriodicEventType::GetEventId()));

        chip::app::Clusters::ElectricalEnergyMeasurement::Events::PeriodicEnergyMeasured::DecodableType decodedEvent;
        ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 1500);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 800);
    }

    // Test MeasurementDataForEndpoint
    {
        const MeasurementData * data = MeasurementDataForEndpoint(kTestEndpointId);
        ASSERT_NE(data, nullptr);

        // Verify data structure contains expected values from previous sets
        EXPECT_EQ(data->measurementAccuracy.measurementType, MeasurementTypeEnum::kApparentEnergy);
        EXPECT_TRUE(data->cumulativeImported.HasValue());
        EXPECT_EQ(data->cumulativeImported.Value().energy, 5000);
        EXPECT_TRUE(data->cumulativeExported.HasValue());
        EXPECT_EQ(data->cumulativeExported.Value().energy, 2000);
        EXPECT_TRUE(data->periodicImported.HasValue());
        EXPECT_EQ(data->periodicImported.Value().energy, 1500);
        EXPECT_TRUE(data->periodicExported.HasValue());
        EXPECT_EQ(data->periodicExported.Value().energy, 800);
        EXPECT_TRUE(data->cumulativeReset.HasValue());
    }

    // Test with non-existent endpoint
    {
        EXPECT_EQ(FindElectricalEnergyMeasurementClusterOnEndpoint(999), nullptr);
        EXPECT_EQ(MeasurementDataForEndpoint(999), nullptr);
        EXPECT_EQ(SetMeasurementAccuracy(999, {}), CHIP_ERROR_NOT_FOUND);
        EXPECT_EQ(SetCumulativeReset(999, {}), CHIP_ERROR_NOT_FOUND);
        EXPECT_FALSE(NotifyCumulativeEnergyMeasured(999, {}, {}));
        EXPECT_FALSE(NotifyPeriodicEnergyMeasured(999, {}, {}));
    }

    // Cleanup
    attrAccess.Shutdown();
}

} // namespace
