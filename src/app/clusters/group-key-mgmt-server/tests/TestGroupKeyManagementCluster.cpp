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
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsGeneratedCommandsListEqualTo;

chip::FabricIndex kTestFabricIndex = Testing::kTestFabrixIndex;
const chip::GroupId kTestGroupId   = 0x1234;
constexpr uint16_t kTestKeySetId   = 1;

namespace TestHelpers {

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

struct TestGroupKeyManagementCluster : public ::testing::Test
{

    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext mTestContext;
    Credentials::GroupDataProviderImpl mRealProvider;
    Crypto::DefaultSessionKeystore mMockKeystore;
    FabricTestFixture fabricHelper{ &mTestContext.StorageDelegate() };

    GroupKeyManagementCluster mCluster{ { fabricHelper.GetFabricTable(), mRealProvider } };

    ClusterTester tester{ mCluster };

    void SetUp() override
    {
        auto * storage = &mTestContext.StorageDelegate();

        mRealProvider.SetStorageDelegate(storage);
        mRealProvider.SetSessionKeystore(&mMockKeystore);
        ASSERT_EQ(mRealProvider.Init(), CHIP_NO_ERROR);
        ASSERT_EQ(mCluster.Startup(mTestContext.Get()), CHIP_NO_ERROR);
        CHIP_ERROR err = fabricHelper.SetUpTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        Credentials::SetGroupDataProvider(&mRealProvider);
        tester.SetFabricIndex(kTestFabricIndex);
    }

    void TearDown() override
    {
        tester.SetFabricIndex(kUndefinedFabricIndex);
        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        Credentials::SetGroupDataProvider(nullptr);
        CHIP_ERROR err = fabricHelper.TearDownTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
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

TEST_F(TestGroupKeyManagementCluster, CompileTest)
{
    ASSERT_EQ(mCluster.GetClusterFlags({ kRootEndpointId, GroupKeyManagement::Id }), BitFlags<ClusterQualityFlags>());
}

TEST_F(TestGroupKeyManagementCluster, CommandsTest)
{
    // Check required accepted commands are present
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              {
                                                  GroupKeyManagement::Commands::KeySetWrite::kMetadataEntry,
                                                  GroupKeyManagement::Commands::KeySetRead::kMetadataEntry,
                                                  GroupKeyManagement::Commands::KeySetRemove::kMetadataEntry,
                                                  GroupKeyManagement::Commands::KeySetReadAllIndices::kMetadataEntry,
                                              }));

    // Check required generated commands are present
    ASSERT_TRUE(IsGeneratedCommandsListEqualTo(mCluster,
                                               {
                                                   GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Id,
                                                   GroupKeyManagement::Commands::KeySetReadResponse::Id,
                                               }));
}
TEST_F(TestGroupKeyManagementCluster, AttributesTest)
{
    std::vector<app::DataModel::AttributeEntry> mandatoryAttributes(GroupKeyManagement::Attributes::kMandatoryMetadata.begin(),
                                                                    GroupKeyManagement::Attributes::kMandatoryMetadata.end());

    // There are only mandatory attributes in this cluster, so it should match the ones in Metadata exactly
    ASSERT_TRUE(chip::Testing::IsAttributesListEqualTo(mCluster, std::move(mandatoryAttributes)));
}

// Cluster should accept writing multiple group keys with the same KeySetID but different Group IDs
TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttributeSameKeySetDifferentGroup)
{
    auto keys = TestHelpers::CreateGroupKeyMapList(2, kTestFabricIndex, kTestGroupId, kTestKeySetId, 1, 0);
    PrepopulateGroupKeyMap(keys, kTestFabricIndex);
    VerifyGroupKeysMatch(kTestFabricIndex, keys);
}

// Cluster should reject a write containing duplicate keys for the same group/keyset combination.
TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttributeDuplicateKey)
{
    //  Intentionally creates two identical entries (duplicate group/keyset combination).
    // by setting increments  to 0 (groupIdIncrement = 0, keySetIdIncrement = 0)
    auto keys = TestHelpers::CreateGroupKeyMapList(2, kTestFabricIndex, kTestGroupId, kTestKeySetId, 0, 0);

    auto listToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

    CHIP_ERROR err = tester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite).GetUnderlyingError();

    ASSERT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);

    // Explicitly check that the write stops at the first duplicate entry,
    // but the valid entries processed *before* the error (i.e., keys[0]) are persisted.
    std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> expectedKeys;
    expectedKeys.push_back(keys[0]);

    VerifyGroupKeysMatch(kTestFabricIndex, expectedKeys);
}

TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttribute)
{
    auto keys = TestHelpers::CreateGroupKeyMapList(2, kTestFabricIndex);

    PrepopulateGroupKeyMap(keys, kTestFabricIndex);
    VerifyGroupKeysMatch(kTestFabricIndex, keys);
}

const chip::EndpointId kTestEndpoint1 = 10;
const chip::EndpointId kTestEndpoint2 = 11;
constexpr uint64_t kStartTimeOffset   = 100;

