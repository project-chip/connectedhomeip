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

#include <app/clusters/group-key-mgmt-server/group-key-mgmt-cluster.h>

#include <app/clusters/testing/AttributeTesting.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/GroupKeyManagement/ClusterId.h>
#include <clusters/GroupKeyManagement/Enums.h>
#include <clusters/GroupKeyManagement/Ids.h>
#include <clusters/GroupKeyManagement/Metadata.h>
#include <clusters/GroupKeyManagement/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <app/data-model-provider/tests/WriteTesting.h>

#include <lib/core/TLVReader.h>

#include <credentials/GroupDataProvider.h>

#include <app/server-cluster/testing/TestServerClusterContext.h>

// #include <app/data-model-provider/tests/ReadTesting.h>
// #include <lib/core/TLVWriter.h>
// #include <app/clusters/testing/AttributeTesting.h>
// #include <app/data-model/List.h>
// #include <lib/support/Span.h>
// #include <lib/support/logging/CHIPLogging.h>
//  #include <app/AttributeValueDecoder.h>
// #include <app/AttributeValueEncoder.h>
// #include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

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

namespace TestHelpers {

template <typename ClusterT, typename T>
CHIP_ERROR WriteClusterAttribute(ClusterT & cluster, const chip::app::ConcreteAttributePath & path, const T & listToWrite)
{
    chip::app::Testing::WriteOperation writeOperation(path);
    writeOperation.SetSubjectDescriptor(chip::app::Testing::kAdminSubjectDescriptor);
    // 1. Manually encode the list to a TLV buffer.
    uint8_t tlvBuffer[1024];
    chip::TLV::TLVWriter writer;
    writer.Init(tlvBuffer);

    chip::TLV::TLVType outerContainer;
    CHIP_ERROR err = writer.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Array, outerContainer);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    for (const auto & item : listToWrite)
    {
        err = item.EncodeForWrite(writer, chip::TLV::AnonymousTag());
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }

    err = writer.EndContainer(outerContainer);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    // 2. Create a TLV reader from the buffer.
    chip::TLV::TLVReader reader;
    reader.Init(tlvBuffer, writer.GetLengthWritten());
    ReturnErrorOnFailure(reader.Next()); // Position reader on the array

    // 3. FIX: Create the decoder with BOTH the reader AND the SubjectDescriptor.
    // The SubjectDescriptor provides the security context (like the fabric index) for the write.
    chip::app::AttributeValueDecoder decoder(reader, *writeOperation.GetRequest().subjectDescriptor);

    // 4. Pass the correctly constructed decoder to the cluster.
    return cluster.WriteAttribute(writeOperation.GetRequest(), decoder).GetUnderlyingError();
}

} // namespace TestHelpers

class FakeGroupDataProvider : public chip::Credentials::GroupDataProvider
{
public:
    std::map<chip::FabricIndex, std::vector<GroupKey>> mFabricKeys;
    FakeGroupDataProvider() : GroupDataProvider(5, 10) {}

    class VectorIterator : public GroupKeyIterator
    {
    public:
        VectorIterator(const std::vector<GroupKey> & vec) : mVector(vec) {}
        size_t Count() override { return mVector.size(); }
        bool Next(GroupKey & item) override
        {
            if (mIndex >= mVector.size())
            {
                return false;
            }
            item = mVector[mIndex++];
            return true;
        }
        void Release() override { delete this; }

    private:
        const std::vector<GroupKey> & mVector;
        size_t mIndex = 0;
    };

