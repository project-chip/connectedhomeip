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

#include <app/clusters/electrical-energy-measurement-server/ElectricalEnergyMeasurementCluster.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
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

const Structs::MeasurementAccuracyRangeStruct::Type kTestAccuracyRanges[] = {
    { .rangeMin   = 0,
      .rangeMax   = 1'000'000'000'000'000, // 1 million Mwh
      .percentMax = MakeOptional(static_cast<chip::Percent100ths>(500)),
      .percentMin = MakeOptional(static_cast<chip::Percent100ths>(50)) }
};

const Structs::MeasurementAccuracyStruct::Type kTestAccuracy = {
    .measurementType  = MeasurementTypeEnum::kElectricalEnergy,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 1'000'000'000'000'000,
    .accuracyRanges   = DataModel::List<const Structs::MeasurementAccuracyRangeStruct::Type>(kTestAccuracyRanges)
};

class MockEEMDelegate : public ElectricalEnergyMeasurement::Delegate
{
public:
    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override { return DataModel::MakeNullable(mCumulativeImported); }
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override { return DataModel::MakeNullable(mCumulativeExported); }
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override { return DataModel::MakeNullable(mPeriodicImported); }
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override { return DataModel::MakeNullable(mPeriodicExported); }

    int64_t mCumulativeImported = 0;
    int64_t mCumulativeExported = 0;
    int64_t mPeriodicImported   = 0;
    int64_t mPeriodicExported   = 0;
};

struct TestElectricalEnergyMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}

    MockEEMDelegate mDelegate;
    TimerDelegateMock mTimerDelegate;
};

