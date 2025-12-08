/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <access/Privilege.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/ConcreteClusterPath.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVReader.h>
#include <lib/support/ReadOnlyBuffer.h>

#include <cstdlib>

using namespace chip;
using namespace chip::Testing;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Testing;
using namespace chip::Protocols::InteractionModel;

namespace {

class FakeDefaultServerCluster : public DefaultServerCluster
{
public:
    constexpr FakeDefaultServerCluster(ConcreteClusterPath && path) : DefaultServerCluster(std::move(path)) {}

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override
    {
        switch (request.path.mAttributeId)
        {
        case Globals::Attributes::FeatureMap::Id:
            return encoder.Encode<uint32_t>(0);
        case Globals::Attributes::ClusterRevision::Id:
            return encoder.Encode<uint32_t>(123);
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    void TestIncreaseDataVersion() { IncreaseDataVersion(); }
    void TestNotifyAttributeChanged(AttributeId attributeId) { NotifyAttributeChanged(attributeId); }
    ActionReturnStatus TestNotifyAttributeChangedIfSuccess(AttributeId attributeId, ActionReturnStatus status)
    {
        return NotifyAttributeChangedIfSuccess(attributeId, status);
    }
};

} // namespace

TEST(TestDefaultServerCluster, TestDataVersion)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    DataVersion v1 = cluster.GetDataVersion({ 1, 2 });
    cluster.TestIncreaseDataVersion();
    ASSERT_EQ(cluster.GetDataVersion({ 1, 2 }), v1 + 1);
}

TEST(TestDefaultServerCluster, TestFlagsDefault)
{
    FakeDefaultServerCluster cluster({ 1, 2 });
    ASSERT_EQ(cluster.GetClusterFlags({ 1, 2 }).Raw(), 0u);
}

TEST(TestDefaultServerCluster, ListWriteNotification)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    // this does not test anything really, except we get 100% coverage and we see that we do not crash
    cluster.ListAttributeWriteNotification({ 1, 2, 3 }, DataModel::ListWriteOperation::kListWriteBegin, 1);
    cluster.ListAttributeWriteNotification({ 1, 2, 3 }, DataModel::ListWriteOperation::kListWriteFailure, 1);
}

TEST(TestDefaultServerCluster, AttributesDefault)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    ReadOnlyBufferBuilder<AttributeEntry> attributes;

    ASSERT_EQ(cluster.Attributes({ 1, 1 }, attributes), CHIP_NO_ERROR);

    ReadOnlyBuffer<AttributeEntry> data = attributes.TakeBuffer();

    // 5 global attributes are currently supported. Ensure they are returned.
    ASSERT_EQ(data.size(), 5u);

    ASSERT_EQ(data[0].attributeId, Globals::Attributes::FeatureMap::Id);
    ASSERT_EQ(data[1].attributeId, Globals::Attributes::ClusterRevision::Id);
    ASSERT_EQ(data[2].attributeId, Globals::Attributes::GeneratedCommandList::Id);
    ASSERT_EQ(data[3].attributeId, Globals::Attributes::AcceptedCommandList::Id);
    ASSERT_EQ(data[4].attributeId, Globals::Attributes::AttributeList::Id);

    // first 2 are normal, the rest are list
    for (size_t i = 0; i < 5; i++)
    {
        ASSERT_EQ(data[i].HasFlags(AttributeQualityFlags::kListAttribute), (i >= 2));
        ASSERT_EQ(data[i].GetReadPrivilege(), Access::Privilege::kView);
        ASSERT_FALSE(data[i].GetWritePrivilege().has_value());
    }
}

TEST(TestDefaultServerCluster, ListWriteIsANoop)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    // this is really for coverage, we are not calling anything useful
    cluster.ListAttributeWriteNotification({ 1 /* endpoint */, 2 /* cluster */, 3 /* attribute */ },
                                           DataModel::ListWriteOperation::kListWriteBegin, 1);
    cluster.ListAttributeWriteNotification({ 1 /* endpoint */, 2 /* cluster */, 3 /* attribute */ },
                                           DataModel::ListWriteOperation::kListWriteSuccess, 1);
}

