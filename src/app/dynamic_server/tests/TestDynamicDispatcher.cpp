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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeValueEncoder.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/dynamic_server/AccessControl.h>
#include <app/dynamic_server/DynamicDispatcher.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/util/attribute-storage-detail.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>
#include <lib/support/CodeUtils.h>
#include <pw_unit_test/framework.h>

#include <utility>
#include <vector>

// Dynamic endpoints do not require generated cluster initialization.
void InitDataModelHandler() {}

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::Testing;

class TestCluster : public DefaultServerCluster
{
public:
    using DefaultServerCluster::DefaultServerCluster;

    ActionReturnStatus ReadAttribute(const ReadAttributeRequest & request, AttributeValueEncoder & encoder) override
    {
        return encoder.Encode<uint32_t>(42);
    }

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AttributeEntry> & builder) override
    {
        static constexpr AttributeEntry attributes[] = { { 1, {}, Access::Privilege::kView, Access::Privilege::kOperate } };
        ReturnErrorOnFailure(builder.ReferenceExisting(attributes));
        return DefaultServerCluster::Attributes(path, builder);
    }

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<AcceptedCommandEntry> & builder) override
    {
        static constexpr AcceptedCommandEntry commands[] = { { 1, {}, Access::Privilege::kOperate } };
        return builder.ReferenceExisting(commands);
    }

    std::optional<ActionReturnStatus> InvokeCommand(const InvokeRequest & request, TLV::TLVReader & reader,
                                                    CommandHandler * handler) override
    {
        mCommandInvoked = true;
        return Protocols::InteractionModel::Status::InvalidCommand;
    }

    bool mCommandInvoked = false;
};

class ChangeListener : public AttributeChangeListener
{
public:
    void OnAttributeChanged(const ConcreteAttributePath & path, AttributeChangeType type) override
    {
        EXPECT_EQ(type, AttributeChangeType::kReportable);
        mAttributes.push_back(path);
    }

    void OnEndpointChanged(EndpointId endpoint, EndpointChangeType type) override
    {
        EXPECT_EQ(emberAfFindEndpointType(endpoint) != nullptr, type == EndpointChangeType::kAdded);
        mEndpoints.emplace_back(endpoint, type);
    }

    std::vector<ConcreteAttributePath> mAttributes;
    std::vector<std::pair<EndpointId, EndpointChangeType>> mEndpoints;
};

class TestDynamicDispatcher : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        mProvider.Reset();
        mProvider.RegisterAttributeChangeListener(mListener);
        ASSERT_EQ(emberAfEndpointCount(), 1);
    }

    void TearDown() override { mProvider.UnregisterAttributeChangeListener(mListener); }

    void UnregisterIfRegistered(TestCluster & cluster)
    {
        if (mProvider.Registry().Get(cluster.GetPaths().front()) == &cluster)
        {
            EXPECT_EQ(dynamic_server::UnregisterServerCluster(cluster), CHIP_NO_ERROR);
        }
    }

    CodegenDataModelProvider & mProvider = CodegenDataModelProvider::Instance();
    ChangeListener mListener;
};

