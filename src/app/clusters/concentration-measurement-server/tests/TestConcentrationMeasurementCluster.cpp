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

#include <app/clusters/concentration-measurement-server/ConcentrationMeasurementCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/AttributeIds.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Enums.h>
#include <clusters/CarbonDioxideConcentrationMeasurement/Metadata.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ConcentrationMeasurement;
using namespace chip::Testing;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr ClusterId kTestClusterId   = CarbonDioxideConcentrationMeasurement::Id;
constexpr EndpointId kTestEndpointId = kRootEndpointId;
constexpr float kTestMin             = 0.0f;
constexpr float kTestMax             = 100.0f;
constexpr float kTestUncertainty     = 0.5f;

ConcentrationMeasurementCluster::Config MakeNumericConfig(BitFlags<Feature> extraFeatures = {})
{
    BitFlags<Feature> features(Feature::kNumericMeasurement);
    features.SetRaw(features.Raw() | extraFeatures.Raw());
    return {
        kTestClusterId,
        features,
        MeasurementMediumEnum::kAir,
        MeasurementUnitEnum::kPpm,
        DataModel::MakeNullable(kTestMin),
        DataModel::MakeNullable(kTestMax),
        kTestUncertainty,
    };
}

ConcentrationMeasurementCluster::Config MakeLevelConfig(BitFlags<Feature> levelFeatures = {})
{
    BitFlags<Feature> features(Feature::kLevelIndication);
    features.SetRaw(features.Raw() | levelFeatures.Raw());
    return {
        kTestClusterId,
        features,
        MeasurementMediumEnum::kAir,
        MeasurementUnitEnum::kUnknownEnumValue,
    };
}

// Fixture: numeric measurement cluster (NUM + PEAK + AVG).
// ClusterTester is held as a member so the cluster is started with its context,
// ensuring SetAttributeValue notifications reach the tester's dirty list.

struct TestNumericMeasurementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestNumericMeasurementCluster() :
        cluster(kTestEndpointId, MakeNumericConfig(BitFlags<Feature>(Feature::kPeakMeasurement, Feature::kAverageMeasurement)))
    {}

    ConcentrationMeasurementCluster cluster;
    ClusterTester tester{ cluster };
};

// Fixture: level-indication cluster (LEV + MEDIUM + CRITICAL).

struct TestLevelIndicationCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestLevelIndicationCluster() :
        cluster(kTestEndpointId, MakeLevelConfig(BitFlags<Feature>(Feature::kMediumLevel, Feature::kCriticalLevel)))
    {}

    ConcentrationMeasurementCluster cluster;
    ClusterTester tester{ cluster };
};

} // namespace

// Attribute list composition

TEST_F(TestNumericMeasurementCluster, AttributeList_NumericPeakAverage)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::MeasurementMedium::kMetadataEntry,
                                            Attributes::MeasuredValue::kMetadataEntry,
                                            Attributes::MinMeasuredValue::kMetadataEntry,
                                            Attributes::MaxMeasuredValue::kMetadataEntry,
                                            Attributes::Uncertainty::kMetadataEntry,
                                            Attributes::MeasurementUnit::kMetadataEntry,
                                            Attributes::PeakMeasuredValue::kMetadataEntry,
                                            Attributes::PeakMeasuredValueWindow::kMetadataEntry,
                                            Attributes::AverageMeasuredValue::kMetadataEntry,
                                            Attributes::AverageMeasuredValueWindow::kMetadataEntry,
                                        }));
}

TEST_F(TestLevelIndicationCluster, AttributeList_LevelIndication)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::MeasurementMedium::kMetadataEntry,
                                            Attributes::LevelValue::kMetadataEntry,
                                        }));
}

// ReadAttribute

TEST_F(TestNumericMeasurementCluster, ReadAttributes_FeatureMapAndRevision)
{
    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
    // kNumericMeasurement | kPeakMeasurement | kAverageMeasurement
    const uint32_t expected = to_underlying(Feature::kNumericMeasurement) | to_underlying(Feature::kPeakMeasurement) |
        to_underlying(Feature::kAverageMeasurement);
    EXPECT_EQ(features, expected);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, CarbonDioxideConcentrationMeasurement::kRevision);
}

TEST_F(TestNumericMeasurementCluster, ReadAttributes_MediumUnitAndUncertainty)
{
    uint8_t medium{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::MeasurementMedium::Id, medium), CHIP_NO_ERROR);
    EXPECT_EQ(medium, to_underlying(MeasurementMediumEnum::kAir));

    uint8_t unit{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::MeasurementUnit::Id, unit), CHIP_NO_ERROR);
    EXPECT_EQ(unit, to_underlying(MeasurementUnitEnum::kPpm));

    float uncertainty{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::Uncertainty::Id, uncertainty), CHIP_NO_ERROR);
    EXPECT_FLOAT_EQ(uncertainty, kTestUncertainty);
}

