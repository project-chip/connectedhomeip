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
#include <clusters/LevelControl/Attributes.h>
#include <clusters/LevelControl/Commands.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LevelControl;

struct TestLevelControlScenes : public LevelControlTestBase
{
};

TEST_F(TestLevelControlScenes, TestSerializeScene)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(55, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);

    EXPECT_EQ(cluster.SerializeSave(kTestEndpointId, LevelControl::Id, serializedBytes), CHIP_NO_ERROR);

    // Decode to verify
    app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> list;
    EXPECT_EQ(cluster.DecodeAttributeValueList(serializedBytes, list), CHIP_NO_ERROR);

    auto iter = list.begin();
    EXPECT_TRUE(iter.Next());
    auto pair = iter.GetValue();
    EXPECT_EQ(pair.attributeID, Attributes::CurrentLevel::Id);
    EXPECT_EQ(pair.valueUnsigned8.Value(), 55u);
    EXPECT_FALSE(iter.Next());
}

TEST_F(TestLevelControlScenes, TestSerializeSceneNullLevel)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // CurrentLevel defaults to Null

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);

    EXPECT_EQ(cluster.SerializeSave(kTestEndpointId, LevelControl::Id, serializedBytes), CHIP_NO_ERROR);

    // Decode to verify
    app::DataModel::DecodableList<ScenesManagement::Structs::AttributeValuePairStruct::DecodableType> list;
    EXPECT_EQ(cluster.DecodeAttributeValueList(serializedBytes, list), CHIP_NO_ERROR);

    auto iter = list.begin();
    // Expect NO items, or at least NO CurrentLevel item.
    // If it saves 0, it will have one item with value 0.
    bool foundCurrentLevel = false;
    while (iter.Next())
    {
        if (iter.GetValue().attributeID == Attributes::CurrentLevel::Id)
        {
            foundCurrentLevel = true;
        }
    }
    EXPECT_FALSE(foundCurrentLevel);
}

TEST_F(TestLevelControlScenes, TestApplyScene)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Create serialized scene with level 20
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);

    // Manually create list to simulate saved scene
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;
    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::CurrentLevel::Id;
    pairs[0].valueUnsigned8.SetValue(20);
    app::DataModel::List<AttributeValuePair> list(pairs);

    EXPECT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    // Apply Scene with 1000ms transition
    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, LevelControl::Id, serializedBytes, 1000), CHIP_NO_ERROR);

    // Timer should be active (transition from 0 to 20 over 1000ms)
    EXPECT_TRUE(mockTimer.IsTimerActive(nullptr));

    // Advance to end
    while (mockTimer.IsTimerActive(nullptr))
    {
        AdvanceClock(System::Clock::Milliseconds64(100));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 20u);
}

TEST_F(TestLevelControlScenes, TestApplySceneImmediate)
{
    // Set a default transition time of 10s (100ds) to verify it is IGNORED when ApplyScene(0) is called.
    LevelControlCluster cluster{
        LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOffTransitionTime(100)
    };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());

    // Scene with level 50
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;
    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::CurrentLevel::Id;
    pairs[0].valueUnsigned8.SetValue(50);
    app::DataModel::List<AttributeValuePair> list(pairs);
    EXPECT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    // Apply Scene with 0ms (Immediate)
    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, LevelControl::Id, serializedBytes, 0), CHIP_NO_ERROR);

    // Should be immediate -> No Timer
    EXPECT_FALSE(mockTimer.IsTimerActive(nullptr));

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 50u);
}

TEST_F(TestLevelControlScenes, TestApplySceneWhileOff)
{
    chip::app::Clusters::OnOffCluster::Context onOffContext{ mockTimer };
    chip::app::Clusters::OnOffCluster onOffCluster{ kTestEndpointId, onOffContext };

    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate).WithOnOff(onOffCluster) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    EXPECT_EQ(onOffCluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster
                    .MoveToLevel(0, DataModel::MakeNullable(static_cast<uint16_t>(0)),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff),
                                 BitMask<LevelControl::OptionsBitmap>(LevelControl::OptionsBitmap::kExecuteIfOff))
                    .IsSuccess());
    EXPECT_EQ(onOffCluster.SetOnOff(false), CHIP_NO_ERROR); // OFF

    // Scene with level 50
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;
    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::CurrentLevel::Id;
    pairs[0].valueUnsigned8.SetValue(50);
    app::DataModel::List<AttributeValuePair> list(pairs);
    EXPECT_EQ(cluster.EncodeAttributeValueList(list, serializedBytes), CHIP_NO_ERROR);

    // Apply Scene with 0ms
    // Should succeed and update level even though Off
    EXPECT_EQ(cluster.ApplyScene(kTestEndpointId, LevelControl::Id, serializedBytes, 0), CHIP_NO_ERROR);

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 50u);
}
