/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include <app/clusters/fan-control-server/FanControlCluster.h>
#include <app/clusters/fan-control-server/fan-control-delegate.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/FanControl/Attributes.h>
#include <clusters/FanControl/Enums.h>
#include <clusters/FanControl/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl;
using namespace chip::Testing;

using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId = 1;

class TestFanControlDelegate : public FanControl::Delegate
{
public:
    TestFanControlDelegate(EndpointId endpoint) : Delegate(endpoint) {}
    Protocols::InteractionModel::Status HandleStep(StepDirectionEnum, bool, bool) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
};

TestFanControlDelegate gTestDelegate(kTestEndpointId);

FanControlCluster::Config MakeTestConfig()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate).WithFanModeSequence(FanModeSequenceEnum::kOffLowHigh);
}

FanControlCluster::Config MakeTestConfigWithStep()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate)
        .WithFanModeSequence(FanModeSequenceEnum::kOffLowHigh)
        .WithStep();
}

FanControlCluster::Config MakeTestConfigWithMultiSpeed()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate)
        .WithSpeedMax(10)
        .WithFanModeSequence(FanModeSequenceEnum::kOffLowHigh);
}

FanControlCluster::Config MakeTestConfigWithAuto()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate).WithFanModeSequence(FanModeSequenceEnum::kOffLowHighAuto);
}

FanControlCluster::Config MakeTestConfigWithMultiSpeedAndAuto()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate)
        .WithSpeedMax(10)
        .WithFanModeSequence(FanModeSequenceEnum::kOffLowHighAuto);
}

FanControlCluster::Config MakeTestConfigOffHigh()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate).WithFanModeSequence(FanModeSequenceEnum::kOffHigh);
}

FanControlCluster::Config MakeTestConfigOffHighAuto()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate).WithFanModeSequence(FanModeSequenceEnum::kOffHighAuto);
}

FanControlCluster::Config MakeTestConfigOffLowMedHigh()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate).WithFanModeSequence(FanModeSequenceEnum::kOffLowMedHigh);
}

FanControlCluster::Config MakeTestConfigWithRocking()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate)
        .WithFanModeSequence(FanModeSequenceEnum::kOffLowHigh)
        .WithRockSupport(BitMask<RockBitmap>(RockBitmap::kRockLeftRight));
}

FanControlCluster::Config MakeTestConfigWithWind()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate)
        .WithFanModeSequence(FanModeSequenceEnum::kOffLowHigh)
        .WithWindSupport(BitMask<WindBitmap>(WindBitmap::kSleepWind));
}

FanControlCluster::Config MakeTestConfigWithAirflowDirection()
{
    return FanControlCluster::Config(kTestEndpointId, &gTestDelegate)
        .WithFanModeSequence(FanModeSequenceEnum::kOffLowHigh)
        .WithAirflowDirection();
}

template <FanControlCluster::Config (*ConfigFn)()>
struct TestFanControlClusterFixture : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestServerClusterContext testContext;
    FanControlCluster cluster{ ConfigFn() };
};

using TestFanControlCluster                      = TestFanControlClusterFixture<MakeTestConfig>;
using TestFanControlClusterWithStep              = TestFanControlClusterFixture<MakeTestConfigWithStep>;
using TestFanControlClusterWithMultiSpeed        = TestFanControlClusterFixture<MakeTestConfigWithMultiSpeed>;
using TestFanControlClusterWithAuto              = TestFanControlClusterFixture<MakeTestConfigWithAuto>;
using TestFanControlClusterWithMultiSpeedAndAuto = TestFanControlClusterFixture<MakeTestConfigWithMultiSpeedAndAuto>;
using TestFanControlClusterOffHigh               = TestFanControlClusterFixture<MakeTestConfigOffHigh>;
using TestFanControlClusterOffHighAuto           = TestFanControlClusterFixture<MakeTestConfigOffHighAuto>;
using TestFanControlClusterOffLowMedHigh         = TestFanControlClusterFixture<MakeTestConfigOffLowMedHigh>;
using TestFanControlClusterWithRocking           = TestFanControlClusterFixture<MakeTestConfigWithRocking>;
using TestFanControlClusterWithWind              = TestFanControlClusterFixture<MakeTestConfigWithWind>;
using TestFanControlClusterWithAirflowDirection  = TestFanControlClusterFixture<MakeTestConfigWithAirflowDirection>;

} // namespace

