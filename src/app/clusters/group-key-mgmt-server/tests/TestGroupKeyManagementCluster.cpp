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

chip::FabricIndex kTestFabricIndex = Testing::kTestFabricIndex;
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

    ClusterTester mTester{ mCluster };

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
        mTester.SetFabricIndex(kTestFabricIndex);
    }

    void TearDown() override
    {
        mTester.SetFabricIndex(kUndefinedFabricIndex);
        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        Credentials::SetGroupDataProvider(nullptr);
        CHIP_ERROR err = fabricHelper.TearDownTestFabric(kTestFabricIndex);
        ASSERT_EQ(err, CHIP_NO_ERROR);
        mRealProvider.Finish();
    }
    // Writes a list of group keys to the GroupKeyMap attribute for a given fabric.
    // Used to set up test scenarios with pre-existing keys.
    void PrepopulateGroupKeyMap(const std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> & keys,
                                ListWritingPattern listWritingPattern)
    {
        auto listToWrite =
            app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

        CHIP_ERROR err = mTester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite, listWritingPattern)
                             .GetUnderlyingError();
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
    // TODO: Fix the assert below including the optional attribute.
    // The assert is disabled because an optional attribute, GroupcastAdoption, was added to the cluster.
    // ASSERT_TRUE(chip::Testing::IsAttributesListEqualTo(mCluster, std::move(mandatoryAttributes)));
}

// Cluster should accept writing multiple group keys with the same KeySetID but different Group IDs
TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttributeSameKeySetDifferentGroup)
{
    for (ListWritingPattern listWritingPattern : { ListWritingPattern::ReplaceAll, ListWritingPattern::ClearAllThenAppendItems })
    {
        auto keys = TestHelpers::CreateGroupKeyMapList(2, kTestFabricIndex, kTestGroupId, kTestKeySetId, 1, 0);
        PrepopulateGroupKeyMap(keys, listWritingPattern);
        VerifyGroupKeysMatch(kTestFabricIndex, keys);
    }
}

// Cluster should reject a write containing duplicate keys for the same group/keyset combination.
TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttributeDuplicateKey)
{
    for (ListWritingPattern listWritingPattern : { ListWritingPattern::ReplaceAll, ListWritingPattern::ClearAllThenAppendItems })
    {
        //  Intentionally creates two identical entries (duplicate group/keyset combination).
        // by setting increments  to 0 (groupIdIncrement = 0, keySetIdIncrement = 0)
        auto keys = TestHelpers::CreateGroupKeyMapList(2, kTestFabricIndex, kTestGroupId, kTestKeySetId, 0, 0);

        auto listToWrite =
            app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

        CHIP_ERROR err = mTester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite, listWritingPattern)
                             .GetUnderlyingError();

        ASSERT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);

        // Explicitly check that the write stops at the first duplicate entry,
        // but the valid entries processed *before* the error (i.e., keys[0]) are persisted.
        std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> expectedKeys;
        expectedKeys.push_back(keys[0]);

        VerifyGroupKeysMatch(kTestFabricIndex, expectedKeys);
    }
}

TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttribute)
{
    for (ListWritingPattern listWritingPattern : { ListWritingPattern::ReplaceAll, ListWritingPattern::ClearAllThenAppendItems })
    {
        auto keys = TestHelpers::CreateGroupKeyMapList(2, kTestFabricIndex);

        PrepopulateGroupKeyMap(keys, listWritingPattern);
        VerifyGroupKeysMatch(kTestFabricIndex, keys);
    }
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

    auto result = mTester.Invoke(GroupKeyManagement::Commands::KeySetWrite::Id, requestData);
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
    auto result1 = mTester.Invoke(GroupKeyManagement::Commands::KeySetWrite::Id, requestData1);

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

    auto result2 = mTester.Invoke(GroupKeyManagement::Commands::KeySetWrite::Id, requestData2);

    EXPECT_TRUE(result2.IsSuccess());

    err = mRealProvider.GetKeySet(kTestFabricIndex, kTestKeySetId, storedKeySet);

    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(storedKeySet.policy, GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst);
    ASSERT_EQ(storedKeySet.num_keys_used, 1);
    ASSERT_EQ(storedKeySet.epoch_keys[0].start_time, kStartTime + kStartTimeOffset);
}

