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
    DataModel::Nullable<int64_t> GetCumulativeEnergyImported() override { return mCumulativeImported; }
    DataModel::Nullable<int64_t> GetCumulativeEnergyExported() override { return mCumulativeExported; }
    DataModel::Nullable<int64_t> GetPeriodicEnergyImported() override { return mPeriodicImported; }
    DataModel::Nullable<int64_t> GetPeriodicEnergyExported() override { return mPeriodicExported; }

    DataModel::Nullable<int64_t> mCumulativeImported;
    DataModel::Nullable<int64_t> mCumulativeExported;
    DataModel::Nullable<int64_t> mPeriodicImported;
    DataModel::Nullable<int64_t> mPeriodicExported;
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

        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> nullableEnergyData(energyData);

        cluster.CumulativeEnergySnapshot(nullableEnergyData, nullableEnergyData);
        cluster.PeriodicEnergySnapshot(nullableEnergyData, nullableEnergyData);

        ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct resetData;
        DataModel::Nullable<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> nullableResetData(resetData);
        EXPECT_EQ(cluster.SetCumulativeEnergyReset(nullableResetData), CHIP_NO_ERROR);

        // Read back via getters and verify values
        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;

        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        EXPECT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 1000);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        EXPECT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 1000);

        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        EXPECT_FALSE(readValue.IsNull());

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        EXPECT_FALSE(readValue.IsNull());

        DataModel::Nullable<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> readResetValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyReset(readResetValue), CHIP_NO_ERROR);
        EXPECT_FALSE(readResetValue.IsNull());

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

        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

        DataModel::Nullable<ElectricalEnergyMeasurementCluster::CumulativeEnergyResetStruct> readResetValue;
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
    DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> nullableEnergyData(energyData);

    cluster.CumulativeEnergySnapshot(nullableEnergyData, nullableEnergyData);
    cluster.PeriodicEnergySnapshot(nullableEnergyData, nullableEnergyData);

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

    DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> nullableImported(energyImported);
    DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> nullableExported(energyExported);

    // Test cumulative energy snapshot
    {
        cluster.CumulativeEnergySnapshot(nullableImported, nullableExported);

        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 10000);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
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
        cluster.PeriodicEnergySnapshot(nullableImported, nullableExported);

        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 10000);

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
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

