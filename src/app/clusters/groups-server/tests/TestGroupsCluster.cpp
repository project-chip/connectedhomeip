/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/groups-server/GroupsCluster.h>
#include <app/clusters/identify-server/IdentifyIntegrationDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Groups/Attributes.h>
#include <clusters/Groups/Commands.h>
#include <clusters/Groups/Metadata.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/ScopedNodeId.h>
#include <lib/core/StringBuilderAdapters.h>

#include <memory>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::Credentials;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr KeysetId kKeysetId         = 123;
constexpr FabricIndex kFabricIndex1  = kTestFabricIndex + 1;
constexpr FabricIndex kFabricIndex2  = kTestFabricIndex + 2;
constexpr FabricIndex kFabricIndex3  = kTestFabricIndex + 3;
constexpr FabricIndex kFabricIndex4  = kTestFabricIndex + 4;
constexpr FabricIndex kFabricIndex5  = kTestFabricIndex + 5;
constexpr FabricIndex kFabricIndex6  = kTestFabricIndex + 6;

Protocols::InteractionModel::Status CodeFor(uint8_t response_code)
{
    return static_cast<Protocols::InteractionModel::Status>(response_code);
}

class MockIdentifyIntegrationDelegate : public IdentifyIntegrationDelegate
{
public:
    bool IsIdentifying() override { return mIsIdentifying; }
    void SetIsIdentifying(bool isIdentifying) { mIsIdentifying = isIdentifying; }

private:
    bool mIsIdentifying = false;
};

class MockScenesIntegrationDelegate : public scenes::ScenesIntegrationDelegate
{
public:
    CHIP_ERROR GroupWillBeRemoved(FabricIndex fabricIndex, GroupId groupId) override
    {
        mGroupWillBeRemovedCallCount++;
        mLastFabricIndex = fabricIndex;
        mLastGroupId     = groupId;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR StoreCurrentGlobalScene(FabricIndex fabricIndex) override { return CHIP_NO_ERROR; }

    CHIP_ERROR RecallGlobalScene(FabricIndex fabricIndex) override { return CHIP_NO_ERROR; }

    CHIP_ERROR MakeSceneInvalidForAllFabrics() override { return CHIP_NO_ERROR; }

    uint32_t mGroupWillBeRemovedCallCount = 0;
    FabricIndex mLastFabricIndex          = kUndefinedFabricIndex;
    GroupId mLastGroupId                  = kUndefinedGroupId;
};

class TestGroupsCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mCluster       = std::make_unique<GroupsCluster>(kTestEndpointId,
                                                   GroupsCluster::Context{
                                                             .groupDataProvider   = mGroupDataProvider,
                                                             .scenesIntegration   = &mScenesDelegate,
                                                             .identifyIntegration = &mIdentifyDelegate,
                                                   });
        mClusterTester = std::make_unique<ClusterTester>(*mCluster);

        mGroupDataProvider.SetStorageDelegate(&mClusterTester->GetServerClusterContext().storage);
        mGroupDataProvider.SetSessionKeystore(&mSessionKeystore);
        ASSERT_EQ(mGroupDataProvider.Init(), CHIP_NO_ERROR);
        mClusterTester->SetFabricIndex(kTestFabricIndex);
    }

    void TearDown() override
    {
        mGroupDataProvider.Finish();
        mClusterTester.reset();
        mCluster.reset();
    }

