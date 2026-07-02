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

#include <app/ConcreteClusterPath.h>
#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>
#include <clusters/SmokeCoAlarm/ClusterId.h>
#include <clusters/SmokeCoAlarm/Enums.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/core/DataModelTypes.h>
#include <pw_unit_test/framework.h>

// Required when linking against the codegen mock model.
void InitDataModelHandler() {}

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

namespace {

constexpr chip::EndpointId kTestEndpointId = 1; // matches old-style callers that pass endpoint 1

static std::array<ExpressedStateEnum, SmokeCoAlarmServer::kPriorityOrderLength> sPriorityOrder = {
    ExpressedStateEnum::kInoperative, ExpressedStateEnum::kSmokeAlarm,     ExpressedStateEnum::kInterconnectSmoke,
    ExpressedStateEnum::kCOAlarm,     ExpressedStateEnum::kInterconnectCO, ExpressedStateEnum::kHardwareFault,
    ExpressedStateEnum::kTesting,     ExpressedStateEnum::kEndOfService,   ExpressedStateEnum::kBatteryAlert
};

SmokeCoAlarmCluster::Config MakeFullConfig()
{
    SmokeCoAlarmCluster::Config cfg;
    cfg.featureMap.Set(Feature::kSmokeAlarm).Set(Feature::kCoAlarm);
    cfg.optionalAttribs = SmokeCoAlarmCluster::OptionalAttributeSet(SmokeCoAlarmCluster::OptionalAttributeSet::All());
    return cfg;
}

struct TestSmokeCoAlarmBackwardsCompat : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

// Fixture that initialises the singleton — the old pattern: call Instance() without holding a pointer.
struct TestSmokeCoAlarmBackwardsCompatInitialized : public TestSmokeCoAlarmBackwardsCompat
{
    void SetUp() override { ASSERT_EQ(SmokeCoAlarmServer::Instance().Init(kTestEndpointId, MakeFullConfig()), CHIP_NO_ERROR); }
    void TearDown() override
    {
        SmokeCoAlarmServer & inst = SmokeCoAlarmServer::Instance();
        inst.~SmokeCoAlarmServer();
        new (&inst) SmokeCoAlarmServer{};
    }
};

// Fixture with no optional attributes configured — verifies absent-attribute guards return false.
struct TestSmokeCoAlarmBackwardsCompatNoOptionals : public TestSmokeCoAlarmBackwardsCompat
{
    void SetUp() override
    {
        SmokeCoAlarmCluster::Config cfg;
        cfg.featureMap.Set(Feature::kSmokeAlarm).Set(Feature::kCoAlarm);
        // optionalAttribs left empty
        ASSERT_EQ(SmokeCoAlarmServer::Instance().Init(kTestEndpointId, cfg), CHIP_NO_ERROR);
    }
    void TearDown() override
    {
        SmokeCoAlarmServer & inst = SmokeCoAlarmServer::Instance();
        inst.~SmokeCoAlarmServer();
        new (&inst) SmokeCoAlarmServer{};
    }
};

} // namespace

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, Instance_ClusterIsRegistered)
{
    auto * reg = chip::app::CodegenDataModelProvider::Instance().Registry().Get(
        chip::app::ConcreteClusterPath(kTestEndpointId, SmokeCoAlarm::Id));
    EXPECT_NE(reg, nullptr);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, DoubleInit_IsNoOp)
{
    EXPECT_EQ(SmokeCoAlarmServer::Instance().Init(kTestEndpointId, MakeFullConfig()), CHIP_NO_ERROR);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, Features_ReportedCorrectly)
{
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetFeatures(kTestEndpointId).Has(Feature::kSmokeAlarm));
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SupportsSmokeAlarm(kTestEndpointId));
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SupportsCOAlarm(kTestEndpointId));
}