TEST_F(TestElectricalEnergyMeasurementCluster, TestConstructorWithAccuracy)
{
    BitMask<Feature> features(Feature::kImportedEnergy, Feature::kCumulativeEnergy);
    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId + 10,
        .featureFlags       = features,
        .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
        .accuracyStruct     = kTestAccuracy,
        .delegate           = mDelegate,
        .timerDelegate      = mTimerDelegate,
    });

    Structs::MeasurementAccuracyStruct::Type readAccuracy;
    cluster.GetMeasurementAccuracy(readAccuracy);

    EXPECT_EQ(readAccuracy.measurementType, MeasurementTypeEnum::kElectricalEnergy);
    EXPECT_TRUE(readAccuracy.measured);
    EXPECT_EQ(readAccuracy.minMeasuredValue, 0);
    EXPECT_EQ(readAccuracy.maxMeasuredValue, 1'000'000'000'000'000);
    ASSERT_EQ(readAccuracy.accuracyRanges.size(), 1u);
    EXPECT_EQ(readAccuracy.accuracyRanges[0].rangeMin, 0);
    EXPECT_EQ(readAccuracy.accuracyRanges[0].rangeMax, 1'000'000'000'000'000);
    EXPECT_EQ(readAccuracy.accuracyRanges[0].percentMax.Value(), 500);
    EXPECT_EQ(readAccuracy.accuracyRanges[0].percentMin.Value(), 50);

    // Verify zero-copy: the ranges Span should point directly at the static data
    EXPECT_EQ(readAccuracy.accuracyRanges.data(), kTestAccuracyRanges);
}

TEST_F(TestElectricalEnergyMeasurementCluster, AttributeListTest)
{
    TestServerClusterContext context;

    // Test 1: No features activated - should only have mandatory Accuracy attribute
    {
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = noFeatures,
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster, { Attributes::Accuracy::kMetadataEntry }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 2: All features activated - optional attributes are automatically determined by features
    {
        BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                     Feature::kPeriodicEnergy);

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = allFeatures,
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet().Set<CumulativeEnergyReset::Id>(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
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
        BitMask<Feature> noFeatures;

        ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .featureFlags       = noFeatures,
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet().Set<CumulativeEnergyReset::Id>(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

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
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet().Set<CumulativeEnergyReset::Id>(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
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
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

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
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet().Set<CumulativeEnergyReset::Id>(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(CumulativeEnergyImported::Id, value).IsSuccess());
        }

        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(CumulativeEnergyExported::Id, value).IsSuccess());
        }

        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(PeriodicEnergyImported::Id, value).IsSuccess());
        }

        {
            DataModel::Nullable<Structs::EnergyMeasurementStruct::DecodableType> value;
            EXPECT_TRUE(tester.ReadAttribute(PeriodicEnergyExported::Id, value).IsSuccess());
        }

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

    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet().Set<CumulativeEnergyReset::Id>(),
        .accuracyStruct     = kTestAccuracy,
        .delegate           = mDelegate,
        .timerDelegate      = mTimerDelegate,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

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

    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet().Set<CumulativeEnergyReset::Id>(),
        .accuracyStruct     = kTestAccuracy,
        .delegate           = mDelegate,
        .timerDelegate      = mTimerDelegate,
    });

    EXPECT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    auto & logOnlyEvents = testContext.EventsGenerator();

    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyImported;
    energyImported.energy = 10000;

    ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct energyExported;
    energyExported.energy = 5000;

    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalImported(energyImported);
    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> optionalExported(energyExported);

    // Test cumulative energy snapshot
    {
        cluster.CumulativeEnergySnapshot(optionalImported, optionalExported);

        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 10000);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 5000);

        auto event = logOnlyEvents.GetNextEvent();

        using CumulativeEventType = Events::CumulativeEnergyMeasured::Type;
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ((*event).eventOptions.mPath,
                  ConcreteEventPath(kTestEndpointId, CumulativeEventType::GetClusterId(), CumulativeEventType::GetEventId()));
        Events::CumulativeEnergyMeasured::DecodableType decodedEvent;

        ASSERT_EQ((*event).GetEventData(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 10000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 5000);
    }

    // Test periodic energy snapshot
    {
        cluster.PeriodicEnergySnapshot(optionalImported, optionalExported);

        Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 10000);

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_TRUE(readValue.HasValue());
        EXPECT_EQ(readValue.Value().energy, 5000);

        auto event = logOnlyEvents.GetNextEvent();

        using PeriodicEventType = Events::PeriodicEnergyMeasured::Type;
        ASSERT_TRUE(event.has_value());
        EXPECT_EQ((*event).eventOptions.mPath,
                  ConcreteEventPath(kTestEndpointId, PeriodicEventType::GetClusterId(), PeriodicEventType::GetEventId()));
        Events::PeriodicEnergyMeasured::DecodableType decodedEvent;

        ASSERT_EQ((*event).GetEventData(decodedEvent), CHIP_NO_ERROR);

        ASSERT_TRUE(decodedEvent.energyImported.HasValue());
        EXPECT_EQ(decodedEvent.energyImported.Value().energy, 10000);

        ASSERT_TRUE(decodedEvent.energyExported.HasValue());
        EXPECT_EQ(decodedEvent.energyExported.Value().energy, 5000);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, GenerateReportPullsDelegateAndGeneratesEvents)
{
    TestServerClusterContext testContext;

    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kExportedEnergy, Feature::kCumulativeEnergy,
                                 Feature::kPeriodicEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
        .accuracyStruct     = kTestAccuracy,
        .delegate           = mDelegate,
        .timerDelegate      = mTimerDelegate,
    });

    EXPECT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    auto & logOnlyEvents = testContext.EventsGenerator();

    // Set delegate values
    mDelegate.mCumulativeImported = 42000;
    mDelegate.mCumulativeExported = 10000;
    mDelegate.mPeriodicImported   = 500;
    mDelegate.mPeriodicExported   = 200;

    // Advance clock past min report interval so GenerateReport works
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMinReportInterval);

    cluster.GenerateReport();

    // Verify cumulative event
    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        Events::CumulativeEnergyMeasured::DecodableType decoded;
        ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);

        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 42000);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 10000);
    }

    // Verify periodic event
    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        Events::PeriodicEnergyMeasured::DecodableType decoded;
        ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);

        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 500);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 200);
    }

    // Verify internal measurement data was updated
    Optional<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
    EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
    ASSERT_TRUE(readValue.HasValue());
    EXPECT_EQ(readValue.Value().energy, 42000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, GenerateReportRespectsMinInterval)
{
    TestServerClusterContext testContext;

    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kCumulativeEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
        .accuracyStruct     = kTestAccuracy,
        .delegate           = mDelegate,
        .timerDelegate      = mTimerDelegate,
    });

    EXPECT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    auto & logOnlyEvents = testContext.EventsGenerator();

    mDelegate.mCumulativeImported = 100;

    // Advance the clock to avoid having the last report = 0, impossible in practice but would happen in tests without this.
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(100));

    // First report should work (clock at 0, no previous report)
    cluster.GenerateReport();
    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
    }

    // Second call 1 ms after first report -- should be silently dropped (min 1s)
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1));
    mDelegate.mCumulativeImported = 200;
    cluster.GenerateReport();
    {
        auto event = logOnlyEvents.GetNextEvent();
        EXPECT_FALSE(event.has_value());
    }

    // Advance past min interval
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMinReportInterval);
    cluster.GenerateReport();
    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        Events::CumulativeEnergyMeasured::DecodableType decoded;
        ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 200);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, TimerFiresAndGeneratesReport)
{
    TestServerClusterContext testContext;

    BitMask<Feature> allFeatures(Feature::kImportedEnergy, Feature::kCumulativeEnergy);

    ElectricalEnergyMeasurementCluster cluster(ElectricalEnergyMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .featureFlags       = allFeatures,
        .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
        .accuracyStruct     = kTestAccuracy,
        .delegate           = mDelegate,
        .timerDelegate      = mTimerDelegate,
    });

    EXPECT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    auto & logOnlyEvents = testContext.EventsGenerator();

    mDelegate.mCumulativeImported = 99999;

    // Advance clock by the report interval (default 60s) -- should trigger TimerFired
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMaxReportInterval);

    auto event = logOnlyEvents.GetNextEvent();
    ASSERT_TRUE(event.has_value());

    Events::CumulativeEnergyMeasured::DecodableType decoded;
    ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);
    ASSERT_TRUE(decoded.energyImported.HasValue());
    EXPECT_EQ(decoded.energyImported.Value().energy, 99999);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
