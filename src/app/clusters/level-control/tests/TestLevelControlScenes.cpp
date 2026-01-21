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

struct TestLevelControlScenes : public LevelControlTestBase
{
};

TEST_F(TestLevelControlScenes, TestSerializeScene)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    cluster.SetCurrentLevel(55);

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

TEST_F(TestLevelControlScenes, TestApplyScene)
{
    LevelControlCluster cluster{ LevelControlCluster::Config(kTestEndpointId, mockTimer, mockDelegate) };
    chip::Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    cluster.SetCurrentLevel(0);

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
    EXPECT_TRUE(mockTimer.IsTimerActive(&cluster));

    // Advance to end
    while (mockTimer.IsTimerActive(&cluster))
    {
        mockTimer.AdvanceClock(System::Clock::Milliseconds64(100));
    }

    DataModel::Nullable<uint8_t> readLevel;
    EXPECT_TRUE(tester.ReadAttribute(Attributes::CurrentLevel::Id, readLevel).IsSuccess());
    EXPECT_EQ(readLevel.Value(), 20u);
}
