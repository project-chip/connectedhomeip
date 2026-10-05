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
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/groups-server/StubbedGroupsCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Groups/Attributes.h>
#include <clusters/Groups/Commands.h>
#include <clusters/Groups/Metadata.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/StringBuilderAdapters.h>

#include <memory>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::Credentials;
using namespace chip::Testing;
using namespace chip::Protocols::InteractionModel;

namespace {

constexpr EndpointId kTestEndpointId  = 1;
constexpr EndpointId kOtherEndpointId = 2;
constexpr KeysetId kKeysetId          = 123;

class TestStubbedGroupsCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mCluster       = std::make_unique<StubbedGroupsCluster>(kTestEndpointId,
                                                          StubbedGroupsCluster::Context{
                                                                    .groupDataProvider = mGroupDataProvider,
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
    void SetupKeySet(FabricIndex fabricIndex)
    {
        GroupDataProvider::KeySet keySet(kKeysetId, GroupDataProvider::SecurityPolicy::kTrustFirst, 1);
        memset(&keySet.epoch_keys[0], 0, sizeof(keySet.epoch_keys[0]));
        keySet.epoch_keys[0].start_time = 0;
        uint8_t compressed_fabric_id[]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
        ASSERT_EQ(mGroupDataProvider.SetKeySet(fabricIndex, ByteSpan(compressed_fabric_id), keySet), CHIP_NO_ERROR);
    }

    void AddGroupWithEndpoint(FabricIndex fabricIndex, GroupId groupId, EndpointId endpointId, uint16_t groupKeyIndex = 0)
    {
        ASSERT_EQ(mGroupDataProvider.SetGroupKeyAt(fabricIndex, groupKeyIndex, GroupDataProvider::GroupKey(groupId, kKeysetId)),
                  CHIP_NO_ERROR);
        ASSERT_EQ(mGroupDataProvider.SetGroupInfo(fabricIndex, GroupDataProvider::GroupInfo(groupId, "")), CHIP_NO_ERROR);
        ASSERT_EQ(mGroupDataProvider.AddEndpoint(fabricIndex, groupId, endpointId), CHIP_NO_ERROR);
    }

    GroupDataProviderImpl mGroupDataProvider{ 16, 16 };
    Crypto::DefaultSessionKeystore mSessionKeystore;
    std::unique_ptr<StubbedGroupsCluster> mCluster;
    std::unique_ptr<ClusterTester> mClusterTester;
};

TEST_F(TestStubbedGroupsCluster, TestReadAttributes)
{
    uint32_t featureMap = 0;
    EXPECT_EQ(mClusterTester->ReadAttribute(Groups::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, to_underlying(Groups::Feature::kGroupNames));

    uint8_t nameSupport = 0;
    EXPECT_EQ(mClusterTester->ReadAttribute(Groups::Attributes::NameSupport::Id, nameSupport), CHIP_NO_ERROR);
    EXPECT_TRUE(nameSupport & (1 << 7));

    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester->ReadAttribute(Groups::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, Groups::kRevision);
}

TEST_F(TestStubbedGroupsCluster, TestAddGroupReturnsInvalidState)
{
    Groups::Commands::AddGroup::Type request;
    request.groupID   = 1;
    request.groupName = "Test"_span;
    auto result       = mClusterTester->Invoke<Groups::Commands::AddGroup::Type>(request);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidInState));
}

TEST_F(TestStubbedGroupsCluster, TestViewGroupReturnsInvalidState)
{
    Groups::Commands::ViewGroup::Type request;
    request.groupID = 1;
    auto result     = mClusterTester->Invoke<Groups::Commands::ViewGroup::Type>(request);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidInState));
}

TEST_F(TestStubbedGroupsCluster, TestGetGroupMembershipReturnsInvalidState)
{
    Groups::Commands::GetGroupMembership::Type request;
    request.groupList = Span<const GroupId>();
    auto result       = mClusterTester->Invoke<Groups::Commands::GetGroupMembership::Type>(request);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidInState));
}

TEST_F(TestStubbedGroupsCluster, TestAddGroupIfIdentifyingReturnsInvalidState)
{
    Groups::Commands::AddGroupIfIdentifying::Type request;
    request.groupID   = 1;
    request.groupName = "Test"_span;
    auto result       = mClusterTester->Invoke<Groups::Commands::AddGroupIfIdentifying::Type>(request);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidInState));
}

TEST_F(TestStubbedGroupsCluster, TestRemoveGroupRemovesEndpoint)
{
    constexpr GroupId kGroupId = 42;

    SetupKeySet(kTestFabricIndex);
    AddGroupWithEndpoint(kTestFabricIndex, kGroupId, kTestEndpointId);
    ASSERT_TRUE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId, kTestEndpointId));

    Groups::Commands::RemoveGroup::Type request;
    request.groupID = kGroupId;
    auto result     = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(request);
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(static_cast<Status>(result.response->status), Status::Success);
    EXPECT_EQ(result.response->groupID, kGroupId);

    EXPECT_FALSE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId, kTestEndpointId));
}

TEST_F(TestStubbedGroupsCluster, TestRemoveGroupNotFound)
{
    Groups::Commands::RemoveGroup::Type request;
    request.groupID = 99;
    auto result     = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(request);
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(static_cast<Status>(result.response->status), Status::NotFound);
    EXPECT_EQ(result.response->groupID, 99u);
}

TEST_F(TestStubbedGroupsCluster, TestRemoveGroupInvalidGroupId)
{
    Groups::Commands::RemoveGroup::Type request;
    request.groupID = kUndefinedGroupId;
    auto result     = mClusterTester->Invoke<Groups::Commands::RemoveGroup::Type>(request);
    ASSERT_TRUE(result.response.has_value());
    EXPECT_EQ(static_cast<Status>(result.response->status), Status::ConstraintError);
    EXPECT_EQ(result.response->groupID, kUndefinedGroupId);
}

TEST_F(TestStubbedGroupsCluster, TestRemoveAllGroupsRemovesEndpoint)
{
    constexpr GroupId kGroupId1 = 10;
    constexpr GroupId kGroupId2 = 20;

    SetupKeySet(kTestFabricIndex);
    AddGroupWithEndpoint(kTestFabricIndex, kGroupId1, kTestEndpointId, 0);
    AddGroupWithEndpoint(kTestFabricIndex, kGroupId2, kTestEndpointId, 1);

    ASSERT_EQ(mGroupDataProvider.AddEndpoint(kTestFabricIndex, kGroupId1, kOtherEndpointId), CHIP_NO_ERROR);

    ASSERT_TRUE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId1, kTestEndpointId));
    ASSERT_TRUE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId2, kTestEndpointId));
    ASSERT_TRUE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId1, kOtherEndpointId));

    Groups::Commands::RemoveAllGroups::Type request;
    auto result = mClusterTester->Invoke<Groups::Commands::RemoveAllGroups::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    EXPECT_FALSE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId1, kTestEndpointId));
    EXPECT_FALSE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId2, kTestEndpointId));
    EXPECT_TRUE(mGroupDataProvider.HasEndpoint(kTestFabricIndex, kGroupId1, kOtherEndpointId));
}

} // namespace
