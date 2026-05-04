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
#include <app/persistence/AttributePersistence.h>
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

using chip::Protocols::InteractionModel::Status;
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

struct TestLevelControlBase : public LevelControlTestBase
{
};

TEST_F(TestLevelControlBase, TestAcceptedCommands)
{
    // Verify that all commands are present even without the OnOff feature enabled
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::MoveToLevel::kMetadataEntry,
                                                  Commands::Move::kMetadataEntry,
                                                  Commands::Step::kMetadataEntry,
                                                  Commands::Stop::kMetadataEntry,
                                                  Commands::MoveToLevelWithOnOff::kMetadataEntry,
                                                  Commands::MoveWithOnOff::kMetadataEntry,
                                                  Commands::StepWithOnOff::kMetadataEntry,
                                                  Commands::StopWithOnOff::kMetadataEntry,
                                              }));
}

TEST_F(TestLevelControlBase, TestReadAttributes)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Cluster Revision
    uint16_t clusterRevision{};
    EXPECT_TRUE(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision).IsSuccess());
    EXPECT_EQ(clusterRevision, LevelControl::kRevision);

    // Current Level
    DataModel::Nullable<uint8_t> currentLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, currentLevel).IsSuccess());
    EXPECT_TRUE(currentLevel.IsNull());
}

TEST_F(TestLevelControlBase, TestWriteOptions)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    BitMask<OptionsBitmap> options;
    options.Set(OptionsBitmap::kExecuteIfOff);

    EXPECT_TRUE(tester.WriteAttribute(Attributes::Options::Id, options).IsSuccess());
}

TEST_F(TestLevelControlBase, TestReadMinMaxLevel)
{
    // Default
    {
        LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        uint8_t minLevel;
        EXPECT_FALSE(tester.ReadAttribute(Attributes::MinLevel::Id, minLevel).IsSuccess());
    }

    // Configured
    {
        LevelControlCluster cluster{
            LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithMinLevel(10).WithMaxLevel(200)
        };
        chip::Testing::ClusterTester tester(cluster);
        EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        uint8_t minLevel = 0;
        EXPECT_TRUE(tester.ReadAttribute(Attributes::MinLevel::Id, minLevel).IsSuccess());
        EXPECT_EQ(minLevel, 10u);
    }
}

TEST_F(TestLevelControlBase, TestDefaultMoveRate)
{
    // Configured
    {
        LevelControlCluster cluster{
            LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithDefaultMoveRate(50)
        };
        chip::Testing::TestServerClusterContext context;
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        chip::Testing::ClusterTester tester(cluster);

        DataModel::Nullable<uint8_t> defaultMoveRate;
        EXPECT_TRUE(tester.ReadAttribute(Attributes::DefaultMoveRate::Id, defaultMoveRate).IsSuccess());
        EXPECT_EQ(defaultMoveRate.Value(), 50u);
    }
}

TEST_F(TestLevelControlBase, TestWriteDefaultMoveRateZero)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithDefaultMoveRate(50) };
    chip::Testing::ClusterTester tester(cluster);
    // EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(tester.WriteAttribute(Attributes::DefaultMoveRate::Id, DataModel::MakeNullable<uint8_t>(0)), Status::ConstraintError);
}

TEST_F(TestLevelControlBase, TestAttributes)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Full cluster has base attributes
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::CurrentLevel::kMetadataEntry,
                                            Attributes::Options::kMetadataEntry,
                                            Attributes::OnLevel::kMetadataEntry,
                                        }));
}

TEST_F(TestLevelControlBase, TestDelegateCallbacks)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithDefaultMoveRate(50) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    BitMask<OptionsBitmap> options;
    options.Set(OptionsBitmap::kExecuteIfOff);
    EXPECT_TRUE(tester.WriteAttribute(Attributes::Options::Id, options).IsSuccess());
    EXPECT_EQ(mockDelegate.mOptions, options);
}

TEST_F(TestLevelControlBase, TestMaxLevelConstraint)
{
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithMinLevel(1).WithMaxLevel(200)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(10, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    Commands::MoveToLevel::Type data;
    data.level = 201; // > MaxLevel
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_FALSE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
}

TEST_F(TestLevelControlBase, TestMoveRateZero)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::Move::Type data;
    data.moveMode = MoveModeEnum::kUp;
    data.rate.SetNonNull(0); // Invalid
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_EQ(tester.Invoke(Commands::Move::Id, data).status, Protocols::InteractionModel::Status::InvalidCommand);
}

TEST_F(TestLevelControlBase, TestStepSizeZero)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::Step::Type data;
    data.stepMode = StepModeEnum::kUp;
    data.stepSize = 0; // Invalid
    data.transitionTime.SetNonNull(0);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_EQ(tester.Invoke(Commands::Step::Id, data).status, Protocols::InteractionModel::Status::InvalidCommand);
}

TEST_F(TestLevelControlBase, TestNormalConfigWorks)
{
    LevelControlCluster::Config config(kTestEndpointId, mockTimer, mockDelegate);
    config.WithMinLevel(10).WithMaxLevel(200);

    EXPECT_EQ(config.mMinLevel, 10u);
    EXPECT_EQ(config.mMaxLevel, 200u);
}

