/*
 *
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

    OnOffCluster mCluster{ kTestEndpointId, { .timerDelegate = mMockTimerDelegate } };

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
    bool onOff = true;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_FALSE(onOff);

    uint16_t revision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, kRevision);

    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestOnOffCluster, TestCommands)
{
    // Step 1: Test On Command
    EXPECT_TRUE(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_TRUE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    bool onOff = false;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_TRUE(onOff);

    // Step 2: Test Off Command
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_FALSE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_FALSE(onOff);

    // Step 3: Test Toggle Command (from Off to On)
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Toggle::Type>(Commands::Toggle::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_TRUE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_TRUE(onOff);

    // Step 4: Test Toggle Command (from On to Off)
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Toggle::Type>(Commands::Toggle::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_FALSE(mMockDelegate.mOnOff);

    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
    EXPECT_FALSE(onOff);
}

TEST_F(TestOnOffCluster, TestNoPersistenceUseDefaultsOnStartup)
{
    MockOnOffDelegate mockDelegate;
    TimerDelegateMock mockTimerDelegate;
    TestServerClusterContext context;

    // Step 1: Initial startup, set to ON
    {
        OnOffCluster cluster(kTestEndpointId, { .timerDelegate = mockTimerDelegate, .defaults = { .onOff = true } });
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster); // Uses its own context

        bool onOff = false;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
        EXPECT_TRUE(onOff);
        EXPECT_TRUE(cluster.GetOnOff());
    }

    // Step 2:
    //   - Restart, verify state is NOT persisted (should be default OFF)
    //   - We reuse the same context so persistence is the same. Only defaults apply.
    {
        OnOffCluster cluster(kTestEndpointId, { .timerDelegate = mockTimerDelegate, .defaults = { .onOff = false } });
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster); // Uses its own context

        bool onOff = true; // Initialize to true to ensure it's changed
        EXPECT_EQ(tester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
        EXPECT_FALSE(onOff);
        EXPECT_FALSE(cluster.GetOnOff());
    }
}

TEST_F(TestOnOffCluster, TestPersistence)

{
    MockOnOffDelegate mockDelegate;
    TimerDelegateMock mockTimerDelegate;
    TestServerClusterContext context;

    // Step 1: Initial startup, set to ON and check persistence
    {
        OnOffCluster cluster(kTestEndpointId, { .timerDelegate = mockTimerDelegate });
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        EXPECT_TRUE(tester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());
        bool onOff = false;
        EXPECT_EQ(tester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
        EXPECT_TRUE(onOff);

        // Check that the value is persisted in storage
        uint8_t persistedValue = 0;
        MutableByteSpan span(&persistedValue, sizeof(persistedValue));
        EXPECT_EQ(context.Get().attributeStorage.ReadValue(
                      ConcreteAttributePath(kTestEndpointId, Clusters::OnOff::Id, Attributes::OnOff::Id), span),
                  CHIP_NO_ERROR);
        EXPECT_TRUE(persistedValue != 0);
    }

    // Step 2: Restart, verify state IS persisted
    {
        OnOffCluster cluster(kTestEndpointId, { .timerDelegate = mockTimerDelegate });
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        chip::Testing::ClusterTester tester(cluster);

        bool onOff = false; // Initialize to false to ensure it's changed
        EXPECT_EQ(tester.ReadAttribute(Attributes::OnOff::Id, onOff), CHIP_NO_ERROR);
        EXPECT_TRUE(onOff);
    }
}

TEST_F(TestOnOffCluster, TestDefaultValue)
{
    MockOnOffDelegate mockDelegate;
    TimerDelegateMock mockTimerDelegate;
    TestServerClusterContext context;

    // Test with default false
    {
        OnOffCluster cluster(kTestEndpointId, { .timerDelegate = mockTimerDelegate, .defaults = { .onOff = false } });
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_FALSE(cluster.GetOnOff());
    }

    // Test with default true
    {
        OnOffCluster cluster(kTestEndpointId, { .timerDelegate = mockTimerDelegate, .defaults = { .onOff = true } });
        cluster.AddDelegate(&mockDelegate);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
        EXPECT_TRUE(cluster.GetOnOff());
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
    OnOffCluster mCluster{ kTestEndpointId,
                           { .timerDelegate = mMockTimerDelegate, .featureMap = BitMask<Feature>(Feature::kOffOnly) } };
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
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());

    EXPECT_EQ(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).status,
              Protocols::InteractionModel::Status::UnsupportedCommand);

    EXPECT_EQ(mClusterTester.Invoke<Commands::Toggle::Type>(Commands::Toggle::Type()).status,
              Protocols::InteractionModel::Status::UnsupportedCommand);
}

TEST_F(TestOnOffCluster, TestMultipleDelegates)
{
    MockOnOffDelegate secondaryDelegate;
    mCluster.AddDelegate(&secondaryDelegate);

    // Step 1: On Command - Both delegates should be called
    EXPECT_TRUE(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());

    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_TRUE(mMockDelegate.mOnOff);
    mMockDelegate.mCalled = false;

    EXPECT_TRUE(secondaryDelegate.mCalled);
    EXPECT_TRUE(secondaryDelegate.mOnOff);
    secondaryDelegate.mCalled = false;

    // Step 2: Remove secondary delegate
    mCluster.RemoveDelegate(&secondaryDelegate);

    // Step 3: Off Command - Only primary should be called
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());

    EXPECT_TRUE(mMockDelegate.mCalled);
    EXPECT_FALSE(mMockDelegate.mOnOff);

    EXPECT_FALSE(secondaryDelegate.mCalled);
    EXPECT_TRUE(secondaryDelegate.mOnOff); // Should remain true (last state)
}

TEST_F(TestOnOffCluster, TestSceneSupport)
{
    // Step 1: Turn ON
    EXPECT_TRUE(mClusterTester.Invoke<Commands::On::Type>(Commands::On::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mOnOff);

    // Step 2: Serialize the current state
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(mCluster.SerializeSave(kTestEndpointId, Clusters::OnOff::Id, serializedBytes), CHIP_NO_ERROR);
    EXPECT_GT(serializedBytes.size(), 0u);

    // Step 3: Turn OFF
    EXPECT_TRUE(mClusterTester.Invoke<Commands::Off::Type>(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // Step 4: Apply the saved scene to restore the ON state
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 0), CHIP_NO_ERROR);
    EXPECT_TRUE(mMockDelegate.mOnOff);
}

TEST_F(TestOnOffCluster, TestSceneInvalidAttribute)
{
    using AttributeValuePair = ScenesManagement::Structs::AttributeValuePairStruct::Type;

    // Construct invalid scene data with a wrong Attribute ID.
    AttributeValuePair pairs[1];
    pairs[0].attributeID = Attributes::OnOff::Id + 1;
    pairs[0].valueUnsigned8.SetValue(1);

    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    app::DataModel::List<AttributeValuePair> attributeValueList(pairs);

    EXPECT_EQ(mCluster.EncodeAttributeValueList(attributeValueList, serializedBytes), CHIP_NO_ERROR);

    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 0), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestOnOffCluster, TestSceneTransition)
{
    // Step 1: Start ON
    EXPECT_TRUE(mClusterTester.Invoke(Commands::On::Type()).IsSuccess());
    EXPECT_TRUE(mMockDelegate.mOnOff);

    // Step 2: Serialize the ON state
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(mCluster.SerializeSave(kTestEndpointId, Clusters::OnOff::Id, serializedBytes), CHIP_NO_ERROR);

    // Step 3: Turn OFF
    EXPECT_TRUE(mClusterTester.Invoke(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // Step 4: Apply Scene to restore ON state with a 1000ms transition
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 1000), CHIP_NO_ERROR);

    // OnOff should still be FALSE as the transition is in progress.
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // Step 5: Advance time to complete the transition
    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1000));

    EXPECT_TRUE(mMockDelegate.mOnOff);
}

TEST_F(TestOnOffCluster, TestSceneTransitionCancellation)
{
    // Step 1: Start OFF
    EXPECT_TRUE(mClusterTester.Invoke(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mMockDelegate.mOnOff);

    // Step 2: Prepare a scene that would turn the light ON.
    uint8_t buffer[128];
    MutableByteSpan serializedBytes(buffer);
    EXPECT_EQ(mCluster.SetOnOff(true), CHIP_NO_ERROR); // Manually set to save ON
    EXPECT_EQ(mCluster.SerializeSave(kTestEndpointId, Clusters::OnOff::Id, serializedBytes), CHIP_NO_ERROR);
    EXPECT_EQ(mCluster.SetOnOff(false), CHIP_NO_ERROR); // Revert to OFF

    // Step 3: Apply the scene with a 1000ms transition.
    EXPECT_EQ(mCluster.ApplyScene(kTestEndpointId, Clusters::OnOff::Id, serializedBytes, 1000), CHIP_NO_ERROR);
    EXPECT_FALSE(mMockDelegate.mOnOff);
    EXPECT_TRUE(mCluster.IsSceneTransitionPending());

    // Step 4: Send an "Off" Command, which should cancel the ongoing transition.
    EXPECT_TRUE(mClusterTester.Invoke(Commands::Off::Type()).IsSuccess());
    EXPECT_FALSE(mCluster.IsSceneTransitionPending());

    // Step 5: Advance time beyond the original transition time.
    mMockTimerDelegate.AdvanceClock(System::Clock::Milliseconds64(1000));

    // The state should remain OFF because the transition was cancelled.
    EXPECT_FALSE(mMockDelegate.mOnOff);
}

} // namespace
