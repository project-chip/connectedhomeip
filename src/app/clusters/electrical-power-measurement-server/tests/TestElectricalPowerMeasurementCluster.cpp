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

#include <app/clusters/electrical-power-measurement-server/ElectricalPowerMeasurementCluster.h>
#include <app/clusters/electrical-power-measurement-server/tests/MockElectricalPowerMeasurementDelegate.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/ElectricalPowerMeasurement/Attributes.h>
#include <clusters/ElectricalPowerMeasurement/Events.h>
#include <clusters/ElectricalPowerMeasurement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalPowerMeasurement;
using namespace chip::app::Clusters::ElectricalPowerMeasurement::Attributes;
using namespace chip::Test;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestElectricalPowerMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

TEST_F(TestElectricalPowerMeasurementCluster, BasicConfigurations)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // No features, no optional attributes
    {
        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(),
            .optionalAttributes = ElectricalPowerMeasurementCluster::OptionalAttributesSet(),
        });

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(
            IsAttributesListEqualTo(cluster,
                                    { Attributes::PowerMode::kMetadataEntry, Attributes::NumberOfMeasurementTypes::kMetadataEntry,
                                      Attributes::Accuracy::kMetadataEntry, Attributes::ActivePower::kMetadataEntry }));
        cluster.Shutdown();
    }

    // Test DirectCurrent feature only
    {
        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(Feature::kDirectCurrent),
            .optionalAttributes = ElectricalPowerMeasurementCluster::OptionalAttributesSet(),
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        cluster.Shutdown();
    }
}

TEST_F(TestElectricalPowerMeasurementCluster, ComplexFeatureSets)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // Test AlternatingCurrent with optional AC attributes
    {
        ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
        optionalAttrs.Set<Attributes::ReactiveCurrent::Id>();
        optionalAttrs.Set<Attributes::RMSVoltage::Id>();
        optionalAttrs.Set<Attributes::Frequency::Id>();

        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(Feature::kAlternatingCurrent),
            .optionalAttributes = optionalAttrs,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        cluster.Shutdown();
    }

    // Test full feature set
    {
        ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
        optionalAttrs.Set<Attributes::Ranges::Id>();
        optionalAttrs.Set<Attributes::Voltage::Id>();
        optionalAttrs.Set<Attributes::ReactiveCurrent::Id>();
        optionalAttrs.Set<Attributes::RMSVoltage::Id>();
        optionalAttrs.Set<Attributes::Frequency::Id>();
        optionalAttrs.Set<Attributes::NeutralCurrent::Id>();

        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(Feature::kDirectCurrent, Feature::kAlternatingCurrent, Feature::kPolyphasePower,
                                         Feature::kHarmonics, Feature::kPowerQuality),
            .optionalAttributes = optionalAttrs,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        cluster.Shutdown();
    }
}

TEST_F(TestElectricalPowerMeasurementCluster, FeatureConformance)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // PolyphasePower requires AlternatingCurrent
    {
        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(Feature::kPolyphasePower),
            .optionalAttributes = ElectricalPowerMeasurementCluster::OptionalAttributesSet(),
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    }

    // Harmonics requires AlternatingCurrent
    {
        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(Feature::kHarmonics),
            .optionalAttributes = ElectricalPowerMeasurementCluster::OptionalAttributesSet(),
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    }

    // PowerQuality requires AlternatingCurrent
    {
        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(Feature::kPowerQuality),
            .optionalAttributes = ElectricalPowerMeasurementCluster::OptionalAttributesSet(),
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    }
}

TEST_F(TestElectricalPowerMeasurementCluster, AttributeConformance)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // ReactiveCurrent requires AlternatingCurrent
    {
        ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
        optionalAttrs.Set<Attributes::ReactiveCurrent::Id>();

        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(),
            .optionalAttributes = optionalAttrs,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    }

    // NeutralCurrent requires PolyphasePower
    {
        ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
        optionalAttrs.Set<Attributes::NeutralCurrent::Id>();

        ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
            .endpointId         = kTestEndpointId,
            .delegate           = delegate,
            .features           = BitMask<Feature>(),
            .optionalAttributes = optionalAttrs,
        });
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INCORRECT_STATE);
    }
}