TEST_F(TestElectricalEnergyMeasurementCluster, GenerateSnapshotsPullsDelegateAndGeneratesEvents)
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
    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(42000));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(10000));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(500));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(200));

    // Advance clock past min report interval so GenerateSnapshots works
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMinReportInterval);

    cluster.GenerateSnapshots();

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
    DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
    EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
    ASSERT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().energy, 42000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, GenerateSnapshotsRespectsMinInterval)
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

    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(100));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(50));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(10));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(5));

    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(100));

    // First report -- all 4 attributes should notify, both events should fire.
    cluster.GenerateSnapshots();
    {
        auto cumulativeEvent = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(cumulativeEvent.has_value());
        Events::CumulativeEnergyMeasured::DecodableType decoded;
        ASSERT_EQ(cumulativeEvent->GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 100);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 50);

        auto periodicEvent = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(periodicEvent.has_value());
        Events::PeriodicEnergyMeasured::DecodableType periodicDecoded;
        ASSERT_EQ(periodicEvent->GetEventData(periodicDecoded), CHIP_NO_ERROR);
        ASSERT_TRUE(periodicDecoded.energyImported.HasValue());
        EXPECT_EQ(periodicDecoded.energyImported.Value().energy, 10);
        ASSERT_TRUE(periodicDecoded.energyExported.HasValue());
        EXPECT_EQ(periodicDecoded.energyExported.Value().energy, 5);
    }

    // +1 ms: inside min interval -- all 4 attributes should be gated, no events.
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1));
    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(200));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(150));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(20));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(15));
    cluster.GenerateSnapshots();
    {
        EXPECT_FALSE(logOnlyEvents.GetNextEvent().has_value());
    }

    // Attribute values are still updated internally even when gated.
    {
        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 200);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 150);

        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 20);

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 15);
    }

    // Advance past min interval -- all 4 should notify, both events should fire with latest values.
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMinReportInterval);
    cluster.GenerateSnapshots();
    {
        auto cumulativeEvent = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(cumulativeEvent.has_value());
        Events::CumulativeEnergyMeasured::DecodableType decoded;
        ASSERT_EQ(cumulativeEvent->GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 200);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 150);

        auto periodicEvent = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(periodicEvent.has_value());
        Events::PeriodicEnergyMeasured::DecodableType periodicDecoded;
        ASSERT_EQ(periodicEvent->GetEventData(periodicDecoded), CHIP_NO_ERROR);
        ASSERT_TRUE(periodicDecoded.energyImported.HasValue());
        EXPECT_EQ(periodicDecoded.energyImported.Value().energy, 20);
        ASSERT_TRUE(periodicDecoded.energyExported.HasValue());
        EXPECT_EQ(periodicDecoded.energyExported.Value().energy, 15);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, TimerFiresAndGeneratesReport)
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

    // Establish stored values with a first report.
    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(1));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(1));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(1));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(1));
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(100));
    cluster.GenerateSnapshots();
    ASSERT_TRUE(logOnlyEvents.GetNextEvent().has_value());
    ASSERT_TRUE(logOnlyEvents.GetNextEvent().has_value());

    // Rate-limited update within 1 s -- starts the delay timer.
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1));
    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(99999));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(88888));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(7777));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(6666));
    cluster.GenerateSnapshots();
    EXPECT_FALSE(logOnlyEvents.GetNextEvent().has_value());

    // Advance past the max-delay timer -- should fire and produce a full report.
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMaxReportDelayInterval);

    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        Events::CumulativeEnergyMeasured::DecodableType decoded;
        ASSERT_EQ(event->GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 99999);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 88888);
    }

    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value());
        Events::PeriodicEnergyMeasured::DecodableType decoded;
        ASSERT_EQ(event->GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 7777);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 6666);
    }

    // All 4 attributes must be readable with the correct values.
    {
        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 99999);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 88888);

        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 7777);

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 6666);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestElectricalEnergyMeasurementCluster, TestGenerateSnapshots)
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

    {
        // Test when the delegate has no data, all values are null
        mDelegate.mCumulativeImported.SetNull();
        mDelegate.mCumulativeExported.SetNull();
        mDelegate.mPeriodicImported.SetNull();
        mDelegate.mPeriodicExported.SetNull();

        DataModel::Nullable<int64_t> testImported = mDelegate.GetCumulativeEnergyImported();
        DataModel::Nullable<int64_t> testExported = mDelegate.GetCumulativeEnergyExported();
        EXPECT_TRUE(testImported.IsNull());
        EXPECT_TRUE(testExported.IsNull());

        testImported = mDelegate.GetPeriodicEnergyImported();
        testExported = mDelegate.GetPeriodicEnergyExported();
        EXPECT_TRUE(testImported.IsNull());
        EXPECT_TRUE(testExported.IsNull());

        cluster.GenerateSnapshots();
        {
            auto event = logOnlyEvents.GetNextEvent();
            EXPECT_FALSE(event.has_value());
        }

        mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(100));
        mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(50));
        mDelegate.mPeriodicImported.SetNull();
        mDelegate.mPeriodicExported.SetNull();

        cluster.GenerateSnapshots();
        {
            auto event = logOnlyEvents.GetNextEvent();
            ASSERT_TRUE(event.has_value());
            Events::CumulativeEnergyMeasured::DecodableType decoded;
            ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);
            ASSERT_TRUE(decoded.energyImported.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().energy, 100);
            EXPECT_FALSE(decoded.energyImported.Value().startTimestamp.HasValue());
            EXPECT_FALSE(decoded.energyImported.Value().startSystime.HasValue());
            ASSERT_TRUE(decoded.energyImported.Value().endTimestamp.HasValue());
            ASSERT_TRUE(decoded.energyImported.Value().endSystime.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().endSystime.Value(), 0u);
        }

        // Second cumulative reading: still no start* (cumulative BuildMeasurement omits carry-over).
        mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(200));
        mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMinReportInterval);
        cluster.GenerateSnapshots();
        {
            auto event = logOnlyEvents.GetNextEvent();
            ASSERT_TRUE(event.has_value());
            Events::CumulativeEnergyMeasured::DecodableType decoded;
            ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);
            ASSERT_TRUE(decoded.energyImported.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().energy, 200);
            EXPECT_FALSE(decoded.energyImported.Value().startTimestamp.HasValue());
            EXPECT_FALSE(decoded.energyImported.Value().startSystime.HasValue());
            ASSERT_TRUE(decoded.energyImported.Value().endSystime.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().endSystime.Value(),
                      static_cast<uint64_t>(mTimerDelegate.GetCurrentMonotonicTimestamp().count()));
        }
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);

    // Carry-over of endTimestamp -> startTimestamp is periodic-only; exercise with periodic cluster.
    {
        BitMask<Feature> periodicFeatures(Feature::kImportedEnergy, Feature::kPeriodicEnergy);
        ElectricalEnergyMeasurementCluster periodicCluster(ElectricalEnergyMeasurementCluster::Config{
            .endpointId         = static_cast<EndpointId>(kTestEndpointId + 1),
            .featureFlags       = periodicFeatures,
            .optionalAttributes = ElectricalEnergyMeasurementCluster::OptionalAttributesSet(),
            .accuracyStruct     = kTestAccuracy,
            .delegate           = mDelegate,
            .timerDelegate      = mTimerDelegate,
        });
        EXPECT_EQ(periodicCluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        mDelegate.mCumulativeImported.SetNull();
        mDelegate.mCumulativeExported.SetNull();
        mDelegate.mPeriodicImported = DataModel::MakeNullable(static_cast<int64_t>(10));
        mDelegate.mPeriodicExported.SetNull();

        periodicCluster.GenerateSnapshots();
        uint32_t firstPeriodicEndTimestamp = 0;
        {
            auto event = logOnlyEvents.GetNextEvent();
            ASSERT_TRUE(event.has_value());
            Events::PeriodicEnergyMeasured::DecodableType decoded;
            ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);
            ASSERT_TRUE(decoded.energyImported.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().energy, 10);
            EXPECT_FALSE(decoded.energyImported.Value().startTimestamp.HasValue());
            ASSERT_TRUE(decoded.energyImported.Value().endTimestamp.HasValue());
            firstPeriodicEndTimestamp = decoded.energyImported.Value().endTimestamp.Value();
        }

        mDelegate.mPeriodicImported = DataModel::MakeNullable(static_cast<int64_t>(20));
        mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMinReportInterval);
        periodicCluster.GenerateSnapshots();
        {
            auto event = logOnlyEvents.GetNextEvent();
            ASSERT_TRUE(event.has_value());
            Events::PeriodicEnergyMeasured::DecodableType decoded;
            ASSERT_EQ((*event).GetEventData(decoded), CHIP_NO_ERROR);
            ASSERT_TRUE(decoded.energyImported.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().energy, 20);
            ASSERT_TRUE(decoded.energyImported.Value().startTimestamp.HasValue());
            EXPECT_EQ(decoded.energyImported.Value().startTimestamp.Value(), firstPeriodicEndTimestamp);
        }

        periodicCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestElectricalEnergyMeasurementCluster, RateLimitedUpdateFlushesOnMaxDelayTimer)
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

    // Seed all 4 attributes with initial values and get a clean first report.
    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(100));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(50));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(10));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(5));
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(100));
    cluster.GenerateSnapshots();
    {
        // Drain the two initial events (cumulative + periodic).
        ASSERT_TRUE(logOnlyEvents.GetNextEvent().has_value());
        ASSERT_TRUE(logOnlyEvents.GetNextEvent().has_value());
    }

    // Update all 4 delegate values inside the min interval -- all should be rate-limited.
    mTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1));
    mDelegate.mCumulativeImported = DataModel::MakeNullable(static_cast<int64_t>(200));
    mDelegate.mCumulativeExported = DataModel::MakeNullable(static_cast<int64_t>(150));
    mDelegate.mPeriodicImported   = DataModel::MakeNullable(static_cast<int64_t>(20));
    mDelegate.mPeriodicExported   = DataModel::MakeNullable(static_cast<int64_t>(15));
    cluster.GenerateSnapshots();
    EXPECT_FALSE(logOnlyEvents.GetNextEvent().has_value());

    // Fire the max-delay timer -- all 4 dirty attributes should flush, both events should be generated.
    mTimerDelegate.AdvanceClock(ElectricalEnergyMeasurementCluster::kMaxReportDelayInterval);

    // CumulativeEnergyMeasured event
    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value()) << "Max-delay timer did not produce CumulativeEnergyMeasured event";
        Events::CumulativeEnergyMeasured::DecodableType decoded;
        ASSERT_EQ(event->GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 200);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 150);
    }

    // PeriodicEnergyMeasured event
    {
        auto event = logOnlyEvents.GetNextEvent();
        ASSERT_TRUE(event.has_value()) << "Max-delay timer did not produce PeriodicEnergyMeasured event";
        Events::PeriodicEnergyMeasured::DecodableType decoded;
        ASSERT_EQ(event->GetEventData(decoded), CHIP_NO_ERROR);
        ASSERT_TRUE(decoded.energyImported.HasValue());
        EXPECT_EQ(decoded.energyImported.Value().energy, 20);
        ASSERT_TRUE(decoded.energyExported.HasValue());
        EXPECT_EQ(decoded.energyExported.Value().energy, 15);
    }

    // All 4 attributes should be readable with the latest values.
    {
        DataModel::Nullable<ElectricalEnergyMeasurementCluster::EnergyMeasurementStruct> readValue;
        EXPECT_EQ(cluster.GetCumulativeEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 200);

        EXPECT_EQ(cluster.GetCumulativeEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 150);

        EXPECT_EQ(cluster.GetPeriodicEnergyImported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 20);

        EXPECT_EQ(cluster.GetPeriodicEnergyExported(readValue), CHIP_NO_ERROR);
        ASSERT_FALSE(readValue.IsNull());
        EXPECT_EQ(readValue.Value().energy, 15);
    }

    // No stale events left over.
    EXPECT_FALSE(logOnlyEvents.GetNextEvent().has_value());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
