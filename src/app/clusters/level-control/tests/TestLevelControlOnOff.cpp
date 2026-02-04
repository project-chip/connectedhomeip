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
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(10, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

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

TEST_F(TestLevelControlOnOff, TestExecuteIfOff_OverrideOff)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(10, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Set Options to HAVE ExecuteIfOff
    BitMask<OptionsBitmap> options;
    options.Set(OptionsBitmap::kExecuteIfOff);
    EXPECT_TRUE(tester.WriteAttribute(Attributes::Options::Id, options).IsSuccess());

    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    // Command: MoveToLevel 20.
    // Mask = ExecuteIfOff (we want to override this bit)
    // Override = 0 (we want to set it to 0, i.e., Don't Execute)
    Commands::MoveToLevel::Type data;
    data.level = 20;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.Set(OptionsBitmap::kExecuteIfOff);
    data.optionsOverride.ClearAll(); // Force bit to 0

    // Expectation: Command succeeds (status success) but DOES NOT CHANGE LEVEL because it was suppressed by override.
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u); // Should remain 10
}

TEST_F(TestLevelControlOnOff, TestWriteOnLevel)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> onLevel;
    onLevel.SetNonNull(50);
    EXPECT_TRUE(tester.WriteAttribute(Attributes::OnLevel::Id, onLevel).IsSuccess());
}

TEST_F(TestLevelControlOnOff, TestMoveToLevelWithOnOffCommand)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // 1. Move Up to 10 with OnOff. Should Turn On immediately.
    Commands::MoveToLevelWithOnOff::Type data;
    data.level = 10;
    data.transitionTime.SetNonNull(100);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    EXPECT_TRUE(onOffCluster.GetOnOff());

    // Advance to end
    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u);

    // 2. Move Down to 0 with OnOff. Should Turn Off AT END.
    data.level = 0;

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    // Should NOT have turned off yet (it's decreasing)
    EXPECT_TRUE(onOffCluster.GetOnOff());

    // Advance to end
    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 0u);

    // NOW it should have called SetOnOff(false)
    EXPECT_FALSE(onOffCluster.GetOnOff());
}

TEST_F(TestLevelControlOnOff, TestMoveWithOnOff)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initialize level to 0
    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Initial: Off
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    // Move Up with OnOff -> Should turn On
    Commands::MoveWithOnOff::Type data;
    data.moveMode = MoveModeEnum::kUp;
    data.rate.SetNonNull(10);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveWithOnOff::Id, data).IsSuccess());

    EXPECT_TRUE(onOffCluster.GetOnOff());
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));
}

TEST_F(TestLevelControlOnOff, TestStepWithOnOff)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initialize level to 0
    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Initial: Off
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    // Step Up with OnOff -> Should turn On
    Commands::StepWithOnOff::Type data;
    data.stepMode = StepModeEnum::kUp;
    data.stepSize = 10;
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::StepWithOnOff::Id, data).IsSuccess());

    EXPECT_TRUE(onOffCluster.GetOnOff());

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u);
}

TEST_F(TestLevelControlOnOff, TestOnOffAttributes)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)
                                     .WithOnOff(onOffCluster)
                                     .WithOnOffTransitionTime(0)
                                     .WithOnTransitionTime(0)
                                     .WithOffTransitionTime(0) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        { Attributes::CurrentLevel::kMetadataEntry, Attributes::Options::kMetadataEntry,
                                          Attributes::OnLevel::kMetadataEntry, Attributes::OnTransitionTime::kMetadataEntry,
                                          Attributes::OffTransitionTime::kMetadataEntry,
                                          Attributes::OnOffTransitionTime::kMetadataEntry }));
}

TEST_F(TestLevelControlOnOff, TestOnOffChanged)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    // Configure OnOffTransitionTime
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster).WithOnOffTransitionTime(100)
    }; // 10s

    onOffCluster.AddDelegate(&cluster);

    chip::Testing::ClusterTester tester(cluster);

    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial State: On
    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(200, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // 1. Turn OFF

    // Should store 200, move to MinLevel (0) over 10s.
    // Then, because OnLevel is null, it should RESTORE the stored level (200) at the end.

    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    DataModel::Nullable<uint8_t> readLevel;

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());

    // Spec: If OnLevel is not defined, set the CurrentLevel to the stored level.
    // Stored level was 200.
    EXPECT_EQ(readLevel.Value(), 200u);

    // 2. Turn ON

    // Should restore 200 (from stored), over 10s.

    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);

    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());

    EXPECT_EQ(readLevel.Value(), 200u);
}