TEST_F(TestElectricalPowerMeasurementCluster, ReadBasicAttributes)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // Test values
    const PowerModeEnum expectedPowerMode          = PowerModeEnum::kAc;
    const uint8_t expectedNumMeasurementTypes      = 2;
    const int64_t expectedActivePower              = 1000000;
    const int64_t expectedVoltage                  = 120000;
    const int64_t expectedActiveCurrent            = 10000;
    const MeasurementTypeEnum expectedAccuracyType = MeasurementTypeEnum::kActivePower;
    const int64_t expectedAccuracyMinMeasuredValue = 0;
    const int64_t expectedAccuracyMaxMeasuredValue = 10000000;

    delegate.SetPowerMode(expectedPowerMode);
    delegate.SetNumberOfMeasurementTypes(expectedNumMeasurementTypes);
    delegate.SetActivePower(DataModel::Nullable<int64_t>(expectedActivePower));
    delegate.SetVoltage(DataModel::Nullable<int64_t>(expectedVoltage));
    delegate.SetActiveCurrent(DataModel::Nullable<int64_t>(expectedActiveCurrent));

    Structs::MeasurementAccuracyStruct::Type accuracy;
    accuracy.measurementType  = expectedAccuracyType;
    accuracy.measured         = true;
    accuracy.minMeasuredValue = expectedAccuracyMinMeasuredValue;
    accuracy.maxMeasuredValue = expectedAccuracyMaxMeasuredValue;
    delegate.AddAccuracy(accuracy);

    ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
    optionalAttrs.Set<Attributes::Voltage::Id>();
    optionalAttrs.Set<Attributes::ActiveCurrent::Id>();

    ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = delegate,
        .features           = BitMask<Feature>(),
        .optionalAttributes = optionalAttrs,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    // Read mandatory attributes
    PowerModeEnum powerMode;
    EXPECT_EQ(tester.ReadAttribute(PowerMode::Id, powerMode), CHIP_NO_ERROR);
    EXPECT_EQ(powerMode, expectedPowerMode);

    uint8_t numTypes;
    EXPECT_EQ(tester.ReadAttribute(NumberOfMeasurementTypes::Id, numTypes), CHIP_NO_ERROR);
    EXPECT_EQ(numTypes, expectedNumMeasurementTypes);

    DataModel::Nullable<int64_t> activePower;
    EXPECT_EQ(tester.ReadAttribute(ActivePower::Id, activePower), CHIP_NO_ERROR);
    EXPECT_FALSE(activePower.IsNull());
    EXPECT_EQ(activePower.Value(), expectedActivePower);

    // Read optional attributes
    DataModel::Nullable<int64_t> voltage;
    EXPECT_EQ(tester.ReadAttribute(Voltage::Id, voltage), CHIP_NO_ERROR);
    EXPECT_FALSE(voltage.IsNull());
    EXPECT_EQ(voltage.Value(), expectedVoltage);

    DataModel::Nullable<int64_t> activeCurrent;
    EXPECT_EQ(tester.ReadAttribute(ActiveCurrent::Id, activeCurrent), CHIP_NO_ERROR);
    EXPECT_FALSE(activeCurrent.IsNull());
    EXPECT_EQ(activeCurrent.Value(), expectedActiveCurrent);

    // Test null values
    delegate.SetVoltage(DataModel::NullNullable);
    EXPECT_EQ(tester.ReadAttribute(Voltage::Id, voltage), CHIP_NO_ERROR);
    EXPECT_TRUE(voltage.IsNull());

    cluster.Shutdown();
}

