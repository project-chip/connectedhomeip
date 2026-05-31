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

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SmokeCoAlarm;

namespace {

constexpr EndpointId kTestEndpointId = 1; // matches old-style callers that pass endpoint 1

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

// Fixture that owns a server and exposes it only through Instance() — the old singleton pattern.
struct TestSmokeCoAlarmBackwardsCompatInitialized : public TestSmokeCoAlarmBackwardsCompat
{
    void SetUp() override { ASSERT_EQ(server.Init(), CHIP_NO_ERROR); }
    SmokeCoAlarmServer server{ kTestEndpointId, MakeFullConfig() };
};

} // namespace

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, Instance_ReturnsRegisteredServer)
{
    // Old code calls SmokeCoAlarmServer::Instance() without holding a pointer.
    EXPECT_EQ(&SmokeCoAlarmServer::Instance(), &server);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, Init_RegistersClusterInRegistry)
{
    auto * reg = CodegenDataModelProvider::Instance().Registry().Get(ConcreteClusterPath(kTestEndpointId, SmokeCoAlarm::Id));
    EXPECT_NE(reg, nullptr);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, DoubleInit_IsNoOp)
{
    EXPECT_EQ(server.Init(), CHIP_NO_ERROR);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, Features_ReportedCorrectly)
{
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SupportsSmokeAlarm());
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SupportsCOAlarm());
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetFeatures().Has(Feature::kSmokeAlarm));
}

// Mirrors the pattern used by SmokeCOAlarmManager.cpp: Instance().SetXxx(endpointId, value)
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

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetBatteryAlert(kTestEndpointId, AlarmStateEnum::kWarning)));
    AlarmStateEnum battery{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetBatteryAlert(kTestEndpointId, battery));
    EXPECT_EQ(battery, AlarmStateEnum::kWarning);

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetHardwareFaultAlert(kTestEndpointId, true)));
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    bool hwFault{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetHardwareFaultAlert(kTestEndpointId, hwFault));
    EXPECT_TRUE(hwFault);

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetEndOfServiceAlert(kTestEndpointId, EndOfServiceEnum::kExpired)));
    SmokeCoAlarmServer::Instance().SetExpressedStateByPriority(kTestEndpointId, sPriorityOrder);
    EndOfServiceEnum eos{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetEndOfServiceAlert(kTestEndpointId, eos));
    EXPECT_EQ(eos, EndOfServiceEnum::kExpired);

    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetSmokeSensitivityLevel(kTestEndpointId, SensitivityEnum::kLow)));
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
    EXPECT_TRUE((SmokeCoAlarmServer::Instance().SetInterconnectCOAlarm(kTestEndpointId, AlarmStateEnum::kWarning)));
    SmokeCoAlarmServer::Instance().SetContaminationState(kTestEndpointId, ContaminationStateEnum::kCritical);
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

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, GettersViaEndpointOverloads_Delegate)
{
    // Verify endpoint-taking Get* overloads compile and forward to the cluster.
    SmokeCoAlarmServer::Instance().SetSmokeState(kTestEndpointId, AlarmStateEnum::kWarning);

    AlarmStateEnum smoke{};
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetSmokeState(kTestEndpointId, smoke));
    EXPECT_EQ(smoke, AlarmStateEnum::kWarning);

    EXPECT_TRUE(SmokeCoAlarmServer::Instance().GetFeatures(kTestEndpointId).Has(Feature::kSmokeAlarm));
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SupportsSmokeAlarm(kTestEndpointId));
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SupportsCOAlarm(kTestEndpointId));
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, SetTestInProgress_ViaInstance)
{
    EXPECT_TRUE(SmokeCoAlarmServer::Instance().SetTestInProgress(kTestEndpointId, true));
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