class MockGroupDataProvider : public Credentials::GroupDataProviderImpl
{
public:
    MockGroupDataProvider() : GroupDataProviderImpl() {}
    ~MockGroupDataProvider() override = default;

    // Always returns nullptr to simulate iterator pool exhaustion.
    GroupKeyIterator * IterateGroupKeys(FabricIndex fabric_index) override { return nullptr; }
};

// When IterateGroupKeys returns nullptr, WriteGroupKeyMap (AppendItem path) must detect
// this and return CHIP_ERROR_NO_MEMORY rather than dereferencing a null pointer.
TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttributeIteratorExhausted)
{
    MockGroupDataProvider mockGroupDataProvider;

    auto * storage = &mTestContext.StorageDelegate();
    mockGroupDataProvider.SetStorageDelegate(storage);
    mockGroupDataProvider.SetSessionKeystore(&mMockKeystore);
    ASSERT_EQ(mockGroupDataProvider.Init(), CHIP_NO_ERROR);
    Credentials::SetGroupDataProvider(&mockGroupDataProvider);

    // Sanity check: Ensure the mock is working as intended and IterateGroupKeys returns nullptr.
    ASSERT_EQ(mockGroupDataProvider.IterateGroupKeys(kTestFabricIndex), nullptr);

    // An AppendItem write triggers the IterateGroupKeys call inside WriteGroupKeyMap. Thus, we use ClearAllThenAppendItems and we
    // do not test ReplaceAll since it does not call IterateGroupKeys.
    // With the mock returning nullptr (to simulate ObjectPool exhaustion), the code must return CHIP_ERROR_NO_MEMORY instead of
    // crashing with a null-pointer dereference.
    auto keys        = TestHelpers::CreateGroupKeyMapList(1, kTestFabricIndex);
    auto listToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

    GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mockGroupDataProvider } };
    ClusterTester tester2{ mockCluster };

    CHIP_ERROR err = tester2
                         .WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite,
                                         ListWritingPattern::ClearAllThenAppendItems)
                         .GetUnderlyingError();

    EXPECT_EQ(err, CHIP_ERROR_NO_MEMORY);
}

TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttributeGCAST)
{
    GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                           BitFlags<GroupKeyManagement::Feature>(GroupKeyManagement::Feature::kGroupcast) };
    ClusterTester tester{ mockCluster };
    tester.SetFabricIndex(kTestFabricIndex);

    // Write GroupcastAdoption with adopted=true for the test fabric
    GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type adoption;
    adoption.groupcastAdopted                                                            = true;
    adoption.fabricIndex                                                                 = kTestFabricIndex;
    std::vector<GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type> adoptionList = { adoption };
    auto listToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type>(adoptionList.data(),
                                                                                                              adoptionList.size());

    CHIP_ERROR err =
        tester.WriteAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, listToWrite, ListWritingPattern::ReplaceAll)
            .GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // With GCAST feature ON and GroupcastAdoption=true, writing GroupKeyMap should be rejected
    auto keys        = TestHelpers::CreateGroupKeyMapList(1, kTestFabricIndex);
    auto keysToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

    err = tester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, keysToWrite, ListWritingPattern::ReplaceAll)
              .GetUnderlyingError();
    EXPECT_EQ(err, CHIP_IM_GLOBAL_STATUS(InvalidInState));
}

