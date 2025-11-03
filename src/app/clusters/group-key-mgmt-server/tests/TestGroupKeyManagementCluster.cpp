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
#include <app/clusters/testing/TestReadWriteAttribute.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/GroupKeyManagement/ClusterId.h>
#include <clusters/GroupKeyManagement/Enums.h>
#include <clusters/GroupKeyManagement/Ids.h>
#include <clusters/GroupKeyManagement/Metadata.h>
#include <clusters/GroupKeyManagement/Structs.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::Test;
namespace {

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
    ReadOnlyBufferBuilder<chip::CommandId> generatedCommandsBuilder;
    ASSERT_EQ(cluster.GeneratedCommands(groupKeyManagementPath, generatedCommandsBuilder), CHIP_NO_ERROR);
    ReadOnlyBuffer<chip::CommandId> generatedCommands = generatedCommandsBuilder.TakeBuffer();

    ASSERT_EQ(generatedCommands.size(), GroupKeyManagement::Commands::kGeneratedCommandsCount);
    ASSERT_EQ(generatedCommands[0], GroupKeyManagement::Commands::KeySetReadAllIndicesResponse::Id);
    ASSERT_EQ(generatedCommands[1], GroupKeyManagement::Commands::KeySetReadResponse::Id);
}

TEST_F(TestGroupKeyManagementCluster, AttributesTest)
{
    GroupKeyManagementCluster cluster;
    ConcreteClusterPath groupKeyManagementPath = ConcreteClusterPath(kRootEndpointId, GroupKeyManagement::Id);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributesBuilder;
    ASSERT_EQ(cluster.Attributes(groupKeyManagementPath, attributesBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.ReferenceExisting(DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

    // There are only mandatory attributes in this cluster, so it should match the ones in Metadata exactly
    ASSERT_EQ(expectedBuilder.AppendElements(GroupKeyManagement::Attributes::kMandatoryMetadata), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

class MockSessionKeystore : public chip::Crypto::SessionKeystore
{
public:
    using P256ECDHDerivedSecret        = chip::Crypto::P256ECDHDerivedSecret;
    using Symmetric128BitsKeyByteArray = chip::Crypto::Symmetric128BitsKeyByteArray;
    using Aes128KeyHandle              = chip::Crypto::Aes128KeyHandle;
    using Hmac128KeyHandle             = chip::Crypto::Hmac128KeyHandle;
    using HkdfKeyHandle                = chip::Crypto::HkdfKeyHandle;
    using Symmetric128BitsKeyHandle    = chip::Crypto::Symmetric128BitsKeyHandle;
    using AttestationChallenge         = chip::Crypto::AttestationChallenge;

    CHIP_ERROR CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Aes128KeyHandle & key) override { return CHIP_NO_ERROR; }
    CHIP_ERROR CreateKey(const Symmetric128BitsKeyByteArray & keyMaterial, Hmac128KeyHandle & key) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CreateKey(const ByteSpan & keyMaterial, HkdfKeyHandle & key) override { return CHIP_NO_ERROR; }
    void DestroyKey(Symmetric128BitsKeyHandle & key) override {}
    void DestroyKey(HkdfKeyHandle & key) override {}

    CHIP_ERROR DeriveKey(const P256ECDHDerivedSecret & secret, const ByteSpan & salt, const ByteSpan & info,
                         Aes128KeyHandle & key) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR DeriveSessionKeys(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info, Aes128KeyHandle & i2rKey,
                                 Aes128KeyHandle & r2iKey, AttestationChallenge & attestationChallenge) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR DeriveSessionKeys(const HkdfKeyHandle & secretKey, const ByteSpan & salt, const ByteSpan & info,
                                 Aes128KeyHandle & i2rKey, Aes128KeyHandle & r2iKey,
                                 AttestationChallenge & attestationChallenge) override
    {
        return CHIP_NO_ERROR;
    }
};
namespace TestHelpers {

GroupKeyManagement::Structs::GroupKeyMapStruct::Type CreateKey(chip::GroupId groupId, uint16_t keySetId,
                                                               chip::FabricIndex fabricIndex)
{
    GroupKeyManagement::Structs::GroupKeyMapStruct::Type key;
    key.groupId       = groupId;
    key.groupKeySetID = keySetId;
    key.fabricIndex   = fabricIndex;
    return key;
}
std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type>
CreateGroupKeyMapList(size_t count, chip::FabricIndex fabricIndex, chip::GroupId startGroupId = 0x1001, uint16_t startKeySetId = 1,
                      uint16_t groupIdIncrement = 1, uint16_t keySetIdIncrement = 1)
{
    std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> list;
    for (size_t i = 0; i < count; ++i)
    {
        chip::GroupId currentGroupId = static_cast<chip::GroupId>(startGroupId + (i * groupIdIncrement));
        uint16_t currentKeySetId     = static_cast<uint16_t>(startKeySetId + (i * keySetIdIncrement));
        list.push_back(CreateKey(currentGroupId, currentKeySetId, fabricIndex));
    }
    return list;
}

} // namespace TestHelpers

struct TestGroupKeyManagementClusterWithStorage : public TestGroupKeyManagementCluster
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    chip::Test::TestServerClusterContext mTestStack;
    chip::Credentials::GroupDataProviderImpl mRealProvider;
    MockSessionKeystore mMockKeystore;
    GroupKeyManagementCluster mCluster;

    // This context object is non-assignable
    decltype(mTestStack.Create()) mContext;

    // We initialize mContext here
    TestGroupKeyManagementClusterWithStorage() : mContext(mTestStack.Create()) {}

    void SetUp() override
    {
        auto * storage = &mTestStack.StorageDelegate();
        ASSERT_NE(storage, nullptr);

        mRealProvider.SetStorageDelegate(storage);
        mRealProvider.SetSessionKeystore(&mMockKeystore);
        ASSERT_EQ(mRealProvider.Init(), CHIP_NO_ERROR);

        chip::Credentials::SetGroupDataProvider(&mRealProvider);

        ASSERT_EQ(mCluster.Startup(mContext), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mCluster.Shutdown();
        chip::Credentials::SetGroupDataProvider(nullptr);
        mRealProvider.Finish();
    }

    void PrepopulateGroupKeyMap(const std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> & keys)
    {
        auto listToWrite =
            chip::app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

        auto path = chip::app::ConcreteAttributePath(chip::kRootEndpointId, GroupKeyManagement::Id,
                                                     GroupKeyManagement::Attributes::GroupKeyMap::Id);

        CHIP_ERROR err = chip::Test::WriteClusterAttribute(mCluster, path, listToWrite);
        ASSERT_EQ(err, CHIP_NO_ERROR);
    }

    void VerifyGroupKeysMatch(const chip::FabricIndex fabricIndex,
                              const std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> & expectedKeys)
    {
        chip::Credentials::GroupDataProvider::GroupKeyIterator * iterator = mRealProvider.IterateGroupKeys(fabricIndex);
        ASSERT_NE(iterator, nullptr);
        ASSERT_EQ(iterator->Count(), expectedKeys.size());

        size_t i = 0;
        chip::Credentials::GroupDataProvider::GroupKey storedKey;
        while (iterator->Next(storedKey))
        {
            ASSERT_LT(i, expectedKeys.size());
            if (i < expectedKeys.size())
            {
                EXPECT_EQ(storedKey.group_id, expectedKeys[i].groupId);
                EXPECT_EQ(storedKey.keyset_id, expectedKeys[i].groupKeySetID);
            }
            i++;
        }

        ASSERT_EQ(i, expectedKeys.size());
        iterator->Release();
    }
};
// Cluster should accept writing multiple group keys with the same KeySetID but different Group IDs
TEST_F(TestGroupKeyManagementClusterWithStorage, TestWriteGroupKeyMapAttributeSameKeySetDifferentGroup)
{
    const chip::FabricIndex fabricIndex = chip::app::Testing::kTestFabrixIndex;
    chip::GroupId startGroupId          = 0x1001;
    uint16_t sharedKeySetId             = 5;

    auto keys = TestHelpers::CreateGroupKeyMapList(2, fabricIndex, startGroupId, sharedKeySetId, 0 /* keySetIdIncrement */);

    PrepopulateGroupKeyMap(keys);
    VerifyGroupKeysMatch(fabricIndex, keys);
}

// Cluster should reject a write containing duplicate keys for the same group/keyset combination.
TEST_F(TestGroupKeyManagementClusterWithStorage, TestWriteGroupKeyMapAttributeDuplicateKey)
{
    const chip::FabricIndex fabricIndex = chip::app::Testing::kTestFabrixIndex;
    chip::GroupId groupId               = 100;
    uint16_t keysetId                   = 5;

    auto keys = TestHelpers::CreateGroupKeyMapList(2, fabricIndex, groupId, keysetId, 0, 0);

    auto listToWrite =
        chip::app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());
    auto path = chip::app::ConcreteAttributePath(chip::kRootEndpointId, GroupKeyManagement::Id,
                                                 GroupKeyManagement::Attributes::GroupKeyMap::Id);

    CHIP_ERROR err = chip::Test::WriteClusterAttribute(mCluster, path, listToWrite);

    ASSERT_EQ(err, CHIP_ERROR_DUPLICATE_KEY_ID);

    std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> expectedKeys;
    expectedKeys.push_back(keys[0]);

    VerifyGroupKeysMatch(fabricIndex, expectedKeys);
}

TEST_F(TestGroupKeyManagementClusterWithStorage, TestWriteGroupKeyMapAttribute)
{
    const chip::FabricIndex fabricIndex = chip::app::Testing::kTestFabrixIndex;
    auto keys                           = TestHelpers::CreateGroupKeyMapList(2, fabricIndex);

    PrepopulateGroupKeyMap(keys);
    VerifyGroupKeysMatch(fabricIndex, keys);
}
} // namespace
