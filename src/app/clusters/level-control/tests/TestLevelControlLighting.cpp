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

struct TestLevelControlLighting : public LevelControlTestBase
{
};

TEST_F(TestLevelControlLighting, TestStartUpCurrentLevel)
{
    {
        DataModel::Nullable<uint8_t> startup;
        startup.SetNonNull(50);
        LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithLighting(startup) };
        chip::Testing::ClusterTester tester(cluster);

        EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

        DataModel::Nullable<uint8_t> currentLevel;
        EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, currentLevel).IsSuccess());
        EXPECT_EQ(currentLevel.Value(), 50u);
    }
}

TEST_F(TestLevelControlLighting, TestInitialLevelValidation)
{
    // Test case to confirm bug fix: InitialCurrentLevel(0) should be clamped to MinLevel(1) if Lighting is enabled.
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)
                                     .WithInitialCurrentLevel(0)
                                     .WithLighting(DataModel::NullNullable) };
    chip::Testing::ClusterTester tester(cluster);

    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> currentLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, currentLevel).IsSuccess());

    // Expectation: Clamped to MinLevel (1)
    EXPECT_EQ(currentLevel.Value(), 1u);
    EXPECT_GE(currentLevel.Value(), cluster.GetMinLevel());
}

TEST_F(TestLevelControlLighting, TestLightingEnforcesConstraints)
{
    {
        LevelControlCluster::Config config(kTestEndpointId, mockTimer, mockDelegate);
        config.WithLighting(DataModel::NullNullable);

        EXPECT_EQ(config.mMinLevel, 1u);
        EXPECT_EQ(config.mMaxLevel, 254u);
        EXPECT_TRUE(config.mFeatureMap.Has(LevelControl::Feature::kLighting));
    }
}

TEST_F(TestLevelControlLighting, TestLightingAttributesPresence)
{
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithLighting(DataModel::NullNullable)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        { Attributes::CurrentLevel::kMetadataEntry, Attributes::Options::kMetadataEntry,
                                          Attributes::OnLevel::kMetadataEntry, Attributes::MinLevel::kMetadataEntry,
                                          Attributes::MaxLevel::kMetadataEntry, Attributes::StartUpCurrentLevel::kMetadataEntry,
                                          Attributes::RemainingTime::kMetadataEntry }));
}

TEST_F(TestLevelControlLighting, TestRemainingTimeDefault)
{
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithLighting(DataModel::NullNullable)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t remainingTime = 0;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 0u);
}

TEST_F(TestLevelControlLighting, TestRemainingTime)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)
                                     .WithLighting(DataModel::NullNullable)
                                     .WithMaxLevel(254) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(1, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Move to 101 over 100ds (10s).
    Commands::MoveToLevel::Type data;
    data.level = 101;
    data.transitionTime.SetNonNull(100);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    uint16_t remainingTime;

    // Advance 1s (1000ms) in 100ms steps to ensure recursive timers fire
    for (int i = 0; i < 10; i++)
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    // Now it should be updated.
    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    // 10s total, 1s elapsed -> 9s remaining (90ds).
    EXPECT_EQ(remainingTime, 90);

    // Advance 5s more (50 ticks of 100ms)
    for (int i = 0; i < 50; i++)
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }
    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 40);

    // Finish
    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 0);
}

