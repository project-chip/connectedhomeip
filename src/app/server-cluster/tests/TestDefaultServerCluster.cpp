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
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLVReader.h>

#include <cstdlib>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::DataModel;
using namespace chip::app::Testing;
using namespace chip::Protocols::InteractionModel;

namespace {

class FakeDefaultServerCluster : public DefaultServerCluster
{
public:
    FakeDefaultServerCluster(ClusterId id) : mClusterId(id) {}

    ClusterId GetClusterId() const override { return mClusterId; }

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

private:
    ClusterId mClusterId;
};

} // namespace

TEST(TestDefaultServerCluster, TestDataVersion)
{
    FakeDefaultServerCluster cluster(1);

    DataVersion v1 = cluster.GetDataVersion();
    cluster.IncreaseDataVersion();
    ASSERT_EQ(cluster.GetDataVersion(), v1 + 1);
}

TEST(TestDefaultServerCluster, TestFlagsDefault)
{
    FakeDefaultServerCluster cluster(1);
    ASSERT_EQ(cluster.GetClusterFlags().Raw(), 0u);
}

TEST(TestDefaultServerCluster, AttributesDefault)
{
    FakeDefaultServerCluster cluster(1);

    DataModel::ListBuilder<AttributeEntry> attributes;

    ASSERT_EQ(cluster.Attributes({ 1, 1 }, attributes), CHIP_NO_ERROR);

    ReadOnlyBuffer<AttributeEntry> data = attributes.TakeBuffer();

    // 5 global attributes are currently supported. Ensure they are makred
    ASSERT_EQ(data.size(), 5u);

    ASSERT_EQ(data[0].attributeId, Globals::Attributes::ClusterRevision::Id);
    ASSERT_EQ(data[1].attributeId, Globals::Attributes::FeatureMap::Id);
    ASSERT_EQ(data[2].attributeId, Globals::Attributes::AttributeList::Id);
    ASSERT_EQ(data[3].attributeId, Globals::Attributes::AcceptedCommandList::Id);
    ASSERT_EQ(data[4].attributeId, Globals::Attributes::GeneratedCommandList::Id);

    // first 2 are normal, the rest are list
    for (size_t i = 0; i < 5; i++)
    {
        ASSERT_EQ(data[i].flags.Has(AttributeQualityFlags::kListAttribute), (i >= 2));
        ASSERT_EQ(data[i].readPrivilege, Access::Privilege::kView);
        ASSERT_FALSE(data[i].writePrivilege.has_value());
    }
}

TEST(TestDefaultServerCluster, CommandsDefault)
{
    FakeDefaultServerCluster cluster(1);

    DataModel::ListBuilder<AcceptedCommandEntry> acceptedCommands;
    ASSERT_EQ(cluster.AcceptedCommands({ 1, 1 }, acceptedCommands), CHIP_NO_ERROR);
    ASSERT_TRUE(acceptedCommands.TakeBuffer().empty());

    DataModel::ListBuilder<CommandId> generatedCommands;
    ASSERT_EQ(cluster.GeneratedCommands({ 1, 1 }, generatedCommands), CHIP_NO_ERROR);
    ASSERT_TRUE(generatedCommands.TakeBuffer().empty());
}

TEST(TestDefaultServerCluster, WriteAttributeDefault)
{
    FakeDefaultServerCluster cluster(1);

    WriteOperation test(0 /* endpoint */, 1 /* cluster */, 1234 /* attribute */);
    test.SetSubjectDescriptor(kAdminSubjectDescriptor);

    AttributeValueDecoder decoder = test.DecoderFor<uint32_t>(12345);

    ASSERT_EQ(cluster.WriteAttribute(test.GetRequest(), decoder), Status::UnsupportedWrite);
    ASSERT_FALSE(decoder.TriedDecode());
}

TEST(TestDefaultServerCluster, InvokeDefault)
{
    FakeDefaultServerCluster cluster(1);

    TLV::TLVReader tlvReader;
    InvokeRequest request;

    request.path = { 0 /* endpoint */, 1 /* cluster */, 1234 /* command */ };

    ASSERT_EQ(cluster.InvokeCommand(request, tlvReader, nullptr /* command handler, assumed unused here */),
              Status::UnsupportedCommand);
}
