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

constexpr EndpointId kTestEndpointId = kRootEndpointId;

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

struct TestSmokeCoAlarmBackwardsCompatInitialized : public TestSmokeCoAlarmBackwardsCompat
{
    void SetUp() override { ASSERT_EQ(server.Init(), CHIP_NO_ERROR); }
    SmokeCoAlarmServer server{ kTestEndpointId, MakeFullConfig() };
};

} // namespace

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
    EXPECT_TRUE(server.SupportsSmokeAlarm());
    EXPECT_TRUE(server.SupportsCOAlarm());
    EXPECT_TRUE(server.GetFeatures().Has(Feature::kSmokeAlarm));
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, SettersAndGetters_RoundTrip)
{
    // Alarm states
    EXPECT_TRUE(server.SetSmokeState(AlarmStateEnum::kWarning));
    AlarmStateEnum smoke{};
    EXPECT_TRUE(server.GetSmokeState(smoke));
    EXPECT_EQ(smoke, AlarmStateEnum::kWarning);

    EXPECT_TRUE(server.SetCOState(AlarmStateEnum::kCritical));
    AlarmStateEnum co{};
    EXPECT_TRUE(server.GetCOState(co));
    EXPECT_EQ(co, AlarmStateEnum::kCritical);

    EXPECT_TRUE(server.SetBatteryAlert(AlarmStateEnum::kWarning));
    AlarmStateEnum battery{};
    EXPECT_TRUE(server.GetBatteryAlert(battery));
    EXPECT_EQ(battery, AlarmStateEnum::kWarning);

    // Boolean alerts
    EXPECT_TRUE(server.SetHardwareFaultAlert(true));
    bool hwFault{};
    EXPECT_TRUE(server.GetHardwareFaultAlert(hwFault));
    EXPECT_TRUE(hwFault);

    EXPECT_TRUE(server.SetEndOfServiceAlert(EndOfServiceEnum::kExpired));
    EndOfServiceEnum eos{};
    EXPECT_TRUE(server.GetEndOfServiceAlert(eos));
    EXPECT_EQ(eos, EndOfServiceEnum::kExpired);

    // Sensitivity
    EXPECT_TRUE(server.SetSmokeSensitivityLevel(SensitivityEnum::kLow));
    SensitivityEnum sensitivity{};
    EXPECT_TRUE(server.GetSmokeSensitivityLevel(sensitivity));
    EXPECT_EQ(sensitivity, SensitivityEnum::kLow);
}

TEST_F(TestSmokeCoAlarmBackwardsCompatInitialized, RequestSelfTest_ForwardsToCluster)
{
    EXPECT_TRUE(server.RequestSelfTest());
    ExpressedStateEnum expressed{};
    EXPECT_TRUE(server.GetExpressedState(expressed));
    EXPECT_EQ(expressed, ExpressedStateEnum::kTesting);
}