TEST_F(TestLevelControlLighting, TestRemainingTimeReporting)
{
    chip::Testing::TestServerClusterContext context;
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithLighting(DataModel::NullNullable)
    };
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(cluster);
    auto & changeListener = context.ChangeListener();

    EXPECT_TRUE(cluster
                    .MoveToLevel(1, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // 1. Short transition (< 1s). Should NOT report.
    // Move to 10 over 5ds (0.5s).
    Commands::MoveToLevel::Type data;
    data.level = 10;
    data.transitionTime.SetNonNull(5);
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    changeListener.DirtyList().clear();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    // Should NOT report RemainingTime (value 5)
    bool reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::RemainingTime::Id)
            reported = true;
    }
    EXPECT_FALSE(reported);

    // Wait to finish
    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    // At end (0), it might report? Logic says: if (remainingTimeDs == 0 && mLastReported != 0).
    // mLastReported is 0. So no report.
    changeListener.DirtyList().clear();

    // 2. Long transition (10s). Should report.
    data.level = 100;
    data.transitionTime.SetNonNull(100);

    changeListener.DirtyList().clear();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    // Should report start (100ds)
    // Case 1: mLast=0, remaining=100 > 10. Yes.
    reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::RemainingTime::Id)
            reported = true;
    }
    EXPECT_TRUE(reported);
    changeListener.DirtyList().clear();

    // Advance 0.5s. Remaining 95. Delta 5. No report (countdown).
    for (int i = 0; i < 5; ++i)
        AdvanceClock(System::Clock::Milliseconds64(100));

    reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::RemainingTime::Id)
            reported = true;
    }
    EXPECT_FALSE(reported);

    // Advance 1s more (total 1.5s). Remaining 85.
    // Not a new transition. Not 0. Should NOT report.
    for (int i = 0; i < 10; ++i)
        AdvanceClock(System::Clock::Milliseconds64(100));
    reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::RemainingTime::Id)
            reported = true;
    }
    EXPECT_FALSE(reported);

    // 3. New Command (Interrupt).
    // Invoke MoveToLevel to same level but faster?
    // Let's invoke new command. Transition 20s (200ds).
    // Remaining was 85. New will be 200.
    // Delta |200 - 85| = 115 > 10.
    // Should report.

    data.level = 200;
    data.transitionTime.SetNonNull(200);
    changeListener.DirtyList().clear();
    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::RemainingTime::Id)
            reported = true;
    }
    EXPECT_TRUE(reported);
}

TEST_F(TestLevelControlLighting, TestReportingAtTransitionEnd)
{
    // Regression test for issue where the final level report was suppressed
    // if the transition finished within the quieter reporting interval.

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::TestServerClusterContext context;
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(cluster);
    auto & changeListener = context.ChangeListener();

    // 1. Initialize to a known level (e.g., 200).
    // This should trigger an initial report.
    EXPECT_TRUE(cluster
                    .MoveToLevel(200, DataModel::MakeNullable<uint16_t>(0u),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Verify init reported
    bool initReported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::CurrentLevel::Id)
            initReported = true;
    }
    EXPECT_TRUE(initReported);
    changeListener.DirtyList().clear();

    // 2. Start a transition that will land on 201 shortly.
    // We want the transition to end *within* 1 second of the last report (which was just now).
    // Let's say we move to 201 in 500ms.
    // 500ms = 5 ds.
    Commands::MoveToLevel::Type data;
    data.level = 201;
    data.transitionTime.SetNonNull(5); // 0.5 seconds
    data.optionsMask.ClearAll();
    data.optionsOverride.ClearAll();

    EXPECT_TRUE(tester.Invoke(Commands::MoveToLevel::Id, data).IsSuccess());

    // 3. Advance time by 500ms to complete the transition.
    // This calls TimerFired -> SetCurrentLevel(201).
    mockClock.AdvanceMonotonic(System::Clock::Milliseconds64(500));
    AdvanceClock(System::Clock::Milliseconds64(500));

    // 4. Verify that the transition completed.
    EXPECT_EQ(cluster.GetCurrentLevel().Value(), 201u);
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));

    // 5. Verify that a report was generated for the final level (201).
    bool reported = false;
    for (auto & id : changeListener.DirtyList())
    {
        if (id.mAttributeId == Attributes::CurrentLevel::Id)
            reported = true;
    }
    EXPECT_TRUE(reported) << "CurrentLevel should be reported at the end of transition, even if < 1s from last report";
}
