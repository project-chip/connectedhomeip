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

#include <app/clusters/microwave-oven-control-server/MicrowaveOvenControlCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using namespace chip::Testing;
using Status      = Protocols::InteractionModel::Status;
using AppDelegate = MicrowaveOvenControl::AppDelegate;

constexpr uint32_t kDefaultCookTimeSec = 30u;

class TestIntegrationDelegate : public IntegrationDelegate
{
public:
    virtual uint8_t GetCurrentOperationalState() const override { return 0; }

    virtual CHIP_ERROR GetNormalOperatingMode(uint8_t & mode) const override
    {
        mode = 0;
        return CHIP_NO_ERROR;
    }

    virtual bool IsSupportedMode(uint8_t mode) const override { return mode == 0; }

    virtual bool IsSupportedOperationalStateCommand(EndpointId endpointId, CommandId commandId) const override
    {
        return endpointId == kRootEndpointId && commandId == OperationalState::Commands::Start::Id;
    }
};

class TestDelegate : public AppDelegate
{
public:
    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(uint8_t cookMode, uint32_t cookTimeSec,
                                                                           bool startAfterSetting,
                                                                           Optional<uint8_t> powerSettingNum,
                                                                           Optional<uint8_t> wattSettingIndex) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    Protocols::InteractionModel::Status HandleModifyCookTimeSecondsCallback(uint32_t finalcookTimeSec) override
    {
        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR GetWattSettingByIndex(uint8_t index, uint16_t & wattSetting) override
    {
        VerifyOrReturnError(index < MATTER_ARRAY_SIZE(mWattSettingList), CHIP_ERROR_NOT_FOUND);
        wattSetting = mWattSettingList[index];
        return CHIP_NO_ERROR;
    }

    uint8_t GetPowerSettingNum() const override { return mPowerSettingNum; }

    uint8_t GetMinPowerNum() const override { return kMinPowerNum; }

    uint8_t GetMaxPowerNum() const override { return kMaxPowerNum; }

    uint8_t GetPowerStepNum() const override { return kPowerStepNum; }

    uint8_t GetCurrentWattIndex() const override { return mSelectedWattIndex; };

    uint32_t GetMaxCookTimeSec() const override { return kMaxCookTimeSec; }

    uint16_t GetWattRating() const override { return mWattRating; };

private:
    static constexpr uint8_t kMinPowerNum            = 20u;
    static constexpr uint8_t kMaxPowerNum            = 90u;
    static constexpr uint8_t kPowerStepNum           = 10u;
    static constexpr uint32_t kMaxCookTimeSec        = 86400u;
    static constexpr uint8_t kDefaultPowerSettingNum = kMaxPowerNum;

    uint8_t mPowerSettingNum   = kDefaultPowerSettingNum;
    uint8_t mSelectedWattIndex = 0;
    uint16_t mWattRating       = 0;

    const uint16_t mWattSettingList[5] = { 100u, 300u, 500u, 800u, 1000u };
};

struct TestMicrowaveOvenControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestMicrowaveOvenControlCluster() {}

    TestServerClusterContext testContext;
    TestDelegate testDelegate;
    bool supportsAddMoreTime{};
    MicrowaveOvenControlCluster::OptionalAttributeSet optionalAttributeSet{};
    TestIntegrationDelegate testIntegrationDelegate;
    MicrowaveOvenControlCluster::Config defaultConfig{
        .feature              = BitFlags<Feature>{},
        .optionalAttributeSet = optionalAttributeSet,
        .supportsAddMoreTime  = supportsAddMoreTime,
        .integrationDelegate  = testIntegrationDelegate,
        .appDelegate          = testDelegate,
    };
};

void TestMandatoryAttributes(ClusterTester & tester, Feature feature, TestDelegate & delegate)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    ASSERT_EQ(revision, MicrowaveOvenControl::kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    ASSERT_EQ(featureMap, to_underlying(feature));

    uint32_t cookTime{};
    ASSERT_EQ(tester.ReadAttribute(CookTime::Id, cookTime), CHIP_NO_ERROR);
    ASSERT_EQ(cookTime, kDefaultCookTimeSec);

    uint32_t maxCookTime{};
    ASSERT_EQ(tester.ReadAttribute(MaxCookTime::Id, maxCookTime), CHIP_NO_ERROR);
    ASSERT_EQ(maxCookTime, delegate.GetMaxCookTimeSec());
}

} // namespace

