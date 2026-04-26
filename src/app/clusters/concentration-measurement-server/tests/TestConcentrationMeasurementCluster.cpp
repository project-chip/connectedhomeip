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

#include <pw_unit_test/framework.h>

#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;
using namespace chip::app::Clusters::ConcentrationMeasurement::Attributes;
using namespace chip::Testing;

static constexpr ClusterId kTestClusterId = CarbonDioxideConcentrationMeasurement::Id;

struct TestConcentrationMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
};

// ── Feature auto-promotion ────────────────────────────────────────────────────

TEST_F(TestConcentrationMeasurementCluster, PeakMeasurementImpliesNumeric)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kPeakMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.HasFeature(Feature::kNumericMeasurement));
    EXPECT_TRUE(cluster.HasFeature(Feature::kPeakMeasurement));
}

TEST_F(TestConcentrationMeasurementCluster, AverageMeasurementImpliesNumeric)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kAverageMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.HasFeature(Feature::kNumericMeasurement));
    EXPECT_TRUE(cluster.HasFeature(Feature::kAverageMeasurement));
}

TEST_F(TestConcentrationMeasurementCluster, MediumLevelImpliesLevelIndication)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kMediumLevel), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.HasFeature(Feature::kLevelIndication));
    EXPECT_TRUE(cluster.HasFeature(Feature::kMediumLevel));
}

TEST_F(TestConcentrationMeasurementCluster, CriticalLevelImpliesLevelIndication)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kCriticalLevel), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.HasFeature(Feature::kLevelIndication));
    EXPECT_TRUE(cluster.HasFeature(Feature::kCriticalLevel));
}

// ── Read path: constructor-fixed attributes ───────────────────────────────────
// Verifies that values supplied at construction time are correctly returned by ReadAttribute.

TEST_F(TestConcentrationMeasurementCluster, MandatoryAttributesReadable)
{
    DefaultDelegate d(MeasurementMediumEnum::kWater, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>{}, d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    uint16_t revision{};
    EXPECT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);

    uint32_t featureMap{};
    EXPECT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);

    MeasurementMediumEnum medium{};
    EXPECT_EQ(tester.ReadAttribute(MeasurementMedium::Id, medium), CHIP_NO_ERROR);
    EXPECT_EQ(medium, MeasurementMediumEnum::kWater);
}

TEST_F(TestConcentrationMeasurementCluster, NumericFixedAttributesReadable)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpb, DataModel::MakeNullable(0.0f),
                      DataModel::MakeNullable(500.0f), 1.5f);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kNumericMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    DataModel::Nullable<float> minValue{};
    EXPECT_EQ(tester.ReadAttribute(MinMeasuredValue::Id, minValue), CHIP_NO_ERROR);
    ASSERT_FALSE(minValue.IsNull());
    EXPECT_FLOAT_EQ(minValue.Value(), 0.0f);

    DataModel::Nullable<float> maxValue{};
    EXPECT_EQ(tester.ReadAttribute(MaxMeasuredValue::Id, maxValue), CHIP_NO_ERROR);
    ASSERT_FALSE(maxValue.IsNull());
    EXPECT_FLOAT_EQ(maxValue.Value(), 500.0f);

    float uncertainty{};
    EXPECT_EQ(tester.ReadAttribute(Uncertainty::Id, uncertainty), CHIP_NO_ERROR);
    EXPECT_FLOAT_EQ(uncertainty, 1.5f);

    MeasurementUnitEnum unit{};
    EXPECT_EQ(tester.ReadAttribute(MeasurementUnit::Id, unit), CHIP_NO_ERROR);
    EXPECT_EQ(unit, MeasurementUnitEnum::kPpb);
}

// ── Set* write-then-read round-trips ─────────────────────────────────────────
// cluster.Set*() → tester.ReadAttribute() exercises the full path:
//   cluster validation → delegate write → delegate read → TLV encode/decode.

TEST_F(TestConcentrationMeasurementCluster, SetMeasuredValueRoundTrip)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kNumericMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Starts null before any write.
    DataModel::Nullable<float> readback{};
    EXPECT_EQ(tester.ReadAttribute(MeasuredValue::Id, readback), CHIP_NO_ERROR);
    EXPECT_TRUE(readback.IsNull());

    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(200.0f)), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(MeasuredValue::Id, readback), CHIP_NO_ERROR);
    ASSERT_FALSE(readback.IsNull());
    EXPECT_FLOAT_EQ(readback.Value(), 200.0f);

    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::Nullable<float>()), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(MeasuredValue::Id, readback), CHIP_NO_ERROR);
    EXPECT_TRUE(readback.IsNull());
}

TEST_F(TestConcentrationMeasurementCluster, SetPeakRoundTrip)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kPeakMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetPeakMeasuredValue(DataModel::MakeNullable(88.8f)), CHIP_NO_ERROR);
    DataModel::Nullable<float> peakValue{};
    EXPECT_EQ(tester.ReadAttribute(PeakMeasuredValue::Id, peakValue), CHIP_NO_ERROR);
    ASSERT_FALSE(peakValue.IsNull());
    EXPECT_FLOAT_EQ(peakValue.Value(), 88.8f);

    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(1800u), CHIP_NO_ERROR);
    uint32_t peakWindow{};
    EXPECT_EQ(tester.ReadAttribute(PeakMeasuredValueWindow::Id, peakWindow), CHIP_NO_ERROR);
    EXPECT_EQ(peakWindow, 1800u);
}