TEST_F(TestLevelControlOnOff, TestOnOffChangedDefaultLevel)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    // Test that if OnLevel is null and StoredLevel is unknown (null or 0?), it defaults to MaxLevel.
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster).WithOnOffTransitionTime(0)
    }; // Immediate
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 0 (Min)
    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Turn ON. No OnLevel set. No StoredLevel (as we haven't turned off from a high level).
    // Should default to MaxLevel (254).
    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 254u);
}

TEST_F(TestLevelControlOnOff, TestRestorationBehaviorWhenOnLevelNull)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    // Reproduce TC-LVL-3.1 scenario:
    // Device has Lighting (MinLevel=1) and OnOff.
    // OnLevel is null (default).
    // When turning Off, it should transition to MinLevel but then RESTORE the stored level.

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)
                                     .WithLighting(DataModel::NullNullable) // MinLevel=1
                                     .WithOnOff(onOffCluster) };            // Dependency active
    onOffCluster.AddDelegate(&cluster);

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 100, On
    EXPECT_TRUE(cluster
                    .MoveToLevel(100, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Turn Off
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    // Expectation:
    // 1. Stored Level = 100.
    // 2. Transition to MinLevel (1).
    // 3. Since OnLevel is null, restore Stored Level (100).
    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 100u);
}

TEST_F(TestLevelControlOnOff, TestMoveToLevelWithOnOffReentrancy)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);

    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 0, Off
    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

    // Command: Move to 254 (Max) with OnOff.
    // This triggers SetOnOff(true) -> OnOffChanged(true).
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
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Verify we reach 254 eventually
    int limit = 300;
    while (mockTimer.IsTimerActive(nullptr) && limit-- > 0)
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 254u);
}

TEST_F(TestLevelControlOnOff, TestStoredLevelCorruption)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // 1. Set Level 200. On.
    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster
                    .MoveToLevel(200, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Prime mLevelBeforeTurnedOff by turning Off and On once
    // Off -> Stores 200.
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);
    while (mockTimer.IsTimerActive(nullptr))
        AdvanceClock(System::Clock::Milliseconds64(1000));
    EXPECT_FALSE(onOffCluster.GetOnOff());

    // On -> Restores 200.
    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);
    while (mockTimer.IsTimerActive(nullptr))
        AdvanceClock(System::Clock::Milliseconds64(1000));
    EXPECT_TRUE(onOffCluster.GetOnOff());

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 200u);

    // 2. MoveToLevelWithOnOff(MinLevel, time=10s).
    // This should NOT overwrite mLevelBeforeTurnedOff (200) with MinLevel (0/1).
    uint8_t minLevel = cluster.GetMinLevel();
    Commands::MoveToLevelWithOnOff::Type data;
    data.level = minLevel;
    data.transitionTime.SetNonNull(100);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevelWithOnOff::Id, data).IsSuccess());

    // 3. Advance time to finish.
    int limit = 200;
    while (mockTimer.IsTimerActive(nullptr) && limit-- > 0)
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    EXPECT_FALSE(onOffCluster.GetOnOff());

    // 4. Turn On (Simulate OnOff Cluster On command)
    // Should restore 200 (from mLevelBeforeTurnedOff) NOT MinLevel (0/1).
    EXPECT_EQ(onOffCluster.SetOnOff(true), CHIP_NO_ERROR);

    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    // 5. Check Level.
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 200u);
}

TEST_F(TestLevelControlOnOff, TestImmediateMoveToMinLevelWithOnOff)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster).WithMinLevel(1)
    };
    onOffCluster.AddDelegate(&cluster);
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: Level 100, Off
    EXPECT_TRUE(cluster
                    .MoveToLevel(100, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR);

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
    EXPECT_FALSE(onOffCluster.GetOnOff());
}
