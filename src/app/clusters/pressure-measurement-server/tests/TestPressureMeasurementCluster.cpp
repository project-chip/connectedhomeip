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

TEST_F(TestPressureMeasurementCluster, MeasuredValue)
{
    PressureMeasurementCluster::Config config;
    config.minMeasuredValue.SetNonNull(-10);
    config.maxMeasuredValue.SetNonNull(10);
    PressureMeasurementCluster cluster(kRootEndpointId, config);
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

    // -32768 is outside configured range (-10 to 10) — rejected
    measuredValue.SetNonNull(-32768);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPressureMeasurementCluster, MeasuredValueNullRange)
{
    // Both min and max null — any valid value accepted
    PressureMeasurementCluster cluster(kRootEndpointId);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> measuredValue{};

    measuredValue.SetNonNull(-32767);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

    measuredValue.SetNonNull(32767);
    EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);

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

    // Read all scaled attributes
    DataModel::Nullable<int16_t> scaledValue{};
    ASSERT_EQ(tester.ReadAttribute(ScaledValue::Id, scaledValue), CHIP_NO_ERROR);

    DataModel::Nullable<int16_t> minScaledValue{};
    ASSERT_EQ(tester.ReadAttribute(MinScaledValue::Id, minScaledValue), CHIP_NO_ERROR);
    EXPECT_EQ(minScaledValue.Value(), -100);

    DataModel::Nullable<int16_t> maxScaledValue{};
    ASSERT_EQ(tester.ReadAttribute(MaxScaledValue::Id, maxScaledValue), CHIP_NO_ERROR);
    EXPECT_EQ(maxScaledValue.Value(), 10000);

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

TEST_F(TestPressureMeasurementCluster, ScaledToleranceBeforeExtendedFeature)
{
    // WithScaledTolerance before WithExtendedFeature — order should not matter
    PressureMeasurementCluster::Config config;
    config.minMeasuredValue.SetNonNull(30);
    config.maxMeasuredValue.SetNonNull(10000);
    config.WithScaledTolerance(50);
    config.WithExtendedFeature(DataModel::Nullable<int16_t>(-100), DataModel::Nullable<int16_t>(10000), -1);
    PressureMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 1u);

    uint16_t scaledTolerance{};
    ASSERT_EQ(tester.ReadAttribute(ScaledTolerance::Id, scaledTolerance), CHIP_NO_ERROR);
    EXPECT_EQ(scaledTolerance, 50);

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

    // SetScaledValue without EXT feature should fail
    DataModel::Nullable<int16_t> sv;
    sv.SetNonNull(100);
    EXPECT_EQ(cluster.SetScaledValue(sv), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
