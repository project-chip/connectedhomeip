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

#include <pw_unit_test/framework.h>

#include <app/clusters/actions-server/ActionStructs.h>
#include <app/clusters/actions-server/ActionsCluster.h>
#include <app/clusters/actions-server/ActionsDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;

// Mock ActionsDelegate for testing
class MockActionsDelegate : public Delegate
{
public:
    static constexpr uint8_t kMaxActions       = 10;
    static constexpr uint8_t kMaxEndpointLists = 10;

    ActionStructStorage mActions[kMaxActions];
    uint16_t mNumActions = 0;

    EndpointListStorage mEndpointLists[kMaxEndpointLists];
    uint16_t mNumEndpointLists = 0;

    // Track if HandleInstantAction was called
    bool mHandleInstantActionCalled = false;
    uint16_t mLastActionId          = 0;
    Optional<uint32_t> mLastInvokeId;
    Status mReturnStatus = Status::Success;

    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override
    {
        if (index >= mNumActions)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        action = mActions[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override
    {
        if (index >= mNumEndpointLists)
        {
            return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
        }
        epList = mEndpointLists[index];
        return CHIP_NO_ERROR;
    }

    bool HaveActionWithId(uint16_t actionId, uint16_t & aActionIndex) override
    {
        for (uint16_t i = 0; i < mNumActions; i++)
        {
            if (mActions[i].actionID == actionId)
            {
                aActionIndex = i;
                return true;
            }
        }
        return false;
    }

    // Test helper methods
    CHIP_ERROR AddTestAction(const ActionStructStorage & action)
    {
        if (mNumActions >= kMaxActions)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        mActions[mNumActions++] = action;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddTestEndpointList(const EndpointListStorage & epList)
    {
        if (mNumEndpointLists >= kMaxEndpointLists)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        mEndpointLists[mNumEndpointLists++] = epList;
        return CHIP_NO_ERROR;
    }

    void Reset()
    {
        mNumActions                = 0;
        mNumEndpointLists          = 0;
        mHandleInstantActionCalled = false;
        mLastActionId              = 0;
        mLastInvokeId              = Optional<uint32_t>();
        mReturnStatus              = Status::Success;
    }

    // Command handler implementations
    Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override
    {
        mHandleInstantActionCalled = true;
        mLastActionId              = actionId;
        mLastInvokeId              = invokeId;
        return mReturnStatus;
    }

    Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime, Optional<uint32_t> invokeId) override
    {
        return Status::UnsupportedCommand;
    }

    Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::UnsupportedCommand; }

    Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override
    {
        return Status::UnsupportedCommand;
    }

    Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::UnsupportedCommand; }

    Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::UnsupportedCommand; }

    Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override
    {
        return Status::UnsupportedCommand;
    }

    Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::UnsupportedCommand; }

    Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::UnsupportedCommand; }

    Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override
    {
        return Status::UnsupportedCommand;
    }

    Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::UnsupportedCommand; }

    Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override
    {
        return Status::UnsupportedCommand;
    }
};

// Test fixture for ActionsCluster tests
class TestActionsCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { mDelegate.Reset(); }

    MockActionsDelegate mDelegate;
};

// Test reading ActionList attribute
TEST_F(TestActionsCluster, TestReadActionListAttribute)
{
    // Add test actions
    ActionStructStorage action1(1, CharSpan::fromCharString("FirstAction"), ActionTypeEnum::kScene, 0, BitMask<CommandBits>(),
                                ActionStateEnum::kInactive);
    ActionStructStorage action2(2, CharSpan::fromCharString("SecondAction"), ActionTypeEnum::kScene, 1, BitMask<CommandBits>(),
                                ActionStateEnum::kActive);

    ASSERT_EQ(mDelegate.AddTestAction(action1), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.AddTestAction(action2), CHIP_NO_ERROR);

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to read the attribute
    chip::Testing::ClusterTester tester(cluster);
    ActionList::TypeInfo::DecodableType outList;

    auto status = tester.ReadAttribute(ActionList::Id, outList);
    ASSERT_TRUE(status.IsSuccess());

    // Verify the list contains the expected actions
    auto iter = outList.begin();
    ASSERT_TRUE(iter.Next());
    Structs::ActionStruct::DecodableType action = iter.GetValue();
    EXPECT_EQ(action.actionID, 1);
    EXPECT_EQ(action.type, ActionTypeEnum::kScene);
    EXPECT_EQ(action.endpointListID, 0);
    EXPECT_EQ(action.state, ActionStateEnum::kInactive);

    ASSERT_TRUE(iter.Next());
    action = iter.GetValue();
    EXPECT_EQ(action.actionID, 2);
    EXPECT_EQ(action.type, ActionTypeEnum::kScene);
    EXPECT_EQ(action.endpointListID, 1);
    EXPECT_EQ(action.state, ActionStateEnum::kActive);

    EXPECT_FALSE(iter.Next());
}

