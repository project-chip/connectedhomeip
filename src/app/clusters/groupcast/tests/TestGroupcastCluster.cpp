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

#include <app/MessageDef/CommandDataIB.h>
#include <app/clusters/groupcast/GroupcastCluster.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/Groupcast/Enums.h>
#include <clusters/Groupcast/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters::Groupcast;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

class MockCommandHandler : public app::CommandHandler
{
public:
    ~MockCommandHandler() override {}

    struct ResponseRecord
    {
        app::ConcreteCommandPath path;
        CommandId commandId;
        System::PacketBufferHandle encodedData;
    };

    CHIP_ERROR FallibleAddStatus(const app::ConcreteCommandPath & aRequestCommandPath,
                                 const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                 const char * context = nullptr) override
    {
        return CHIP_NO_ERROR;
    }

    void AddStatus(const app::ConcreteCommandPath & aRequestCommandPath,
                   const Protocols::InteractionModel::ClusterStatusCode & aStatus, const char * context = nullptr) override
    {
        CHIP_ERROR err = FallibleAddStatus(aRequestCommandPath, aStatus, context);
        VerifyOrDie(err == CHIP_NO_ERROR);
    }

    FabricIndex GetAccessingFabricIndex() const override { return mFabricIndex; }

    CHIP_ERROR AddResponseData(const app::ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                               const app::DataModel::EncodableToTLV & aEncodable) override
    {
        System::PacketBufferHandle handle = MessagePacketBuffer::New(1024);
        VerifyOrReturnError(!handle.IsNull(), CHIP_ERROR_NO_MEMORY);
        TLV::TLVWriter baseWriter;
        baseWriter.Init(handle->Start(), handle->MaxDataLength());
        app::DataModel::FabricAwareTLVWriter writer(baseWriter, mFabricIndex);
        TLV::TLVType ct;
        ReturnErrorOnFailure(
            static_cast<TLV::TLVWriter &>(writer).StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, ct));
        ReturnErrorOnFailure(aEncodable.EncodeTo(writer, TLV::ContextTag(app::CommandDataIB::Tag::kFields)));
        ReturnErrorOnFailure(static_cast<TLV::TLVWriter &>(writer).EndContainer(ct));
        handle->SetDataLength(static_cast<TLV::TLVWriter &>(writer).GetLengthWritten());
        mResponse.path        = aRequestCommandPath;
        mResponse.commandId   = aResponseCommandId;
        mResponse.encodedData = std::move(handle);
        return CHIP_NO_ERROR;
    }

    void AddResponse(const app::ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                     const app::DataModel::EncodableToTLV & aEncodable) override
    {
        (void) AddResponseData(aRequestCommandPath, aResponseCommandId, aEncodable);
    }

    bool IsTimedInvoke() const override { return false; }
    void FlushAcksRightAwayOnSlowCommand() override {}
    Access::SubjectDescriptor GetSubjectDescriptor() const override { return Access::SubjectDescriptor{}; }
    Messaging::ExchangeContext * GetExchangeContext() const override { return nullptr; }

    const ResponseRecord & GetResponse() const { return mResponse; }

    // Optional for test configuration
    void SetFabricIndex(FabricIndex index) { mFabricIndex = index; }

private:
    ResponseRecord mResponse;
    FabricIndex mFabricIndex = 0;
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestGroupcastCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestGroupcastCluster, TestAttributes)
{
    app::Clusters::GroupcastCluster cluster(BitFlags<Feature>{ Feature::kSender });
    // Attributes
    {
        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kRootEndpointId, app::Clusters::Groupcast::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      Attributes::Membership::kMetadataEntry,
                      Attributes::MaxMembershipCount::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    // Read attributes for expected values
    {
        chip::Test::ClusterTester tester(cluster);
        uint16_t revision{};
        ASSERT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
        ASSERT_EQ(revision, app::Clusters::Groupcast::kRevision);

        // Validate Constructor sets features correctly and is readable from attribute
        uint32_t features{};
        ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, features), CHIP_NO_ERROR);
        ASSERT_EQ(features, to_underlying(Feature::kSender));
    }
}

TEST_F(TestGroupcastCluster, TestAcceptedCommands)
{
    app::Clusters::GroupcastCluster cluster(BitFlags<Feature>{ Feature::kListener });
    ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
    ASSERT_EQ(cluster.AcceptedCommands({ kRootEndpointId, app::Clusters::Groupcast::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.AppendElements({
                  Commands::JoinGroup::kMetadataEntry,
                  Commands::LeaveGroup::kMetadataEntry,
                  Commands::UpdateGroupKey::kMetadataEntry,
                  Commands::ExpireGracePeriod::kMetadataEntry,
                  Commands::ConfigureAuxiliaryACL::kMetadataEntry,
              }),
              CHIP_NO_ERROR);
    ASSERT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

TEST_F(TestGroupcastCluster, TestJoinGroupCommand)
{
    chip::Test::TestServerClusterContext context;
    app::Clusters::GroupcastCluster cluster(BitFlags<Feature>{ Feature::kListener });
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Form a valid JoinGroup command
    const uint8_t keyData[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    const EndpointId kEndpoints[] = { 1 };

    Commands::JoinGroup::Type cmdData;
    cmdData.groupID         = 1;
    cmdData.endpoints       = chip::app::DataModel::List<const EndpointId>(kEndpoints, MATTER_ARRAY_SIZE(kEndpoints));
    cmdData.keyID           = 0xAABBCCDD;
    cmdData.key             = MakeOptional(ByteSpan(keyData));
    cmdData.gracePeriod     = MakeOptional(0U);
    cmdData.useAuxiliaryACL = MakeOptional(true);

    MockCommandHandler cmdHandler;
    chip::Test::ClusterTester tester(cluster);
    auto result = tester.InvokeCommand(Commands::JoinGroup::Id, cmdData, &cmdHandler);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().GetStatusCode().GetStatus(),    // NOLINT(bugprone-unchecked-optional-access)
              Protocols::InteractionModel::Status::Failure); // Currently expect Failure as JoinGroup command returns
                                                             // CHIP_ERROR_NOT_IMPLEMENTED
}
} // namespace