// spec states - group should be deleted if the endpoint member list's last member is removed
TEST_F(TestGroupKeyManagementCluster, TestGroupEndpointLifecycle)
{
    CHIP_ERROR err;

    auto doesGroupExist = [&](GroupId gid) {
        auto * iter = mRealProvider.IterateGroupInfo(kTestFabricIndex);
        if (iter == nullptr)
        {
            return false;
        }
        chip::Credentials::GroupDataProvider::GroupInfo info;
        bool found = false;
        while (iter->Next(info))
        {
            if (info.group_id == gid)
            {
                found = true;
                break;
            }
        }
        iter->Release();
        return found;
    };

    auto getEndpointCountForGroup = [&](GroupId gid) {
        auto * iter = mRealProvider.IterateEndpoints(kTestFabricIndex, gid);
        if (iter == nullptr)
        {
            return size_t(0);
        }
        size_t count = iter->Count();
        iter->Release();
        return count;
    };

    ASSERT_FALSE(doesGroupExist(kTestGroupId));
    ASSERT_EQ(getEndpointCountForGroup(kTestGroupId), 0u);

    err = mRealProvider.AddEndpoint(kTestFabricIndex, kTestGroupId, kTestEndpoint1);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ASSERT_TRUE(doesGroupExist(kTestGroupId));
    ASSERT_EQ(getEndpointCountForGroup(kTestGroupId), 1u);

    err = mRealProvider.AddEndpoint(kTestFabricIndex, kTestGroupId, kTestEndpoint2);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ASSERT_TRUE(doesGroupExist(kTestGroupId));
    ASSERT_EQ(getEndpointCountForGroup(kTestGroupId), 2u);

    err = mRealProvider.RemoveEndpoint(kTestFabricIndex, kTestGroupId, kTestEndpoint1);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ASSERT_TRUE(doesGroupExist(kTestGroupId));
    ASSERT_EQ(getEndpointCountForGroup(kTestGroupId), 1u);

    err = mRealProvider.RemoveEndpoint(kTestFabricIndex, kTestGroupId, kTestEndpoint2);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    ASSERT_FALSE(doesGroupExist(kTestGroupId));
    ASSERT_EQ(getEndpointCountForGroup(kTestGroupId), 0u);
}

const uint64_t kStartTime = 123456789;

static const chip::Credentials::GroupDataProvider::EpochKey kKey = {
    .start_time = 0, .key = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 }
};

static const chip::Credentials::GroupDataProvider::EpochKey kNewKey = {
    .start_time = 0, .key = { 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }
};

// writing a valid key set
TEST_F(TestGroupKeyManagementCluster, TestKeySetWriteCommand)
{

    GroupKeyManagement::Commands::KeySetWrite::Type requestData;

    requestData.groupKeySet = {
        .groupKeySetID          = kTestKeySetId,
        .groupKeySecurityPolicy = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst,
        .epochKey0              = chip::ByteSpan(kKey.key, sizeof(kKey.key)),
        .epochStartTime0        = kStartTime,
        .epochKey1              = DataModel::NullNullable,
        .epochStartTime1        = DataModel::NullNullable,
        .epochKey2              = DataModel::NullNullable,
        .epochStartTime2        = DataModel::NullNullable,
    };

    auto result = tester.Invoke(GroupKeyManagement::Commands::KeySetWrite::Id, requestData);
    EXPECT_TRUE(result.IsSuccess());
    Credentials::GroupDataProvider::KeySet storedKeySet;
    CHIP_ERROR err = mRealProvider.GetKeySet(kTestFabricIndex, kTestKeySetId, storedKeySet);
    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(storedKeySet.keyset_id, kTestKeySetId);
    ASSERT_EQ(storedKeySet.policy, GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst);
    ASSERT_EQ(storedKeySet.num_keys_used, 1);
    ASSERT_EQ(storedKeySet.epoch_keys[0].start_time, kStartTime);
}

// Spec states that writing a KeySet with an existing KeySetID on the same fabric should overwrite the existing one
TEST_F(TestGroupKeyManagementCluster, TestKeySetWriteSameId)
{
    GroupKeyManagement::Commands::KeySetWrite::Type requestData1;
    requestData1.groupKeySet = {
        .groupKeySetID          = kTestKeySetId,
        .groupKeySecurityPolicy = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst,
        .epochKey0              = chip::ByteSpan(kKey.key, sizeof(kKey.key)),
        .epochStartTime0        = kStartTime,
        .epochKey1              = DataModel::NullNullable,
        .epochStartTime1        = DataModel::NullNullable,
        .epochKey2              = DataModel::NullNullable,
        .epochStartTime2        = DataModel::NullNullable,
    };
    auto result1 = tester.Invoke(GroupKeyManagement::Commands::KeySetWrite::Id, requestData1);

    EXPECT_TRUE(result1.IsSuccess());
    chip::Credentials::GroupDataProvider::KeySet storedKeySet;
    CHIP_ERROR err = mRealProvider.GetKeySet(kTestFabricIndex, kTestKeySetId, storedKeySet);

    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(storedKeySet.policy, GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst);

    GroupKeyManagement::Commands::KeySetWrite::Type requestData2;
    requestData2.groupKeySet = {
        .groupKeySetID          = kTestKeySetId,
        .groupKeySecurityPolicy = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst,
        .epochKey0              = chip::ByteSpan(kNewKey.key, sizeof(kNewKey.key)),
        .epochStartTime0        = kStartTime + kStartTimeOffset,
        .epochKey1              = DataModel::NullNullable,
        .epochStartTime1        = DataModel::NullNullable,
        .epochKey2              = DataModel::NullNullable,
        .epochStartTime2        = DataModel::NullNullable,
    };

    auto result2 = tester.Invoke(GroupKeyManagement::Commands::KeySetWrite::Id, requestData2);

    EXPECT_TRUE(result2.IsSuccess());

    err = mRealProvider.GetKeySet(kTestFabricIndex, kTestKeySetId, storedKeySet);

    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(storedKeySet.policy, GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst);
    ASSERT_EQ(storedKeySet.num_keys_used, 1);
    ASSERT_EQ(storedKeySet.epoch_keys[0].start_time, kStartTime + kStartTimeOffset);
}

} // namespace