TEST(TestDefaultServerCluster, CommandsDefault)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    ReadOnlyBufferBuilder<AcceptedCommandEntry> acceptedCommands;
    ASSERT_EQ(cluster.AcceptedCommands({ 1, 1 }, acceptedCommands), CHIP_NO_ERROR);
    ASSERT_TRUE(acceptedCommands.TakeBuffer().empty());

    ReadOnlyBufferBuilder<CommandId> generatedCommands;
    ASSERT_EQ(cluster.GeneratedCommands({ 1, 1 }, generatedCommands), CHIP_NO_ERROR);
    ASSERT_TRUE(generatedCommands.TakeBuffer().empty());
}

TEST(TestDefaultServerCluster, WriteAttributeDefault)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    WriteOperation test(0 /* endpoint */, 1 /* cluster */, 1234 /* attribute */);
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<uint32_t>(12345);

    ASSERT_EQ(cluster.WriteAttribute(test.GetRequest(), decoder), Status::UnsupportedWrite);
    ASSERT_FALSE(decoder.TriedDecode());
}

TEST(TestDefaultServerCluster, InvokeDefault)
{
    FakeDefaultServerCluster cluster({ 1, 2 });

    TLV::TLVReader tlvReader;
    InvokeRequest request;

    request.path = { 0 /* endpoint */, 1 /* cluster */, 1234 /* command */ };

    ASSERT_EQ(cluster.InvokeCommand(request, tlvReader, nullptr /* command handler, assumed unused here */),
              Status::UnsupportedCommand);
}

TEST(TestDefaultServerCluster, NotifyAttributeChanged)
{
    constexpr ClusterId kEndpointId = 321;
    constexpr ClusterId kClusterId  = 1122;
    FakeDefaultServerCluster cluster({ kEndpointId, kClusterId });

    // When no ServerClusterContext is set, only the data version should change.
    DataVersion oldVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });

    cluster.TestNotifyAttributeChanged(123);
    ASSERT_NE(cluster.GetDataVersion({ kEndpointId, kClusterId }), oldVersion);

    // Create a ServerClusterContext and verify that attribute change notifications are processed.
    TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    oldVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });
    cluster.TestNotifyAttributeChanged(234);
    ASSERT_NE(cluster.GetDataVersion({ kEndpointId, kClusterId }), oldVersion);

    ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
    ASSERT_EQ(context.ChangeListener().DirtyList()[0], AttributePathParams(kEndpointId, kClusterId, 234));
}

TEST(TestDefaultServerCluster, NotifyAttributeChangedIfSuccess)
{
    constexpr ClusterId kEndpointId = 321;
    constexpr ClusterId kClusterId  = 1122;
    FakeDefaultServerCluster cluster({ kEndpointId, kClusterId });

    // When no ServerClusterContext is set, only the data version should change.
    DataVersion oldVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });

    ASSERT_EQ(cluster.TestNotifyAttributeChangedIfSuccess(123, Status::Success), Status::Success);
    DataVersion newVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });
    ASSERT_NE(newVersion, oldVersion);

    oldVersion = newVersion;
    ASSERT_EQ(cluster.TestNotifyAttributeChangedIfSuccess(123, Status::Failure), Status::Failure);
    newVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });
    ASSERT_EQ(newVersion, oldVersion);

    // Create a ServerClusterContext and verify that attribute change notifications are processed.
    TestServerClusterContext context;
    ASSERT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    oldVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });
    ASSERT_EQ(cluster.TestNotifyAttributeChangedIfSuccess(234, Status::Success), Status::Success);
    ASSERT_NE(cluster.GetDataVersion({ kEndpointId, kClusterId }), oldVersion);

    ASSERT_EQ(context.ChangeListener().DirtyList().size(), 1u);
    ASSERT_EQ(context.ChangeListener().DirtyList()[0], AttributePathParams(kEndpointId, kClusterId, 234));

    // now test a failure - nothing should be marked dirty
    oldVersion = cluster.GetDataVersion({ kEndpointId, kClusterId });
    context.ChangeListener().DirtyList().clear();
    ASSERT_EQ(cluster.TestNotifyAttributeChangedIfSuccess(345, Status::Failure), Status::Failure);
    ASSERT_EQ(cluster.GetDataVersion({ kEndpointId, kClusterId }), oldVersion);
    ASSERT_TRUE(context.ChangeListener().DirtyList().empty());
}