TEST_F(TestFanControlCluster, AttributeList)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            FanControl::Attributes::FanMode::kMetadataEntry,
                                            FanControl::Attributes::FanModeSequence::kMetadataEntry,
                                            FanControl::Attributes::PercentSetting::kMetadataEntry,
                                            FanControl::Attributes::PercentCurrent::kMetadataEntry,
                                        }));
}

TEST_F(TestFanControlCluster, ReadGlobalAttributes)
{
    ClusterTester tester(cluster);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, FanControl::kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestFanControlClusterWithStep, FeatureMapIncludesStep)
{
    ClusterTester tester(cluster);
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kStep));
}

TEST_F(TestFanControlClusterWithMultiSpeed, FeatureMapIncludesMultiSpeed)
{
    ClusterTester tester(cluster);
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kMultiSpeed));
}

TEST_F(TestFanControlClusterWithAuto, FeatureMapIncludesAuto)
{
    ClusterTester tester(cluster);
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kAuto));
}

TEST_F(TestFanControlClusterWithMultiSpeedAndAuto, FeatureMapIncludesMultiSpeedAndAuto)
{
    ClusterTester tester(cluster);
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kMultiSpeed) | static_cast<uint32_t>(Feature::kAuto));
}

TEST_F(TestFanControlClusterOffHigh, FeatureMapIsZero)
{
    ClusterTester tester(cluster);
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestFanControlClusterOffHighAuto, FeatureMapIncludesAuto)
{
    ClusterTester tester(cluster);
    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kAuto));
}

TEST_F(TestFanControlCluster, ReadFanMode)
{
    ClusterTester tester(cluster);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kOff);
}

TEST_F(TestFanControlCluster, SetFanMode_ReturnsConstraintError_WhenValueIsUnknown)
{
    ClusterTester tester(cluster);

    auto status = tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kUnknownEnumValue);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestFanControlCluster, WriteFanMode)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kHigh), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kHigh);
}

TEST_F(TestFanControlCluster, WritePercentSetting)
{
    ClusterTester tester(cluster);

    DataModel::Nullable<chip::Percent> percentSetting;
    percentSetting.SetNonNull(50);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::PercentSetting::Id, percentSetting), CHIP_NO_ERROR);

    DataModel::Nullable<chip::Percent> readBack;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::PercentSetting::Id, readBack), CHIP_NO_ERROR);
    ASSERT_FALSE(readBack.IsNull());
    EXPECT_EQ(readBack.Value(), 50);
}

TEST_F(TestFanControlCluster, StepCommandUnsupportedWithoutFeature)
{
    ClusterTester tester(cluster);
    Commands::Step::Type commandData;
    commandData.direction = StepDirectionEnum::kIncrease;

    auto result = tester.Invoke(commandData);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_FALSE(result.status.value().IsSuccess());
    }
}

TEST_F(TestFanControlClusterWithStep, StepCommandSupportedWithFeature)
{
    ClusterTester tester(cluster);
    Commands::Step::Type commandData;
    commandData.direction = StepDirectionEnum::kIncrease;

    auto result = tester.Invoke(commandData);
    ASSERT_TRUE(result.status.has_value());
    if (result.status.has_value())
    {
        EXPECT_TRUE(result.status.value().IsSuccess());
    }
}

TEST_F(TestFanControlClusterWithMultiSpeed, FanModeOff_ZeroesPercentSettingAndSpeedSetting)
{
    ClusterTester tester(cluster);

    DataModel::Nullable<chip::Percent> percentSetting;
    percentSetting.SetNonNull(50);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::PercentSetting::Id, percentSetting), CHIP_NO_ERROR);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kOff), CHIP_NO_ERROR);

    DataModel::Nullable<chip::Percent> readPercent;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::PercentSetting::Id, readPercent), CHIP_NO_ERROR);
    ASSERT_FALSE(readPercent.IsNull());
    EXPECT_EQ(readPercent.Value(), 0);

    DataModel::Nullable<uint8_t> readSpeed;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::SpeedSetting::Id, readSpeed), CHIP_NO_ERROR);
    ASSERT_FALSE(readSpeed.IsNull());
    EXPECT_EQ(readSpeed.Value(), 0);
}

