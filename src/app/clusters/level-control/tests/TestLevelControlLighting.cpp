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

    uint16_t remainingTime;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 0u);
}

TEST_F(TestLevelControlLighting, TestRemainingTime)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate)
                                     .WithLighting(DataModel::NullNullable)
                                     .WithMinLevel(0)
                                     .WithMaxLevel(254) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    cluster.SetCurrentLevel(1);

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
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(100));
    }

    // Now it should be updated.
    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    // 10s total, 1s elapsed -> 9s remaining (90ds).
    EXPECT_EQ(remainingTime, 90);

    // Advance 5s more (50 ticks of 100ms)
    for (int i = 0; i < 50; i++)
    {
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(100));
    }
    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 40);

    // Finish
    while (mockTimer.IsTimerActive(&cluster))
    {
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(100));
    }

    EXPECT_TRUE(tester.ReadAttribute(Attributes::RemainingTime::Id, remainingTime).IsSuccess());
    EXPECT_EQ(remainingTime, 0);
}