protected:
    void SetupKeySet(FabricIndex fabricIndex, KeysetId keysetId)
    {
        GroupDataProvider::KeySet keySet(keysetId, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
        memset(&keySet.epoch_keys[0], 0, sizeof(keySet.epoch_keys[0]));
        keySet.epoch_keys[0].start_time = 0;
        uint8_t compressed_fabric_id[]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
        ASSERT_EQ(mGroupDataProvider.SetKeySet(fabricIndex, ByteSpan(compressed_fabric_id), keySet), CHIP_NO_ERROR);
    }

    void MapGroupToKeyset(FabricIndex fabricIndex, GroupId groupId, KeysetId keysetId, uint16_t groupIndex = 0)
    {
        ASSERT_EQ(mGroupDataProvider.SetGroupKeyAt(fabricIndex, groupIndex, GroupDataProvider::GroupKey(groupId, keysetId)),
                  CHIP_NO_ERROR);
    }

    ClusterTester::InvokeResult<Groups::Commands::AddGroupResponse::DecodableType> InvokeAddGroup(GroupId groupId,
                                                                                                  const char * groupName)
    {
        Groups::Commands::AddGroup::Type request;
        request.groupID   = groupId;
        request.groupName = CharSpan::fromCharString(groupName);
        return mClusterTester->Invoke<Groups::Commands::AddGroup::Type>(request);
    }

    ClusterTester::InvokeResult<Groups::Commands::ViewGroupResponse::DecodableType> InvokeViewGroup(GroupId groupId)
    {
        Groups::Commands::ViewGroup::Type request;
        request.groupID = groupId;
        return mClusterTester->Invoke<Groups::Commands::ViewGroup::Type>(request);
    }

    bool IsGroupInProvider(FabricIndex fabricIndex, GroupId groupId)
    {
        GroupDataProvider::GroupInfo info;
        for (size_t i = 0; i < mGroupDataProvider.GetMaxGroupsPerFabric(); ++i)
        {
            if (mGroupDataProvider.GetGroupInfoAt(fabricIndex, i, info) == CHIP_NO_ERROR)
            {
                if (info.group_id == groupId)
                {
                    return true;
                }
            }
        }
        return false;
    }

    GroupDataProviderImpl mGroupDataProvider;
    Crypto::DefaultSessionKeystore mSessionKeystore;
    MockIdentifyIntegrationDelegate mIdentifyDelegate;
    MockScenesIntegrationDelegate mScenesDelegate;
    std::unique_ptr<GroupsCluster> mCluster;
    std::unique_ptr<ClusterTester> mClusterTester;
};

// Tests reading the mandatory attributes of the Groups cluster.
// Verifies FeatureMap (must support GroupNames), NameSupport (bit 7 must be 1),
// and ClusterRevision attributes are present and correct.
TEST_F(TestGroupsCluster, TestReadAttributes)
{
    uint32_t featureMap = 0;
    EXPECT_EQ(mClusterTester->ReadAttribute(Groups::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, to_underlying(Groups::Feature::kGroupNames));

    uint8_t nameSupport = 0;
    EXPECT_EQ(mClusterTester->ReadAttribute(Groups::Attributes::NameSupport::Id, nameSupport), CHIP_NO_ERROR);
    EXPECT_TRUE(nameSupport & (1 << 7)); // Spec: NameSupport attribute, bit 7 (GroupNames) SHALL be equal to bit 0 of FeatureMap

    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester->ReadAttribute(Groups::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, Groups::kRevision);
}

// Tests the basic success case of the AddGroup command.
// Spec: Adds the endpoint to the group, updates the name, and returns SUCCESS.
TEST_F(TestGroupsCluster, TestAddGroup)
{
    constexpr GroupId kGroupId = 1;

    SetupKeySet(kTestFabricIndex, kKeysetId);
    MapGroupToKeyset(kTestFabricIndex, kGroupId, kKeysetId);

    auto result = InvokeAddGroup(kGroupId, "Test Group");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->groupID, kGroupId);

    // Verify the group was added
    EXPECT_TRUE(IsGroupInProvider(kTestFabricIndex, kGroupId));
}

// Tests the AddGroup command with an invalid GroupID (0).
// Spec: GroupID must be >= 1, otherwise returns CONSTRAINT_ERROR.
TEST_F(TestGroupsCluster, TestAddGroupInvalidId)
{
    auto result = InvokeAddGroup(0, "Invalid Group");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::ConstraintError);
}

// Tests the AddGroup command with a GroupName exceeding the maximum length (16).
// Spec: GroupName must be <= 16 chars, otherwise returns CONSTRAINT_ERROR.
TEST_F(TestGroupsCluster, TestAddGroupLongName)
{
    auto result = InvokeAddGroup(2, "This Group Name Is Way Too Long");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::ConstraintError);
}

