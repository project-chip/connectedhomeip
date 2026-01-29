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

#include <app/clusters/illuminance-measurement-server/IlluminanceMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/IlluminanceMeasurement/Attributes.h>
#include <clusters/IlluminanceMeasurement/Enums.h>
#include <clusters/IlluminanceMeasurement/Metadata.h>
#include <clusters/IlluminanceMeasurement/Structs.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IlluminanceMeasurement;
using namespace chip::app::Clusters::IlluminanceMeasurement::Attributes;
using namespace chip::Testing;

struct TestIlluminanceMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestIlluminanceMeasurementCluster() {}

    TestServerClusterContext testContext;
};

} // namespace

TEST_F(TestIlluminanceMeasurementCluster, AttributeTest)
{
    {
        IlluminanceMeasurementCluster cluster(kRootEndpointId, IlluminanceMeasurementCluster::OptionalAttributeSet(),
                                              IlluminanceMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, IlluminanceMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), {}), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        const DataModel::AttributeEntry optionalAttributes[] = { Tolerance::kMetadataEntry, LightSensorType::kMetadataEntry };
        IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<Tolerance::Id>();
        optionalAttributeSet.Set<LightSensorType::Id>();
        IlluminanceMeasurementCluster cluster(kRootEndpointId, optionalAttributeSet,
                                              IlluminanceMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
        ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, IlluminanceMeasurement::Id), attributes), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
        AttributeListBuilder listBuilder(expected);
        ASSERT_EQ(listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes), optionalAttributeSet), CHIP_NO_ERROR);
        ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

void TestMandatoryAttributes(ClusterTester & tester)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    MeasuredValue::TypeInfo::Type measuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MeasuredValue::Id, measuredValue), CHIP_NO_ERROR);

    MinMeasuredValue::TypeInfo::Type minMeasuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MinMeasuredValue::Id, minMeasuredValue), CHIP_NO_ERROR);

    MaxMeasuredValue::TypeInfo::Type maxMeasuredValue{};
    ASSERT_EQ(tester.ReadAttribute(MaxMeasuredValue::Id, maxMeasuredValue), CHIP_NO_ERROR);
}

TEST_F(TestIlluminanceMeasurementCluster, ReadAttributeTest)
{
    {
        IlluminanceMeasurementCluster cluster(kRootEndpointId, IlluminanceMeasurementCluster::OptionalAttributeSet(),
                                              IlluminanceMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        IlluminanceMeasurementCluster::OptionalAttributeSet optionalAttributeSet;
        optionalAttributeSet.Set<Tolerance::Id>();
        optionalAttributeSet.Set<LightSensorType::Id>();
        IlluminanceMeasurementCluster cluster(kRootEndpointId, optionalAttributeSet,
                                              IlluminanceMeasurementCluster::StartupConfiguration{});
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester);

        Tolerance::TypeInfo::Type tolerance{};
        ASSERT_EQ(tester.ReadAttribute(Tolerance::Id, tolerance), CHIP_NO_ERROR);

        LightSensorType::TypeInfo::Type lightSensorType{};
        ASSERT_EQ(tester.ReadAttribute(LightSensorType::Id, lightSensorType), CHIP_NO_ERROR);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestIlluminanceMeasurementCluster, MeasuredValue)
{
    {
        IlluminanceMeasurementCluster cluster(kRootEndpointId, IlluminanceMeasurementCluster::OptionalAttributeSet(),
                                              IlluminanceMeasurementCluster::StartupConfiguration{
                                                  .minMeasuredValue = 1,
                                                  .maxMeasuredValue = 3,
                                              });
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        MeasuredValue::TypeInfo::Type measuredValue;
        EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_ERROR_INVALID_ARGUMENT);

        measuredValue.SetNonNull(0);
        EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
        MeasuredValue::TypeInfo::Type measuredVal = cluster.GetMeasuredValue();
        EXPECT_EQ(measuredVal.Value(), measuredValue.Value());

        measuredValue.SetNonNull(1);
        EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_NO_ERROR);
        measuredVal = cluster.GetMeasuredValue();
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
        EXPECT_EQ(cluster.SetMeasuredValue(measuredValue), CHIP_ERROR_INVALID_ARGUMENT);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
