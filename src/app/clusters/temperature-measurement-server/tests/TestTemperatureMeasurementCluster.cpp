/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/temperature-measurement-server/TemperatureMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::TemperatureMeasurement;
using namespace chip::app::Clusters::TemperatureMeasurement::Attributes;
using namespace chip::Testing;

struct TestTemperatureMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestTemperatureMeasurementCluster() {}

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

TEST_F(TestTemperatureMeasurementCluster, AttributeTest)
{
    {
        TemperatureMeasurementCluster cluster(kRootEndpointId, TemperatureMeasurementCluster::OptionalAttributeSet(),
                                              TemperatureMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, TemperatureMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const DataModel::AttributeEntry optionalAttributes[] = { Tolerance::kMetadataEntry };
        TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<Tolerance::Id>();
        TemperatureMeasurementCluster cluster(kRootEndpointId, optionalAttributeSet,
                                              TemperatureMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, TemperatureMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestTemperatureMeasurementCluster, ReadAttributeTest)
{
    {
        TemperatureMeasurementCluster cluster(kRootEndpointId, TemperatureMeasurementCluster::OptionalAttributeSet(),
                                              TemperatureMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        TemperatureMeasurementCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<Tolerance::Id>();
        TemperatureMeasurementCluster cluster(kRootEndpointId, optionalAttributeSet,
                                              TemperatureMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        uint16_t tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestTemperatureMeasurementCluster, MeasuredValue)
{
    TemperatureMeasurementCluster cluster(kRootEndpointId, TemperatureMeasurementCluster::OptionalAttributeSet(),
                                          TemperatureMeasurementCluster::StartupConfiguration{
                                              .minMeasuredValue = DataModel::Nullable<int16_t>(1),
                                              .maxMeasuredValue = DataModel::Nullable<int16_t>(3),
                                          });
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> measuredValue{};
    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue.Value()), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(1);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
    DataModel::Nullable<int16_t> measuredVal = cluster.GetMeasuredValue();
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

    DataModel::Nullable<int16_t> minMeasuredValue{};
    DataModel::Nullable<int16_t> maxMeasuredValue{};
    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue.Value()), CHIP_NO_ERROR);

    measuredValue.SetNonNull(4);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(32766);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(-27316);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    minMeasuredValue.SetNonNull(1);
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(0);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue.Value()), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(32766);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNonNull(3);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(4);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    measuredValue.SetNonNull(-27316);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestTemperatureMeasurementCluster, MeasuredValueRange)
{
    TemperatureMeasurementCluster cluster(kRootEndpointId, TemperatureMeasurementCluster::OptionalAttributeSet(),
                                          TemperatureMeasurementCluster::StartupConfiguration{});
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> minMeasuredValue{};
    DataModel::Nullable<int16_t> maxMeasuredValue{};
    minMeasuredValue.SetNonNull(-27315);
    maxMeasuredValue.SetNonNull(-27314);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    DataModel::Nullable<int16_t> minMeasuredVal = cluster.GetMinMeasuredValue();
    DataModel::Nullable<int16_t> maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal.Value(), minMeasuredValue.Value());
    EXPECT_EQ(maxMeasuredVal.Value(), maxMeasuredValue.Value());

    minMeasuredValue.SetNonNull(32766);
    maxMeasuredValue.SetNonNull(32767);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal.Value(), minMeasuredValue.Value());
    EXPECT_EQ(maxMeasuredVal.Value(), maxMeasuredValue.Value());

    minMeasuredValue.SetNonNull(-27316);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    minMeasuredValue.SetNonNull(32767);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    minMeasuredValue.SetNonNull(32766);
    maxMeasuredValue.SetNonNull(32766);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal, minMeasuredValue);
    EXPECT_EQ(maxMeasuredVal, maxMeasuredValue);

    minMeasuredValue.SetNonNull(32766);
    maxMeasuredValue.SetNull();
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal, minMeasuredValue);
    EXPECT_EQ(maxMeasuredVal, maxMeasuredValue);

    minMeasuredValue.SetNull();
    maxMeasuredValue.SetNonNull(32766);
    EXPECT_EQ(cluster.SetMeasuredValueRange(minMeasuredValue, maxMeasuredValue), CHIP_NO_ERROR);
    minMeasuredVal = cluster.GetMinMeasuredValue();
    maxMeasuredVal = cluster.GetMaxMeasuredValue();
    EXPECT_EQ(minMeasuredVal, minMeasuredValue);
    EXPECT_EQ(maxMeasuredVal, maxMeasuredValue);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
