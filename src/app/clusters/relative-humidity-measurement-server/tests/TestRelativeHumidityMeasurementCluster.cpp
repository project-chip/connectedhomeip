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

#include <app/clusters/relative-humidity-measurement-server/RelativeHumidityMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::RelativeHumidityMeasurement;
using namespace chip::app::Clusters::RelativeHumidityMeasurement::Attributes;
using namespace chip::Testing;

struct TestRelativeHumidityMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
};

void TestMandatoryAttributes(ClusterTester & tester)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, kRevision);

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

// Verify Attributes() returns the correct set for all optional-attribute combinations.
TEST_F(TestRelativeHumidityMeasurementCluster, AttributeListTest)
{
    // No optional attributes
    {
        RelativeHumidityMeasurementCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, RelativeHumidityMeasurement::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With Tolerance optional attribute
    {
        const DataModel::AttributeEntry optionalAttributes[] = { Tolerance::kMetadataEntry };
        RelativeHumidityMeasurementCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<Tolerance::Id>();

        RelativeHumidityMeasurementCluster cluster(kRootEndpointId, RelativeHumidityMeasurementCluster::Config{}.WithTolerance(0));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, RelativeHumidityMeasurement::Id), attributes),
                  CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// Verify ReadAttribute returns correct values for all mandatory attributes.
TEST_F(TestRelativeHumidityMeasurementCluster, ReadAttributeTest)
{
    // No optional attributes
    {
        RelativeHumidityMeasurementCluster cluster(kRootEndpointId);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        TestMandatoryAttributes(tester);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // With Tolerance
    {
        RelativeHumidityMeasurementCluster cluster(kRootEndpointId,
                                                   RelativeHumidityMeasurementCluster::Config{}.WithTolerance(100));
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        TestMandatoryAttributes(tester);

        uint16_t tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);
        EXPECT_EQ(tolerance, 100u);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// Verify SetMeasuredValue enforces range constraints against min/max.
TEST_F(TestRelativeHumidityMeasurementCluster, MeasuredValue)
{
    RelativeHumidityMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable(uint16_t(100));
    config.maxMeasuredValue = DataModel::MakeNullable(uint16_t(9000));
    RelativeHumidityMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    // In-range values succeed
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(100))), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMeasuredValue().Value(), 100u);

    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(5000))), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMeasuredValue().Value(), 5000u);

    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(9000))), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMeasuredValue().Value(), 9000u);

    // Below min → ConstraintError
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(99))), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Above max → ConstraintError
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(9001))), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Above absolute max (10000) → ConstraintError
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(10001))), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Null is always valid
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetMeasuredValue().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Verify constructor stores Config values correctly.
TEST_F(TestRelativeHumidityMeasurementCluster, ConfigValues)
{
    RelativeHumidityMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable(uint16_t(0));
    config.maxMeasuredValue = DataModel::MakeNullable(uint16_t(10000));
    RelativeHumidityMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetMinMeasuredValue().Value(), 0u);
    EXPECT_EQ(cluster.GetMaxMeasuredValue().Value(), 10000u);

    // Boundary: min at spec max, max at spec max
    RelativeHumidityMeasurementCluster::Config config2;
    config2.minMeasuredValue = DataModel::MakeNullable(uint16_t(9999));
    config2.maxMeasuredValue = DataModel::MakeNullable(uint16_t(10000));
    RelativeHumidityMeasurementCluster cluster2(kRootEndpointId, config2);
    ASSERT_EQ(cluster2.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(cluster2.GetMinMeasuredValue().Value(), 9999u);
    EXPECT_EQ(cluster2.GetMaxMeasuredValue().Value(), 10000u);

    // Both null is valid (unknown range)
    RelativeHumidityMeasurementCluster cluster3(kRootEndpointId);
    ASSERT_EQ(cluster3.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster3.GetMinMeasuredValue().IsNull());
    EXPECT_TRUE(cluster3.GetMaxMeasuredValue().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    cluster2.Shutdown(ClusterShutdownType::kClusterShutdown);
    cluster3.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Verify SetMeasuredValue enforces bounds when only one of min/max is set.
TEST_F(TestRelativeHumidityMeasurementCluster, MeasuredValueWithPartialRange)
{
    // Only MinMeasuredValue set (MaxMeasuredValue null)
    {
        RelativeHumidityMeasurementCluster::Config config;
        config.minMeasuredValue = DataModel::MakeNullable(uint16_t(500));
        RelativeHumidityMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        // Below min → ConstraintError
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(499))), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // At min → success
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(500))), CHIP_NO_ERROR);
        // At absolute max → success (no MaxMeasuredValue constraint)
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(10000))), CHIP_NO_ERROR);
        // Above absolute max → ConstraintError
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(10001))), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Only MaxMeasuredValue set (MinMeasuredValue null)
    {
        RelativeHumidityMeasurementCluster::Config config;
        config.maxMeasuredValue = DataModel::MakeNullable(uint16_t(3000));
        RelativeHumidityMeasurementCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        // At max → success
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(3000))), CHIP_NO_ERROR);
        // Above max → ConstraintError
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(3001))), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        // At 0 (no min constraint) → success
        EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(0))), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// Verify tolerance at spec boundary is accepted.
TEST_F(TestRelativeHumidityMeasurementCluster, ToleranceBoundary)
{
    RelativeHumidityMeasurementCluster cluster(kRootEndpointId, RelativeHumidityMeasurementCluster::Config{}.WithTolerance(2048));
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    uint16_t tolerance{};
    ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);
    EXPECT_EQ(tolerance, 2048u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Verify SetMeasuredValue enforces the configured range.
TEST_F(TestRelativeHumidityMeasurementCluster, MeasuredValueRespectsConfiguredRange)
{
    RelativeHumidityMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable(uint16_t(1000));
    config.maxMeasuredValue = DataModel::MakeNullable(uint16_t(2000));
    RelativeHumidityMeasurementCluster cluster(kRootEndpointId, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(999))), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(1000))), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(2000))), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(uint16_t(2001))), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