// Tests the AddGroup command when the group table is full.
// Spec: If node requires security material for the group and it doesn't exist,
// returns UNSUPPORTED_ACCESS. This happens when the key map is full.
TEST_F(TestGroupsCluster, TestAddGroupMaxGroups)
{
    mClusterTester->SetFabricIndex(kFabricIndex1);
    const uint16_t max_groups = mGroupDataProvider.GetMaxGroupsPerFabric();

    SetupKeySet(kFabricIndex1, kKeysetId);

    // Fill up the group table and group key map up to max_groups
    for (uint16_t i = 0; i < max_groups; ++i)
    {
        auto kGroupId = static_cast<GroupId>(i + 1);

        MapGroupToKeyset(kFabricIndex1, kGroupId, kKeysetId, i);

        StringBuilder<16> groupName;
        groupName.AddFormat("Group %d", i);
        auto result = InvokeAddGroup(kGroupId, groupName.c_str());
        EXPECT_TRUE(result.IsSuccess());
        ASSERT_TRUE(result.response.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    }

    // Now GroupInfo table is full. GroupKey table is also full.
    // Try to add the extra group. This will fail KeyExists check because we cannot add a GroupKey entry.
    auto extraGroupId = static_cast<GroupId>(max_groups + 1);
    auto result       = InvokeAddGroup(extraGroupId, "Max Group");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::UnsupportedAccess);
}

// Tests the AddGroup command for a group without prior key setup.
// Spec: If node requires security material for the group and it doesn't exist,
// returns UNSUPPORTED_ACCESS.
TEST_F(TestGroupsCluster, TestAddGroup_UnsupportedAccess)
{
    constexpr GroupId kGroupId = 1;

    // No SetupKeySet or MapGroupToKeyset for kTestFabricIndex

    auto result = InvokeAddGroup(kGroupId, "Test Group");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::UnsupportedAccess);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->groupID, kGroupId);

    // Verify the group was not added
    EXPECT_FALSE(IsGroupInProvider(kTestFabricIndex, kGroupId));
}

// Tests the ViewGroup command.
// Spec: Responds with group name if found (SUCCESS), or NOT_FOUND.
// CONSTRAINT_ERROR for GroupID < 1.
TEST_F(TestGroupsCluster, TestViewGroup)
{
    mClusterTester->SetFabricIndex(kFabricIndex2);
    constexpr GroupId kGroupId = 1;
    const char * kGroupName    = "View Group Test";

    // 1. Test NotFound
    auto result = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::NotFound);

    // 2. Add the group
    SetupKeySet(kFabricIndex2, kKeysetId);
    MapGroupToKeyset(kFabricIndex2, kGroupId, kKeysetId);
    auto addResult = InvokeAddGroup(kGroupId, kGroupName);
    ASSERT_TRUE(addResult.IsSuccess());
    ASSERT_TRUE(addResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(addResult.response->status), Protocols::InteractionModel::Status::Success);

    // 3. Test Success
    result = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->groupID, kGroupId);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.response->groupName.data_equal(CharSpan::fromCharString(kGroupName)));

    // 4. Test Invalid ID
    result = InvokeViewGroup(0);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::ConstraintError);
}

// Tests the RemoveGroup command.
// Spec: Removes endpoint membership from the group (SUCCESS), or NOT_FOUND.
// CONSTRAINT_ERROR for GroupID < 1.
TEST_F(TestGroupsCluster, TestRemoveGroup)
{
    mClusterTester->SetFabricIndex(kFabricIndex3);
    constexpr GroupId kGroupId      = 10;
    constexpr GroupId kOtherGroupId = 11;

    // 1. Add the group to be removed
    SetupKeySet(kFabricIndex3, kKeysetId);
    MapGroupToKeyset(kFabricIndex3, kGroupId, kKeysetId);
    auto addResult = InvokeAddGroup(kGroupId, "RemoveTest");
    ASSERT_TRUE(addResult.IsSuccess());
    ASSERT_TRUE(addResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(addResult.response->status), Protocols::InteractionModel::Status::Success);

    // 2. Test NotFound on a different GroupId
    Groups::Commands::RemoveGroup::Type removeRequest;
    removeRequest.groupID = kOtherGroupId;
    auto result           = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(removeRequest);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::NotFound);

    // 3. Test Success on the added group
    removeRequest.groupID = kGroupId;
    result                = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(removeRequest);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(result.response->groupID, kGroupId);

    // Verify group is removed
    auto viewResult = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::NotFound);

    // 4. Test Invalid ID
    removeRequest.groupID = 0;
    result                = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(removeRequest);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::ConstraintError);
}

