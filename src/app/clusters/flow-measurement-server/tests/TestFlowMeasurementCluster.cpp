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

#include <app/clusters/flow-measurement-server/FlowMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FlowMeasurement;
using namespace chip::app::Clusters::FlowMeasurement::Attributes;
using namespace chip::Testing;

// Exposes protected SetMeasuredValueRange for testing
class TestableFlowMeasurementCluster : public FlowMeasurementCluster
{
public:
    using FlowMeasurementCluster::FlowMeasurementCluster;
    using FlowMeasurementCluster::SetMeasuredValueRange;
};

struct TestFlowMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestFlowMeasurementCluster() {}

    TestServerClusterContext testContext;
};

void TestMandatoryAttributes(ClusterTester & tester)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> measuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MeasuredValue::Id, measuredValue), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> minMeasuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MinMeasuredValue::Id, minMeasuredValue), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> maxMeasuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MaxMeasuredValue::Id, maxMeasuredValue), CHIP_NO_ERROR);
}

} // namespace

TEST_F(TestFlowMeasurementCluster, AttributeTest)
{
    {
        FlowMeasurementCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, FlowMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const DataModel::AttributeEntry optionalAttributes[] = { Tolerance::kMetadataEntry };
        FlowMeasurementCluster::Config config;
        config.WithTolerance(100);
        FlowMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, FlowMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), config.mOptionalAttributeSet),
                  CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestFlowMeasurementCluster, ReadAttributeTest)
{
    {
        FlowMeasurementCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        FlowMeasurementCluster::Config config;
        config.WithTolerance(0);
        FlowMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        uint16_t tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestFlowMeasurementCluster, ReadUnsupportedAttribute)
{
    FlowMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Reading an attribute not in the cluster should fail
    uint16_t dummy{};
    EXPECT_NE(tester.ReadAttribute(0xFFF0, dummy), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFlowMeasurementCluster, ConstructorVariants)
{
    // Only min set (max null)
    {
        FlowMeasurementCluster::Config config;
        config.minMeasuredValue.SetNonNull(10);
        FlowMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetMinMeasuredValue().Value(), 10);
        EXPECT_TRUE(cluster.GetMaxMeasuredValue().IsNull());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Only max set (min null)
    {
        FlowMeasurementCluster::Config config;
        config.maxMeasuredValue.SetNonNull(500);
        FlowMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(cluster.GetMinMeasuredValue().IsNull());
        EXPECT_EQ(cluster.GetMaxMeasuredValue().Value(), 500);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With tolerance
    {
        FlowMeasurementCluster::Config config;
        config.minMeasuredValue.SetNonNull(0);
        config.maxMeasuredValue.SetNonNull(2048);
        config.WithTolerance(2048);
        FlowMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        uint16_t tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);
        EXPECT_EQ(tolerance, 2048);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestFlowMeasurementCluster, InvalidRangeDefaultsToNull)
{
    // min == max == 0 is invalid (max must be >= min + 1).
    // SetMeasuredValueRange should reject this.
    TestableFlowMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> min;
    DataModel::Nullable<uint16_t> max;
    min.SetNonNull(0);
    max.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValueRange(min, max), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Verify the range was not changed (still null from default)
    EXPECT_TRUE(cluster.GetMinMeasuredValue().IsNull());
    EXPECT_TRUE(cluster.GetMaxMeasuredValue().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFlowMeasurementCluster, MeasuredValue)
{
    FlowMeasurementCluster::Config config;
    config.minMeasuredValue.SetNonNull(1);
    config.maxMeasuredValue.SetNonNull(3);
    TestableFlowMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> measuredValue{};
    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(1);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    DataModel::Nullable<uint16_t> measuredVal = cluster.GetMeasuredValue();
    EXPECT_EQ(measuredVal.Value(), measuredValue.Value());

    measuredValue.SetNonNull(2);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    measuredVal = cluster.GetMeasuredValue();
    EXPECT_EQ(measuredVal.Value(), measuredValue.Value());

    measuredValue.SetNonNull(3);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    measuredVal = cluster.GetMeasuredValue();
    EXPECT_EQ(measuredVal.Value(), measuredValue.Value());

    measuredValue.SetNonNull(4);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    measuredVal = cluster.GetMeasuredValue();
    EXPECT_EQ(measuredVal, measuredValue);

    DataModel::Nullable<uint16_t> minMeasuredValue{};
    DataModel::Nullable<uint16_t> maxMeasuredValue{};
    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(4);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(65534);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    minMeasuredValue.SetNonNull(1);
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(65534);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNonNull(3);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(4);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestFlowMeasurementCluster, MeasuredValueRange)
{
    TestableFlowMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<uint16_t> minMeasuredValue{};
    DataModel::Nullable<uint16_t> maxMeasuredValue{};
    minMeasuredValue.SetNonNull(0);
    maxMeasuredValue.SetNonNull(1);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    DataModel::Nullable<uint16_t> minMeasuredVal = cluster.GetMinMeasuredValue();
    DataModel::Nullable<uint16_t> maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal.Value(), minMeasuredValue.Value());
    EXPECT_EQ(maxMeasuredVal.Value(), maxMeasuredValue.Value());

    minMeasuredValue.SetNonNull(65533);
    maxMeasuredValue.SetNonNull(65534);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal.Value(), minMeasuredValue.Value());
    EXPECT_EQ(maxMeasuredVal.Value(), maxMeasuredValue.Value());

    // min exceeds max allowed (65533)
    minMeasuredValue.SetNonNull(65534);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // max exceeds max allowed (65534)
    minMeasuredValue.SetNonNull(0);
    maxMeasuredValue.SetNonNull(65535);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // min == max (must be min + 1)
    minMeasuredValue.SetNonNull(100);
    maxMeasuredValue.SetNonNull(100);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // both null
    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal, minMeasuredValue);
    EXPECT_EQ(maxMeasuredVal, maxMeasuredValue);

    // only min set
    minMeasuredValue.SetNonNull(65533);
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal, minMeasuredValue);
    EXPECT_EQ(maxMeasuredVal, maxMeasuredValue);

    // only max set
    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNonNull(65534);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal, minMeasuredValue);
    EXPECT_EQ(maxMeasuredVal, maxMeasuredValue);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