// Mirrors the pattern used by SmokeCoAlarmManager.cpp: Instance().SetXxx(endpointId, value)
// followed by Instance().SetExpressedStateByPriority(endpointId, priorityOrder).
TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, SettersViaInstance_RoundTrip)
{
    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetSmokeState(kTestEndpointId, AlarmStateEnum::kCritical)));
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    AlarmStateEnum smoke{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetSmokeState(kTestEndpointId, smoke));
    EXPECT_EQ(smoke, AlarmStateEnum::kCritical);
    ExpressedStateEnum expressed{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetExpressedState(kTestEndpointId, expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kSmokeAlarm);

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetCOState(kTestEndpointId, AlarmStateEnum::kWarning)));
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    AlarmStateEnum co{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetCOState(kTestEndpointId, co));
    EXPECT_EQ(co, AlarmStateEnum::kWarning);

    SmokeCoAlarmServer::Instance().SetBatteryAlert(kTestEndpointId, AlarmStateEnum::kWarning);
    AlarmStateEnum battery{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetBatteryAlert(kTestEndpointId, battery));
    EXPECT_EQ(battery, AlarmStateEnum::kWarning);

    SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(kTestEndpointId, true);
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    bool hwFault{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetHardwareFaultAlert(kTestEndpointId, hwFault));
    EXPECT_TRUE(hwFault);

    SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(kTestEndpointId, EndOfServiceEnum::kExpired);
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    EndOfServiceEnum eos{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetEndOfServiceAlert(kTestEndpointId, eos));
    EXPECT_EQ(eos, EndOfServiceEnum::kExpired);

    SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(kTestEndpointId, SensitivityEnum::kLow);
    SensitivityEnum sensitivity{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetSmokeSensitivityLevel(kTestEndpointId, sensitivity));
    EXPECT_EQ(sensitivity, SensitivityEnum::kLow);

    // Muting is blocked while any alarm is Critical; clear smoke first.
    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetSmokeState(kTestEndpointId, AlarmStateEnum::kNormal)));
    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetDeviceMuted(kTestEndpointId, MuteStateEnum::kMuted)));
    MuteStateEnum muted{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetDeviceMuted(kTestEndpointId, muted));
    EXPECT_EQ(muted, MuteStateEnum::kMuted);

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(kTestEndpointId, AlarmStateEnum::kWarning)));
    AlarmStateEnum interconnectSmoke{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetInterconnectSmokeAlarm(kTestEndpointId, interconnectSmoke));
    EXPECT_EQ(interconnectSmoke, AlarmStateEnum::kWarning);

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(kTestEndpointId, AlarmStateEnum::kWarning)));
    AlarmStateEnum interconnectCO{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetInterconnectCOAlarm(kTestEndpointId, interconnectCO));
    EXPECT_EQ(interconnectCO, AlarmStateEnum::kWarning);

    SmokeCoAlarmServer::Instance().SetContaminationState(kTestEndpointId, ContaminationStateEnum::kCritical);
    ContaminationStateEnum contamination{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetContaminationState(kTestEndpointId, contamination));
    EXPECT_EQ(contamination, ContaminationStateEnum::kCritical);

    SmokeCoAlarmServer::Instance().SetExpiryDate(kTestEndpointId, 1234567890);
    uint32_t expiry{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetExpiryDate(kTestEndpointId, expiry));
    EXPECT_EQ(expiry, 1234567890u);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, ClearViaInstance_ResetsState)
{
    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetSmokeState(kTestEndpointId, AlarmStateEnum::kCritical)));
    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetSmokeState(kTestEndpointId, AlarmStateEnum::kNormal)));
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    AlarmStateEnum smoke{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetSmokeState(kTestEndpointId, smoke));
    EXPECT_EQ(smoke, AlarmStateEnum::kNormal);
    ExpressedStateEnum expressed{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetExpressedState(kTestEndpointId, expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kNormal);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, RequestSelfTest_ForwardsToCluster)
{
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().RequestSelfTest(kTestEndpointId));
    ExpressedStateEnum expressed{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetExpressedState(kTestEndpointId, expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kTesting);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, SetTestInProgress_ViaInstance)
{
    SmokeCoAlarmServer::Instance().SetTestInProgress(kTestEndpointId, true);
    bool v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetTestInProgress(kTestEndpointId, v));
    EXPECT_TRUE(v);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, SetUnmountedState_ViaInstance)
{
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SetUnmountedState(kTestEndpointId, true));
    bool v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetUnmountedState(kTestEndpointId, v));
    EXPECT_TRUE(v);
}

// ---- Tests for absent optional attributes (no optionalAttribs configured) ----

TEST_F(TestSmokeCoAlarmBackwardsCompatNoOptionals, DeviceMuted_SetRejected_GetReturnsDefault)
{
    EXPECT_FALSE(SmokeCoAlarmServer::Instance().SetDeviceMuted(kTestEndpointId, MuteStateEnum::kMuted));
    MuteStateEnum v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetDeviceMuted(kTestEndpointId, v));
    EXPECT_EQ(v, MuteStateEnum::kNotMuted);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatNoOptionals, InterconnectSmokeAlarm_SetRejected_GetReturnsDefault)
{
    EXPECT_FALSE(SmokeCoAlarmServer::Instance().SetInterconnectSmokeAlarm(kTestEndpointId, AlarmStateEnum::kWarning));
    AlarmStateEnum v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetInterconnectSmokeAlarm(kTestEndpointId, v));
    EXPECT_EQ(v, AlarmStateEnum::kNormal);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatNoOptionals, InterconnectCOAlarm_SetRejected_GetReturnsDefault)
{
    EXPECT_FALSE(SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(kTestEndpointId, AlarmStateEnum::kWarning));
    AlarmStateEnum v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetInterconnectCOAlarm(kTestEndpointId, v));
    EXPECT_EQ(v, AlarmStateEnum::kNormal);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatNoOptionals, ExpiryDate_GetReturnsDefault)
{
    uint32_t v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetExpiryDate(kTestEndpointId, v));
    EXPECT_EQ(v, 0u);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatNoOptionals, UnmountedState_SetRejected_GetReturnsDefault)
{
    EXPECT_FALSE(SmokeCoAlarmServer::Instance().SetUnmountedState(kTestEndpointId, true));
    bool v{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetUnmountedState(kTestEndpointId, v));
    EXPECT_FALSE(v);
}