// Tests the GetGroupMembership command.
// Spec: If GroupList is empty, returns all groups endpoint is member of.
// If GroupList is not empty, returns intersection of member groups and GroupList.
// Capacity field indicates remaining group capacity.
TEST_F(TestGroupsCluster, TestGetGroupMembership)
{
    mClusterTester->SetFabricIndex(kFabricIndex4);

    SetupKeySet(kFabricIndex4, kKeysetId);

    // Add some groups
    constexpr GroupId kGroupId1 = 1;
    constexpr GroupId kGroupId2 = 5;
    constexpr GroupId kGroupId3 = 10;
    MapGroupToKeyset(kFabricIndex4, kGroupId1, kKeysetId, 0);
    MapGroupToKeyset(kFabricIndex4, kGroupId2, kKeysetId, 1);
    MapGroupToKeyset(kFabricIndex4, kGroupId3, kKeysetId, 2);

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId1, "G1").response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId2, "G2").response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId3, "G3").response->status), Protocols::InteractionModel::Status::Success);

    // Test cases
    Groups::Commands::GetGroupMembership::Type request;
    constexpr GroupId kNonExistentGroup = 100;

    // 1. Empty group list - expected to return all added groups.
    request.groupList = Span<const GroupId>();
    auto result       = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(result.response->capacity.IsNull()); // Capacity is allowed to be null
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    auto iter = result.response->groupList.begin();
    EXPECT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId1);
    EXPECT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId2);
    EXPECT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId3);
    EXPECT_FALSE(iter.Next());

    // 2. Non-existent group - expected to return an empty list.
    const GroupId groupList2[] = { kNonExistentGroup };
    request.groupList          = Span<const GroupId>(groupList2);
    result                     = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    iter = result.response->groupList.begin();
    EXPECT_FALSE(iter.Next());

    // 3. Some exist, some don't - expected to return only kGroupId1 and kGroupId3.
    const GroupId groupList3[] = { kGroupId1, kNonExistentGroup, kGroupId3 };
    request.groupList          = Span<const GroupId>(groupList3);
    result                     = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    iter = result.response->groupList.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId1);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId3);
    EXPECT_FALSE(iter.Next());

    // 4. All exist - expected to return all groups in the list.
    const GroupId groupList4[] = { kGroupId1, kGroupId2, kGroupId3 };
    request.groupList          = Span<const GroupId>(groupList4);
    result                     = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    iter = result.response->groupList.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId1);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId2);
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId3);
    EXPECT_FALSE(iter.Next());
}

// Tests the RemoveAllGroups command.
// Spec: Removes all group memberships for the server endpoint.
TEST_F(TestGroupsCluster, TestRemoveAllGroups)
{
    mClusterTester->SetFabricIndex(kFabricIndex5);

    SetupKeySet(kFabricIndex5, kKeysetId);

    // Add some groups
    constexpr GroupId kGroupId1 = 1;
    constexpr GroupId kGroupId2 = 5;
    MapGroupToKeyset(kFabricIndex5, kGroupId1, kKeysetId, 0);
    MapGroupToKeyset(kFabricIndex5, kGroupId2, kKeysetId, 1);

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId1, "G1").response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId2, "G2").response->status), Protocols::InteractionModel::Status::Success);

    // Call RemoveAllGroups
    Groups::Commands::RemoveAllGroups::Type removeAllRequest;
    auto result = mClusterTester->Invoke<Groups::Commands::RemoveAllGroups::Type>(removeAllRequest);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.response.has_value()); // This command has no response payload

    // Verify groups are removed
    EXPECT_FALSE(IsGroupInProvider(kFabricIndex5, kGroupId1));
    EXPECT_FALSE(IsGroupInProvider(kFabricIndex5, kGroupId2));
}