TEST_F(TestFanControlClusterWithMultiSpeedAndAuto, FanModeAuto_NullsPercentSettingAndSpeedSetting)
{
    ClusterTester tester(cluster);

    // OnOff must be on for Current attributes to track the setting values.
    cluster.SetOnOffState(true);

    DataModel::Nullable<chip::Percent> percentSetting;
    percentSetting.SetNonNull(50);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::PercentSetting::Id, percentSetting), CHIP_NO_ERROR);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kAuto), CHIP_NO_ERROR);

    DataModel::Nullable<chip::Percent> readPercent;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::PercentSetting::Id, readPercent), CHIP_NO_ERROR);
    EXPECT_TRUE(readPercent.IsNull());

    DataModel::Nullable<uint8_t> readSpeed;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::SpeedSetting::Id, readSpeed), CHIP_NO_ERROR);
    EXPECT_TRUE(readSpeed.IsNull());

    chip::Percent percentCurrent;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::PercentCurrent::Id, percentCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(percentCurrent, 50);

    uint8_t speedCurrent;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::SpeedCurrent::Id, speedCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(speedCurrent, 5); // 50% of SpeedMax=10
}

TEST_F(TestFanControlCluster, PercentSettingZero_SetsFanModeOff)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kHigh), CHIP_NO_ERROR);

    DataModel::Nullable<chip::Percent> percentSetting;
    percentSetting.SetNonNull(0);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::PercentSetting::Id, percentSetting), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kOff);
}

TEST_F(TestFanControlClusterWithMultiSpeed, SpeedSettingZero_SetsFanModeOff)
{
    ClusterTester tester(cluster);

    DataModel::Nullable<chip::Percent> percentSetting;
    percentSetting.SetNonNull(50);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::PercentSetting::Id, percentSetting), CHIP_NO_ERROR);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kHigh), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> speedSetting;
    speedSetting.SetNonNull(0);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::SpeedSetting::Id, speedSetting), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kOff);
}

TEST_F(TestFanControlClusterWithMultiSpeed, SetSpeedSetting_ReturnsConstraintError_WhenValueExceedsSpeedMax)
{
    ClusterTester tester(cluster);

    DataModel::Nullable<uint8_t> speedSetting;
    speedSetting.SetNonNull(11); // SpeedMax is 10 in MakeTestConfigWithMultiSpeed
    auto status = tester.WriteAttribute(FanControl::Attributes::SpeedSetting::Id, speedSetting);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestFanControlClusterWithMultiSpeed, PercentSettingUpdatesSpeedSetting)
{
    ClusterTester tester(cluster);

    DataModel::Nullable<chip::Percent> percentSetting;
    percentSetting.SetNonNull(50);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::PercentSetting::Id, percentSetting), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> readSpeed;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::SpeedSetting::Id, readSpeed), CHIP_NO_ERROR);
    ASSERT_FALSE(readSpeed.IsNull());
    EXPECT_EQ(readSpeed.Value(), 5); // 50% of SpeedMax=10
}

TEST_F(TestFanControlClusterWithMultiSpeed, SpeedSettingUpdatesPercentSetting)
{
    ClusterTester tester(cluster);

    DataModel::Nullable<uint8_t> speedSetting;
    speedSetting.SetNonNull(5);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::SpeedSetting::Id, speedSetting), CHIP_NO_ERROR);

    DataModel::Nullable<chip::Percent> readPercent;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::PercentSetting::Id, readPercent), CHIP_NO_ERROR);
    ASSERT_FALSE(readPercent.IsNull());
    EXPECT_EQ(readPercent.Value(), 50); // speed 5 of 10 = 50%
}

TEST_F(TestFanControlCluster, FanModeOn_MapsToHigh)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kOn), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kHigh);
}

TEST_F(TestFanControlClusterWithAuto, FanModeSmart_MapsToAuto)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kSmart), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kAuto);
}

TEST_F(TestFanControlCluster, FanModeSmart_MapsToHigh_WhenAutoNotSupported)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kSmart), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kHigh);
}

TEST_F(TestFanControlClusterOffHigh, FanModeLow_ReturnsInvalidInState)
{
    ClusterTester tester(cluster);

    auto status = tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kLow);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidInState);
}

TEST_F(TestFanControlClusterOffHigh, FanModeMedium_ReturnsInvalidInState)
{
    ClusterTester tester(cluster);

    auto status = tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kMedium);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidInState);
}

