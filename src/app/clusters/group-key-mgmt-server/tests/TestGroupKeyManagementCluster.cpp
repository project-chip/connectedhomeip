/*
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

#include <app/clusters/group-key-mgmt-server/GroupKeyManagementCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/GroupKeyManagement/ClusterId.h>
#include <clusters/GroupKeyManagement/Enums.h>
#include <clusters/GroupKeyManagement/Ids.h>
#include <clusters/GroupKeyManagement/Metadata.h>
#include <clusters/GroupKeyManagement/Structs.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/support/ReadOnlyBuffer.h>

namespace {
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::Testing;

struct TestGroupKeyManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestGroupKeyManagementCluster, CompileTest)
{
    GroupKeyManagementCluster cluster;
    ASSERT_EQ(cluster.GetClusterFlags({ kRootEndpointId, GroupKeyManagement::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestGroupKeyManagementCluster, CommandsTest)
{
    GroupKeyManagementCluster cluster;
    ConcreteClusterPath groupKeyManagementPath = ConcreteClusterPath(kRootEndpointId, GroupKeyManagement::Id);

    // Check required accepted commands are present
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommandsBuilder;
    ASSERT_EQ(cluster.AcceptedCommands(groupKeyManagementPath, acceptedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<DataModel::AcceptedCommandEntry> acceptedCommands = acceptedCommandsBuilder.TakeBuffer();
    ASSERT_EQ(acceptedCommands.size(), GroupKeyManagement::Commands::kAcceptedCommandsCount);

    ASSERT_EQ(acceptedCommands[0].commandId, GroupKeyManagement::Commands::KeySetWrite::Id);
    ASSERT_EQ(acceptedCommands[0].GetInvokePrivilege(),
              GroupKeyManagement::Commands::KeySetWrite::kMetadataEntry.GetInvokePrivilege());

    ASSERT_EQ(acceptedCommands[1].commandId, GroupKeyManagement::Commands::KeySetRead::Id);
    ASSERT_EQ(acceptedCommands[1].GetInvokePrivilege(),
              GroupKeyManagement::Commands::KeySetRead::kMetadataEntry.GetInvokePrivilege());

    ASSERT_EQ(acceptedCommands[2].commandId, GroupKeyManagement::Commands::KeySetRemove::Id);
    ASSERT_EQ(acceptedCommands[2].GetInvokePrivilege(),
              GroupKeyManagement::Commands::KeySetRemove::kMetadataEntry.GetInvokePrivilege());

    ASSERT_EQ(acceptedCommands[3].commandId, GroupKeyManagement::Commands::KeySetReadAllIndices::Id);
    ASSERT_EQ(acceptedCommands[3].GetInvokePrivilege(),
              GroupKeyManagement::Commands::KeySetReadAllIndices::kMetadataEntry.GetInvokePrivilege());

    // Check required generated commands are present
    ReadOnlyBufferBuilder<CommandId> generatedCommandsBuilder;
    ASSERT_EQ(cluster.GeneratedCommands(groupKeyManagementPath, generatedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

    ASSERT_EQ(generatedCommands.size(), GroupKeyManagement::Commands::kGeneratedCommandsCount);
    ASSERT_EQ(generatedCommands[0], GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Id);
    ASSERT_EQ(generatedCommands[1], GroupKeyManagement::Commands::KeySetReadResponse::Id);
}

TEST_F(TestGroupKeyManagementCluster, AttributesTest)
{
    GroupKeyManagementCluster cluster;
    std::vector<app::DataModel::AttributeEntry> mandatoryAttributes(GroupKeyManagement::Attributes::kMandatoryMetadata.begin(),
                                                                    GroupKeyManagement::Attributes::kMandatoryMetadata.end());

    // There are only mandatory attributes in this cluster, so it should match the ones in Metadata exactly
    ASSERT_TRUE(chip::Testing::IsAttributesListEqualTo(cluster, std::move(mandatoryAttributes)));
}

namespace TestHelpers {

constexpr GroupId kTestGroupId     = 0x1001;
constexpr uint16_t kTestKeySetId   = 1;
const FabricIndex kTestFabricIndex = 1;

GroupKeyManagement::Structs::GroupKeyMapStruct::Type CreateKey(GroupId groupId, uint16_t keySetId, FabricIndex fabricIndex)
{
    GroupKeyManagement::Structs::GroupKeyMapStruct::Type key;
    key.groupId       = groupId;
    key.groupKeySetID = keySetId;
    key.fabricIndex   = fabricIndex;
    return key;
}
std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type>
CreateGroupKeyMapList(size_t count, FabricIndex fabricIndex, GroupId startGroupId = kTestGroupId,
                      uint16_t startKeySetId = kTestKeySetId, uint16_t groupIdIncrement = 1, uint16_t keySetIdIncrement = 1)
{
    std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> list;
    for (size_t i = 0; i < count; ++i)
    {
        GroupId currentGroupId   = static_cast<GroupId>(startGroupId + (i * groupIdIncrement));
        uint16_t currentKeySetId = static_cast<uint16_t>(startKeySetId + (i * keySetIdIncrement));
        list.push_back(CreateKey(currentGroupId, currentKeySetId, fabricIndex));
    }
    return list;
}

} // namespace TestHelpers

struct TestGroupKeyManagementClusterWithStorage : public TestGroupKeyManagementCluster
{
    TestServerClusterContext mTestContext;
    Credentials::GroupDataProviderImpl mRealProvider;
    Crypto::DefaultSessionKeystore mMockKeystore;
    GroupKeyManagementCluster mCluster;
    ClusterTester tester;

    TestGroupKeyManagementClusterWithStorage() : tester(mCluster) {}

    void SetUp() override
    {
        if (Server::GetInstance().GetFabricTable().FabricCount() > 0)
        {
            Server::GetInstance().GetFabricTable().DeleteAllFabrics();
        }

        auto * storage = &mTestContext.StorageDelegate();

        mRealProvider.SetStorageDelegate(storage);
        mRealProvider.SetSessionKeystore(&mMockKeystore);

        ASSERT_EQ(mRealProvider.Init(), CHIP_NO_ERROR);
        ASSERT_EQ(mCluster.Startup(mTestContext.Get()), CHIP_NO_ERROR);

        Credentials::SetGroupDataProvider(&mRealProvider);
        tester.SetFabricIndex(TestHelpers::kTestFabricIndex);
    }

    void TearDown() override
    {
        tester.SetFabricIndex(kUndefinedFabricIndex);
        mCluster.Shutdown();
        Credentials::SetGroupDataProvider(nullptr);
        mRealProvider.Finish();
    }
    // Writes a list of group keys to the GroupKeyMap attribute for a given fabric.
    // Used to set up test scenarios with pre-existing keys.
    void PrepopulateGroupKeyMap(const std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> & keys,
                                FabricIndex fabricIndex)
    {
        auto listToWrite =
            app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

        CHIP_ERROR err = tester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite).GetUnderlyingError();
        ASSERT_EQ(err, CHIP_NO_ERROR);
    }

    // Checks that the stored group keys for a fabric match the expected list.
    // Validates group IDs and keyset IDs to ensure attribute writes succeeded.
    void VerifyGroupKeysMatch(const FabricIndex fabricIndex,
                              const std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> & expectedKeys)
    {
        auto * iterator = mRealProvider.IterateGroupKeys(fabricIndex);
        ASSERT_NE(iterator, nullptr);

        ASSERT_EQ(iterator->Count(), expectedKeys.size());

        size_t i = 0;
        Credentials::GroupDataProvider::GroupKey storedKey;
        while (iterator->Next(storedKey))
        {
            ASSERT_LT(i, expectedKeys.size());
            EXPECT_EQ(storedKey.group_id, expectedKeys[i].groupId);
            EXPECT_EQ(storedKey.keyset_id, expectedKeys[i].groupKeySetID);
            ++i;
        }

        ASSERT_EQ(i, expectedKeys.size());
        iterator->Release(); // ensure this frees memory or returns to pool
    }
};
// Cluster should accept writing multiple group keys with the same KeySetID but different Group IDs
TEST_F(TestGroupKeyManagementClusterWithStorage, TestWriteGroupKeyMapAttributeSameKeySetDifferentGroup)
{
    auto keys = TestHelpers::CreateGroupKeyMapList(2, TestHelpers::kTestFabricIndex, TestHelpers::kTestGroupId,
                                                   TestHelpers::kTestKeySetId, 1, 0);
    PrepopulateGroupKeyMap(keys, TestHelpers::kTestFabricIndex);
    VerifyGroupKeysMatch(TestHelpers::kTestFabricIndex, keys);
}

// Cluster should reject a write containing duplicate keys for the same group/keyset combination.
TEST_F(TestGroupKeyManagementClusterWithStorage, TestWriteGroupKeyMapAttributeDuplicateKey)
{
    //  Intentionally creates two identical entries (duplicate group/keyset combination).
    // by setting increments  to 0 (groupIdIncrement = 0, keySetIdIncrement = 0)
    auto keys = TestHelpers::CreateGroupKeyMapList(2, TestHelpers::kTestFabricIndex, TestHelpers::kTestGroupId,
                                                   TestHelpers::kTestKeySetId, 0, 0);

    auto listToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

    CHIP_ERROR err = tester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite).GetUnderlyingError();

    ASSERT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);

    // Explicitly check that the write stops at the first duplicate entry,
    // but the valid entries processed *before* the error (i.e., keys[0]) are persisted.
    std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> expectedKeys;
    expectedKeys.push_back(keys[0]);

    VerifyGroupKeysMatch(TestHelpers::kTestFabricIndex, expectedKeys);
}

TEST_F(TestGroupKeyManagementClusterWithStorage, TestWriteGroupKeyMapAttribute)
{
    auto keys = TestHelpers::CreateGroupKeyMapList(2, TestHelpers::kTestFabricIndex);

    PrepopulateGroupKeyMap(keys, TestHelpers::kTestFabricIndex);
    VerifyGroupKeysMatch(TestHelpers::kTestFabricIndex, keys);
}
} // namespace