// Tests the AddGroupIfIdentifying command.
// Spec: Adds group membership only if the endpoint is currently identifying.
// If not identifying, the command shall return SUCCESS but take no other action.
TEST_F(TestGroupsCluster, TestAddGroupIfIdentifying)
{
    mClusterTester->SetFabricIndex(kFabricIndex6);
    constexpr GroupId kGroupId = 1;

    SetupKeySet(kFabricIndex6, kKeysetId);
    MapGroupToKeyset(kFabricIndex6, kGroupId, kKeysetId);

    Groups::Commands::AddGroupIfIdentifying::Type request;
    request.groupID   = kGroupId;
    request.groupName = "Identify"_span;

    // 1. Not identifying - Should return SUCCESS but not add the group
    mIdentifyDelegate.SetIsIdentifying(false);
    auto result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.response.has_value()); // No response payload for this command
    EXPECT_FALSE(IsGroupInProvider(kFabricIndex6, kGroupId));

    // 2. Start identifying
    mIdentifyDelegate.SetIsIdentifying(true);

    // 3. Add group while identifying - Should add the group and return SUCCESS.
    result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.response.has_value()); // No response payload for this command
    EXPECT_TRUE(IsGroupInProvider(kFabricIndex6, kGroupId));
    auto viewResult = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(viewResult.response->groupID, kGroupId);

    // 4. Stop identifying
    mIdentifyDelegate.SetIsIdentifying(false);

    // 5. Add group while not identifying (on a different GroupId) - Should return SUCCESS but not add the group
    constexpr GroupId kOtherGroupId = kGroupId + 1;
    request.groupID                 = kOtherGroupId;
    MapGroupToKeyset(kFabricIndex6, kOtherGroupId, kKeysetId, 1);
    result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.response.has_value());
    EXPECT_FALSE(IsGroupInProvider(kFabricIndex6, kOtherGroupId));
}

// Tests that calling AddGroup on an existing group updates the group name.
// Spec: If the endpoint is already a member, the group name SHALL be updated.
TEST_F(TestGroupsCluster, TestAddGroupUpdateName)
{
    constexpr GroupId kGroupId = 1;

    SetupKeySet(kTestFabricIndex, kKeysetId);
    MapGroupToKeyset(kTestFabricIndex, kGroupId, kKeysetId);

    auto result = InvokeAddGroup(kGroupId, "Original Name");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);

    // Update the group name
    result = InvokeAddGroup(kGroupId, "Updated Name");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);

    // Verify the group name was updated
    auto viewResult = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(viewResult.response->groupName.data_equal("Updated Name"_span));
}

// Tests adding a group with an empty name.
// Spec: GroupName field MAY be set to the empty string if the client has no name.
TEST_F(TestGroupsCluster, TestAddGroupEmptyName)
{
    constexpr GroupId kGroupId = 1;

    SetupKeySet(kTestFabricIndex, kKeysetId);
    MapGroupToKeyset(kTestFabricIndex, kGroupId, kKeysetId);

    auto result = InvokeAddGroup(kGroupId, "");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);

    // Verify the group name is empty
    auto viewResult = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(viewResult.response->groupName.empty());
}

// Tests adding a group with the maximum allowed name length (16 chars).
// Spec: GroupName has a max length of 16.
TEST_F(TestGroupsCluster, TestAddGroupMaxLengthName)
{
    constexpr GroupId kGroupId = 1;

    SetupKeySet(kTestFabricIndex, kKeysetId);
    MapGroupToKeyset(kTestFabricIndex, kGroupId, kKeysetId);

    auto result = InvokeAddGroup(kGroupId, "1234567890123456"); // 16 characters
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);

    // Verify the group name
    auto viewResult = InvokeViewGroup(kGroupId);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_TRUE(viewResult.response->groupName.data_equal("1234567890123456"_span));
}

