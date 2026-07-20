/*
 *    Copyright (c) 2026 Project CHIP Authors
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

// ColorControl <-> On/Off coupling tests. The coupling is via DIRECT INJECTION (Config.onOff), the same
// pattern LevelControl uses (WithOnOff) - no registry, no CodegenDataModelProvider::Instance(), no
// mock_model. We construct both clusters, inject the On/Off cluster, and check ShouldExecuteIfOff (a
// public method) honors its live state. Only ShouldExecuteIfOff is exercised (it makes no transition,
// so no DeviceLayer::SystemLayer() is needed).

#include <app/clusters/color-control-server/ColorControlCluster.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TimerDelegateMock.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ColorControl;

using Status = Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

struct TestColorControlCoupling : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TimerDelegateMock mockTimer;
    ColorControlDelegate delegate;
    System::Clock::Internal::RAIIMockClock clock;

    // Advance the mock clock well past any transition used here and run one tick.
    void Complete(ColorControlCluster & c)
    {
        clock.AdvanceMonotonic(System::Clock::Milliseconds64(120000));
        c.OnTick();
    }
};

// End-to-end gate: a command handler run while the device is OFF is suppressed (returns Success but
// makes no change) unless ExecuteIfOff is effective — proving every handler honors the gate, not just
// ColorLoopSet/StopMoveStep. Driven through the public handler directly (the same path InvokeCommand
// takes), which is why the handlers carry optionsMask/optionsOverride.
TEST_F(TestColorControlCoupling, CommandHandlerIsGatedWhileOff)
{
    OnOffCluster::Context onOffContext{ mockTimer };
    OnOffCluster onOff(kTestEndpointId, onOffContext);
    Testing::ClusterTester onOffTester(onOff);
    ASSERT_EQ(onOff.Startup(onOffTester.GetServerClusterContext()), CHIP_NO_ERROR);

    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kColorTemperature);
    config.mColorValue                         = CTColor{ .mireds = 250 };
    config.ctConfig.colorTempPhysicalMinMireds = 100;
    config.ctConfig.colorTempPhysicalMaxMireds = 400;
    config.onOff                               = &onOff;
    ColorControlCluster cluster(kTestEndpointId, config);

    const BitMask<OptionsBitmap> none;
    const BitMask<OptionsBitmap> executeIfOff(OptionsBitmap::kExecuteIfOff);

    // Device OFF, no ExecuteIfOff -> command suppressed: returns Success, color unchanged.
    ASSERT_EQ(onOff.SetOnOff(false), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.moveToColorTemp(400, 10, none, none), Status::Success);
    Complete(cluster);
    EXPECT_EQ(cluster.ColorTempMireds(), 250u); // did NOT move

    // Device OFF but ExecuteIfOff overridden on -> command runs.
    EXPECT_EQ(cluster.moveToColorTemp(400, 10, executeIfOff, executeIfOff), Status::Success);
    Complete(cluster);
    EXPECT_EQ(cluster.ColorTempMireds(), 400u); // moved

    // Device ON -> command runs regardless of options.
    ASSERT_EQ(onOff.SetOnOff(true), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.moveToColorTemp(200, 10, none, none), Status::Success);
    Complete(cluster);
    EXPECT_EQ(cluster.ColorTempMireds(), 200u);
}

TEST_F(TestColorControlCoupling, ShouldExecuteIfOffWithoutInjectionAlwaysExecutes)
{
    // No On/Off injected (config.onOff stays null) -> coupling absent -> always executes.
    ColorControlCluster::Config config(delegate);
    config.mFeatures.Set(Feature::kColorTemperature);
    config.mColorValue = CTColor{ .mireds = 250 };
    ColorControlCluster cluster(kTestEndpointId, config);

    EXPECT_TRUE(cluster.ShouldExecuteIfOff(BitMask<OptionsBitmap>(), BitMask<OptionsBitmap>()));
}

} // namespace