TEST_F(TestElectricalPowerMeasurementCluster, ReadRangesList)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // Test values for range 1
    const MeasurementTypeEnum expectedRange1Type = MeasurementTypeEnum::kActivePower;
    const int64_t expectedRange1Min              = 0;
    const int64_t expectedRange1Max              = 5000000;
    const uint32_t expectedRange1StartTimestamp  = 0;
    const uint32_t expectedRange1EndTimestamp    = 3600;
    const uint32_t expectedRange1MinTimestamp    = 0;
    const uint32_t expectedRange1MaxTimestamp    = 1800;
    const uint64_t expectedRange1StartSystime    = 0;
    const uint64_t expectedRange1EndSystime      = 3600000;
    const uint64_t expectedRange1MinSystime      = 0;
    const uint64_t expectedRange1MaxSystime      = 1800000;

    // Test values for range 2
    const MeasurementTypeEnum expectedRange2Type = MeasurementTypeEnum::kReactivePower;
    const int64_t expectedRange2Min              = -1000000;
    const int64_t expectedRange2Max              = 1000000;
    const uint32_t expectedRange2StartTimestamp  = 3600;
    const uint32_t expectedRange2EndTimestamp    = 7200;
    const uint32_t expectedRange2MinTimestamp    = 5000;
    const uint32_t expectedRange2MaxTimestamp    = 6000;
    const uint64_t expectedRange2StartSystime    = 3600000;
    const uint64_t expectedRange2EndSystime      = 7200000;
    const uint64_t expectedRange2MinSystime      = 5000000;
    const uint64_t expectedRange2MaxSystime      = 6000000;

    // Add range entries
    Structs::MeasurementRangeStruct::Type range1;
    range1.measurementType = expectedRange1Type;
    range1.min             = expectedRange1Min;
    range1.max             = expectedRange1Max;
    range1.startTimestamp  = MakeOptional(expectedRange1StartTimestamp);
    range1.endTimestamp    = MakeOptional(expectedRange1EndTimestamp);
    range1.minTimestamp    = MakeOptional(expectedRange1MinTimestamp);
    range1.maxTimestamp    = MakeOptional(expectedRange1MaxTimestamp);
    range1.startSystime    = MakeOptional(expectedRange1StartSystime);
    range1.endSystime      = MakeOptional(expectedRange1EndSystime);
    range1.minSystime      = MakeOptional(expectedRange1MinSystime);
    range1.maxSystime      = MakeOptional(expectedRange1MaxSystime);
    delegate.AddRange(range1);

    Structs::MeasurementRangeStruct::Type range2;
    range2.measurementType = expectedRange2Type;
    range2.min             = expectedRange2Min;
    range2.max             = expectedRange2Max;
    range2.startTimestamp  = MakeOptional(expectedRange2StartTimestamp);
    range2.endTimestamp    = MakeOptional(expectedRange2EndTimestamp);
    range2.minTimestamp    = MakeOptional(expectedRange2MinTimestamp);
    range2.maxTimestamp    = MakeOptional(expectedRange2MaxTimestamp);
    range2.startSystime    = MakeOptional(expectedRange2StartSystime);
    range2.endSystime      = MakeOptional(expectedRange2EndSystime);
    range2.minSystime      = MakeOptional(expectedRange2MinSystime);
    range2.maxSystime      = MakeOptional(expectedRange2MaxSystime);
    delegate.AddRange(range2);

    ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
    optionalAttrs.Set<Attributes::Ranges::Id>();

    ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = delegate,
        .features           = BitMask<Feature>(),
        .optionalAttributes = optionalAttrs,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    // Read and decode the Ranges list
    Ranges::TypeInfo::DecodableType ranges;
    EXPECT_EQ(tester.ReadAttribute(Ranges::Id, ranges), CHIP_NO_ERROR);

    size_t count = 0;
    auto iter    = ranges.begin();
    while (iter.Next())
    {
        const auto & range = iter.GetValue();
        if (count == 0)
        {
            // Verify first range
            EXPECT_EQ(range.measurementType, expectedRange1Type);
            EXPECT_EQ(range.min, expectedRange1Min);
            EXPECT_EQ(range.max, expectedRange1Max);
            EXPECT_TRUE(range.startTimestamp.HasValue());
            EXPECT_EQ(range.startTimestamp.Value(), expectedRange1StartTimestamp);
            EXPECT_TRUE(range.endTimestamp.HasValue());
            EXPECT_EQ(range.endTimestamp.Value(), expectedRange1EndTimestamp);
            EXPECT_TRUE(range.minTimestamp.HasValue());
            EXPECT_EQ(range.minTimestamp.Value(), expectedRange1MinTimestamp);
            EXPECT_TRUE(range.maxTimestamp.HasValue());
            EXPECT_EQ(range.maxTimestamp.Value(), expectedRange1MaxTimestamp);
            EXPECT_TRUE(range.startSystime.HasValue());
            EXPECT_EQ(range.startSystime.Value(), expectedRange1StartSystime);
            EXPECT_TRUE(range.endSystime.HasValue());
            EXPECT_EQ(range.endSystime.Value(), expectedRange1EndSystime);
            EXPECT_TRUE(range.minSystime.HasValue());
            EXPECT_EQ(range.minSystime.Value(), expectedRange1MinSystime);
            EXPECT_TRUE(range.maxSystime.HasValue());
            EXPECT_EQ(range.maxSystime.Value(), expectedRange1MaxSystime);
        }
        else if (count == 1)
        {
            // Verify second range
            EXPECT_EQ(range.measurementType, expectedRange2Type);
            EXPECT_EQ(range.min, expectedRange2Min);
            EXPECT_EQ(range.max, expectedRange2Max);
            EXPECT_TRUE(range.startTimestamp.HasValue());
            EXPECT_EQ(range.startTimestamp.Value(), expectedRange2StartTimestamp);
            EXPECT_TRUE(range.endTimestamp.HasValue());
            EXPECT_EQ(range.endTimestamp.Value(), expectedRange2EndTimestamp);
            EXPECT_TRUE(range.minTimestamp.HasValue());
            EXPECT_EQ(range.minTimestamp.Value(), expectedRange2MinTimestamp);
            EXPECT_TRUE(range.maxTimestamp.HasValue());
            EXPECT_EQ(range.maxTimestamp.Value(), expectedRange2MaxTimestamp);
            EXPECT_TRUE(range.startSystime.HasValue());
            EXPECT_EQ(range.startSystime.Value(), expectedRange2StartSystime);
            EXPECT_TRUE(range.endSystime.HasValue());
            EXPECT_EQ(range.endSystime.Value(), expectedRange2EndSystime);
            EXPECT_TRUE(range.minSystime.HasValue());
            EXPECT_EQ(range.minSystime.Value(), expectedRange2MinSystime);
            EXPECT_TRUE(range.maxSystime.HasValue());
            EXPECT_EQ(range.maxSystime.Value(), expectedRange2MaxSystime);
        }
        count++;
    }
    EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);

    cluster.Shutdown();
}

