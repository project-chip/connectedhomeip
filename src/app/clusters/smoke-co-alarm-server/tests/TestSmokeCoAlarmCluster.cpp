/*
 *
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

#include <app/clusters/smoke-co-alarm-server/smokeCOAlarmCluster.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/SmokeCoAlarm/AttributeIds.h>
#include <clusters/SmokeCoAlarm/Enums.h>
#include <clusters/SmokeCoAlarm/Metadata.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;
using namespace chip::Testing;
using chip::Protocols::InteractionModel::Status;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId = 1;

constexpr std::array<ExpressedStateEnum, SmokeCoAlarmCluster::kPriorityOrderLength> kDefaultPriority = {
    ExpressedStateEnum::kSmokeAlarm,        ExpressedStateEnum::kCOAlarm,        ExpressedStateEnum::kBatteryAlert,
    ExpressedStateEnum::kTesting,           ExpressedStateEnum::kHardwareFault,  ExpressedStateEnum::kEndOfService,
    ExpressedStateEnum::kInterconnectSmoke, ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kInoperative,
};

SmokeCoAlarmCluster::Config MakeFullConfig()
{
    SmokeCoAlarmCluster::Config cfg;
    cfg.featureMap.Set(Feature::kSmokeAlarm).Set(Feature::kCoAlarm);
    cfg.optionalAttribs = SmokeCoAlarmCluster::OptionalAttributeSet(SmokeCoAlarmCluster::OptionalAttributeSet::All());
    return cfg;
}

struct TestSmokeCoAlarmBase : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

struct TestSmokeCoAlarmCluster : public TestSmokeCoAlarmBase
{
    void SetUp() override { ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    SmokeCoAlarmCluster cluster{ kTestEndpointId, MakeFullConfig() };
    ClusterTester tester{ cluster };
};

} // namespace

TEST_F(TestSmokeCoAlarmBase, AttributeList_MandatoryOnly)
{
    SmokeCoAlarmCluster cluster(kTestEndpointId);
    ClusterTester t(cluster);
    ASSERT_EQ(cluster.Startup(t.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_TRUE(
        IsAttributesListEqualTo(cluster,
                                { Attributes::ExpressedState::kMetadataEntry, Attributes::BatteryAlert::kMetadataEntry,
                                  Attributes::TestInProgress::kMetadataEntry, Attributes::HardwareFaultAlert::kMetadataEntry,
                                  Attributes::EndOfServiceAlert::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestSmokeCoAlarmBase, AttributeList_SmokeAndCOFeatures)
{
    SmokeCoAlarmCluster::Config cfg;
    cfg.featureMap.Set(Feature::kSmokeAlarm).Set(Feature::kCoAlarm);
    SmokeCoAlarmCluster cluster(kTestEndpointId, cfg);
    ClusterTester t(cluster);
    ASSERT_EQ(cluster.Startup(t.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster,
        { Attributes::ExpressedState::kMetadataEntry, Attributes::SmokeState::kMetadataEntry, Attributes::COState::kMetadataEntry,
          Attributes::BatteryAlert::kMetadataEntry, Attributes::TestInProgress::kMetadataEntry,
          Attributes::HardwareFaultAlert::kMetadataEntry, Attributes::EndOfServiceAlert::kMetadataEntry,
          Attributes::ContaminationState::kMetadataEntry, Attributes::SmokeSensitivityLevel::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestSmokeCoAlarmBase, AttributeList_FullConfig)
{
    SmokeCoAlarmCluster cluster(kTestEndpointId, MakeFullConfig());
    ClusterTester t(cluster);
    ASSERT_EQ(cluster.Startup(t.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_TRUE(IsAttributesListEqualTo(
        cluster,
        { Attributes::ExpressedState::kMetadataEntry, Attributes::SmokeState::kMetadataEntry, Attributes::COState::kMetadataEntry,
          Attributes::BatteryAlert::kMetadataEntry, Attributes::DeviceMuted::kMetadataEntry,
          Attributes::TestInProgress::kMetadataEntry, Attributes::HardwareFaultAlert::kMetadataEntry,
          Attributes::EndOfServiceAlert::kMetadataEntry, Attributes::InterconnectSmokeAlarm::kMetadataEntry,
          Attributes::InterconnectCOAlarm::kMetadataEntry, Attributes::ContaminationState::kMetadataEntry,
          Attributes::SmokeSensitivityLevel::kMetadataEntry, Attributes::ExpiryDate::kMetadataEntry,
          Attributes::Unmounted::kMetadataEntry }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestSmokeCoAlarmCluster, ReadAttributes_FeatureMapRevisionAndDefaults)
{
    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_EQ(features, to_underlying(Feature::kSmokeAlarm) | to_underlying(Feature::kCoAlarm));

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, SmokeCoAlarm::kRevision);

    uint8_t expressedState{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::ExpressedState::Id, expressedState), CHIP_NO_ERROR);
    EXPECT_EQ(expressedState, to_underlying(ExpressedStateEnum::kNormal));

    uint8_t sensitivity{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::SmokeSensitivityLevel::Id, sensitivity), CHIP_NO_ERROR);
    EXPECT_EQ(sensitivity, to_underlying(SensitivityEnum::kHigh));
}

TEST_F(TestSmokeCoAlarmCluster, SetSmokeState_ReadBack)
{
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kWarning));
    uint8_t val{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::SmokeState::Id, val), CHIP_NO_ERROR);
    EXPECT_EQ(val, to_underlying(AlarmStateEnum::kWarning));

    // Same value is a no-op (returns true but value unchanged)
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kWarning));
    uint8_t val2{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::SmokeState::Id, val2), CHIP_NO_ERROR);
    EXPECT_EQ(val2, to_underlying(AlarmStateEnum::kWarning));
}

TEST_F(TestSmokeCoAlarmCluster, SetSmokeState_CriticalAutoUnmutes)
{
    ASSERT_TRUE(cluster.SetDeviceMuted(MuteStateEnum::kMuted));
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kCritical));
    MuteStateEnum muted{};
    ASSERT_TRUE(cluster.GetDeviceMuted(muted));
    EXPECT_EQ(muted, MuteStateEnum::kNotMuted);
}

TEST_F(TestSmokeCoAlarmCluster, SetDeviceMuted_BlockedByCriticalAlarms)
{
    struct
    {
        bool (*setter)(SmokeCoAlarmCluster &, AlarmStateEnum);
        bool (*resetter)(SmokeCoAlarmCluster &, AlarmStateEnum);
    } cases[] = {
        { [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetSmokeState(v); },
          [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetSmokeState(v); } },
        { [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetCOState(v); },
          [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetCOState(v); } },
        { [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetBatteryAlert(v); },
          [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetBatteryAlert(v); } },
        { [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetInterconnectSmokeAlarm(v); },
          [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetInterconnectSmokeAlarm(v); } },
        { [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetInterconnectCOAlarm(v); },
          [](SmokeCoAlarmCluster & c, AlarmStateEnum v) { return c.SetInterconnectCOAlarm(v); } },
    };
    for (auto & tc : cases)
    {
        ASSERT_TRUE(tc.setter(cluster, AlarmStateEnum::kCritical));
        EXPECT_FALSE(cluster.SetDeviceMuted(MuteStateEnum::kMuted));
        ASSERT_TRUE(tc.resetter(cluster, AlarmStateEnum::kNormal));
    }
}

TEST_F(TestSmokeCoAlarmCluster, SetDeviceMuted_AllowedWhenWarning)
{
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kWarning));
    EXPECT_TRUE(cluster.SetDeviceMuted(MuteStateEnum::kMuted));
    MuteStateEnum muted{};
    ASSERT_TRUE(cluster.GetDeviceMuted(muted));
    EXPECT_EQ(muted, MuteStateEnum::kMuted);
}

TEST_F(TestSmokeCoAlarmCluster, RequestSelfTest_SetsTestingState)
{
    ASSERT_TRUE(cluster.RequestSelfTest());
    ExpressedStateEnum expressed{};
    ASSERT_TRUE(cluster.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kTesting);
    EXPECT_FALSE(cluster.RequestSelfTest()); // blocked while testing
}

TEST_F(TestSmokeCoAlarmCluster, RequestSelfTest_BlockedWhenAlarming)
{
    auto checkBlocked = [&](auto setter, AlarmStateEnum alarm) {
        ASSERT_TRUE(setter(alarm));
        cluster.SetExpressedStateByPriority(kDefaultPriority);
        EXPECT_FALSE(cluster.RequestSelfTest());
        ASSERT_TRUE(setter(AlarmStateEnum::kNormal));
        cluster.SetExpressedStateByPriority(kDefaultPriority);
    };
    checkBlocked([&](AlarmStateEnum v) { return cluster.SetSmokeState(v); }, AlarmStateEnum::kWarning);
    checkBlocked([&](AlarmStateEnum v) { return cluster.SetCOState(v); }, AlarmStateEnum::kWarning);
    checkBlocked([&](AlarmStateEnum v) { return cluster.SetInterconnectSmokeAlarm(v); }, AlarmStateEnum::kWarning);
    checkBlocked([&](AlarmStateEnum v) { return cluster.SetInterconnectCOAlarm(v); }, AlarmStateEnum::kWarning);
}

TEST_F(TestSmokeCoAlarmCluster, SetExpressedStateByPriority_PicksFirst)
{
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kWarning));
    ASSERT_TRUE(cluster.SetCOState(AlarmStateEnum::kCritical));

    cluster.SetExpressedStateByPriority(kDefaultPriority); // smoke first
    ExpressedStateEnum expressed{};
    ASSERT_TRUE(cluster.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kSmokeAlarm);

    const std::array<ExpressedStateEnum, SmokeCoAlarmCluster::kPriorityOrderLength> coFirst = {
        ExpressedStateEnum::kCOAlarm,           ExpressedStateEnum::kSmokeAlarm,     ExpressedStateEnum::kBatteryAlert,
        ExpressedStateEnum::kTesting,           ExpressedStateEnum::kHardwareFault,  ExpressedStateEnum::kEndOfService,
        ExpressedStateEnum::kInterconnectSmoke, ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kInoperative,
    };
    cluster.SetExpressedStateByPriority(coFirst);
    ASSERT_TRUE(cluster.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kCOAlarm);
}

TEST_F(TestSmokeCoAlarmCluster, SetExpressedStateByPriority_AllClear)
{
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kWarning));
    cluster.SetExpressedStateByPriority(kDefaultPriority);
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kNormal));
    cluster.SetExpressedStateByPriority(kDefaultPriority);
    ExpressedStateEnum expressed{};
    ASSERT_TRUE(cluster.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kNormal);
}

TEST_F(TestSmokeCoAlarmCluster, InvokeCommand_SelfTestRequest)
{
    Commands::SelfTestRequest::Type request;

    auto ok = tester.Invoke(request);
    EXPECT_TRUE(ok.IsSuccess());
    ExpressedStateEnum expressed{};
    ASSERT_TRUE(cluster.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kTesting);

    // Now alarming → Busy
    ASSERT_TRUE(cluster.SetTestInProgress(false));
    ASSERT_TRUE(cluster.SetSmokeState(AlarmStateEnum::kWarning));
    cluster.SetExpressedStateByPriority(kDefaultPriority);
    auto busy = tester.Invoke(request);
    ASSERT_FALSE(busy.IsSuccess());
    auto busyStatus = busy.GetStatusCode();
    ASSERT_TRUE(busyStatus.has_value());
    if (busyStatus.has_value())
    {
        EXPECT_EQ(busyStatus->GetStatus(), Status::Busy);
    }
}

TEST_F(TestSmokeCoAlarmCluster, WriteAttribute_SmokeSensitivityLevel)
{
    ASSERT_EQ(tester.WriteAttribute(Attributes::SmokeSensitivityLevel::Id, static_cast<uint8_t>(SensitivityEnum::kStandard)),
              CHIP_NO_ERROR);
    uint8_t level{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::SmokeSensitivityLevel::Id, level), CHIP_NO_ERROR);
    EXPECT_EQ(level, to_underlying(SensitivityEnum::kStandard));
}

TEST_F(TestSmokeCoAlarmCluster, WriteAttribute_ReadonlyRejected)
{
    EXPECT_EQ(tester.WriteAttribute(Attributes::ExpressedState::Id, static_cast<uint8_t>(ExpressedStateEnum::kSmokeAlarm)),
              CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
    EXPECT_EQ(tester.WriteAttribute(Attributes::SmokeState::Id, static_cast<uint8_t>(AlarmStateEnum::kCritical)),
              CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
}

TEST_F(TestSmokeCoAlarmCluster, SetUnmountedState_InoperativeWhenUnmounted)
{
    SmokeCoAlarmCluster::Config cfg = MakeFullConfig();
    cfg.inoperativeWhenUnmounted    = true;
    SmokeCoAlarmCluster local(kTestEndpointId, cfg);
    ClusterTester t(local);
    ASSERT_EQ(local.Startup(t.GetServerClusterContext()), CHIP_NO_ERROR);

    ASSERT_TRUE(local.SetUnmountedState(true));
    ExpressedStateEnum expressed{};
    ASSERT_TRUE(local.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kInoperative);

    ASSERT_TRUE(local.SetUnmountedState(false));
    ASSERT_TRUE(local.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kNormal);

    local.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestSmokeCoAlarmCluster, SetAlarmState_CriticalAutoUnmutes_AllSources)
{
    // SmokeState is already covered by SetSmokeState_CriticalAutoUnmutes;
    // verify the same invariant holds for the remaining four alarm sources.
    auto runCase = [&](auto setFn) {
        ASSERT_TRUE(cluster.SetDeviceMuted(MuteStateEnum::kMuted));
        ASSERT_TRUE(setFn(AlarmStateEnum::kCritical));
        MuteStateEnum muted{};
        ASSERT_TRUE(cluster.GetDeviceMuted(muted));
        EXPECT_EQ(muted, MuteStateEnum::kNotMuted);
        ASSERT_TRUE(setFn(AlarmStateEnum::kNormal)); // reset for next iteration
    };

    runCase([&](AlarmStateEnum v) { return cluster.SetCOState(v); });
    runCase([&](AlarmStateEnum v) { return cluster.SetBatteryAlert(v); });
    runCase([&](AlarmStateEnum v) { return cluster.SetInterconnectSmokeAlarm(v); });
    runCase([&](AlarmStateEnum v) { return cluster.SetInterconnectCOAlarm(v); });
}

TEST_F(TestSmokeCoAlarmCluster, SetTestInProgress_Completion)
{
    ASSERT_TRUE(cluster.SetTestInProgress(true));
    ASSERT_TRUE(cluster.SetTestInProgress(false));
    bool inProgress{};
    ASSERT_TRUE(cluster.GetTestInProgress(inProgress));
    EXPECT_FALSE(inProgress);
}

TEST_F(TestSmokeCoAlarmCluster, RemainingAlarmAttributes_ReadBack)
{
    AlarmStateEnum alarmVal{};
    ASSERT_TRUE(cluster.SetInterconnectSmokeAlarm(AlarmStateEnum::kWarning));
    ASSERT_TRUE(cluster.GetInterconnectSmokeAlarm(alarmVal));
    EXPECT_EQ(alarmVal, AlarmStateEnum::kWarning);

    ASSERT_TRUE(cluster.SetInterconnectCOAlarm(AlarmStateEnum::kCritical));
    ASSERT_TRUE(cluster.GetInterconnectCOAlarm(alarmVal));
    EXPECT_EQ(alarmVal, AlarmStateEnum::kCritical);

    ASSERT_TRUE(cluster.SetHardwareFaultAlert(true));
    bool hw{};
    ASSERT_TRUE(cluster.GetHardwareFaultAlert(hw));
    EXPECT_TRUE(hw);

    ASSERT_TRUE(cluster.SetEndOfServiceAlert(EndOfServiceEnum::kExpired));
    EndOfServiceEnum eos{};
    ASSERT_TRUE(cluster.GetEndOfServiceAlert(eos));
    EXPECT_EQ(eos, EndOfServiceEnum::kExpired);

    ASSERT_TRUE(cluster.SetContaminationState(ContaminationStateEnum::kCritical));
    ContaminationStateEnum cs{};
    ASSERT_TRUE(cluster.GetContaminationState(cs));
    EXPECT_EQ(cs, ContaminationStateEnum::kCritical);

    uint32_t date{};
    ASSERT_TRUE(cluster.GetExpiryDate(date));
    EXPECT_EQ(date, 0u);
}