    CHIP_ERROR SetGroupKeyAt(chip::FabricIndex fabric_index, size_t index, const GroupKey & groupKey) override
    {
        auto & keyList = mFabricKeys[fabric_index];
        if (index > keyList.size())
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        if (index == keyList.size())
        {
            keyList.push_back(groupKey); // Append
        }
        else
        {
            keyList[index] = groupKey; // Replace existing
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetGroupKeyAt(chip::FabricIndex fabric_index, size_t index, GroupKey & groupKey) override
    {
        if (mFabricKeys.count(fabric_index) == 0 || index >= mFabricKeys[fabric_index].size())
        {
            return CHIP_ERROR_NOT_FOUND;
        }
        groupKey = mFabricKeys[fabric_index][index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveGroupKeys(chip::FabricIndex fabric_index) override
    {
        if (mFabricKeys.count(fabric_index) > 0)
        {
            mFabricKeys.at(fabric_index).clear();
        }
        return CHIP_NO_ERROR;
    }

    GroupKeyIterator * IterateGroupKeys(chip::FabricIndex fabric_index) override
    {
        if (mFabricKeys.count(fabric_index) == 0)
        {
            // Return an iterator for an empty vector to avoid creating an entry in the map
            static const std::vector<GroupKey> kEmpty;
            return new VectorIterator(kEmpty);
        }
        return new VectorIterator(mFabricKeys.at(fabric_index));
    }

    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    void Finish() override {}
    CHIP_ERROR SetGroupInfo(chip::FabricIndex, const GroupInfo &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetGroupInfo(chip::FabricIndex, chip::GroupId, GroupInfo &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveGroupInfo(chip::FabricIndex, chip::GroupId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetGroupInfoAt(chip::FabricIndex, size_t, const GroupInfo &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetGroupInfoAt(chip::FabricIndex, size_t, GroupInfo &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveGroupInfoAt(chip::FabricIndex, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    bool HasEndpoint(chip::FabricIndex, chip::GroupId, chip::EndpointId) override { return false; }
    CHIP_ERROR AddEndpoint(chip::FabricIndex, chip::GroupId, chip::EndpointId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveEndpoint(chip::FabricIndex, chip::GroupId, chip::EndpointId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveEndpoint(chip::FabricIndex, chip::EndpointId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    GroupInfoIterator * IterateGroupInfo(chip::FabricIndex) override { return nullptr; }
    EndpointIterator * IterateEndpoints(chip::FabricIndex, std::optional<chip::GroupId> = std::nullopt) override { return nullptr; }
    CHIP_ERROR RemoveGroupKeyAt(chip::FabricIndex, size_t) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetKeySet(chip::FabricIndex, const chip::ByteSpan &, const KeySet &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetKeySet(chip::FabricIndex, chip::KeysetId, KeySet &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR RemoveKeySet(chip::FabricIndex, chip::KeysetId) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR GetIpkKeySet(chip::FabricIndex, KeySet &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    KeySetIterator * IterateKeySets(chip::FabricIndex) override { return nullptr; }
    CHIP_ERROR RemoveFabric(chip::FabricIndex) override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    GroupSessionIterator * IterateGroupSessions(uint16_t) override { return nullptr; }
    chip::Crypto::SymmetricKeyContext * GetKeyContext(chip::FabricIndex, chip::GroupId) override { return nullptr; }
};

struct ClusterTestContext
{
    ::chip::app::Clusters::GroupKeyManagementCluster & mCluster;
    ClusterTestContext(::chip::app::Clusters::GroupKeyManagementCluster & cluster, FakeGroupDataProvider & provider) :
        mCluster(cluster)
    {
        ::chip::Credentials::SetGroupDataProvider(&provider);
    }
    ~ClusterTestContext()
    {
        mCluster.Shutdown();
        ::chip::Credentials::SetGroupDataProvider(nullptr);
    }
};

TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttribute)
{
    using namespace ::chip;
    using namespace ::chip::app;
    using namespace ::chip::app::Clusters;

    ::chip::Test::TestServerClusterContext testStack;
    FakeGroupDataProvider fakeProvider;
    GroupKeyManagementCluster cluster;
    ClusterTestContext clusterContext(cluster, fakeProvider);

    auto context = testStack.Create();
    ASSERT_EQ(cluster.Startup(context), CHIP_NO_ERROR);

    const FabricIndex fabricIndex = chip::app::Testing::kTestFabrixIndex;

    GroupKeyManagement::Structs::GroupKeyMapStruct::Type key1;
    key1.groupId       = 0x1001;
    key1.groupKeySetID = 1;
    key1.fabricIndex   = fabricIndex;

    GroupKeyManagement::Structs::GroupKeyMapStruct::Type key2;
    key2.groupId       = 0x1002;
    key2.groupKeySetID = 2;
    key2.fabricIndex   = fabricIndex;

    const GroupKeyManagement::Structs::GroupKeyMapStruct::Type items[] = { key1, key2 };
    auto listToWrite = DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(items);
    auto path = ConcreteAttributePath(kRootEndpointId, GroupKeyManagement::Id, GroupKeyManagement::Attributes::GroupKeyMap::Id);

    CHIP_ERROR err = TestHelpers::WriteClusterAttribute(cluster, path, listToWrite);

    ASSERT_EQ(err, CHIP_NO_ERROR);
    ASSERT_EQ(fakeProvider.mFabricKeys.count(fabricIndex), static_cast<size_t>(1));
    ASSERT_EQ(fakeProvider.mFabricKeys[fabricIndex].size(), static_cast<size_t>(2));

    const auto & storedKeys = fakeProvider.mFabricKeys[fabricIndex];
    EXPECT_EQ(storedKeys[0].group_id, key1.groupId);
    EXPECT_EQ(storedKeys[0].keyset_id, key1.groupKeySetID);
    EXPECT_EQ(storedKeys[1].group_id, key2.groupId);
    EXPECT_EQ(storedKeys[1].keyset_id, key2.groupKeySetID);
}

} // namespace