TEST_F(TestFanControlClusterOffHighAuto, FanModeAuto_Succeeds)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kAuto), CHIP_NO_ERROR);

    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kAuto);
}

TEST_F(TestFanControlCluster, FanModeAuto_ReturnsInvalidInState_WhenAutoNotSupported)
{
    ClusterTester tester(cluster);

    auto status = tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kAuto);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidInState);
}

TEST_F(TestFanControlCluster, FanModeMedium_ReturnsInvalidInState)
{
    ClusterTester tester(cluster);

    auto status = tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kMedium);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::InvalidInState);
}

TEST_F(TestFanControlClusterOffLowMedHigh, FanModeLowAndMedium_Succeed)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kLow), CHIP_NO_ERROR);
    FanModeEnum fanMode;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kLow);

    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::FanMode::Id, FanModeEnum::kMedium), CHIP_NO_ERROR);
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::FanMode::Id, fanMode), CHIP_NO_ERROR);
    EXPECT_EQ(fanMode, FanModeEnum::kMedium);
}

TEST_F(TestFanControlCluster, SetRockSetting_ReturnsUnsupportedAttribute_WhenRockingNotSupported)
{
    ClusterTester tester(cluster);

    BitMask<RockBitmap> rockSetting(RockBitmap::kRockLeftRight);
    auto status = tester.WriteAttribute(FanControl::Attributes::RockSetting::Id, rockSetting);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::UnsupportedAttribute);
}

TEST_F(TestFanControlCluster, SetWindSetting_ReturnsUnsupportedAttribute_WhenWindNotSupported)
{
    ClusterTester tester(cluster);

    BitMask<WindBitmap> windSetting(WindBitmap::kSleepWind);
    auto status = tester.WriteAttribute(FanControl::Attributes::WindSetting::Id, windSetting);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::UnsupportedAttribute);
}

TEST_F(TestFanControlClusterWithRocking, SetRockSetting_Succeeds_WhenValueIsSubsetOfSupport)
{
    ClusterTester tester(cluster);

    BitMask<RockBitmap> rockSetting(RockBitmap::kRockLeftRight);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::RockSetting::Id, rockSetting), CHIP_NO_ERROR);

    BitMask<RockBitmap> readBack;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::RockSetting::Id, readBack), CHIP_NO_ERROR);
    EXPECT_EQ(readBack.Raw(), rockSetting.Raw());
}

TEST_F(TestFanControlClusterWithRocking, SetRockSetting_ReturnsConstraintError_WhenValueHasUnsupportedBits)
{
    ClusterTester tester(cluster);

    BitMask<RockBitmap> unsupportedBits(RockBitmap::kRockUpDown);
    auto status = tester.WriteAttribute(FanControl::Attributes::RockSetting::Id, unsupportedBits);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestFanControlClusterWithWind, SetWindSetting_Succeeds_WhenValueIsSubsetOfSupport)
{
    ClusterTester tester(cluster);

    BitMask<WindBitmap> windSetting(WindBitmap::kSleepWind);
    ASSERT_EQ(tester.WriteAttribute(FanControl::Attributes::WindSetting::Id, windSetting), CHIP_NO_ERROR);

    BitMask<WindBitmap> readBack;
    ASSERT_EQ(tester.ReadAttribute(FanControl::Attributes::WindSetting::Id, readBack), CHIP_NO_ERROR);
    EXPECT_EQ(readBack.Raw(), windSetting.Raw());
}

TEST_F(TestFanControlClusterWithWind, SetWindSetting_ReturnsConstraintError_WhenValueHasUnsupportedBits)
{
    ClusterTester tester(cluster);

    BitMask<WindBitmap> unsupportedBits(WindBitmap::kNaturalWind);
    auto status = tester.WriteAttribute(FanControl::Attributes::WindSetting::Id, unsupportedBits);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::ConstraintError);
}

TEST_F(TestFanControlClusterWithAirflowDirection, SetAirflowDirection_ReturnsConstraintError_WhenValueIsUnknown)
{
    ClusterTester tester(cluster);

    auto status = tester.WriteAttribute(FanControl::Attributes::AirflowDirection::Id, AirflowDirectionEnum::kUnknownEnumValue);
    EXPECT_EQ(status, Protocols::InteractionModel::Status::ConstraintError);
}