TEST_F(TestElectricalPowerMeasurementCluster, ReadHarmonicAttributes)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // Test values for harmonic currents
    const uint8_t expectedCurrent1Order = 1;
    const int64_t expectedCurrent1Value = 1000;
    const uint8_t expectedCurrent2Order = 3;
    const int64_t expectedCurrent2Value = 500;

    // Test values for harmonic phases
    const uint8_t expectedPhase1Order = 1;
    const int64_t expectedPhase1Value = 0;
    const uint8_t expectedPhase2Order = 3;
    const int64_t expectedPhase2Value = 45;

    // Add harmonic current measurements
    Structs::HarmonicMeasurementStruct::Type current1;
    current1.order = expectedCurrent1Order;
    current1.measurement.SetNonNull(expectedCurrent1Value);
    delegate.AddHarmonicCurrent(current1);

    Structs::HarmonicMeasurementStruct::Type current2;
    current2.order = expectedCurrent2Order;
    current2.measurement.SetNonNull(expectedCurrent2Value);
    delegate.AddHarmonicCurrent(current2);

    // Add harmonic phase measurements
    Structs::HarmonicMeasurementStruct::Type phase1;
    phase1.order = expectedPhase1Order;
    phase1.measurement.SetNonNull(expectedPhase1Value);
    delegate.AddHarmonicPhase(phase1);

    Structs::HarmonicMeasurementStruct::Type phase2;
    phase2.order = expectedPhase2Order;
    phase2.measurement.SetNonNull(expectedPhase2Value);
    delegate.AddHarmonicPhase(phase2);

    ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = delegate,
        .features           = BitMask<Feature>(Feature::kAlternatingCurrent, Feature::kHarmonics, Feature::kPowerQuality),
        .optionalAttributes = ElectricalPowerMeasurementCluster::OptionalAttributesSet(),
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    // Read and verify HarmonicCurrents
    {
        HarmonicCurrents::TypeInfo::DecodableType harmonicCurrents;
        EXPECT_EQ(tester.ReadAttribute(HarmonicCurrents::Id, harmonicCurrents), CHIP_NO_ERROR);
        EXPECT_FALSE(harmonicCurrents.IsNull());

        size_t count = 0;
        auto iter    = harmonicCurrents.Value().begin();
        while (iter.Next())
        {
            const auto & current = iter.GetValue();
            if (count == 0)
            {
                EXPECT_EQ(current.order, expectedCurrent1Order);
                EXPECT_FALSE(current.measurement.IsNull());
                EXPECT_EQ(current.measurement.Value(), expectedCurrent1Value);
            }
            else if (count == 1)
            {
                EXPECT_EQ(current.order, expectedCurrent2Order);
                EXPECT_FALSE(current.measurement.IsNull());
                EXPECT_EQ(current.measurement.Value(), expectedCurrent2Value);
            }
            count++;
        }
        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(count, 2u);
    }

    // Read and verify HarmonicPhases
    {
        HarmonicPhases::TypeInfo::DecodableType harmonicPhases;
        EXPECT_EQ(tester.ReadAttribute(HarmonicPhases::Id, harmonicPhases), CHIP_NO_ERROR);
        EXPECT_FALSE(harmonicPhases.IsNull());

        size_t count = 0;
        auto iter    = harmonicPhases.Value().begin();
        while (iter.Next())
        {
            const auto & phase = iter.GetValue();
            if (count == 0)
            {
                EXPECT_EQ(phase.order, expectedPhase1Order);
                EXPECT_FALSE(phase.measurement.IsNull());
                EXPECT_EQ(phase.measurement.Value(), expectedPhase1Value);
            }
            else if (count == 1)
            {
                EXPECT_EQ(phase.order, expectedPhase2Order);
                EXPECT_FALSE(phase.measurement.IsNull());
                EXPECT_EQ(phase.measurement.Value(), expectedPhase2Value);
            }
            count++;
        }
        EXPECT_EQ(iter.GetStatus(), CHIP_NO_ERROR);
        EXPECT_EQ(count, 2u);
    }

    cluster.Shutdown();
}

