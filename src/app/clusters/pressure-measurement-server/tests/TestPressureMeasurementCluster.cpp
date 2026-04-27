/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/pressure-measurement-server/PressureMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PressureMeasurement;
using namespace chip::app::Clusters::PressureMeasurement::Attributes;
using namespace chip::Testing;

// Exposes protected SetMeasuredValueRange for testing
class TestablePressureMeasurementCluster : public PressureMeasurementCluster
{
public:
    using PressureMeasurementCluster::PressureMeasurementCluster;
    using PressureMeasurementCluster::SetMeasuredValueRange;
};

struct TestPressureMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestPressureMeasurementCluster() {}

    TestServerClusterContext testContext;
};

void TestMandatoryAttributes(ClusterTester & tester)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> measuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MeasuredValue::Id, measuredValue), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> minMeasuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MinMeasuredValue::Id, minMeasuredValue), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> maxMeasuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MaxMeasuredValue::Id, maxMeasuredValue), CHIP_NO_ERROR);
}

} // namespace

TEST_F(TestPressureMeasurementCluster, AttributeTest)
{
    {
        PressureMeasurementCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, PressureMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const DataModel::AttributeEntry optionalAttributes[] = { Tolerance::kMetadataEntry };
        PressureMeasurementCluster::Config config;
        config.WithTolerance(100);
        PressureMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, PressureMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), config.mOptionalAttributeSet),
                  CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPressureMeasurementCluster, ReadAttributeTest)
{
    {
        PressureMeasurementCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        PressureMeasurementCluster::Config config;
        config.WithTolerance(0);
        PressureMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        uint16_t tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPressureMeasurementCluster, ReadUnsupportedAttribute)
{
    PressureMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t dummy{};
    EXPECT_NE(tester.ReadAttribute(0xFFF0, dummy), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPressureMeasurementCluster, ConstructorVariants)
{
    // Only min set (max null)
    {
        PressureMeasurementCluster::Config config;
        config.minMeasuredValue.SetNonNull(10);
        PressureMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetMinMeasuredValue().Value(), 10);
        EXPECT_TRUE(cluster.GetMaxMeasuredValue().IsNull());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Only max set (min null)
    {
        PressureMeasurementCluster::Config config;
        config.maxMeasuredValue.SetNonNull(500);
        PressureMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(cluster.GetMinMeasuredValue().IsNull());
        EXPECT_EQ(cluster.GetMaxMeasuredValue().Value(), 500);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With tolerance
    {
        PressureMeasurementCluster::Config config;
        config.minMeasuredValue.SetNonNull(0);
        config.maxMeasuredValue.SetNonNull(2048);
        config.WithTolerance(2048);
        PressureMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        uint16_t tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);
        EXPECT_EQ(tolerance, 2048);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Negative min value (pressure can be negative)
    {
        PressureMeasurementCluster::Config config;
        config.minMeasuredValue.SetNonNull(-100);
        config.maxMeasuredValue.SetNonNull(500);
        PressureMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetMinMeasuredValue().Value(), -100);
        EXPECT_EQ(cluster.GetMaxMeasuredValue().Value(), 500);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPressureMeasurementCluster, InvalidRangeDefaultsToNull)
{
    // min == max == 0 is invalid (max must be >= min + 1).
    TestablePressureMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> min;
    DataModel::Nullable<int16_t> max;
    min.SetNonNull(0);
    max.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_TRUE(cluster.GetMinMeasuredValue().IsNull());
    EXPECT_TRUE(cluster.GetMaxMeasuredValue().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPressureMeasurementCluster, MeasuredValue)
{
    PressureMeasurementCluster::Config config;
    config.minMeasuredValue.SetNonNull(-10);
    config.maxMeasuredValue.SetNonNull(10);
    TestablePressureMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> measuredValue{};

    // Below min — rejected
    measuredValue.SetNonNull(-11);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // At min — accepted
    measuredValue.SetNonNull(-10);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMeasuredValue().Value(), -10);

    // Zero — accepted
    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMeasuredValue().Value(), 0);

    // At max — accepted
    measuredValue.SetNonNull(10);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMeasuredValue().Value(), 10);

    // Above max — rejected
    measuredValue.SetNonNull(11);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Null — always accepted
    measuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetMeasuredValue().IsNull());

    // Null range — any value accepted
    DataModel::Nullable<int16_t> nullMin, nullMax;
    nullMin.SetNull();
    nullMax.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(nullMin, nullMax), CHIP_NO_ERROR);

    measuredValue.SetNonNull(-32766);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(32766);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    // Only min set
    DataModel::Nullable<int16_t> minOnly, maxNull;
    minOnly.SetNonNull(100);
    maxNull.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minOnly, maxNull), CHIP_NO_ERROR);

    measuredValue.SetNonNull(99);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(32766);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    // Only max set
    DataModel::Nullable<int16_t> minNull, maxOnly;
    minNull.SetNull();
    maxOnly.SetNonNull(200);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minNull, maxOnly), CHIP_NO_ERROR);

    measuredValue.SetNonNull(201);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(-32766);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPressureMeasurementCluster, MeasuredValueRange)
{
    TestablePressureMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> min, max;

    // Valid range
    min.SetNonNull(-100);
    max.SetNonNull(100);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMinMeasuredValue().Value(), -100);
    EXPECT_EQ(cluster.GetMaxMeasuredValue().Value(), 100);

    // Max spec bounds
    min.SetNonNull(32766);
    max.SetNonNull(32767);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_NO_ERROR);

    // min exceeds max allowed (32766)
    min.SetNonNull(32767);
    max.SetNonNull(32767);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // min == max (must be min + 1)
    min.SetNonNull(100);
    max.SetNonNull(100);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Both null
    min.SetNull();
    max.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetMinMeasuredValue().IsNull());
    EXPECT_TRUE(cluster.GetMaxMeasuredValue().IsNull());

    // Only min set
    min.SetNonNull(-500);
    max.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_NO_ERROR);

    // Only max set
    min.SetNull();
    max.SetNonNull(500);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPressureMeasurementCluster, ScaledValueEXT)
{
    PressureMeasurementCluster::Config config;
    config.minMeasuredValue.SetNonNull(30);
    config.maxMeasuredValue.SetNonNull(10000);
    config.WithExtendedFeature(DataModel::Nullable<int16_t>(-100), DataModel::Nullable<int16_t>(10000), -1);
    config.WithScaledTolerance(10);
    PressureMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // FeatureMap should have EXT bit set
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 1u);

    // Read scaled attributes
    DataModel::Nullable<int16_t> scaledValue{};
    ASSERT_EQ(tester.ReadAttribute(ScaledValue::Id, scaledValue), CHIP_NO_ERROR);

    int8_t scale{};
    ASSERT_EQ(tester.ReadAttribute(Scale::Id, scale), CHIP_NO_ERROR);
    EXPECT_EQ(scale, -1);

    uint16_t scaledTolerance{};
    ASSERT_EQ(tester.ReadAttribute(ScaledTolerance::Id, scaledTolerance), CHIP_NO_ERROR);
    EXPECT_EQ(scaledTolerance, 10);

    // Set scaled value within range
    DataModel::Nullable<int16_t> sv;
    sv.SetNonNull(5000);
    EXPECT_EQ(cluster.SetScaledValue(sv), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetScaledValue().Value(), 5000);

    // Out of range
    sv.SetNonNull(10001);
    EXPECT_EQ(cluster.SetScaledValue(sv), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Null — accepted
    sv.SetNull();
    EXPECT_EQ(cluster.SetScaledValue(sv), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetScaledValue().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPressureMeasurementCluster, FeatureMapWithoutEXT)
{
    PressureMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