// Tests AddGroupIfIdentifying with an invalid GroupID (0).
// Spec: GroupID must be >= 1, otherwise returns CONSTRAINT_ERROR.
TEST_F(TestGroupsCluster, TestAddGroupIfIdentifying_InvalidId)
{
    mIdentifyDelegate.SetIsIdentifying(true);

    Groups::Commands::AddGroupIfIdentifying::Type request;
    request.groupID   = 0; // Invalid GroupId
    request.groupName = "Identify"_span;

    auto result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_EQ(result.status, Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_FALSE(result.response.has_value());
}

// Tests AddGroupIfIdentifying with a GroupName exceeding the maximum length (16).
// Spec: GroupName must be <= 16 chars, otherwise returns CONSTRAINT_ERROR.
TEST_F(TestGroupsCluster, TestAddGroupIfIdentifying_LongName)
{
    mIdentifyDelegate.SetIsIdentifying(true);

    Groups::Commands::AddGroupIfIdentifying::Type request;
    request.groupID   = 1;
    request.groupName = "This Name Is Way Too Long For Group"_span;

    auto result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_EQ(result.status, Protocols::InteractionModel::Status::ConstraintError);
    EXPECT_FALSE(result.response.has_value());
}

// Tests AddGroupIfIdentifying for a group without prior key setup.
// Spec: If node requires security material and it doesn't exist, returns UNSUPPORTED_ACCESS.
TEST_F(TestGroupsCluster, TestAddGroupIfIdentifying_UnsupportedAccess)
{
    mIdentifyDelegate.SetIsIdentifying(true);

    Groups::Commands::AddGroupIfIdentifying::Type request;
    request.groupID   = 1; // No keyset setup for this group
    request.groupName = "Identify"_span;

    auto result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_EQ(result.status, Protocols::InteractionModel::Status::UnsupportedAccess);
    EXPECT_FALSE(result.response.has_value());
}

// Tests AddGroupIfIdentifying when the group table is full.
// Spec: If there are no available resources, returns RESOURCE_EXHAUSTED.
TEST_F(TestGroupsCluster, TestAddGroupIfIdentifying_ResourceExhausted)
{
    mClusterTester->SetFabricIndex(kFabricIndex1);
    const uint16_t max_groups = mGroupDataProvider.GetMaxGroupsPerFabric();

    SetupKeySet(kFabricIndex1, kKeysetId);

    // Fill up the group table and group key map up to max_groups
    for (uint16_t i = 0; i < max_groups; ++i)
    {
        auto kGroupId = static_cast<GroupId>(i + 1);
        MapGroupToKeyset(kFabricIndex1, kGroupId, kKeysetId, i);
        StringBuilder<16> groupName;
        groupName.AddFormat("Group %d", i);
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId, groupName.c_str()).response->status),
                  Protocols::InteractionModel::Status::Success);
    }

    // Now GroupInfo table is full. GroupKey table is also full.
    // Try to add the extra group while identifying.
    mIdentifyDelegate.SetIsIdentifying(true);
    auto extraGroupId = static_cast<GroupId>(max_groups + 1);
    // Map key - this overwrites the group index 0, but is required to make it seem like the group is valid
    MapGroupToKeyset(kFabricIndex1, extraGroupId, kKeysetId, 0);
    Groups::Commands::AddGroupIfIdentifying::Type request;
    request.groupID   = extraGroupId;
    request.groupName = "Max Group"_span;

    auto result = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    EXPECT_EQ(result.status, Protocols::InteractionModel::Status::ResourceExhausted);
    EXPECT_FALSE(result.response.has_value());
}

// Tests RemoveAllGroups when no groups are currently associated with the endpoint.
// Spec: Server SHALL remove all group memberships. Response is SUCCESS.
TEST_F(TestGroupsCluster, TestRemoveAllGroups_NoGroups)
{
    // Call RemoveAllGroups when no groups are present
    Groups::Commands::RemoveAllGroups::Type removeAllRequest;
    auto result = mClusterTester->Invoke<Groups::Commands::RemoveAllGroups::Type>(removeAllRequest);
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.response.has_value());
}

// Tests that group management is properly scoped to the accessing fabric.
// Spec: "All commands defined in this cluster SHALL only affect groups scoped to the accessing fabric."
TEST_F(TestGroupsCluster, TestFabricScoping)
{
    constexpr GroupId kGroupId1   = 1;
    constexpr GroupId kGroupId2   = 2;
    constexpr KeysetId kKeysetId1 = 123;
    constexpr KeysetId kKeysetId2 = 456;

    // Fabric 1 setup
    mClusterTester->SetFabricIndex(kFabricIndex1);
    SetupKeySet(kFabricIndex1, kKeysetId1);
    MapGroupToKeyset(kFabricIndex1, kGroupId1, kKeysetId1);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId1, "Fabric1Group").response->status), Protocols::InteractionModel::Status::Success);

    // Fabric 2 setup
    mClusterTester->SetFabricIndex(kFabricIndex2);
    SetupKeySet(kFabricIndex2, kKeysetId2);
    MapGroupToKeyset(kFabricIndex2, kGroupId2, kKeysetId2);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(InvokeAddGroup(kGroupId2, "Fabric2Group").response->status), Protocols::InteractionModel::Status::Success);

    // Check isolation
    // Group1 should not be in Fabric 2
    mClusterTester->SetFabricIndex(kFabricIndex2);
    auto viewResult = InvokeViewGroup(kGroupId1);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::NotFound);

    // Group2 should not be in Fabric 1
    mClusterTester->SetFabricIndex(kFabricIndex1);
    viewResult = InvokeViewGroup(kGroupId2);
    EXPECT_TRUE(viewResult.IsSuccess());
    ASSERT_TRUE(viewResult.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::NotFound);

    // GetGroupMembership for Fabric 1 - Should only contain GroupId1
    mClusterTester->SetFabricIndex(kFabricIndex1);
    Groups::Commands::GetGroupMembership::Type request;
    request.groupList = Span<const GroupId>();
    auto result       = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    auto iter = result.response->groupList.begin();
    EXPECT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId1);
    EXPECT_FALSE(iter.Next());

    // GetGroupMembership for Fabric 2 - Should only contain GroupId2
    mClusterTester->SetFabricIndex(kFabricIndex2);
    result = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    iter = result.response->groupList.begin();
    EXPECT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue(), kGroupId2);
    EXPECT_FALSE(iter.Next());
}