TEST_F(TestLevelControlBase, TestImmediateExecution)
{
    // Use 0 transition time to ensure immediate execution in Full Cluster
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(0); // 0 = Immediate
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    // Check Level immediately
    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 100u);

    EXPECT_EQ(mockDelegate.mLevel, 100u);
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));
}

TEST_F(TestLevelControlBase, TestGetters)
{
    LevelControlCluster::Config config(kTestEndpointId, mockTimer, mockDelegate);
    config.WithMinLevel(10)
        .WithMaxLevel(200)
        .WithDefaultMoveRate(50)
        .WithOnTransitionTime(100)
        .WithOffTransitionTime(200)
        .WithOnOffTransitionTime(300);

    LevelControlCluster cluster(config);

    EXPECT_EQ(cluster.GetMinLevel(), 10u);
    EXPECT_EQ(cluster.GetMaxLevel(), 200u);
    EXPECT_EQ(cluster.GetDefaultMoveRate().Value(), 50u);
    EXPECT_EQ(cluster.GetOnTransitionTime().Value(), 100u);
    EXPECT_EQ(cluster.GetOffTransitionTime().Value(), 200u);
    EXPECT_EQ(cluster.GetOnOffTransitionTime(), 300u);
    EXPECT_EQ(cluster.GetRemainingTime(), 0u);
}

TEST_F(TestLevelControlBase, TestFeatureMap)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster::Config config(kTestEndpointId, mockTimer, mockDelegate);
    config.WithOnOff(onOffCluster).WithLighting(DataModel::NullNullable);

    LevelControlCluster cluster(config);

    EXPECT_TRUE(cluster.GetFeatureMap().Has(LevelControl::Feature::kOnOff));
    EXPECT_TRUE(cluster.GetFeatureMap().Has(LevelControl::Feature::kLighting));
}

TEST_F(TestLevelControlBase, TestStateGetters)
{
    LevelControlCluster::Config config(kTestEndpointId, mockTimer, mockDelegate);
    LevelControlCluster cluster(config);

    EXPECT_TRUE(cluster
                    .MoveToLevel(100, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    BitMask<LevelControl::OptionsBitmap> options;
    options.Set(LevelControl::OptionsBitmap::kExecuteIfOff);
    cluster.SetOptions(options);

    EXPECT_EQ(cluster.GetCurrentLevel().Value(), 100u);
    EXPECT_EQ(cluster.GetOptions(), options);
}

// Transitions (Basic)

TEST_F(TestLevelControlBase, TestMoveToLevelCommand)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(100); // 10 seconds
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    // Timer should be active
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Level should still be 0 (time hasn't passed)
    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 0u);
}

TEST_F(TestLevelControlBase, TestMoveToLevel_NullCurrentLevel)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial state: CurrentLevel is Null (default)
    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_TRUE(readLevel.IsNull());

    // Command: Move to 100.
    // Should behave as immediate move (transition from unknown to known).
    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(10); // Should be ignored/treated as immediate
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 100u);
}

TEST_F(TestLevelControlBase, TestTimerFired)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Move to 2 over 2 seconds (20ds). 2 steps. 1000ms per step.
    Commands::MoveToLevel::Type data;
    data.level = 2;
    data.transitionTime.SetNonNull(20);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Advance 1000ms -> Level 1
    AdvanceClock(System::Clock::Milliseconds64(1000));

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 1u);
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr)); // Still going

    // Advance 1000ms -> Level 2
    AdvanceClock(System::Clock::Milliseconds64(1000));

    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 2u);

    // Timer should stop
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));
}

TEST_F(TestLevelControlBase, TestStopCommand)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Start transition
    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNonNull(100);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Stop
    Commands::Stop::Type stopData;
    stopData.optionsMask.ClearAll();
    stopData.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::Stop::Id, stopData).IsSuccess());
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));
}

TEST_F(TestLevelControlBase, TestMoveToLevelFallback)
{
    // Test that if we are transitioning, a new command overrides it.
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOffTransitionTime(0) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Case 1: OnOffTransitionTime = 100 (10s).
    uint16_t onOffTransitionTime = 100;
    EXPECT_TRUE(tester.WriteAttribute(Attributes::OnOffTransitionTime::Id, onOffTransitionTime).IsSuccess());

    Commands::MoveToLevel::Type data;
    data.level = 100;
    data.transitionTime.SetNull();
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    mockTimer.CancelTimer(nullptr); // or call Stop
    Commands::Stop::Type stopData;
    stopData.optionsMask.ClearAll();
    stopData.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::Stop::Id, stopData).IsSuccess());

    // Case 2: OnOffTransitionTime = 0.
    onOffTransitionTime = 0;
    EXPECT_TRUE(tester.WriteAttribute(Attributes::OnOffTransitionTime::Id, onOffTransitionTime).IsSuccess());

    data.level = 50;
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 50u);
}