TEST_F(TestConcentrationMeasurementCluster, SetAverageRoundTrip)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kAverageMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetAverageMeasuredValue(DataModel::MakeNullable(33.3f)), CHIP_NO_ERROR);
    DataModel::Nullable<float> avgValue{};
    EXPECT_EQ(tester.ReadAttribute(AverageMeasuredValue::Id, avgValue), CHIP_NO_ERROR);
    ASSERT_FALSE(avgValue.IsNull());
    EXPECT_FLOAT_EQ(avgValue.Value(), 33.3f);

    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(3600u), CHIP_NO_ERROR);
    uint32_t avgWindow{};
    EXPECT_EQ(tester.ReadAttribute(AverageMeasuredValueWindow::Id, avgWindow), CHIP_NO_ERROR);
    EXPECT_EQ(avgWindow, 3600u);
}

TEST_F(TestConcentrationMeasurementCluster, SetLevelRoundTrip)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(
        kRootEndpointId, kTestClusterId,
        BitFlags<Feature>(Feature::kLevelIndication, Feature::kMediumLevel, Feature::kCriticalLevel), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kMedium), CHIP_NO_ERROR);
    LevelValueEnum level{};
    EXPECT_EQ(tester.ReadAttribute(LevelValue::Id, level), CHIP_NO_ERROR);
    EXPECT_EQ(level, LevelValueEnum::kMedium);
}

// ── Set* feature guards ───────────────────────────────────────────────────────

TEST_F(TestConcentrationMeasurementCluster, SetMeasuredValueRequiresNumericFeature)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kLevelIndication), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestConcentrationMeasurementCluster, SetPeakRequiresPeakFeature)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kNumericMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetPeakMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(60u), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestConcentrationMeasurementCluster, SetAverageRequiresAverageFeature)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kNumericMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetAverageMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(60u), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestConcentrationMeasurementCluster, SetLevelValueRequiresLevelFeature)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kNumericMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

// ── Set* constraint checks ────────────────────────────────────────────────────

TEST_F(TestConcentrationMeasurementCluster, WindowMaxConstraint)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId,
                                            BitFlags<Feature>(Feature::kPeakMeasurement, Feature::kAverageMeasurement), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    constexpr uint32_t kMax = 604800u;

    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(kMax), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(kMax + 1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(kMax), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(kMax + 1), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestConcentrationMeasurementCluster, LevelValueConstraints)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    // kLevelIndication only — no kMediumLevel or kCriticalLevel
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>(Feature::kLevelIndication), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kUnknown), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kMedium), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kCritical), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kUnknownEnumValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

TEST_F(TestConcentrationMeasurementCluster, LevelValueWithMediumAndCriticalFlags)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(
        kRootEndpointId, kTestClusterId,
        BitFlags<Feature>(Feature::kLevelIndication, Feature::kMediumLevel, Feature::kCriticalLevel), d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kUnknown), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kMedium), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kCritical), CHIP_NO_ERROR);
}

// ── Attribute list enumeration ─────────────────────────────────────────────────

TEST_F(TestConcentrationMeasurementCluster, AttributeListNoFeatures)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId, BitFlags<Feature>{}, d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attrs;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, kTestClusterId), attrs), CHIP_NO_ERROR);

    auto buf     = attrs.TakeBuffer();
    auto hasAttr = [&](AttributeId id) {
        for (auto & entry : buf)
        {
            if (entry.attributeId == id)
                return true;
        }
        return false;
    };

    EXPECT_TRUE(hasAttr(MeasurementMedium::Id));
    EXPECT_TRUE(hasAttr(FeatureMap::Id));
    EXPECT_TRUE(hasAttr(ClusterRevision::Id));

    EXPECT_FALSE(hasAttr(MeasuredValue::Id));
    EXPECT_FALSE(hasAttr(PeakMeasuredValue::Id));
    EXPECT_FALSE(hasAttr(AverageMeasuredValue::Id));
    EXPECT_FALSE(hasAttr(LevelValue::Id));
}

TEST_F(TestConcentrationMeasurementCluster, AttributeListAllFeatures)
{
    DefaultDelegate d(MeasurementMediumEnum::kAir, MeasurementUnitEnum::kPpm);
    ConcentrationMeasurementCluster cluster(kRootEndpointId, kTestClusterId,
                                            BitFlags<Feature>(Feature::kNumericMeasurement, Feature::kPeakMeasurement,
                                                              Feature::kAverageMeasurement, Feature::kLevelIndication,
                                                              Feature::kMediumLevel, Feature::kCriticalLevel),
                                            d);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attrs;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, kTestClusterId), attrs), CHIP_NO_ERROR);

    auto buf     = attrs.TakeBuffer();
    auto hasAttr = [&](AttributeId id) {
        for (auto & entry : buf)
        {
            if (entry.attributeId == id)
                return true;
        }
        return false;
    };

    EXPECT_TRUE(hasAttr(MeasurementMedium::Id));
    EXPECT_TRUE(hasAttr(MeasuredValue::Id));
    EXPECT_TRUE(hasAttr(MinMeasuredValue::Id));
    EXPECT_TRUE(hasAttr(MaxMeasuredValue::Id));
    EXPECT_TRUE(hasAttr(Uncertainty::Id));
    EXPECT_TRUE(hasAttr(MeasurementUnit::Id));
    EXPECT_TRUE(hasAttr(PeakMeasuredValue::Id));
    EXPECT_TRUE(hasAttr(PeakMeasuredValueWindow::Id));
    EXPECT_TRUE(hasAttr(AverageMeasuredValue::Id));
    EXPECT_TRUE(hasAttr(AverageMeasuredValueWindow::Id));
    EXPECT_TRUE(hasAttr(LevelValue::Id));
}

} // namespace