TEST_F(TestNumericMeasurementCluster, ReadAttributeAfterSet)
{
    ASSERT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(50.0f)), CHIP_NO_ERROR);

    DataModel::Nullable<float> val;
    ASSERT_EQ(tester.ReadAttribute(Attributes::MeasuredValue::Id, val), CHIP_NO_ERROR);
    ASSERT_FALSE(val.IsNull());
    EXPECT_FLOAT_EQ(val.Value(), 50.0f);

    // Setting to null and reading back
    ASSERT_EQ(cluster.SetMeasuredValue(DataModel::Nullable<float>()), CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(Attributes::MeasuredValue::Id, val), CHIP_NO_ERROR);
    EXPECT_TRUE(val.IsNull());
}

// Feature gating — all numeric setters rejected on a level-only cluster

TEST_F(TestLevelIndicationCluster, NumericSetters_RejectedOnLevelOnlyCluster)
{
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetPeakMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetAverageMeasuredValue(DataModel::MakeNullable(1.0f)), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(100u), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(100u), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

// SetMeasuredValue — range validation including null bypass

TEST_F(TestNumericMeasurementCluster, SetMeasuredValue_RangeValidation)
{
    // In-range: strictly between min (0) and max (100).
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(50.0f)), CHIP_NO_ERROR);

    // Boundary — strict comparison, so exactly min/max is out of range.
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(kTestMin)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(kTestMax)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Out of range.
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(-1.0f)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(200.0f)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    // Null always passes regardless of configured bounds.
    EXPECT_EQ(cluster.SetMeasuredValue(DataModel::Nullable<float>()), CHIP_NO_ERROR);
}

// SetPeakMeasuredValueWindow / SetAverageMeasuredValueWindow

TEST_F(TestNumericMeasurementCluster, SetWindowValue_MaxSecondsEnforced)
{
    constexpr uint32_t kMax = 604800u;

    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(kMax), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetPeakMeasuredValueWindow(kMax + 1), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(kMax), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetAverageMeasuredValueWindow(kMax + 1), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// SetLevelValue — feature gating and sub-feature constraints

TEST_F(TestNumericMeasurementCluster, SetLevelValue_RequiresLevelIndication)
{
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestLevelIndicationCluster, SetLevelValue_BasicValues)
{
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kUnknown), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kMedium), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kCritical), CHIP_NO_ERROR);
}

TEST_F(TestLevelIndicationCluster, SetLevelValue_MediumAndCriticalRequireSubFeature)
{
    // Cluster with LevelIndication but without kMediumLevel/kCriticalLevel.
    ConcentrationMeasurementCluster levelOnlyCluster(kTestEndpointId, MakeLevelConfig());
    ClusterTester localTester(levelOnlyCluster);
    ASSERT_EQ(levelOnlyCluster.Startup(localTester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(levelOnlyCluster.SetLevelValue(LevelValueEnum::kLow), CHIP_NO_ERROR);
    EXPECT_EQ(levelOnlyCluster.SetLevelValue(LevelValueEnum::kMedium), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(levelOnlyCluster.SetLevelValue(LevelValueEnum::kCritical), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    levelOnlyCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestLevelIndicationCluster, SetLevelValue_UnknownEnumRejected)
{
    EXPECT_EQ(cluster.SetLevelValue(LevelValueEnum::kUnknownEnumValue), CHIP_IM_GLOBAL_STATUS(ConstraintError));
}

// Attribute change notifications via dirty list

TEST_F(TestNumericMeasurementCluster, AttributeChangeDirtyList)
{
    ASSERT_EQ(cluster.SetMeasuredValue(DataModel::MakeNullable(10.0f)), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::MeasuredValue::Id));

    tester.GetDirtyList().clear();

    ASSERT_EQ(cluster.SetPeakMeasuredValue(DataModel::MakeNullable(20.0f)), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::PeakMeasuredValue::Id));

    tester.GetDirtyList().clear();

    ASSERT_EQ(cluster.SetPeakMeasuredValueWindow(3600u), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::PeakMeasuredValueWindow::Id));
}

TEST_F(TestLevelIndicationCluster, AttributeChangeDirtyList)
{
    ASSERT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::LevelValue::Id));

    // Setting same value again must NOT re-dirty the attribute.
    tester.GetDirtyList().clear();
    ASSERT_EQ(cluster.SetLevelValue(LevelValueEnum::kLow), CHIP_NO_ERROR);
    EXPECT_FALSE(tester.IsAttributeDirty(Attributes::LevelValue::Id));
}

// Feature implication: constructing with kPeakMeasurement auto-sets kNumericMeasurement

TEST_F(TestNumericMeasurementCluster, FeatureImplication_PeakImpliesNumeric)
{
    ConcentrationMeasurementCluster::Config cfg = {
        kTestClusterId,
        BitFlags<Feature>(Feature::kPeakMeasurement),
        MeasurementMediumEnum::kAir,
        MeasurementUnitEnum::kPpm,
        DataModel::MakeNullable(kTestMin),
        DataModel::MakeNullable(kTestMax),
    };
    ConcentrationMeasurementCluster peakCluster(kTestEndpointId, cfg);
    ClusterTester localTester(peakCluster);
    ASSERT_EQ(peakCluster.Startup(localTester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint32_t features{};
    ASSERT_EQ(localTester.ReadAttribute(Globals::Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_TRUE((features & to_underlying(Feature::kNumericMeasurement)) != 0);

    peakCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