TEST_F(TestLevelControlBase, TestMoveCommand)
{
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithMinLevel(0).WithMaxLevel(254)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Move Up at rate 10 units/s.
    Commands::Move::Type data;
    data.moveMode = MoveModeEnum::kUp;
    data.rate.SetNonNull(10);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::Move::Id, data).IsSuccess());
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Advance 1s -> should increase by 10 (approx)
    for (int i = 0; i < 10; i++)
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u);

    // Stop
    Commands::Stop::Type stopData;
    stopData.optionsMask.ClearAll();
    stopData.optionsOverride.ClearAll();
    EXPECT_TRUE(tester.Invoke(Commands::Stop::Id, stopData).IsSuccess());
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));
}

TEST_F(TestLevelControlBase, TestStepCommand)
{
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithMinLevel(0).WithMaxLevel(254)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Step Up 10 units in 10s (100ds).
    Commands::Step::Type data;
    data.stepMode = StepModeEnum::kUp;
    data.stepSize = 10;
    data.transitionTime.SetNonNull(100);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::Step::Id, data).IsSuccess());
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Advance 10s. Should reach 10.
    for (int i = 0; i < 10; i++)
    {
        AdvanceClock(System::Clock::Milliseconds64(1000));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 10u);

    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));
}

TEST_F(TestLevelControlBase, TestCurrentLevelReporting)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::TestServerClusterContext context;
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(cluster);
    auto & changeListener = context.ChangeListener();

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Verify initial set reported (discrete event)
    bool initReported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::CurrentLevel::Id)
            initReported = true;
    }
    EXPECT_TRUE(initReported);
    changeListener.DirtyList().clear();

    // Start a slow transition: 0 -> 10 over 5 seconds (50ds).
    // 5000ms total. 10 steps. 500ms per step.
    Commands::MoveToLevel::Type data;
    data.level = 10;
    data.transitionTime.SetNonNull(50);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    // Advance 500ms (Step 1: Level 1)
    // 0.5s elapsed. Should NOT report (less than 1s).
    changeListener.DirtyList().clear();
    AdvanceClock(System::Clock::Milliseconds64(500));

    // Verify Level changed
    EXPECT_EQ(cluster.GetCurrentLevel().Value(), 1u);

    // Verify NOT reported
    bool reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::CurrentLevel::Id)
            reported = true;
    }
    EXPECT_FALSE(reported);

    // Advance another 500ms (Step 2: Level 2)
    // 1.0s elapsed from start. Should report now?
    // "At most once per second". Last report was at t=0 (start). Now t=1000ms.
    changeListener.DirtyList().clear();
    AdvanceClock(System::Clock::Milliseconds64(500));
    EXPECT_EQ(cluster.GetCurrentLevel().Value(), 2u);

    reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::CurrentLevel::Id)
            reported = true;
    }
    EXPECT_TRUE(reported);
}

TEST_F(TestLevelControlBase, TestStartUpCurrentLevelPersistence)
{
    chip::Testing::TestServerClusterContext context;

    // 1. Initialize and Write StartUpCurrentLevel
    {
        LevelControlCluster cluster{
            LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithLighting(DataModel::NullNullable)
        };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        chip::Testing::ClusterTester tester(cluster);
        DataModel::Nullable<uint8_t> startupLevel;
        startupLevel.SetNonNull(50);
        EXPECT_TRUE(tester.WriteAttribute(Attributes::StartUpCurrentLevel::Id, startupLevel).IsSuccess());
    }

    // 2. Verify Storage directly
    uint8_t buffer[1];
    MutableByteSpan span(buffer);
    EXPECT_EQ(context.AttributePersistenceProvider().ReadValue(
                  ConcreteAttributePath(kTestEndpointId, LevelControl::Id, Attributes::StartUpCurrentLevel::Id), span),
              CHIP_NO_ERROR);
    EXPECT_EQ(span.size(), 1u);
    EXPECT_EQ(buffer[0], 50u);
}

TEST_F(TestLevelControlBase, TestCurrentLevelPersistence)
{
    chip::Testing::TestServerClusterContext context;

    // 1. Initialize and Set CurrentLevel
    {
        // Start with an initial level so the attribute is not null (MoveToLevel fails if current is null)
        LevelControlCluster cluster{
            LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithInitialCurrentLevel(10)
        };
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        chip::Testing::ClusterTester tester(cluster);

        Commands::MoveToLevel::Type data;
        data.level = 123;
        data.transitionTime.SetNonNull(0);
        data.optionsMask.Set(OptionsBitmap::kExecuteIfOff);
        data.optionsOverride.Set(OptionsBitmap::kExecuteIfOff);
        EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());
    }

    // 2. Verify Storage directly
    uint8_t buffer[1];
    MutableByteSpan span(buffer);
    EXPECT_EQ(context.AttributePersistenceProvider().ReadValue(
                  ConcreteAttributePath(kTestEndpointId, LevelControl::Id, Attributes::CurrentLevel::Id), span),
              CHIP_NO_ERROR);
    EXPECT_EQ(span.size(), 1u);
    EXPECT_EQ(buffer[0], 123u);
}
