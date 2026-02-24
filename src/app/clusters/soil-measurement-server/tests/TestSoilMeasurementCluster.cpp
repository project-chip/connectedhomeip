/*
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

#include <pw_unit_test/framework.h>

#include <app/clusters/soil-measurement-server/SoilMeasurementCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/SoilMeasurement/Attributes.h>
#include <clusters/SoilMeasurement/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SoilMeasurement;
using namespace chip::app::Clusters::SoilMeasurement::Attributes;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

const Globals::Structs::MeasurementAccuracyRangeStruct::Type kDefaultSoilMoistureMeasurementLimitsAccuracyRange[] = {
    { .rangeMin = 0, .rangeMax = 100, .percentMax = MakeOptional(static_cast<chip::Percent100ths>(10)) }
};

const SoilMoistureMeasurementLimits::TypeInfo::Type kDefaultSoilMoistureMeasurementLimits = {
    .measurementType  = Globals::MeasurementTypeEnum::kSoilMoisture,
    .measured         = true,
    .minMeasuredValue = 0,
    .maxMeasuredValue = 100,
    .accuracyRanges   = DataModel::List<const Globals::Structs::MeasurementAccuracyRangeStruct::Type>(
        kDefaultSoilMoistureMeasurementLimitsAccuracyRange)
};

constexpr EndpointId kEndpointWithSoilMeasurement = 1;

class SoilMeasurementClusterLocal : public SoilMeasurementCluster
{
public:
    using SoilMeasurementCluster::SoilMeasurementCluster;

    SoilMoistureMeasurementLimits::TypeInfo::Type GetSoilMoistureMeasurementLimits() const
    {
        return mSoilMoistureMeasurementLimits;
    }

    SoilMoistureMeasuredValue::TypeInfo::Type GetSoilMoistureMeasuredValue() const { return mSoilMoistureMeasuredValue; }
};

struct TestSoilMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(soilMeasurement.Startup(testContext.Get()), CHIP_NO_ERROR); }

    void TearDown() override { soilMeasurement.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestSoilMeasurementCluster() : soilMeasurement(kEndpointWithSoilMeasurement, kDefaultSoilMoistureMeasurementLimits) {}

    TestServerClusterContext testContext;
    SoilMeasurementClusterLocal soilMeasurement;
};

} // namespace

TEST_F(TestSoilMeasurementCluster, AttributeTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(soilMeasurement,
                                        {
                                            SoilMeasurement::Attributes::SoilMoistureMeasurementLimits::kMetadataEntry,
                                            SoilMeasurement::Attributes::SoilMoistureMeasuredValue::kMetadataEntry,
                                        }));
}

TEST_F(TestSoilMeasurementCluster, ReadAttributeTest)
{
    ClusterTester tester(soilMeasurement);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    SoilMoistureMeasuredValue::TypeInfo::DecodableType soilMoistureMeasuredValue;
    ASSERT_EQ(tester.ReadAttribute(SoilMoistureMeasuredValue::Id, soilMoistureMeasuredValue), CHIP_NO_ERROR);

    SoilMoistureMeasurementLimits::TypeInfo::DecodableType soilMoistureMeasurementLimits;
    ASSERT_EQ(tester.ReadAttribute(SoilMoistureMeasurementLimits::Id, soilMoistureMeasurementLimits), CHIP_NO_ERROR);
}

TEST_F(TestSoilMeasurementCluster, SoilMoistureMeasuredValue)
{
    SoilMoistureMeasuredValue::TypeInfo::Type measuredValue;
    measuredValue.SetNull();
    ASSERT_EQ(soilMeasurement.GetSoilMoistureMeasuredValue(), measuredValue);

    measuredValue = 50;
    ASSERT_EQ(soilMeasurement.SetSoilMoistureMeasuredValue(measuredValue), CHIP_NO_ERROR);
    ASSERT_EQ(soilMeasurement.GetSoilMoistureMeasuredValue(), measuredValue);

    measuredValue = 101;
    ASSERT_EQ(soilMeasurement.SetSoilMoistureMeasuredValue(measuredValue), CHIP_ERROR_INVALID_ARGUMENT);

    measuredValue = -1;
    ASSERT_EQ(soilMeasurement.SetSoilMoistureMeasuredValue(measuredValue), CHIP_ERROR_INVALID_ARGUMENT);

    measuredValue.SetNull();
    ASSERT_EQ(soilMeasurement.SetSoilMoistureMeasuredValue(measuredValue), CHIP_NO_ERROR);
    ASSERT_EQ(soilMeasurement.GetSoilMoistureMeasuredValue(), measuredValue);
}

TEST_F(TestSoilMeasurementCluster, SoilMoistureMeasurementLimits)
{
    const auto & measurementLimits = soilMeasurement.GetSoilMoistureMeasurementLimits();
    ASSERT_EQ(measurementLimits.measurementType, kDefaultSoilMoistureMeasurementLimits.measurementType);
    ASSERT_EQ(measurementLimits.measured, kDefaultSoilMoistureMeasurementLimits.measured);
    ASSERT_EQ(measurementLimits.minMeasuredValue, kDefaultSoilMoistureMeasurementLimits.minMeasuredValue);
    ASSERT_EQ(measurementLimits.maxMeasuredValue, kDefaultSoilMoistureMeasurementLimits.maxMeasuredValue);

    const auto & accuracyRange = measurementLimits.accuracyRanges[0];
    const auto & defaultRange  = kDefaultSoilMoistureMeasurementLimits.accuracyRanges[0];
    ASSERT_EQ(accuracyRange.rangeMin, defaultRange.rangeMin);
    ASSERT_EQ(accuracyRange.rangeMax, defaultRange.rangeMax);
    ASSERT_EQ(accuracyRange.percentMax.Value(), defaultRange.percentMax.Value());
    ASSERT_FALSE(accuracyRange.percentMin.HasValue());
    ASSERT_FALSE(accuracyRange.percentTypical.HasValue());
    ASSERT_FALSE(accuracyRange.fixedMax.HasValue());
    ASSERT_FALSE(accuracyRange.fixedMin.HasValue());
    ASSERT_FALSE(accuracyRange.fixedTypical.HasValue());
}