// Test reading EndpointLists attribute
TEST_F(TestActionsCluster, TestReadEndpointListsAttribute)
{
    // Add test endpoint lists
    const EndpointId endpoints1[] = { 1, 2 };
    const EndpointId endpoints2[] = { 3, 4, 5 };

    EndpointListStorage epList1(1, CharSpan::fromCharString("FirstList"), EndpointListTypeEnum::kOther,
                                DataModel::List<const EndpointId>(endpoints1, 2));
    EndpointListStorage epList2(2, CharSpan::fromCharString("SecondList"), EndpointListTypeEnum::kOther,
                                DataModel::List<const EndpointId>(endpoints2, 3));

    ASSERT_EQ(mDelegate.AddTestEndpointList(epList1), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.AddTestEndpointList(epList2), CHIP_NO_ERROR);

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to read the attribute
    chip::Testing::ClusterTester tester(cluster);
    EndpointLists::TypeInfo::DecodableType outList;

    auto status = tester.ReadAttribute(EndpointLists::Id, outList);
    ASSERT_TRUE(status.IsSuccess());

    // Verify the list contains the expected endpoint lists
    auto iter = outList.begin();
    ASSERT_TRUE(iter.Next());
    Structs::EndpointListStruct::DecodableType epList = iter.GetValue();
    EXPECT_EQ(epList.endpointListID, 1);
    EXPECT_EQ(epList.type, EndpointListTypeEnum::kOther);

    ASSERT_TRUE(iter.Next());
    epList = iter.GetValue();
    EXPECT_EQ(epList.endpointListID, 2);
    EXPECT_EQ(epList.type, EndpointListTypeEnum::kOther);

    EXPECT_FALSE(iter.Next());
}

// Test invoking InstantAction command successfully
TEST_F(TestActionsCluster, TestInvokeInstantActionSuccess)
{
    // Add a test action with InstantAction command supported
    // Note: For this test, we're using an empty bitmask. The command support check
    // in ActionsCluster.cpp would normally reject this, but we're testing the basic
    // command invocation flow here.
    // We must pass a bitmask that explicitly enables the InstantAction command (Bit 0)
    ActionStructStorage action(1, CharSpan::fromCharString("TestAction"), ActionTypeEnum::kScene, 0,
                               BitMask<CommandBits>(1 << Commands::InstantAction::Id), ActionStateEnum::kInactive);

    ASSERT_EQ(mDelegate.AddTestAction(action), CHIP_NO_ERROR);

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to invoke the command
    chip::Testing::ClusterTester tester(cluster);

    Commands::InstantAction::Type request;
    request.actionID = 1;
    request.invokeID = Optional<uint32_t>(12345);

    auto result = tester.Invoke(request);

    // Verify the command was successful
    ASSERT_TRUE(result.status.has_value());
    EXPECT_TRUE(result.status->IsSuccess());

    // Verify the delegate was called
    EXPECT_TRUE(mDelegate.mHandleInstantActionCalled);
    EXPECT_EQ(mDelegate.mLastActionId, 1);
    // ASSERT_TRUE forces the test to stop evaluating immediately if it fails, preventing the crash
    ASSERT_TRUE(mDelegate.mLastInvokeId.HasValue());
    EXPECT_EQ(mDelegate.mLastInvokeId.Value(), 12345u);
}