TEST_F(TestElectricalPowerMeasurementCluster, ReadPolyphaseAttributes)
{
    chip::Test::TestServerClusterContext context;
    MockElectricalPowerMeasurementDelegate delegate;

    // Test values
    const int64_t expectedReactiveCurrent = 5000;
    const int64_t expectedRMSVoltage      = 120000;
    const int64_t expectedFrequency       = 60;
    const int64_t expectedPowerFactor     = 9230;
    const int64_t expectedNeutralCurrent  = 2000;

    delegate.SetReactiveCurrent(DataModel::Nullable<int64_t>(expectedReactiveCurrent));
    delegate.SetRMSVoltage(DataModel::Nullable<int64_t>(expectedRMSVoltage));
    delegate.SetFrequency(DataModel::Nullable<int64_t>(expectedFrequency));
    delegate.SetPowerFactor(DataModel::Nullable<int64_t>(expectedPowerFactor));
    delegate.SetNeutralCurrent(DataModel::Nullable<int64_t>(expectedNeutralCurrent));

    ElectricalPowerMeasurementCluster::OptionalAttributesSet optionalAttrs;
    optionalAttrs.Set<Attributes::ReactiveCurrent::Id>();
    optionalAttrs.Set<Attributes::RMSVoltage::Id>();
    optionalAttrs.Set<Attributes::Frequency::Id>();
    optionalAttrs.Set<Attributes::PowerFactor::Id>();
    optionalAttrs.Set<Attributes::NeutralCurrent::Id>();

    ElectricalPowerMeasurementCluster cluster(ElectricalPowerMeasurementCluster::Config{
        .endpointId         = kTestEndpointId,
        .delegate           = delegate,
        .features           = BitMask<Feature>(Feature::kAlternatingCurrent, Feature::kPolyphasePower),
        .optionalAttributes = optionalAttrs,
    });

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    DataModel::Nullable<int64_t> value;

    EXPECT_EQ(tester.ReadAttribute(ReactiveCurrent::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value.Value(), expectedReactiveCurrent);

    EXPECT_EQ(tester.ReadAttribute(RMSVoltage::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value.Value(), expectedRMSVoltage);

    EXPECT_EQ(tester.ReadAttribute(Frequency::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value.Value(), expectedFrequency);

    EXPECT_EQ(tester.ReadAttribute(PowerFactor::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value.Value(), expectedPowerFactor);

    EXPECT_EQ(tester.ReadAttribute(NeutralCurrent::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value.Value(), expectedNeutralCurrent);

    cluster.Shutdown();
}

} // namespace
