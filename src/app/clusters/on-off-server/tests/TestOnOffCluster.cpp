/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/on-off-server/OnOffCluster.h>
#include <clusters/OnOff/Metadata.h>
#include <lib/support/TimerDelegateMock.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OnOff;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

namespace chip::app::Clusters::OnOff {
class OnOffClusterTestAccess
{
public:
    OnOffClusterTestAccess(OnOffCluster & cluster) : mCluster(cluster) {}

    bool IsSceneTransitionPending() { return mCluster.mTimerDelegate.IsTimerActive(&mCluster.mSceneTimer); }

private:
    OnOffCluster & mCluster;
};
} // namespace chip::app::Clusters::OnOff

namespace {

constexpr EndpointId kTestEndpointId = 1;

class MockOnOffDelegate : public OnOffDelegate
{
public:
    bool mOnOff         = false;
    bool mCalled        = false;
    bool mStartupCalled = false;

    void OnOnOffChanged(bool on) override
    {
        mOnOff  = on;
        mCalled = true;
    }

    void OnOffStartup(bool on) override
    {
        mOnOff         = on;
        mStartupCalled = true;
    }
};

struct TestOnOffCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mCluster.AddDelegate(&mMockDelegate);
        EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override {}

    MockOnOffDelegate mMockDelegate;
    TimerDelegateMock mMockTimerDelegate;

    OnOffCluster mCluster{ kTestEndpointId, mMockTimerDelegate };

    ClusterTester mClusterTester{ mCluster };
};

TEST_F(TestOnOffCluster, TestAttributesList)
{
    std::vector<DataModel::AttributeEntry> mandatoryAttributes(Attributes::kMandatoryMetadata.begin(),
                                                               Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, mandatoryAttributes));
}

TEST_F(TestOnOffCluster, TestAcceptedCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              {
                                                  Commands::Off::kMetadataEntry,
                                                  Commands::On::kMetadataEntry,
                                                  Commands::Toggle::kMetadataEntry,
                                              }));
}

TEST_F(TestOnOffCluster, TestReadAttributes)
{
    // Test Read OnOff (Default false)
    bool onOff = true;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_FALSE(onOff);

    // Test Read ClusterRevision
    uint16_t revision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, kRevision);

    // Test Read FeatureMap
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestOnOffCluster, TestCommands)
{
    // 1. On Command
    EXPECT_TRUE(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_TRUE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    // Verify Attribute
    bool onOff = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_TRUE(onOff);

    // 2. Off Command
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_FALSE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_FALSE(onOff);

    // 3. Toggle Command (from Off to On)
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Toggle::Type>(Commands::Toggle::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_TRUE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_TRUE(onOff);

    // 4. Toggle Command (from On to Off)
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Toggle::Type>(Commands::Toggle::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_FALSE(mMockDelegate.mOnOff);

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_FALSE(onOff);
}

TEST_F(TestOnOffCluster, TestNoPersistence)
{
    MockOnOffDelegate mockDelegate;
    TimerDelegateMock mockTimerDelegate;

    // 1. Initial startup, set to ON
    {
        TestServerClusterContext context;
        OnOffCluster cluster(kTestEndpointId, mockTimerDelegate);
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster); // Uses its own context

        EXPECT_TRUE(tester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());
        bool onOff = false;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
        EXPECT_TRUE(onOff);
    }

    // 2. Restart, verify state is NOT persisted (should be default OFF)
    {
        TestServerClusterContext context;
        OnOffCluster cluster(kTestEndpointId, mockTimerDelegate);
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster); // Uses its own context

        bool onOff = true; // Initialize to true to ensure it's changed
        EXPECT_EQ(tester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
        EXPECT_FALSE(onOff); // Expect default value FALSE
    }
}

struct TestOffOnlyOnOffCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mCluster.AddDelegate(&mMockDelegate);
        EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override {}

    MockOnOffDelegate mMockDelegate;
    TimerDelegateMock mMockTimerDelegate;
    OnOffCluster mCluster{ kTestEndpointId, mMockTimerDelegate, BitMask<Feature>(Feature::kOffOnly) };
    ClusterTester mClusterTester{ mCluster };
};

TEST_F(TestOffOnlyOnOffCluster, TestFeatureMap)
{
    uint32_t featureMap = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, static_cast<uint32_t>(Feature::kOffOnly));
}

TEST_F(TestOffOnlyOnOffCluster, TestAcceptedCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster, { Commands::Off::kMetadataEntry }));
}