// Tests that group name changes are node-wide.
// Spec: The server stores a name string, which is set by the client for each assigned group.
// The GroupName associated with the GroupID in the Group Table SHALL be updated to reflect the new GroupName provided for the
// Group, such that subsequent ViewGroup commands yield the same name for all endpoints which have a group association to the given
// GroupID.
TEST_F(TestGroupsCluster, TestGroupNameNodeWide)
{
    constexpr GroupId kGroupId      = 1;
    constexpr EndpointId kEndpoint1 = 1;
    constexpr EndpointId kEndpoint2 = 2;

    SetupKeySet(kTestFabricIndex, kKeysetId);
    MapGroupToKeyset(kTestFabricIndex, kGroupId, kKeysetId);

    // Endpoint 1
    GroupsCluster cluster1(kEndpoint1, { mGroupDataProvider, &mScenesDelegate, &mIdentifyDelegate });
    ClusterTester tester1(cluster1);
    tester1.SetFabricIndex(kTestFabricIndex);

    // Endpoint 2
    GroupsCluster cluster2(kEndpoint2, { mGroupDataProvider, &mScenesDelegate, &mIdentifyDelegate });
    ClusterTester tester2(cluster2);
    tester2.SetFabricIndex(kTestFabricIndex);

    // Add group to Endpoint 1
    {
        Groups::Commands::AddGroup::Type request = { kGroupId, "Group Name 1"_span };
        auto result                              = tester1.Invoke<Groups::Commands::AddGroup::Type>(request);
        ASSERT_TRUE(result.IsSuccess());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    }

    // Add group to Endpoint 2
    {
        Groups::Commands::AddGroup::Type request = { kGroupId, "Group Name 1"_span };
        auto result                              = tester2.Invoke<Groups::Commands::AddGroup::Type>(request);
        ASSERT_TRUE(result.IsSuccess());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    }

    // Update group name from Endpoint 1
    {
        Groups::Commands::AddGroup::Type request = { kGroupId, "Updated Name"_span };
        auto result                              = tester1.Invoke<Groups::Commands::AddGroup::Type>(request);
        ASSERT_TRUE(result.IsSuccess());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);
    }

    // Verify name change on Endpoint 1
    {
        Groups::Commands::ViewGroup::Type request = { kGroupId };
        auto viewResult                           = tester1.Invoke<Groups::Commands::ViewGroup::Type>(request);
        ASSERT_TRUE(viewResult.IsSuccess());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::Success);
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_TRUE(viewResult.response->groupName.data_equal("Updated Name"_span));
    }

    // Verify name change on Endpoint 2
    {
        Groups::Commands::ViewGroup::Type request = { kGroupId };
        auto viewResult                           = tester2.Invoke<Groups::Commands::ViewGroup::Type>(request);
        ASSERT_TRUE(viewResult.IsSuccess());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(CodeFor(viewResult.response->status), Protocols::InteractionModel::Status::Success);
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_TRUE(viewResult.response->groupName.data_equal("Updated Name"_span));
    }
}