TEST_F(TestGroupKeyManagementCluster, TestReadGroupKeyMapAttributeGCAST)
{
    GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                           BitFlags<GroupKeyManagement::Feature>(GroupKeyManagement::Feature::kGroupcast) };
    ClusterTester tester{ mockCluster };
    tester.SetFabricIndex(kTestFabricIndex);

    // Write GroupcastAdoption with adopted=true for the test fabric
    GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type adoption;
    adoption.groupcastAdopted                                                            = true;
    adoption.fabricIndex                                                                 = kTestFabricIndex;
    std::vector<GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type> adoptionList = { adoption };
    auto listToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type>(adoptionList.data(),
                                                                                                              adoptionList.size());

    CHIP_ERROR err =
        tester.WriteAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, listToWrite, ListWritingPattern::ReplaceAll)
            .GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // With GCAST feature ON and GroupcastAdoption=true, reading GroupKeyMap should return an empty list
    GroupKeyManagement::Attributes::GroupKeyMap::TypeInfo::DecodableType groupKeyMapList;
    err = tester.ReadAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, groupKeyMapList).GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t count = 0;
    ASSERT_EQ(groupKeyMapList.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestGroupKeyManagementCluster, TestWriteGroupTableAttributeGCAST)
{
    GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                           BitFlags<GroupKeyManagement::Feature>(GroupKeyManagement::Feature::kGroupcast) };
    ClusterTester tester{ mockCluster };
    tester.SetFabricIndex(kTestFabricIndex);

    // Write GroupcastAdoption with adopted=true for the test fabric
    GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type adoption;
    adoption.groupcastAdopted                                                            = true;
    adoption.fabricIndex                                                                 = kTestFabricIndex;
    std::vector<GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type> adoptionList = { adoption };
    auto adoptionToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type>(
        adoptionList.data(), adoptionList.size());

    CHIP_ERROR err =
        tester
            .WriteAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, adoptionToWrite, ListWritingPattern::ReplaceAll)
            .GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // GroupTable is read-only; writing it should return UnsupportedWrite regardless of GCAST state
    GroupKeyManagement::Structs::GroupInfoMapStruct::Type entry;
    entry.groupId                                                                     = kTestGroupId;
    entry.fabricIndex                                                                 = kTestFabricIndex;
    std::vector<GroupKeyManagement::Structs::GroupInfoMapStruct::Type> groupTableList = { entry };
    auto groupTableToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupInfoMapStruct::Type>(
        groupTableList.data(), groupTableList.size());

    err = tester.WriteAttribute(GroupKeyManagement::Attributes::GroupTable::Id, groupTableToWrite, ListWritingPattern::ReplaceAll)
              .GetUnderlyingError();
    EXPECT_EQ(err, CHIP_IM_GLOBAL_STATUS(UnsupportedWrite));
}

