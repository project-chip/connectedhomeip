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

// Level Control side of the Options.CoupleColorTempToLevel behavior (spec 1.6.6.5). The Color Control
// side of the same feature — the level to mireds mapping — is covered by
// src/app/clusters/color-control-server/tests/TestColorControlCoupling.cpp.

#include <app/clusters/color-control-server/ColorControlIntegrationDelegate.h>
#include <app/clusters/level-control/tests/TestLevelControlCommon.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <clusters/LevelControl/Attributes.h>
#include <clusters/LevelControl/Commands.h>

#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;

namespace {

class MockColorControlIntegrationDelegate : public ColorControlIntegrationDelegate
{
public:
    void CoupleColorTempToLevel(uint8_t currentLevel) override { mCoupledLevels.push_back(currentLevel); }

    std::vector<uint8_t> mCoupledLevels;
};

struct TestLevelControlColorCoupling : public LevelControlTestBase
{
    MockColorControlIntegrationDelegate mockColorControl;

    // Writes the Options attribute so the coupling gate is open. Options has no N quality, so it always
    // starts at 0 and has to be set explicitly.
    void EnableCoupling(chip::Testing::ClusterTester & tester)
    {
        BitMask<OptionsBitmap> options;
        options.Set(OptionsBitmap::kCoupleColorTempToLevel);
        EXPECT_TRUE(tester.WriteAttribute(Attributes::Options::Id, options).IsSuccess());
    }
};

// Without the option bit, a level change must not reach Color Control.
TEST_F(TestLevelControlColorCoupling, TestNoCouplingWhileOptionBitClear)
{
    LevelControlCluster cluster{ kTestEndpointId,
                                 LevelControlCluster::Config(mockTimer, mockDelegate).WithColorControl(mockColorControl) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    EXPECT_TRUE(mockColorControl.mCoupledLevels.empty());
}

// With the bit set, the new level is forwarded.
TEST_F(TestLevelControlColorCoupling, TestCouplesOnLevelChange)
{
    LevelControlCluster cluster{ kTestEndpointId,
                                 LevelControlCluster::Config(mockTimer, mockDelegate).WithColorControl(mockColorControl) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EnableCoupling(tester);

    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    ASSERT_EQ(mockColorControl.mCoupledLevels.size(), 1u);
    EXPECT_EQ(mockColorControl.mCoupledLevels.back(), 100u);
}

// A timed transition couples every intermediate step, so the color temperature tracks the ramp rather
// than jumping once at the end.
TEST_F(TestLevelControlColorCoupling, TestCouplesOnEveryTransitionStep)
{
    LevelControlCluster cluster{ kTestEndpointId,
                                 LevelControlCluster::Config(mockTimer, mockDelegate)
                                     .WithColorControl(mockColorControl)
                                     .WithInitialCurrentLevel(10) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EnableCoupling(tester);

    // 5 units over 1 second.
    Commands::MoveToLevel::Type data;
    data.level = 15;
    data.transitionTime.SetNonNull(10);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    EXPECT_EQ(mockColorControl.mCoupledLevels, (std::vector<uint8_t>{ 11, 12, 13, 14, 15 }));
}

// The option bit stays inert on an endpoint with no Color Control: nothing is wired, nothing crashes.
TEST_F(TestLevelControlColorCoupling, TestOptionBitInertWithoutColorControl)
{
    LevelControlCluster cluster{ kTestEndpointId, LevelControlCluster::Config(mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EnableCoupling(tester);

    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 100u);
}

// Startup restores CurrentLevel but must not couple: Options is volatile, so the bit always reads 0 at
// this point, and Color Control may not have started yet.
TEST_F(TestLevelControlColorCoupling, TestNoCouplingDuringStartup)
{
    LevelControlCluster cluster{ kTestEndpointId,
                                 LevelControlCluster::Config(mockTimer, mockDelegate)
                                     .WithColorControl(mockColorControl)
                                     .WithInitialCurrentLevel(42) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(mockColorControl.mCoupledLevels.empty());

    BitMask<OptionsBitmap> options;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::Options::Id, options).IsSuccess());
    EXPECT_FALSE(options.Has(OptionsBitmap::kCoupleColorTempToLevel));
}

} // namespace