// Tests AddGroup command when group info table is full but key mapping exists.
// Spec: If there are no available resources to add the membership for the server endpoint, the status SHALL be RESOURCE_EXHAUSTED.
TEST_F(TestGroupsCluster, TestAddGroupResourceExhaustedKeyExists)
{
    mClusterTester->SetFabricIndex(kFabricIndex1);
    const uint16_t max_groups = mGroupDataProvider.GetMaxGroupsPerFabric();

    SetupKeySet(kFabricIndex1, kKeysetId);

    // Fill up the group info table to max_groups - 1
    for (uint16_t i = 0; i < max_groups - 1; ++i)
    {
        auto kGroupId = static_cast<GroupId>(i + 1);
        MapGroupToKeyset(kFabricIndex1, kGroupId, kKeysetId, i);
        ASSERT_EQ(mGroupDataProvider.SetGroupInfo(kFabricIndex1, GroupDataProvider::GroupInfo(kGroupId, "Test")), CHIP_NO_ERROR);
    }

    // Add the last group to make the table full
    const GroupId kLastGroupId = max_groups;
    MapGroupToKeyset(kFabricIndex1, kLastGroupId, kKeysetId, max_groups - 1);
    ASSERT_EQ(mGroupDataProvider.SetGroupInfo(kFabricIndex1, GroupDataProvider::GroupInfo(kLastGroupId, "Test")), CHIP_NO_ERROR);

    // There will be no space for this one
    const GroupId kExistingKeyGroupId = max_groups + 1;
    // Ensure the key mapping still exists for GroupId 1
    MapGroupToKeyset(kFabricIndex1, kExistingKeyGroupId, kKeysetId, 0);

    auto result = InvokeAddGroup(kExistingKeyGroupId, "FinalOverflow");
    EXPECT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::ResourceExhausted);
}

// Tests that ScenesIntegrationDelegate is called when a group is removed.
TEST_F(TestGroupsCluster, TestRemoveGroupScenesCleanup)
{
    mClusterTester->SetFabricIndex(kFabricIndex3);
    constexpr GroupId kGroupId = 10;

    SetupKeySet(kFabricIndex3, kKeysetId);
    MapGroupToKeyset(kFabricIndex3, kGroupId, kKeysetId);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(CodeFor(InvokeAddGroup(kGroupId, "SceneTest").response->status), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(mScenesDelegate.mGroupWillBeRemovedCallCount, 0u);

    Groups::Commands::RemoveGroup::Type removeRequest;
    removeRequest.groupID = kGroupId;
    auto result           = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(removeRequest);
    ASSERT_TRUE(result.IsSuccess());
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(CodeFor(result.response->status), Protocols::InteractionModel::Status::Success);

    EXPECT_EQ(mScenesDelegate.mGroupWillBeRemovedCallCount, 1u);
    EXPECT_EQ(mScenesDelegate.mLastFabricIndex, kFabricIndex3);
    EXPECT_EQ(mScenesDelegate.mLastGroupId, kGroupId);
}

// Tests that ScenesIntegrationDelegate is called for each group during RemoveAllGroups.
TEST_F(TestGroupsCluster, TestRemoveAllGroupsScenesCleanup)
{
    mClusterTester->SetFabricIndex(kFabricIndex5);

    SetupKeySet(kFabricIndex5, kKeysetId);

    constexpr GroupId kGroupId1 = 1;
    constexpr GroupId kGroupId2 = 5;
    MapGroupToKeyset(kFabricIndex5, kGroupId1, kKeysetId, 0);
    MapGroupToKeyset(kFabricIndex5, kGroupId2, kKeysetId, 1);

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(CodeFor(InvokeAddGroup(kGroupId1, "G1").response->status), Protocols::InteractionModel::Status::Success);
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    ASSERT_EQ(CodeFor(InvokeAddGroup(kGroupId2, "G2").response->status), Protocols::InteractionModel::Status::Success);

    EXPECT_EQ(mScenesDelegate.mGroupWillBeRemovedCallCount, 0u);

    Groups::Commands::RemoveAllGroups::Type removeAllRequest;
    auto result = mClusterTester->Invoke<Groups::Commands::RemoveAllGroups::Type>(removeAllRequest);
    EXPECT_TRUE(result.IsSuccess());

    EXPECT_EQ(mScenesDelegate.mGroupWillBeRemovedCallCount, 3u); // 2 groups + global scene group
}

} // namespace