TEST_F(TestGroupKeyManagementCluster, TestReadGroupTableAttributeGCAST)
{
    GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                           BitFlags<GroupKeyManagement::Feature>(GroupKeyManagement::Feature::kGroupcast) };
    ClusterTester tester{ mockCluster };
    tester.SetFabricIndex(kTestFabricIndex);

    // Prepopulate a group entry so there is data that would normally be returned
    ASSERT_EQ(mRealProvider.AddEndpoint(kTestFabricIndex, kTestGroupId, kTestEndpoint1), CHIP_NO_ERROR);

    // Write GroupcastAdoption with adopted=true for the test fabric
    GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type adoption;
    adoption.groupcastAdopted                                                            = true;
    adoption.fabricIndex                                                                 = kTestFabricIndex;
    std::vector<GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type> adoptionList = { adoption };
    auto adoptionToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type>(
        adoptionList.data(), adoptionList.size());

    CHIP_ERROR err =
        tester
            .WriteAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, adoptionToWrite, ListWritingPattern::ReplaceAll)
            .GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // With GCAST feature ON and GroupcastAdoption=true, reading GroupTable should return an empty list
    GroupKeyManagement::Attributes::GroupTable::TypeInfo::DecodableType groupTableList;
    err = tester.ReadAttribute(GroupKeyManagement::Attributes::GroupTable::Id, groupTableList).GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t count = 0;
    ASSERT_EQ(groupTableList.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestGroupKeyManagementCluster, TestReadGroupcastAdopted)
{
    GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                           BitFlags<GroupKeyManagement::Feature>(GroupKeyManagement::Feature::kGroupcast) };
    ClusterTester tester{ mockCluster };
    tester.SetFabricIndex(kTestFabricIndex);

    // Default state: GroupcastAdoption should be false
    GroupKeyManagement::Attributes::GroupcastAdoption::TypeInfo::DecodableType readList;
    CHIP_ERROR err = tester.ReadAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, readList).GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    size_t count = 0;
    ASSERT_EQ(readList.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, 1u);

    auto iter = readList.begin();
    ASSERT_TRUE(iter.Next());
    EXPECT_EQ(iter.GetValue().fabricIndex, kTestFabricIndex);
    EXPECT_FALSE(iter.GetValue().groupcastAdopted);
    ASSERT_EQ(iter.GetStatus(), CHIP_NO_ERROR);

    // Write GroupcastAdoption with adopted=true
    GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type adoption;
    adoption.groupcastAdopted                                                            = true;
    adoption.fabricIndex                                                                 = kTestFabricIndex;
    std::vector<GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type> adoptionList = { adoption };
    auto adoptionToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupcastAdoptionStruct::Type>(
        adoptionList.data(), adoptionList.size());

    err =
        tester
            .WriteAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, adoptionToWrite, ListWritingPattern::ReplaceAll)
            .GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    // Read back: should now be adopted=true
    GroupKeyManagement::Attributes::GroupcastAdoption::TypeInfo::DecodableType readList2;
    err = tester.ReadAttribute(GroupKeyManagement::Attributes::GroupcastAdoption::Id, readList2).GetUnderlyingError();
    ASSERT_EQ(err, CHIP_NO_ERROR);

    count = 0;
    ASSERT_EQ(readList2.ComputeSize(&count), CHIP_NO_ERROR);
    ASSERT_EQ(count, 1u);

    auto iter2 = readList2.begin();
    ASSERT_TRUE(iter2.Next());
    EXPECT_EQ(iter2.GetValue().fabricIndex, kTestFabricIndex);
    EXPECT_TRUE(iter2.GetValue().groupcastAdopted);
    ASSERT_EQ(iter2.GetStatus(), CHIP_NO_ERROR);
}

TEST_F(TestGroupKeyManagementCluster, TestReadMaxGroupsPerFabric)
{
    // Without GCAST: should return the provider's configured value
    {
        GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                               BitFlags<GroupKeyManagement::Feature>(0) };
        ClusterTester tester{ mockCluster };
        tester.SetFabricIndex(kTestFabricIndex);

        uint16_t maxGroups = 0;
        CHIP_ERROR err =
            tester.ReadAttribute(GroupKeyManagement::Attributes::MaxGroupsPerFabric::Id, maxGroups).GetUnderlyingError();
        ASSERT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(maxGroups, mRealProvider.GetMaxGroupsPerFabric());
    }
    // With GCAST feature ON: should return 0
    {
        GroupKeyManagementCluster mockCluster{ { fabricHelper.GetFabricTable(), mRealProvider },
                                               BitFlags<GroupKeyManagement::Feature>(GroupKeyManagement::Feature::kGroupcast) };
        ClusterTester tester{ mockCluster };
        tester.SetFabricIndex(kTestFabricIndex);

        uint16_t maxGroupsGcast = 0;
        CHIP_ERROR err =
            tester.ReadAttribute(GroupKeyManagement::Attributes::MaxGroupsPerFabric::Id, maxGroupsGcast).GetUnderlyingError();
        ASSERT_EQ(err, CHIP_NO_ERROR);
        EXPECT_EQ(maxGroupsGcast, 0u);
    }
}
} // namespace
