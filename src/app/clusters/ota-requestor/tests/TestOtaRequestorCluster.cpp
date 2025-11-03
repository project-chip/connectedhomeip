/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <app/clusters/ota-requestor/OtaRequestorCluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Commands.h>
#include <clusters/OtaSoftwareUpdateRequestor/Enums.h>
#include <clusters/OtaSoftwareUpdateRequestor/EventIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct MockOtaRequestor : public OTARequestorInterface
{
public:
    ~MockOtaRequestor() = default;

    void Reset(void) override {}
    void HandleAnnounceOTAProvider(
        chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
        const OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType & commandData) override
    {
        mLastAnnounceCommandPayload = commandData;
    }
    CHIP_ERROR TriggerImmediateQuery(FabricIndex fabricIndex = kUndefinedFabricIndex) override
    {
        return CHIP_NO_ERROR;
    }
    void TriggerImmediateQueryInternal() override {}
    void DownloadUpdate() override {}
    void DownloadUpdateDelayedOnUserConsent() override {}
    void ApplyUpdate() override {}
    void NotifyUpdateApplied() override {}
    CHIP_ERROR GetUpdateStateProgressAttribute(EndpointId endpointId, DataModel::Nullable<uint8_t> & progress) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetUpdateStateAttribute(EndpointId endpointId, OTAUpdateStateEnum & state) override
    {
        return CHIP_NO_ERROR;
    }
    OTAUpdateStateEnum GetCurrentUpdateState() override
    {
        return {};
    }
    uint32_t GetTargetVersion() override
    {
        return 0;
    }
    void CancelImageUpdate() override {}
    CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex) override
    {
        return CHIP_NO_ERROR;
    }
    void SetCurrentProviderLocation(ProviderLocationType providerLocation) override {}
    void SetMetadataForProvider(chip::ByteSpan metadataForProvider) override {}
    void GetProviderLocation(Optional<ProviderLocationType> & providerLocation) override {}
    CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType & providerLocation) override
    {
        return CHIP_NO_ERROR;
    }
    ProviderLocationList::Iterator GetDefaultOTAProviderListIterator() override
    {
        ProviderLocationList::Iterator result(nullptr, 0);
        return result;
    }

    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType GetLastAnnounceCommandPayload() const
    {
        return mLastAnnounceCommandPayload;
    }

private:
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType mLastAnnounceCommandPayload;
};

struct TestOtaRequestorCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOtaRequestorCluster, TestCreate)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
}

TEST_F(TestOtaRequestorCluster, AttributeListTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    EXPECT_EQ(cluster.Attributes(ConcreteClusterPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id), attributes),
              CHIP_NO_ERROR);

    const DataModel::AttributeEntry expectedAttributes[] = {
        OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::kMetadataEntry,
        OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::kMetadataEntry,
        OtaSoftwareUpdateRequestor::Attributes::UpdateState::kMetadataEntry,
        OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    EXPECT_EQ(listBuilder.Append(Span(expectedAttributes), {}), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestOtaRequestorCluster, AcceptedCommandsTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    EXPECT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id), acceptedCommands),
              CHIP_NO_ERROR);

    const DataModel::AcceptedCommandEntry expectedCommands[] = {
        OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expected;
    EXPECT_EQ(expected.ReferenceExisting(expectedCommands), CHIP_NO_ERROR);
    EXPECT_TRUE(chip::Testing::EqualAcceptedCommandSets(acceptedCommands.TakeBuffer(), expected.TakeBuffer()));
}

TEST_F(TestOtaRequestorCluster, GeneratedCommandsTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<CommandId> generatedCommands;
    EXPECT_EQ(cluster.GeneratedCommands(ConcreteClusterPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id), generatedCommands),
              CHIP_NO_ERROR);
    EXPECT_EQ(generatedCommands.TakeBuffer().size(), 0u);
}

TEST_F(TestOtaRequestorCluster, EventInfoTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    DataModel::EventEntry eventInfo;

    EXPECT_EQ(cluster.EventInfo(ConcreteEventPath(kTestEndpointId,
                                                  OtaSoftwareUpdateRequestor::Id,
                                                  OtaSoftwareUpdateRequestor::Events::StateTransition::Id), eventInfo),
              CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege,
              OtaSoftwareUpdateRequestor::Events::StateTransition::kMetadataEntry.readPrivilege);

    EXPECT_EQ(cluster.EventInfo(ConcreteEventPath(kTestEndpointId,
                                                  OtaSoftwareUpdateRequestor::Id,
                                                  OtaSoftwareUpdateRequestor::Events::VersionApplied::Id), eventInfo),
              CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege,
              OtaSoftwareUpdateRequestor::Events::VersionApplied::kMetadataEntry.readPrivilege);

    EXPECT_EQ(cluster.EventInfo(ConcreteEventPath(kTestEndpointId,
                                                  OtaSoftwareUpdateRequestor::Id,
                                                  OtaSoftwareUpdateRequestor::Events::DownloadError::Id), eventInfo),
              CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege,
              OtaSoftwareUpdateRequestor::Events::DownloadError::kMetadataEntry.readPrivilege);
}

TEST_F(TestOtaRequestorCluster, AnnounceOtaProviderCommandTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID = 1234;
    payload.vendorID = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint = 5;

    std::optional<DataModel::ActionReturnStatus> result =
        tester.InvokeCommand(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload, nullptr);
    EXPECT_EQ(result, std::nullopt);

    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType forwarded_payload =
        otaRequestor.GetLastAnnounceCommandPayload();
    EXPECT_EQ(forwarded_payload.providerNodeID, static_cast<NodeId>(1234));
    EXPECT_EQ(forwarded_payload.vendorID, static_cast<VendorId>(4321));
    EXPECT_EQ(forwarded_payload.announcementReason,
              OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable);
    EXPECT_EQ(forwarded_payload.endpoint, 5);
}

TEST_F(TestOtaRequestorCluster, AnnounceOtaProviderCommandInvalidMetadataTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OtaRequestorCluster cluster(kTestEndpointId, otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID = 1234;
    payload.vendorID = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint = 5;
    uint8_t bytes[513] = {'\0'};
    payload.metadataForNode = MakeOptional(ByteSpan(bytes));

    std::optional<DataModel::ActionReturnStatus> result =
        tester.InvokeCommand(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload, nullptr);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidCommand));
}

}  // namespace
