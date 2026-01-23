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

#include <app/clusters/level-control/tests/TestLevelControlCommon.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/LevelControl/Attributes.h>
#include <clusters/LevelControl/Commands.h>
#include <clusters/LevelControl/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;

using chip::Testing::IsAttributesListEqualTo;

struct TestLevelControlOnOff : public LevelControlTestBase
{
};

TEST_F(TestLevelControlOnOff, TestExecuteIfOff)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff() };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetCurrentLevel(10), CHIP_NO_ERROR);
    mockDelegate.mOn = false;

    Commands::MoveToLevel::Type data;
    data.level = 20;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u); // Still 10
}

TEST_F(TestLevelControlOnOff, TestWriteOnLevel)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff() };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> onLevel;
    onLevel.SetNonNull(50);
    EXPECT_TRUE(tester.WriteAttribute(Attributes::OnLevel::Id, onLevel).IsSuccess());
}

TEST_F(TestLevelControlOnOff, TestMoveToLevelWithOnOffCommand)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff() };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetCurrentLevel(0), CHIP_NO_ERROR);

    // 1. Move Up to 10 with OnOff. Should Turn On immediately.
    Commands::MoveToLevelWithOnOff::Type data;
    data.level = 10;
    data.transitionTime.SetNonNull(100);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    mockDelegate.mSetOnOffCalled = false;
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    EXPECT_TRUE(mockDelegate.mSetOnOffCalled);
    EXPECT_TRUE(mockDelegate.mOn);

    // Advance to end
    while (mockTimer.IsTimerActive(&cluster))
    {
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u);

    // 2. Move Down to 0 with OnOff. Should Turn Off AT END.
    data.level                   = 0;
    mockDelegate.mSetOnOffCalled = false;

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    // Should NOT have called SetOnOff yet (it's decreasing)
    EXPECT_FALSE(mockDelegate.mSetOnOffCalled);

    // Advance to end
    while (mockTimer.IsTimerActive(&cluster))
    {
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 0u);

    // NOW it should have called SetOnOff(false)
    EXPECT_TRUE(mockDelegate.mSetOnOffCalled);
    EXPECT_FALSE(mockDelegate.mOn);
}

TEST_F(TestLevelControlOnOff, TestMoveWithOnOff)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff() };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetCurrentLevel(0), CHIP_NO_ERROR);
    mockDelegate.mOn             = false;
    mockDelegate.mSetOnOffCalled = false;

    // Move Up with OnOff -> Should turn On
    Commands::MoveWithOnOff::Type data;
    data.moveMode = MoveModeEnum::kUp;
    data.rate.SetNonNull(10);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveWithOnOff::Id, data).IsSuccess());

    EXPECT_TRUE(mockDelegate.mSetOnOffCalled);
    EXPECT_TRUE(mockDelegate.mOn);
}

TEST_F(TestLevelControlOnOff, TestStepWithOnOff)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff() };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetCurrentLevel(0), CHIP_NO_ERROR);
    mockDelegate.mOn             = false;
    mockDelegate.mSetOnOffCalled = false;

    // Step Up with OnOff -> Should turn On
    Commands::StepWithOnOff::Type data;
    data.stepMode = StepModeEnum::kUp;
    data.stepSize = 10;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::StepWithOnOff::Id, data).IsSuccess());

    EXPECT_TRUE(mockDelegate.mSetOnOffCalled);
    EXPECT_TRUE(mockDelegate.mOn);
}

TEST_F(TestLevelControlOnOff, TestOnOffAttributes)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)
                                     .WithOnOff()
                                     .WithOnOffTransitionTime(0)
                                     .WithOnTransitionTime(0)
                                     .WithOffTransitionTime(0) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {

                                            Attributes::CurrentLevel::kMetadataEntry,

                                            Attributes::Options::kMetadataEntry,

                                            Attributes::OnLevel::kMetadataEntry,

                                            Attributes::OnTransitionTime::kMetadataEntry,

                                            Attributes::OffTransitionTime::kMetadataEntry,

                                            Attributes::OnOffTransitionTime::kMetadataEntry

                                        }));
}

TEST_F(TestLevelControlOnOff, TestOnOffChanged)