TEST_F(TestOffOnlyOnOffCluster, TestInvokeCommands)
{
    // Off should still work
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());

    // On should fail with UnsupportedCommand
    EXPECT_EQ(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).status,
              Protocols::InteractionModel::Status::UnsupportedCommand);

    // Toggle should fail with UnsupportedCommand
    EXPECT_EQ(mClusterTester.Invoke<Commands::Toggle::Type>(Commands::Toggle::Type()).status,
              Protocols::InteractionModel::Status::UnsupportedCommand);
}

TEST_F(TestOnOffCluster, TestMultipleDelegates)
{
    MockOnOffDelegate secondaryDelegate;
    mCluster.AddDelegate(&secondaryDelegate);

    // 1. On Command - Both should be called
    EXPECT_TRUE(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());

    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_TRUE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    EXPECT_TRUE(secondaryDelegate.mCalled);
    EXPECT_TRUE(secondaryDelegate.mOnOff);
    secondaryDelegate.mCalled = false;

    // 2. Remove secondary delegate
    mCluster.RemoveDelegate(&secondaryDelegate);

    // 3. Off Command - Only primary should be called
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());

    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_FALSE(mMockDelegate.mOnOff);

    EXPECT_FALSE(secondaryDelegate.mCalled);
    EXPECT_TRUE(secondaryDelegate.mOnOff); // Should remain true (last state)
}

TEST_F(TestOnOffCluster, TestSceneSupport)
{
    // 1. Setup: Turn ON
    EXPECT_TRUE(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mOnOff);

    // 2. Serialize Save
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(mCluster.SerializeSave(kTestEndpointId, Clusters::OnOff::Id, serializedBytes), CHIP_NO_ERROR);
    EXPECT_GT(serializedBytes.size(), 0u);

    // 3. Turn OFF
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // 4. Apply Scene (Restore ON)
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(mMockDelegate.mOnOff);
}

TEST_F(TestOnOffCluster, TestSceneInvalidAttribute)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    // Construct invalid data (Wrong Attribute ID)
    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::OnOff::Id + 1; // Invalid ID
    pairs[0].valueUnsigned8.SetValue(1);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    app::DataModel::List<AttributeValuePair> attributeValueList(pairs);

    // Encode invalid list
    EXPECT_EQ(mCluster.EncodeAttributeValueList(attributeValueList, serializedBytes), CHIP_NO_ERROR);

    // Apply Scene should fail
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 0), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestOnOffCluster, TestSceneTransition)
{
    // 1. Start ON
    EXPECT_TRUE(mClusterTester.Invoke(Commands::On::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mOnOff);

    // 2. Serialize Save (ON state)
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(mCluster.SerializeSave(kTestEndpointId, Clusters::OnOff::Id, serializedBytes), CHIP_NO_ERROR);

    // 3. Turn OFF
    EXPECT_TRUE(mClusterTester.Invoke(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // 4. Apply Scene (Restore ON) with 1000ms transition
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 1000), CHIP_NO_ERROR);

    // Should still be OFF (transition pending)
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // 5. Advance Clock (1000ms)
    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1000));

    // Should be ON
    EXPECT_TRUE(mMockDelegate.mOnOff);
}

TEST_F(TestOnOffCluster, TestSceneTransitionCancellation)
{
    // 1. Setup: Start OFF
    EXPECT_TRUE(mClusterTester.Invoke(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // 2. Serialize Save (ON state)
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(mCluster.SetOnOff(true), CHIP_NO_ERROR); // Manually set to save ON
    EXPECT_EQ(mCluster.SerializeSave(kTestEndpointId, Clusters::OnOff::Id, serializedBytes), CHIP_NO_ERROR);
    EXPECT_EQ(mCluster.SetOnOff(false), CHIP_NO_ERROR); // Back to OFF

    // 3. Apply Scene (Restore ON) with 1000ms transition
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 1000), CHIP_NO_ERROR);
    EXPECT_FALSE(mMockDelegate.mOnOff);
    EXPECT_TRUE(OnOffClusterTestAccess(mCluster).IsSceneTransitionPending());

    // 4. Send "Off" Command -> Should cancel transition
    EXPECT_TRUE(mClusterTester.Invoke(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(OnOffClusterTestAccess(mCluster).IsSceneTransitionPending());

    // 5. Advance Clock (1000ms)
    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1000));

    // Should STILL be OFF
    EXPECT_FALSE(mMockDelegate.mOnOff);
}

} // namespace