TEST_F(TestMicrowaveOvenControlCluster, AttributeTest)
{
    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CookTime::kMetadataEntry,
                                                MaxCookTime::kMetadataEntry,
                                            }));
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.optionalAttributeSet.Set<WattRating::Id>();
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CookTime::kMetadataEntry,
                                                MaxCookTime::kMetadataEntry,
                                                WattRating::kMetadataEntry,
                                            }));
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerAsNumber;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CookTime::kMetadataEntry,
                                                MaxCookTime::kMetadataEntry,
                                                PowerSetting::kMetadataEntry,
                                            }));
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerNumberLimits;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CookTime::kMetadataEntry,
                                                MaxCookTime::kMetadataEntry,
                                                MinPower::kMetadataEntry,
                                                MaxPower::kMetadataEntry,
                                                PowerStep::kMetadataEntry,
                                            }));
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerInWatts;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                CookTime::kMetadataEntry,
                                                MaxCookTime::kMetadataEntry,
                                                SupportedWatts::kMetadataEntry,
                                                SelectedWattIndex::kMetadataEntry,
                                            }));
    }
}

TEST_F(TestMicrowaveOvenControlCluster, ReadAttributeTest)
{
    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, testDelegate);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.optionalAttributeSet.Set<WattRating::Id>();
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, testDelegate);

        uint16_t wattRating{};
        ASSERT_EQ(tester.ReadAttribute(WattRating::Id, wattRating), CHIP_NO_ERROR);
        ASSERT_EQ(wattRating, testDelegate.GetWattRating());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerAsNumber;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, testDelegate);

        uint8_t powerSetting{};
        ASSERT_EQ(tester.ReadAttribute(PowerSetting::Id, powerSetting), CHIP_NO_ERROR);
        ASSERT_EQ(powerSetting, testDelegate.GetPowerSettingNum());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerNumberLimits;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, testDelegate);

        uint8_t minPower{};
        ASSERT_EQ(tester.ReadAttribute(MinPower::Id, minPower), CHIP_NO_ERROR);
        ASSERT_EQ(minPower, testDelegate.GetMinPowerNum());

        uint8_t maxPower{};
        ASSERT_EQ(tester.ReadAttribute(MaxPower::Id, maxPower), CHIP_NO_ERROR);
        ASSERT_EQ(maxPower, testDelegate.GetMaxPowerNum());

        uint8_t powerStep{};
        ASSERT_EQ(tester.ReadAttribute(PowerStep::Id, powerStep), CHIP_NO_ERROR);
        ASSERT_EQ(powerStep, testDelegate.GetPowerStepNum());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerInWatts;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, testDelegate);

        DataModel::DecodableList<uint16_t> supportedWatts;
        ASSERT_EQ(tester.ReadAttribute(SupportedWatts::Id, supportedWatts), CHIP_NO_ERROR);
        auto it = supportedWatts.begin();
        ASSERT_TRUE(it.Next());
        ASSERT_TRUE(it.GetValue() == 100u);
        ASSERT_TRUE(it.Next());
        ASSERT_TRUE(it.GetValue() == 300u);
        ASSERT_TRUE(it.Next());
        ASSERT_TRUE(it.GetValue() == 500u);
        ASSERT_TRUE(it.Next());
        ASSERT_TRUE(it.GetValue() == 800u);
        ASSERT_TRUE(it.Next());
        ASSERT_TRUE(it.GetValue() == 1000u);
        ASSERT_FALSE(it.Next());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestMicrowaveOvenControlCluster, SetCookTimeSecTest)
{
    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetCookTimeSec(0), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetCookTimeSec(1), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.GetCookTimeSec(), 1u);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetCookTimeSec(testDelegate.GetMaxCookTimeSec()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.GetCookTimeSec(), testDelegate.GetMaxCookTimeSec());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetCookTimeSec(testDelegate.GetMaxCookTimeSec() + 1), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestMicrowaveOvenControlCluster, AcceptedCommandsTest)
{
    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, MicrowaveOvenControl::Id), builder), CHIP_NO_ERROR);
        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> acceptedCommands = builder.TakeBuffer();
        ASSERT_EQ(acceptedCommands.size(), 1u);
        ASSERT_EQ(acceptedCommands[0].commandId, MicrowaveOvenControl::Commands::SetCookingParameters::Id);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.supportsAddMoreTime                 = true;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, MicrowaveOvenControl::Id), builder), CHIP_NO_ERROR);
        ReadOnlyBuffer<DataModel::AcceptedCommandEntry> acceptedCommands = builder.TakeBuffer();
        ASSERT_EQ(acceptedCommands.size(), 2u);
        ASSERT_EQ(acceptedCommands[0].commandId, MicrowaveOvenControl::Commands::SetCookingParameters::Id);
        ASSERT_EQ(acceptedCommands[1].commandId, MicrowaveOvenControl::Commands::AddMoreTime::Id);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