TEST_F(TestDynamicDispatcher, RegisteredClusterDefinesEndpointMetadata)
{
    const ConcreteClusterPath path(1, 0x1234);
    TestCluster cluster(path);
    ServerClusterRegistration registration(cluster);
    ASSERT_EQ(dynamic_server::RegisterServerCluster(registration), CHIP_NO_ERROR);
    auto unregister = ScopeExit([&] { UnregisterIfRegistered(cluster); });

    EXPECT_EQ(emberAfEndpointCount(), 2);
    EXPECT_TRUE(emberAfEndpointIndexIsEnabled(1));
    EXPECT_EQ(emberAfEndpointFromIndex(1), path.mEndpointId);
    EXPECT_EQ(emberAfIndexFromEndpoint(path.mEndpointId), 1);
    EXPECT_EQ(emberAfParentEndpointFromIndex(1), kOtaProviderDynamicEndpointId);
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(path.mEndpointId);
    ASSERT_NE(endpoint, nullptr);
    EXPECT_EQ(endpoint->cluster, nullptr);
    EXPECT_EQ(endpoint->clusterCount, 0);
    EXPECT_EQ(emberAfClusterCountForEndpointType(endpoint, true), 0);
    EXPECT_EQ(emberAfFindServerCluster(path.mEndpointId, path.mClusterId), nullptr);

    ReadOnlyBufferBuilder<ServerClusterEntry> builder;
    ASSERT_EQ(mProvider.ServerClusters(path.mEndpointId, builder), CHIP_NO_ERROR);
    const auto clusters = builder.TakeBuffer();
    ASSERT_EQ(clusters.size(), 1u);
    EXPECT_EQ(clusters[0].clusterId, path.mClusterId);
    EXPECT_EQ(clusters[0].dataVersion, cluster.GetDataVersion(path));
    EXPECT_TRUE(ServerClusterFinder(&mProvider).Find(path).has_value());
    EXPECT_TRUE(AttributeFinder(&mProvider).Find({ path.mEndpointId, path.mClusterId, 1 }).has_value());

    ReadOperation read(path.mEndpointId, path.mClusterId, 1);
    read.SetSubjectDescriptor(kAdminSubjectDescriptor);
    auto encoder = read.StartEncoding();
    EXPECT_TRUE(mProvider.ReadAttribute(read.GetRequest(), *encoder).IsSuccess());
    EXPECT_TRUE(encoder->TriedEncode());

    ReadOnlyBufferBuilder<AcceptedCommandEntry> commandsBuilder;
    ASSERT_EQ(mProvider.AcceptedCommands(path, commandsBuilder), CHIP_NO_ERROR);
    const auto commands = commandsBuilder.TakeBuffer();
    ASSERT_EQ(commands.size(), 1u);
    EXPECT_EQ(commands[0].commandId, 1u);
    InvokeRequest request({ path.mEndpointId, path.mClusterId, 1 }, kAdminSubjectDescriptor);
    TLV::TLVReader reader;
    auto result = mProvider.InvokeCommand(request, reader, nullptr);
    EXPECT_EQ(result.value_or(Protocols::InteractionModel::Status::Failure).GetStatusCode().GetStatus(),
              Protocols::InteractionModel::Status::InvalidCommand);
    EXPECT_TRUE(cluster.mCommandInvoked);

    ASSERT_EQ(dynamic_server::UnregisterServerCluster(cluster), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfEndpointCount(), 1);
    EXPECT_FALSE(emberAfEndpointIndexIsEnabled(1));
    EXPECT_EQ(emberAfFindEndpointType(path.mEndpointId), nullptr);
    EXPECT_EQ(emberAfIndexFromEndpoint(path.mEndpointId), UINT16_MAX);
    EXPECT_FALSE(ServerClusterFinder(&mProvider).Find(path).has_value());
    EXPECT_FALSE(AttributeFinder(&mProvider).Find({ path.mEndpointId, path.mClusterId, 1 }).has_value());
    EXPECT_EQ(mProvider.AcceptedCommands(path, commandsBuilder), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDynamicDispatcher, SharedEndpointNotifiesOnlyOnFirstRegistrationAndLastRemoval)
{
    TestCluster first({ 1, 0x1234 });
    TestCluster second({ 1, 0x1235 });
    ServerClusterRegistration firstRegistration(first);
    ServerClusterRegistration secondRegistration(second);
    auto unregister = ScopeExit([&] {
        UnregisterIfRegistered(first);
        UnregisterIfRegistered(second);
    });

    ASSERT_EQ(dynamic_server::RegisterServerCluster(firstRegistration), CHIP_NO_ERROR);
    ASSERT_EQ(mListener.mEndpoints.size(), 1u);
    EXPECT_EQ(mListener.mEndpoints[0].first, 1);
    EXPECT_EQ(mListener.mEndpoints[0].second, EndpointChangeType::kAdded);
    ASSERT_EQ(mListener.mAttributes.size(), 1u);
    EXPECT_EQ(mListener.mAttributes[0],
              ConcreteAttributePath(kOtaProviderDynamicEndpointId, Clusters::Descriptor::Id,
                                    Clusters::Descriptor::Attributes::PartsList::Id));

    ASSERT_EQ(dynamic_server::RegisterServerCluster(secondRegistration), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfEndpointCount(), 2);
    EXPECT_EQ(mListener.mEndpoints.size(), 1u);
    ASSERT_EQ(dynamic_server::UnregisterServerCluster(first), CHIP_NO_ERROR);
    EXPECT_NE(emberAfFindEndpointType(1), nullptr);
    EXPECT_EQ(mListener.mEndpoints.size(), 1u);

    ASSERT_EQ(dynamic_server::UnregisterServerCluster(second), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfFindEndpointType(1), nullptr);
    ASSERT_EQ(mListener.mEndpoints.size(), 2u);
    EXPECT_EQ(mListener.mEndpoints[1].first, 1);
    EXPECT_EQ(mListener.mEndpoints[1].second, EndpointChangeType::kRemoved);
    ASSERT_EQ(mListener.mAttributes.size(), 2u);
    EXPECT_EQ(mListener.mAttributes[1], mListener.mAttributes[0]);
}

TEST_F(TestDynamicDispatcher, EnumeratesDistinctRegisteredEndpoints)
{
    TestCluster higher({ 9, 0x1234 });
    TestCluster lower({ 7, 0x1234 });
    TestCluster shared({ 7, 0x1235 });
    ServerClusterRegistration higherRegistration(higher);
    ServerClusterRegistration lowerRegistration(lower);
    ServerClusterRegistration sharedRegistration(shared);
    auto unregister = ScopeExit([&] {
        UnregisterIfRegistered(higher);
        UnregisterIfRegistered(lower);
        UnregisterIfRegistered(shared);
    });
    ASSERT_EQ(dynamic_server::RegisterServerCluster(higherRegistration), CHIP_NO_ERROR);
    ASSERT_EQ(dynamic_server::RegisterServerCluster(lowerRegistration), CHIP_NO_ERROR);
    ASSERT_EQ(dynamic_server::RegisterServerCluster(sharedRegistration), CHIP_NO_ERROR);

    EXPECT_EQ(emberAfEndpointCount(), 3);
    EXPECT_EQ(emberAfEndpointFromIndex(0), kOtaProviderDynamicEndpointId);
    EXPECT_EQ(emberAfEndpointFromIndex(1), 7);
    EXPECT_EQ(emberAfEndpointFromIndex(2), 9);
    EXPECT_EQ(emberAfIndexFromEndpoint(7), 1);
    EXPECT_EQ(emberAfIndexFromEndpoint(9), 2);
    EXPECT_EQ(emberAfEndpointFromIndex(3), kInvalidEndpointId);
    EXPECT_FALSE(emberAfEndpointIndexIsEnabled(3));
    EXPECT_EQ(emberAfParentEndpointFromIndex(3), kInvalidEndpointId);

    const auto endpoints = mProvider.EndpointsIgnoreError();
    ASSERT_EQ(endpoints.size(), 3u);
    EXPECT_EQ(endpoints[1].id, 7);
    EXPECT_EQ(endpoints[2].id, 9);
    EXPECT_EQ(endpoints[1].parentId, kOtaProviderDynamicEndpointId);

    ASSERT_EQ(dynamic_server::UnregisterServerCluster(lower), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfEndpointCount(), 3);
    ASSERT_EQ(dynamic_server::UnregisterServerCluster(shared), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfEndpointCount(), 2);
    EXPECT_EQ(emberAfEndpointFromIndex(1), 9);
    EXPECT_EQ(emberAfIndexFromEndpoint(9), 1);
}

TEST_F(TestDynamicDispatcher, FailedRegistrationDoesNotNotify)
{
    TestCluster first({ 1, 0x1234 });
    TestCluster duplicate({ 1, 0x1234 });
    ServerClusterRegistration firstRegistration(first);
    ServerClusterRegistration duplicateRegistration(duplicate);
    auto unregister = ScopeExit([&] { UnregisterIfRegistered(first); });
    ASSERT_EQ(dynamic_server::RegisterServerCluster(firstRegistration), CHIP_NO_ERROR);
    EXPECT_EQ(dynamic_server::RegisterServerCluster(duplicateRegistration), CHIP_ERROR_DUPLICATE_KEY_ID);
    EXPECT_EQ(dynamic_server::UnregisterServerCluster(duplicate), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mListener.mEndpoints.size(), 1u);
    EXPECT_EQ(mListener.mAttributes.size(), 1u);
    EXPECT_EQ(emberAfEndpointCount(), 2);
}

TEST_F(TestDynamicDispatcher, OtaEndpointRemainsVisibleWithoutRegisteredClusters)
{
    TestCluster cluster({ kOtaProviderDynamicEndpointId, 0x1234 });
    ServerClusterRegistration registration(cluster);
    auto unregister = ScopeExit([&] { UnregisterIfRegistered(cluster); });
    ASSERT_EQ(dynamic_server::RegisterServerCluster(registration), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfEndpointCount(), 1);
    EXPECT_TRUE(mListener.mEndpoints.empty());
    const auto clusters = mProvider.ServerClustersIgnoreError(kOtaProviderDynamicEndpointId);
    ASSERT_EQ(clusters.size(), 2u);
    EXPECT_TRUE(ServerClusterFinder(&mProvider)
                    .Find({ kOtaProviderDynamicEndpointId, Clusters::OtaSoftwareUpdateProvider::Id })
                    .has_value());
    ASSERT_EQ(dynamic_server::UnregisterServerCluster(cluster), CHIP_NO_ERROR);
    EXPECT_EQ(emberAfEndpointCount(), 1);
    EXPECT_TRUE(emberAfEndpointIndexIsEnabled(0));
    EXPECT_NE(emberAfFindEndpointType(kOtaProviderDynamicEndpointId), nullptr);
    EXPECT_TRUE(mListener.mEndpoints.empty());
    EXPECT_TRUE(mListener.mAttributes.empty());
}

} // namespace