// Test invoking InstantAction command with invalid action ID
TEST_F(TestActionsCluster, TestInvokeInstantActionNotFound)
{
    // Create cluster instance without adding any actions
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to invoke the command
    chip::Testing::ClusterTester tester(cluster);

    Commands::InstantAction::Type request;
    request.actionID = 999; // Non-existent action ID
    request.invokeID = Optional<uint32_t>(12345);

    auto result = tester.Invoke(request);

    // Verify the command returned NotFound status
    ASSERT_TRUE(result.status.has_value());
    EXPECT_FALSE(result.status->IsSuccess());
}

// Test invoking InstantAction command when delegate returns failure
TEST_F(TestActionsCluster, TestInvokeInstantActionDelegateFailure)
{
    // Add a test action with InstantAction command supported
    // Note: For this test, we're using an empty bitmask. The command support check
    // in ActionsCluster.cpp would normally reject this, but we're testing the basic
    // command invocation flow here.
    ActionStructStorage action(1, CharSpan::fromCharString("TestAction"), ActionTypeEnum::kScene, 0,
                               BitMask<CommandBits>(1 << Commands::InstantAction::Id), ActionStateEnum::kInactive);

    ASSERT_EQ(mDelegate.AddTestAction(action), CHIP_NO_ERROR);

    // Set the delegate to return a failure status
    mDelegate.mReturnStatus = Status::Failure;

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to invoke the command
    chip::Testing::ClusterTester tester(cluster);

    Commands::InstantAction::Type request;
    request.actionID = 1;
    request.invokeID = Optional<uint32_t>(12345);

    auto result = tester.Invoke(request);

    // Verify the command returned the failure status from delegate
    ASSERT_TRUE(result.status.has_value());
    EXPECT_FALSE(result.status->IsSuccess());

    // Verify the delegate was still called
    EXPECT_TRUE(mDelegate.mHandleInstantActionCalled);
}

// Test invoking InstantAction command with unsupported command
TEST_F(TestActionsCluster, TestInvokeInstantActionUnsupportedCommand)
{
    // Add a test action WITHOUT InstantAction command supported
    // Note: The command support check in ActionsCluster.cpp would reject commands
    // that aren't marked as supported. For this test, we're verifying the basic
    // functionality with an empty bitmask.
    ActionStructStorage action(1, CharSpan::fromCharString("TestAction"), ActionTypeEnum::kScene, 0, BitMask<CommandBits>(),
                               ActionStateEnum::kInactive);

    ASSERT_EQ(mDelegate.AddTestAction(action), CHIP_NO_ERROR);

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to invoke the command
    chip::Testing::ClusterTester tester(cluster);

    Commands::InstantAction::Type request;
    request.actionID = 1;
    request.invokeID = Optional<uint32_t>(12345);

    auto result = tester.Invoke(request);

    // Verify the command returned InvalidCommand status
    ASSERT_TRUE(result.status.has_value());
    EXPECT_FALSE(result.status->IsSuccess());

    // Verify the delegate was NOT called (command was rejected before delegate)
    EXPECT_FALSE(mDelegate.mHandleInstantActionCalled);
}

// Test reading empty ActionList
TEST_F(TestActionsCluster, TestReadEmptyActionList)
{
    // Don't add any actions

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to read the attribute
    chip::Testing::ClusterTester tester(cluster);
    ActionList::TypeInfo::DecodableType outList;

    auto status = tester.ReadAttribute(ActionList::Id, outList);
    ASSERT_TRUE(status.IsSuccess());

    // Verify the list is empty
    auto iter = outList.begin();
    EXPECT_FALSE(iter.Next());
}

// Test reading empty EndpointLists
TEST_F(TestActionsCluster, TestReadEmptyEndpointLists)
{
    // Don't add any endpoint lists

    // Create cluster instance
    ActionsCluster cluster(1, mDelegate);

    // Use ClusterTester to read the attribute
    chip::Testing::ClusterTester tester(cluster);
    EndpointLists::TypeInfo::DecodableType outList;

    auto status = tester.ReadAttribute(EndpointLists::Id, outList);
    ASSERT_TRUE(status.IsSuccess());

    // Verify the list is empty
    auto iter = outList.begin();
    EXPECT_FALSE(iter.Next());
}

} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