{

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)

                                     .WithOnOff()

                                     .WithOnOffTransitionTime(100) }; // 10s

    chip::Testing::ClusterTester tester(cluster);

    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetCurrentLevel(200), CHIP_NO_ERROR);

    // 1. Turn OFF

    // Should store 200, move to MinLevel (0) over 10s.

    cluster.OnOffChanged(false);

    EXPECT_TRUE(mockTimer.IsTimerActive(&cluster));

    while (mockTimer.IsTimerActive(&cluster))

    {

        mockTimer.AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    DataModel::Nullable<uint8_t> readLevel;

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());

    EXPECT_EQ(readLevel.Value(), 0u);

    // 2. Turn ON

    // Should restore 200 (from stored), over 10s.

    cluster.OnOffChanged(true);

    EXPECT_TRUE(mockTimer.IsTimerActive(&cluster));

    while (mockTimer.IsTimerActive(&cluster))

    {

        mockTimer.AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());

    EXPECT_EQ(readLevel.Value(), 200u);
}

TEST_F(TestLevelControlOnOff, TestOnOffChangedDefaultLevel)
{
    // Test that if OnLevel is null and StoredLevel is unknown (null or 0?), it defaults to MaxLevel.
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff().WithOnOffTransitionTime(0)
    }; // Immediate
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 0 (Min)
    EXPECT_EQ(cluster.SetCurrentLevel(0), CHIP_NO_ERROR);

    // Turn ON. No OnLevel set. No StoredLevel (as we haven't turned off from a high level).
    // Should default to MaxLevel (254).
    cluster.OnOffChanged(true);

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 254u);
}

class ReentrantMockDelegate : public MockLevelControlDelegate
{
public:
    LevelControlCluster * mCluster = nullptr;
    void SetOnOff(bool on) override
    {
        MockLevelControlDelegate::SetOnOff(on);
        if (mCluster)
        {
            mCluster->OnOffChanged(on);
        }
    }
};

TEST_F(TestLevelControlOnOff, TestMoveToLevelWithOnOffReentrancy)
{
    ReentrantMockDelegate reentrantDelegate;
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, reentrantDelegate).WithOnOff() };
    reentrantDelegate.mCluster = &cluster;

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 0, Off
    EXPECT_EQ(cluster.SetCurrentLevel(0), CHIP_NO_ERROR);
    reentrantDelegate.mOn = false;

    // Command: Move to 254 (Max) with OnOff.
    // This triggers UpdateOnOff(true) -> OnOffChanged(true).
    // OnOffChanged sets level to Min (0) and moves to OnLevel (default 0 as current is 0).
    // This nested move sets mTargetLevel = 0.
    // Without the fix, the outer MoveToLevel uses this 0 as target and stops.

    Commands::MoveToLevelWithOnOff::Type data;
    data.level = 254;
    data.transitionTime.SetNonNull(100); // 10s transition
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    // Verify command succeeds
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    // Verify timer IS active (transition to 254 started)
    EXPECT_TRUE(mockTimer.IsTimerActive(&cluster));

    // Verify we reach 254 eventually
    // Note: MockTimer AdvanceClock might only process one event per call if they reschedule immediately.
    // With 10s transition and 254 steps, we have ~254 ticks.
    int limit = 300;
    while (mockTimer.IsTimerActive(&cluster) && limit-- > 0)
    {
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(100));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 254u);
}

TEST_F(TestLevelControlOnOff, TestImmediateMoveToMinLevelWithOnOff)
{
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff().WithMinLevel(1)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 100, Off
    EXPECT_EQ(cluster.SetCurrentLevel(100), CHIP_NO_ERROR);
    mockDelegate.mOn             = false;
    mockDelegate.mSetOnOffCalled = false;

    // Command: Move to 1 (MinLevel) with OnOff, immediate (0s)
    Commands::MoveToLevelWithOnOff::Type data;
    data.level = 1;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    // Check Level
    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 1u);

    // Check OnOff state.
    // mOn should be FALSE at the end because we moved to MinLevel.
    EXPECT_TRUE(mockDelegate.mSetOnOffCalled);
    EXPECT_FALSE(mockDelegate.mOn);
}
