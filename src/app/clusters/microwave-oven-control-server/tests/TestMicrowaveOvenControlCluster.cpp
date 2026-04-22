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
#include <clusters/MicrowaveOvenControl/Metadata.h>
#include <clusters/OperationalState/Metadata.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::MicrowaveOvenControl;
using namespace chip::app::Clusters::MicrowaveOvenControl::Attributes;
using namespace chip::Testing;

constexpr uint32_t kDefaultCookTimeSec = 30u;

class MicrowaveOvenControlDelegate : public MicrowaveOvenControl::Delegate
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

class OperationalStateDelegate : public OperationalState::Delegate
{
public:
    app::DataModel::Nullable<uint32_t> mCountDownTime;

    app::DataModel::Nullable<uint32_t> GetCountdownTime() override { return mCountDownTime; }

    CHIP_ERROR GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState) override
    {
        if (index >= mOperationalStateList.size())
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        operationalState = mOperationalStateList[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override
    {
        if (index >= mOperationalPhaseList.size())
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        return CopyCharSpanToMutableCharSpan(mOperationalPhaseList[index], operationalPhase);
    }

    void HandlePauseStateCallback(OperationalState::GenericOperationalError & err) override {}
    void HandleResumeStateCallback(OperationalState::GenericOperationalError & err) override {}
    void HandleStartStateCallback(OperationalState::GenericOperationalError & err) override {}
    void HandleStopStateCallback(OperationalState::GenericOperationalError & err) override {}

private:
    Span<const OperationalState::GenericOperationalState> mOperationalStateList;
    Span<const CharSpan> mOperationalPhaseList;
};

class ModeBaseDelegate : public ModeBase::Delegate
{
private:
    using ModeTagStructType = Clusters::detail::Structs::ModeTagStruct::Type;

    ModeTagStructType modeTagsNormal[1] = { { .mfgCode = {}, .value = 0 } };
    ModeTagStructType modeTagsHeavy[2]  = { { .mfgCode = {}, .value = 1 }, { .mfgCode = {}, .value = 2 } };
    ModeTagStructType modeTagsLight[3]  = { { .mfgCode = {}, .value = 3 },
                                            { .mfgCode = {}, .value = 4 },
                                            { .mfgCode = {}, .value = 5 } };

    const Clusters::detail::Structs::ModeOptionStruct::Type kModeOptions[3] = {
        Clusters::detail::Structs::ModeOptionStruct::Type{
            .label = "Normal"_span, .mode = 0, .modeTags = DataModel::List<const ModeTagStructType>(modeTagsNormal) },
        Clusters::detail::Structs::ModeOptionStruct::Type{
            .label = "Heavy"_span, .mode = 1, .modeTags = DataModel::List<const ModeTagStructType>(modeTagsHeavy) },
        Clusters::detail::Structs::ModeOptionStruct::Type{
            .label = "Light"_span, .mode = 2, .modeTags = DataModel::List<const ModeTagStructType>(modeTagsLight) }
    };

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    void HandleChangeToMode(uint8_t mode, ModeBase::Commands::ChangeToModeResponse::Type & response) override {}

    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override
    {
        if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        return chip::CopyCharSpanToMutableCharSpan(kModeOptions[modeIndex].label, label);
    }

    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override
    {
        if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        value = kModeOptions[modeIndex].mode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex, DataModel::List<ModeTagStructType> & tags) override
    {
        if (modeIndex >= MATTER_ARRAY_SIZE(kModeOptions))
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }

        if (tags.size() < kModeOptions[modeIndex].modeTags.size())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        std::copy(kModeOptions[modeIndex].modeTags.begin(), kModeOptions[modeIndex].modeTags.end(), tags.begin());
        tags.reduce_size(kModeOptions[modeIndex].modeTags.size());
        return CHIP_NO_ERROR;
    }
};

struct TestMicrowaveOvenControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestMicrowaveOvenControlCluster() {}

    TestServerClusterContext testContext;
    OperationalStateDelegate operationalStateDelegate;
    OperationalState::Instance operationalStateInstance{ &operationalStateDelegate, kRootEndpointId };
    ModeBaseDelegate modeBaseDelegate;
    ModeBase::Instance microwaveOvenModeInstance{ &modeBaseDelegate, kRootEndpointId, MicrowaveOvenControl::Id, 0 };
    MicrowaveOvenControlDelegate microwaveOvenControlDelegate;
    std::bitset<Commands::kAcceptedCommandsCount> optionalAcceptedCommands{ Commands::AddMoreTime::Id };
    MicrowaveOvenControlCluster::OptionalAttributeSet optionalAttributeSet{};
    MicrowaveOvenControlCluster::Config defaultConfig{
        .feature                   = BitFlags<Feature>{},
        .optionalAttributeSet      = optionalAttributeSet,
        .optionalAcceptedCommands  = optionalAcceptedCommands,
        .opStateInstance           = operationalStateInstance,
        .microwaveOvenModeInstance = microwaveOvenModeInstance,
        .delegate                  = microwaveOvenControlDelegate,
    };
};

void TestMandatoryAttributes(ClusterTester & tester, Feature feature, MicrowaveOvenControlDelegate & delegate)
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

        TestMandatoryAttributes(tester, config.feature, microwaveOvenControlDelegate);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.optionalAttributeSet.Set<WattRating::Id>();
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, microwaveOvenControlDelegate);

        uint16_t wattRating{};
        ASSERT_EQ(tester.ReadAttribute(WattRating::Id, wattRating), CHIP_NO_ERROR);
        ASSERT_EQ(wattRating, microwaveOvenControlDelegate.GetWattRating());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerAsNumber;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, microwaveOvenControlDelegate);

        uint8_t powerSetting{};
        ASSERT_EQ(tester.ReadAttribute(PowerSetting::Id, powerSetting), CHIP_NO_ERROR);
        ASSERT_EQ(powerSetting, microwaveOvenControlDelegate.GetPowerSettingNum());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerNumberLimits;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, microwaveOvenControlDelegate);

        uint8_t minPower{};
        ASSERT_EQ(tester.ReadAttribute(MinPower::Id, minPower), CHIP_NO_ERROR);
        ASSERT_EQ(minPower, microwaveOvenControlDelegate.GetMinPowerNum());

        uint8_t maxPower{};
        ASSERT_EQ(tester.ReadAttribute(MaxPower::Id, maxPower), CHIP_NO_ERROR);
        ASSERT_EQ(maxPower, microwaveOvenControlDelegate.GetMaxPowerNum());

        uint8_t powerStep{};
        ASSERT_EQ(tester.ReadAttribute(PowerStep::Id, powerStep), CHIP_NO_ERROR);
        ASSERT_EQ(powerStep, microwaveOvenControlDelegate.GetPowerStepNum());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        config.feature                             = Feature::kPowerInWatts;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);

        TestMandatoryAttributes(tester, config.feature, microwaveOvenControlDelegate);

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
        ASSERT_EQ(cluster.GetCookTimeSec(), static_cast<uint32_t>(1));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        MicrowaveOvenControlCluster::Config config = defaultConfig;
        MicrowaveOvenControlCluster cluster(kRootEndpointId, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.SetCookTimeSec(microwaveOvenControlDelegate.GetMaxCookTimeSec()), CHIP_NO_ERROR);
        ASSERT_EQ(cluster.GetCookTimeSec(), microwaveOvenControlDelegate.GetMaxCookTimeSec());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
