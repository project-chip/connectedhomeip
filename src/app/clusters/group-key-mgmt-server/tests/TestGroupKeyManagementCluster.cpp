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
#include <clusters/GroupKeyManagement/Enums.h>
#include <clusters/GroupKeyManagement/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/DiagnosticDataProvider.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;

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
    ASSERT_TRUE(Testing::EqualAttributeSets(attributesBuilder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

// Provider that drives the GroupTable read into the endpoint-encoding failure path and makes a
// leaked EndpointIterator observable on any pool backend (the host build uses the heap pool, which
// never exhausts, so the leak cannot be detected by watching IterateEndpoints return nullptr).
class LeakDetectingGroupDataProvider : public Credentials::GroupDataProviderImpl
{
public:
    int liveEndpointIterators = 0;

    // The GroupTable read loops over groups; without at least one group it never builds a
    // GroupTableCodec and the endpoint-encoding path under test is never reached. Yield exactly one.
    GroupInfoIterator * IterateGroupInfo(FabricIndex fabric_index) override { return new SingleGroupInfoIterator(); }

    // Return the counting/flooding iterator instead of the real pool-backed one. This is the hook
    // that both forces the failure (flood) and observes the leak (counter).
    EndpointIterator * IterateEndpoints(FabricIndex fabric_index, std::optional<GroupId> group_id = std::nullopt) override
    {
        return new FloodEndpointIterator(*this);
    }

private:
    // Minimal GroupInfoIterator that yields a single group then stops.
    class SingleGroupInfoIterator : public GroupInfoIterator
    {
    public:
        size_t Count() override { return 1; }
        bool Next(GroupInfo & item) override
        {
            if (mDone)
            {
                return false;
            }
            mDone         = true;
            item.group_id = 0x0001;
            item.SetName("G"_span);
            return true;
        }
        void Release() override { delete this; }

    private:
        bool mDone = false;
    };

    // EndpointIterator that (a) emits far more endpoints than fit in the encode buffer, so
    // writer.Put() fails partway through the endpoints array, and (b) bumps the provider's live
    // counter on construction / down on Release so a skipped Release is observable.
    class FloodEndpointIterator : public EndpointIterator
    {
    public:
        explicit FloodEndpointIterator(LeakDetectingGroupDataProvider & provider) : mProvider(provider)
        {
            ++mProvider.liveEndpointIterators;
        }
        size_t Count() override { return kFloodCount; }
        bool Next(GroupEndpoint & item) override
        {
            if (mEmitted >= kFloodCount)
            {
                return false;
            }
            item.group_id    = 0x0001;
            item.endpoint_id = static_cast<EndpointId>(mEmitted++);
            return true;
        }
        void Release() override
        {
            --mProvider.liveEndpointIterators;
            delete this;
        }

    private:
        // Far exceeds the encoder buffer so writer.Put() is guaranteed to fail inside the endpoints array.
        static constexpr size_t kFloodCount = 1000;
        LeakDetectingGroupDataProvider & mProvider;
        size_t mEmitted = 0;
    };
};

TEST_F(TestGroupKeyManagementCluster, TestGroupTableReadReleasesEndpointIteratorOnEncodeFailure)
{
    LeakDetectingGroupDataProvider leakProvider;

    auto * storage = &mTestContext.StorageDelegate();
    leakProvider.SetStorageDelegate(storage);
    leakProvider.SetSessionKeystore(&mMockKeystore);
    ASSERT_EQ(leakProvider.Init(), CHIP_NO_ERROR);
    Credentials::SetGroupDataProvider(&leakProvider);

    GroupKeyManagementCluster leakCluster{ { fabricHelper.GetFabricTable(), leakProvider } };
    ASSERT_EQ(leakCluster.Startup(mTestContext.Get()), CHIP_NO_ERROR);

    ClusterTester leakTester{ leakCluster };
    leakTester.SetFabricIndex(kTestFabricIndex);

    uint32_t sink = 0;
    (void) leakTester.ReadAttribute(GroupKeyManagement::Attributes::GroupTable::Id, sink);

    // The endpoint iterator was released despite the encode failing.
    // Fails (==1) without the fix, passes (==0) with it.
    EXPECT_EQ(leakProvider.liveEndpointIterators, 0);

    leakCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
