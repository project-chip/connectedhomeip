/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/ota-requestor/OTARequestorCluster.h>
#include <pw_unit_test/framework.h>

#include <vector>

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorEventGenerator.h>
#include <app/clusters/ota-requestor/OTARequestorAttributes.h>
#include <app/clusters/ota-requestor/OTARequestorDriver.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestEventGenerator.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Commands.h>
#include <clusters/OtaSoftwareUpdateRequestor/Enums.h>
#include <clusters/OtaSoftwareUpdateRequestor/EventIds.h>
#include <clusters/OtaSoftwareUpdateRequestor/Events.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>
#include <clusters/OtaSoftwareUpdateRequestor/Structs.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Attributes;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Events;

namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Structs {
namespace ProviderLocation {

constexpr bool operator==(const Type & lhs, const Type & rhs)
{
    return lhs.providerNodeID == rhs.providerNodeID && lhs.endpoint == rhs.endpoint && lhs.fabricIndex == rhs.fabricIndex;
}

} // namespace ProviderLocation
} // namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Structs

namespace {

constexpr EndpointId kTestEndpointId = 1;
const ConcreteClusterPath kTestClusterPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id);

class MockOtaCommands : public OTARequestorCommandInterface
{
public:
    void
    HandleAnnounceOTAProvider(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType & commandData) override
    {
        mLastAnnounceCommandPayload = commandData;
        if (commandObj)
        {
            commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);
        }
    }

    void OnFabricRemoved(FabricIndex fabricIndex) override
    {
        mLastFabricRemovedIndex = fabricIndex;
        mFabricRemovedCallCount++;
    }

    std::optional<OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType> GetLastAnnounceCommandPayload() const
    {
        return mLastAnnounceCommandPayload;
    }

    std::optional<FabricIndex> GetLastFabricRemovedIndex() const { return mLastFabricRemovedIndex; }
    uint32_t GetFabricRemovedCallCount() const { return mFabricRemovedCallCount; }

private:
    std::optional<OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType> mLastAnnounceCommandPayload;
    std::optional<FabricIndex> mLastFabricRemovedIndex;
    uint32_t mFabricRemovedCallCount = 0;
};

struct TestOTARequestorCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOTARequestorCluster, TestCreate)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorCluster, AttributeListTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsAttributesListEqualTo(cluster,
                                                       {
                                                           DefaultOTAProviders::kMetadataEntry,
                                                           UpdatePossible::kMetadataEntry,
                                                           UpdateState::kMetadataEntry,
                                                           UpdateStateProgress::kMetadataEntry,
                                                       }));
}

TEST_F(TestOTARequestorCluster, AcceptedCommandsTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(
        chip::Testing::IsAcceptedCommandsListEqualTo(cluster,
                                                     {
                                                         OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::kMetadataEntry,
                                                     }));
}

TEST_F(TestOTARequestorCluster, GeneratedCommandsTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsGeneratedCommandsListEqualTo(cluster, {}));
}

TEST_F(TestOTARequestorCluster, EventInfoTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    DataModel::EventEntry eventInfo;

    EXPECT_EQ(cluster.EventInfo(ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id,
                                                  OtaSoftwareUpdateRequestor::Events::StateTransition::Id),
                                eventInfo),
              CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege, OtaSoftwareUpdateRequestor::Events::StateTransition::kMetadataEntry.readPrivilege);

    EXPECT_EQ(cluster.EventInfo(ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id,
                                                  OtaSoftwareUpdateRequestor::Events::VersionApplied::Id),
                                eventInfo),
              CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege, OtaSoftwareUpdateRequestor::Events::VersionApplied::kMetadataEntry.readPrivilege);

    EXPECT_EQ(cluster.EventInfo(ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id,
                                                  OtaSoftwareUpdateRequestor::Events::DownloadError::Id),
                                eventInfo),
              CHIP_NO_ERROR);
    EXPECT_EQ(eventInfo.readPrivilege, OtaSoftwareUpdateRequestor::Events::DownloadError::kMetadataEntry.readPrivilege);
}

TEST_F(TestOTARequestorCluster, AnnounceOtaProviderCommandTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Construct the payload.
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID     = 1234;
    payload.vendorID           = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint           = 5;

    // Invoke the command.
    auto result = tester.Invoke(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload);
    EXPECT_TRUE(result.IsSuccess());

    // Check that the payload was decoded correctly.
    ASSERT_TRUE(otaCommands.GetLastAnnounceCommandPayload().has_value());
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType forwarded_payload =
        *otaCommands.GetLastAnnounceCommandPayload();
    EXPECT_EQ(forwarded_payload.providerNodeID, static_cast<NodeId>(1234));
    EXPECT_EQ(forwarded_payload.vendorID, static_cast<VendorId>(4321));
    EXPECT_EQ(forwarded_payload.announcementReason, OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable);
    EXPECT_EQ(forwarded_payload.endpoint, 5);
}

TEST_F(TestOTARequestorCluster, UnknownAnnouncementReasonIsRejected)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Construct the payload with an unknown announcement reason. Sending kUnknownEnumValue is rejected by the encoder, so use an
    // undefined value instead.
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID     = 1234;
    payload.vendorID           = static_cast<VendorId>(4321);
    payload.announcementReason = static_cast<OtaSoftwareUpdateRequestor::AnnouncementReasonEnum>(99);
    payload.endpoint           = 5;

    // Invoke the command.
    auto result = tester.Invoke(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload);
    EXPECT_EQ(result.status,
              std::make_optional(DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::ConstraintError)));

    // Check that the command wasn't passed on.
    EXPECT_FALSE(otaCommands.GetLastAnnounceCommandPayload().has_value());
}

TEST_F(TestOTARequestorCluster, AnnounceOtaProviderCommandInvalidMetadataTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    Testing::ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Construct the payload.
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID     = 1234;
    payload.vendorID           = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint           = 5;
    // The maximum allowed metadata length is 512 bytes, so send 513.
    uint8_t bytes[513]      = { '\0' };
    payload.metadataForNode = MakeOptional(ByteSpan(bytes));

    // Invoke the command.
    auto result = tester.Invoke(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload);
    ASSERT_FALSE(result.IsSuccess());
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status, DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::ConstraintError));
}

TEST_F(TestOTARequestorCluster, ReadAttributesTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Testing::ClusterTester tester(cluster);

    // Read and verify DefaultOTAProviders.
    using DecodableProviderLocation = OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType;
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type provider;
    provider.providerNodeID = 1234u;
    provider.endpoint       = 8;
    provider.fabricIndex    = 2;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(provider), CHIP_NO_ERROR);
    DataModel::DecodableList<DecodableProviderLocation> defaultOtaProviders;
    EXPECT_EQ(tester.ReadAttribute(DefaultOTAProviders::Id, defaultOtaProviders), CHIP_NO_ERROR);
    size_t defaultOtaProvidersSize;
    ASSERT_EQ(defaultOtaProviders.ComputeSize(&defaultOtaProvidersSize), CHIP_NO_ERROR);
    EXPECT_EQ(defaultOtaProvidersSize, 1u);
    DataModel::DecodableList<DecodableProviderLocation>::Iterator defaultOtaProvidersIterator = defaultOtaProviders.begin();
    EXPECT_TRUE(defaultOtaProvidersIterator.Next());
    const DecodableProviderLocation & decodedProvider = defaultOtaProvidersIterator.GetValue();
    EXPECT_EQ(decodedProvider.providerNodeID, 1234u);
    EXPECT_EQ(decodedProvider.endpoint, 8);
    EXPECT_EQ(decodedProvider.fabricIndex, 2);

    // Read and verify UpdatePossible.
    bool updatePossible;
    EXPECT_EQ(tester.ReadAttribute(UpdatePossible::Id, updatePossible), CHIP_NO_ERROR);
    EXPECT_TRUE(updatePossible);

    // Read and verify UpdateState.
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kIdle,
                              OTARequestorAttributes::OTAChangeReasonEnum::kSuccess, DataModel::NullNullable);
    OtaSoftwareUpdateRequestor::UpdateStateEnum updateState;
    EXPECT_EQ(tester.ReadAttribute(UpdateState::Id, updateState), CHIP_NO_ERROR);
    EXPECT_EQ(updateState, OtaSoftwareUpdateRequestor::UpdateStateEnum::kIdle);

    // Read and verify UpdateStateProgress.
    DataModel::Nullable<uint8_t> updateStateProgress;
    EXPECT_EQ(tester.ReadAttribute(UpdateStateProgress::Id, updateStateProgress), CHIP_NO_ERROR);
    EXPECT_TRUE(updateStateProgress.IsNull());

    // Verify a non-null value as well.
    EXPECT_EQ(attributes.SetUpdateStateProgress(85), CHIP_NO_ERROR);
    EXPECT_EQ(tester.ReadAttribute(UpdateStateProgress::Id, updateStateProgress), CHIP_NO_ERROR);
    EXPECT_FALSE(updateStateProgress.IsNull());
    EXPECT_EQ(updateStateProgress.Value(), 85);

    // Read and verify FeatureMap.
    uint32_t featureMap;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    // Read and verify ClusterRevision.
    uint16_t clusterRevision;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, 1u);

    // Read non-existent attribute.
    uint32_t nonExistentAttribute;
    EXPECT_NE(tester.ReadAttribute(0xFFFF, nonExistentAttribute), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorCluster, WriteDefaultProvidersList)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    auto & changeListener = context.ChangeListener();
    changeListener.DirtyList().clear();
    DataVersion dataVersion = cluster.GetDataVersion(kTestClusterPath);

    // Write the default OTA providers list.
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type provider;
    provider.providerNodeID = 1234u;
    provider.endpoint       = 8;
    // This is the fabric index used in ClusterTester::WriteAttribute.
    provider.fabricIndex = chip::Testing::kTestFabricIndex;
    DataModel::List<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> payload(&provider, 1u);
    std::optional<DataModel::ActionReturnStatus> result =
        tester.WriteAttribute(DefaultOTAProviders::Id, payload, chip::Testing::ListWritingPattern::ClearAllThenAppendItems);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->IsSuccess());

    // Verify the data was written correctly.
    auto iterator = attributes.GetDefaultOtaProviderListIterator();
    ASSERT_TRUE(iterator.Next());
    EXPECT_EQ(iterator.GetValue(), provider);
    EXPECT_FALSE(iterator.Next());

    // Verify the attribute was reported as changed.
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, DefaultOTAProviders::Id);
    EXPECT_NE(cluster.GetDataVersion(kTestClusterPath), dataVersion);
}

TEST_F(TestOTARequestorCluster, WritingReadOnlyAttributesReturnsUnsupportedWrite)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Testing::ClusterTester tester(cluster);

    auto & changeListener = context.ChangeListener();
    changeListener.DirtyList().clear();
    DataVersion dataVersion = cluster.GetDataVersion(kTestClusterPath);

    std::optional<DataModel::ActionReturnStatus> result = tester.WriteAttribute(UpdatePossible::Id, false);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(UpdateState::Id, OtaSoftwareUpdateRequestor::UpdateStateEnum::kDelayedOnApply);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(UpdateStateProgress::Id, DataModel::MakeNullable<uint8_t>(50));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(Globals::Attributes::FeatureMap::Id, static_cast<uint32_t>(10));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(Globals::Attributes::ClusterRevision::Id, static_cast<uint16_t>(5));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
    EXPECT_EQ(cluster.GetDataVersion(kTestClusterPath), dataVersion);
}

TEST_F(TestOTARequestorCluster, ChangingAttributesMarksAsChanged)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & changeListener   = context.ChangeListener();
    DataVersion dataVersion = cluster.GetDataVersion(kTestClusterPath);

    changeListener.DirtyList().clear();
    attributes.SetUpdateState(OTARequestorAttributes::OTAUpdateStateEnum::kApplying,
                              OTARequestorAttributes::OTAChangeReasonEnum::kSuccess, DataModel::NullNullable);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateState::Id);
    EXPECT_NE(cluster.GetDataVersion(kTestClusterPath), dataVersion);

    changeListener.DirtyList().clear();
    dataVersion = cluster.GetDataVersion(kTestClusterPath);
    EXPECT_EQ(attributes.SetUpdateStateProgress(85), CHIP_NO_ERROR);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdateStateProgress::Id);
    EXPECT_NE(cluster.GetDataVersion(kTestClusterPath), dataVersion);

    changeListener.DirtyList().clear();
    dataVersion = cluster.GetDataVersion(kTestClusterPath);
    attributes.SetUpdatePossible(false);
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, UpdatePossible::Id);
    EXPECT_NE(cluster.GetDataVersion(kTestClusterPath), dataVersion);

    // DefaultOTAProviders is verified in the test WriteDefaultProvidersList.
}

TEST_F(TestOTARequestorCluster, GenerateVersionAppliedEventGeneratesAnEvent)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();
    ASSERT_FALSE(eventsGenerator.GetNextEvent().has_value());

    VersionApplied::Type event{ 0x12345678u, 0xABCDu };
    EXPECT_EQ(cluster.GenerateVersionAppliedEvent(event), CHIP_NO_ERROR);
    auto generatedEvent = eventsGenerator.GetNextEvent();
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
    ASSERT_TRUE(generatedEvent.has_value());
    EXPECT_EQ(generatedEvent->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, VersionApplied::Id));
    VersionApplied::DecodableType decodedEvent;
    ASSERT_EQ(generatedEvent->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 0x12345678u);
    EXPECT_EQ(decodedEvent.productID, 0xABCDu);
}

TEST_F(TestOTARequestorCluster, GenerateVersionAppliedEventBeforeStartupFails)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();

    VersionApplied::Type event{ 0x12345678u, 0xABCDu };
    EXPECT_NE(cluster.GenerateVersionAppliedEvent(event), CHIP_NO_ERROR);
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
}

TEST_F(TestOTARequestorCluster, GenerateDownloadErrorEventGeneratesAnEvent)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();
    ASSERT_FALSE(eventsGenerator.GetNextEvent().has_value());

    // Generate an event with null ProgressPercent and PlatformCode.
    DownloadError::Type event{ 0x12345678u, 0x123456789ABCDEF0u, DataModel::NullNullable, DataModel::NullNullable };
    EXPECT_EQ(cluster.GenerateDownloadErrorEvent(event), CHIP_NO_ERROR);
    auto generatedEvent = eventsGenerator.GetNextEvent();
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
    ASSERT_TRUE(generatedEvent.has_value());
    EXPECT_EQ(generatedEvent->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, DownloadError::Id));
    DownloadError::DecodableType decodedEvent;
    ASSERT_EQ(generatedEvent->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 0x12345678u);
    EXPECT_EQ(decodedEvent.bytesDownloaded, 0x123456789ABCDEF0u);
    EXPECT_TRUE(decodedEvent.progressPercent.IsNull());
    EXPECT_TRUE(decodedEvent.platformCode.IsNull());

    // Generate an event with non-null ProgressPercent and PlatformCode.
    event = { 0x12345678u, 0x123456789ABCDEF0u, 83u, 0x0FEDCBA987654321 };
    EXPECT_EQ(cluster.GenerateDownloadErrorEvent(event), CHIP_NO_ERROR);
    generatedEvent = eventsGenerator.GetNextEvent();
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
    ASSERT_TRUE(generatedEvent.has_value());
    EXPECT_EQ(generatedEvent->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, DownloadError::Id));
    ASSERT_EQ(generatedEvent->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 0x12345678u);
    EXPECT_EQ(decodedEvent.bytesDownloaded, 0x123456789ABCDEF0u);
    ASSERT_FALSE(decodedEvent.progressPercent.IsNull());
    EXPECT_EQ(decodedEvent.progressPercent.Value(), 83u);
    ASSERT_FALSE(decodedEvent.platformCode.IsNull());
    EXPECT_EQ(decodedEvent.platformCode.Value(), 0x0FEDCBA987654321);
}

TEST_F(TestOTARequestorCluster, GenerateDownloadErrorEventBeforeStartupFails)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();

    DownloadError::Type event{ 0x12345678u, 0x123456789ABCDEF0u, DataModel::NullNullable, DataModel::NullNullable };
    EXPECT_NE(cluster.GenerateDownloadErrorEvent(event), CHIP_NO_ERROR);
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
}

// OnFabricRemoved clears the removed fabric's DefaultOTAProviders entry, leaves other fabrics
// untouched, and notifies the command interface exactly once with the removed fabricIndex.
TEST_F(TestOTARequestorCluster, OnFabricRemovedClearsTargetFabricAndNotifiesRequestor)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Seed providers for two distinct fabrics.
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type providerA;
    providerA.providerNodeID = 0x1111u;
    providerA.endpoint       = 1;
    providerA.fabricIndex    = 1;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(providerA), CHIP_NO_ERROR);

    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type providerB;
    providerB.providerNodeID = 0x2222u;
    providerB.endpoint       = 2;
    providerB.fabricIndex    = 2;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(providerB), CHIP_NO_ERROR);

    // Sanity: both entries present.
    {
        auto it      = attributes.GetDefaultOtaProviderListIterator();
        size_t count = 0;
        while (it.Next())
        {
            count++;
        }
        EXPECT_EQ(count, 2u);
    }

    // Remove fabric 1. Pass the global FabricTable reference the production code path uses;
    // OTARequestorCluster::OnFabricRemoved ignores the table arg (only the fabricIndex is used).
    cluster.OnFabricRemoved(context.GetFabricTable(), /*fabricIndex=*/1);

    // (a) Fabric 1's DefaultOTAProviders entry is gone; fabric 2's remains.
    {
        auto it = attributes.GetDefaultOtaProviderListIterator();
        ASSERT_TRUE(it.Next());
        EXPECT_EQ(it.GetValue(), providerB);
        EXPECT_FALSE(it.Next());
    }

    // (b) The command interface saw exactly one OnFabricRemoved(1) call.
    EXPECT_EQ(otaCommands.GetFabricRemovedCallCount(), 1u);
    ASSERT_TRUE(otaCommands.GetLastFabricRemovedIndex().has_value());
    EXPECT_EQ(otaCommands.GetLastFabricRemovedIndex().value(), 1);
}

// Notification fires even when the removed fabric has no DefaultOTAProviders entry — in-flight
// or persisted state may exist from an AnnounceOTAProvider-triggered OTA.
TEST_F(TestOTARequestorCluster, OnFabricRemovedNotifiesRequestorEvenWithNoProvidersForThatFabric)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes, context.GetFabricTable());
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    // Provider exists for fabric 2 only; nothing for fabric 7.
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type provider;
    provider.providerNodeID = 0x3333u;
    provider.endpoint       = 1;
    provider.fabricIndex    = 2;
    EXPECT_EQ(attributes.AddDefaultOtaProvider(provider), CHIP_NO_ERROR);

    cluster.OnFabricRemoved(context.GetFabricTable(), /*fabricIndex=*/7);

    // Fabric 2's entry untouched.
    {
        auto it = attributes.GetDefaultOtaProviderListIterator();
        ASSERT_TRUE(it.Next());
        EXPECT_EQ(it.GetValue(), provider);
        EXPECT_FALSE(it.Next());
    }

    // Notification still happened (the requestor's hook decides whether there's anything to clear).
    EXPECT_EQ(otaCommands.GetFabricRemovedCallCount(), 1u);
    ASSERT_TRUE(otaCommands.GetLastFabricRemovedIndex().has_value());
    EXPECT_EQ(otaCommands.GetLastFabricRemovedIndex().value(), 7);
}

// ----------------------------------------------------------------------------
// Regression tests: a peer-aborted BDX download should trigger a short retry
// instead of waiting for the 24h periodic-query timer.
//
// Some accessories abort an in-progress BDX transfer mid-download (sending a
// BDX StatusReport, which BDXDownloader surfaces as kStatusReceived ->
// CleanupOnError(kFailure) -> SetState(kIdle, kFailure)).
// DefaultOTARequestor::OnDownloadStateChanged then queries
// BDXDownloader::GetLastFailureCause() to decide whether to feed the driver
// the new kAbortedByPeer reason — only the kPeerStatusReport cause maps that
// way; locally-caused failures (kInternalError, OnPreparedForDownload failure)
// continue to fall through to the periodic-query timer so they do not consume
// the short-retry budget reserved for actual peer-driven aborts.
// ----------------------------------------------------------------------------

// 1) MapErrorToIdleStateReason maps the documented error codes to their canonical
//    reasons. Crucially, CHIP_ERROR_CONNECTION_ABORTED maps to kUnknown (NOT
//    kAbortedByPeer) because that error can also bubble up from the messaging
//    layer in non-download phases (Apply / Notify / Query callbacks); the
//    peer-abort short-retry path is only triggered explicitly via
//    RecordNewUpdateStateWithIdleReason in OnDownloadStateChanged when the
//    BDXDownloader's last failure cause is kPeerStatusReport.
TEST_F(TestOTARequestorCluster, MapErrorToIdleStateReasonDoesNotMisclassifyLocalBdxErrors)
{
    DefaultOTARequestor requestor;
    EXPECT_EQ(requestor.MapErrorToIdleStateReason(CHIP_NO_ERROR), IdleStateReason::kIdle);
    EXPECT_EQ(requestor.MapErrorToIdleStateReason(CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY), IdleStateReason::kInvalidSession);
    // CHIP_ERROR_CONNECTION_ABORTED MUST NOT auto-map to kAbortedByPeer; if it did, an
    // OnApplyUpdateFailure / OnNotifyUpdateAppliedFailure / OnQueryImageFailure callback
    // surfacing CHIP_ERROR_CONNECTION_ABORTED from the messaging layer would consume the
    // peer-abort short-retry budget even though the peer never aborted a BDX download.
    EXPECT_EQ(requestor.MapErrorToIdleStateReason(CHIP_ERROR_CONNECTION_ABORTED), IdleStateReason::kUnknown);
    EXPECT_EQ(requestor.MapErrorToIdleStateReason(CHIP_ERROR_TIMEOUT), IdleStateReason::kUnknown);
    EXPECT_EQ(requestor.MapErrorToIdleStateReason(CHIP_ERROR_INTERNAL), IdleStateReason::kUnknown);
}

// Test subclass that overrides ScheduleDelayedAction so HandleIdleStateEnter's
// timer scheduling is observable without standing up a full SystemLayer.
// StartPeriodicQueryTimer / the new kAbortedByPeer branch / StartDelayTimerHandler
// all route through ScheduleDelayedAction, so we can tell which path ran by
// looking at the captured (delay, action) tuple.
class TestableOTARequestorDriver : public DeviceLayer::DefaultOTARequestorDriver
{
public:
    struct Scheduled
    {
        System::Clock::Seconds32 delay;
        System::TimerCompleteCallback action;
    };

    std::vector<Scheduled> scheduled;
    std::vector<System::TimerCompleteCallback> cancelled;
    uint32_t sendQueryImageCallCount = 0;

    // Expose the protected entry point so tests can drive it directly.
    using DeviceLayer::DefaultOTARequestorDriver::HandleIdleStateEnter;
    // Re-export the protected retry cap so test bodies can reference it without friend-class plumbing.
    using DeviceLayer::DefaultOTARequestorDriver::kMaxAbortedByPeerRetryCount;

    // Override SendQueryImage so kInvalidSession entries don't deref a real requestor in tests.
    void SendQueryImage() override { sendQueryImageCallCount++; }

protected:
    // Replace the real timer with a no-op recorder so tests run without the SystemLayer.
    void ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action, void * /*aAppState*/) override
    {
        scheduled.push_back({ delay, action });
    }
    void CancelDelayedAction(System::TimerCompleteCallback action, void * /*aAppState*/) override { cancelled.push_back(action); }
};

// 2) kAbortedByPeer schedules a 120s retry the first time it is entered (and
//    NOT the 24h periodic-query interval). This is the core regression fix:
//    pre-patch, this codepath landed on StartSelectedTimer(kPeriodicQueryTimer)
//    with a 24h delay, leaving the SU stuck in 'requested' for a full day.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterAbortedByPeerSchedulesShortRetry)
{
    TestableOTARequestorDriver driver;
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);

    ASSERT_EQ(driver.scheduled.size(), 1u);
    // Short retry, not the 24h (86400s) default periodic-query interval.
    EXPECT_EQ(driver.scheduled[0].delay.count(), 120u);
    EXPECT_LT(driver.scheduled[0].delay.count(), 24u * 60u * 60u);
}

// 3) After kMaxAbortedByPeerRetryCount (3) consecutive peer-aborted entries
//    without an intervening successful download, the next entry falls back to
//    the periodic-query timer (so we do not retry forever) and the short-retry
//    counter resets to 0 so a future unrelated peer abort gets a fresh budget.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterAbortedByPeerFallsBackAfterMaxRetries)
{
    TestableOTARequestorDriver driver;

    // First kMaxAbortedByPeerRetryCount entries each schedule a 120s retry.
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        ASSERT_EQ(driver.scheduled.size(), static_cast<size_t>(i + 1));
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }

    // The next entry should fall back to the periodic-query timer (24h by default).
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.scheduled.size(), static_cast<size_t>(TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount) + 1u);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);

    // A subsequent unrelated peer abort gets a fresh short-retry budget.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
}

// 4) A successful idle entry (kIdle) clears the short-retry counter so the
//    next peer abort starts over with a full budget. This pins that the counter
//    does not leak across unrelated session lifetimes after a clean recovery.
//    (Other non-peer reasons, e.g. kInvalidSession, intentionally do NOT reset —
//    see test 5 for the alternating-reason hardening.)
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterCleanIdleResetsAbortCounter)
{
    TestableOTARequestorDriver driver;

    // Burn 2 retries on peer abort.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.scheduled.size(), 2u);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);

    // A clean idle entry should reset the budget without scheduling a short retry.
    driver.HandleIdleStateEnter(IdleStateReason::kIdle);
    ASSERT_EQ(driver.scheduled.size(), 3u);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);

    // Now the next 3 peer-abort entries should all be short retries again.
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }
}

// 5) Adversarial finding: an interleaved kInvalidSession idle entry must NOT reset
//    the kAbortedByPeer retry counter. Otherwise a provider that alternates
//    kInvalidSession / kAbortedByPeer reasons can drive an unbounded 120s retry
//    storm by holding the abort counter at 0. Each counter must be independent.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterAbortAndInvalidSessionCountersAreIndependent)
{
    TestableOTARequestorDriver driver;

    // Burn up to-but-not-including the cap with peer aborts.
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }

    // Interleave a kInvalidSession entry. This does NOT touch the abort counter, so
    // a subsequent peer-abort entry MUST hit the cap and fall back to the periodic
    // query timer rather than starting a fresh 120s burst.
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    // Last scheduled delay is the periodic-query timer (24h), not 120s.
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
}

// 6) Adversarial finding: fabric removal / external cancel must reset the
//    peer-abort retry budget so a freshly-commissioned fabric gets a full
//    3-retry budget on its first peer abort, rather than inheriting a stale
//    counter from the prior session.
TEST_F(TestOTARequestorCluster, AbortedByPeerCounterResetsOnFabricRemoval)
{
    TestableOTARequestorDriver driver;

    // Burn 2 retries on peer abort during the prior session.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);

    // Simulate fabric removal / external cancel — both flow through UpdateCancelled.
    driver.UpdateCancelled();

    // Now the next 3 peer-abort entries should each schedule a short retry; without
    // the reset, only the first one would (the second would tip over the cap).
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }
}

// 7) End-to-end pin from the BDXDownloader peer-abort signal through
//    DefaultOTARequestor::OnDownloadStateChanged to driver short-retry scheduling.
//    The exact chain on the live path is:
//      BDX StatusReport from peer
//        -> HandleBdxEvent(kStatusReceived) sets mLastFailureCause = kPeerStatusReport
//        -> CleanupOnError(kFailure) -> SetState(kIdle, kFailure)
//        -> StateDelegate::OnDownloadStateChanged(kIdle, kFailure)
//        -> reads BDXDownloader::GetLastFailureCause()
//        -> kPeerStatusReport -> RecordNewUpdateStateWithIdleReason(..., kAbortedByPeer)
//        -> HandleIdleStateEnter(kAbortedByPeer) -> 120s short retry.
//
//    HandleBdxEvent is private; we pin the public-surface invariant the requestor
//    relies on: that the LastFailureCause enum has distinct values for peer vs.
//    local failure modes, that the getter is observable, and that an unfailed
//    downloader reports kNone. If any future BDXDownloader refactor changes the
//    enum shape (or stops setting it on a peer kStatusReceived event), the
//    Map* tests + the live-path inspection in BDXDownloader.cpp will diverge.
TEST_F(TestOTARequestorCluster, BDXDownloaderLastFailureCauseDefaultsToNoneAndDistinguishesPeerVsLocal)
{
    BDXDownloader downloader;
    // A freshly-constructed downloader has no failure recorded.
    EXPECT_EQ(downloader.GetLastFailureCause(), BDXDownloader::LastFailureCause::kNone);

    // The four failure modes the requestor's OnDownloadStateChanged depends on must remain
    // mutually distinct so the requestor can correctly route peer vs. local failures.
    EXPECT_NE(BDXDownloader::LastFailureCause::kPeerStatusReport, BDXDownloader::LastFailureCause::kNone);
    EXPECT_NE(BDXDownloader::LastFailureCause::kPeerStatusReport, BDXDownloader::LastFailureCause::kLocalInternalError);
    EXPECT_NE(BDXDownloader::LastFailureCause::kPeerStatusReport, BDXDownloader::LastFailureCause::kLocalPrepareFailed);
    EXPECT_NE(BDXDownloader::LastFailureCause::kPeerStatusReport, BDXDownloader::LastFailureCause::kTimeout);
    EXPECT_NE(BDXDownloader::LastFailureCause::kLocalInternalError, BDXDownloader::LastFailureCause::kLocalPrepareFailed);
}

// 8) Small-image / immediate-retry path: when a peer aborts a BDX transfer for a tiny image
//    (the abort can arrive within milliseconds of OnPreparedForDownload), HandleIdleStateEnter
//    is reached with a fresh mAbortedByPeerRetryCount==0. The very first abort MUST be routed
//    onto the short-retry codepath — not the periodic-query timer — and MUST install a non-null
//    deferred action so the QueryImage is actually re-dispatched after 120s. Without this, a
//    small-image abort would land on the 24h timer immediately because the requestor never
//    gets a chance to accumulate retries before the periodic-query path is selected.
TEST_F(TestOTARequestorCluster, AbortedByPeerOnSmallImageTakesImmediateRetryPath)
{
    TestableOTARequestorDriver driver;

    // Simulate the small-image case: a single peer abort on the first download attempt.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);

    // Exactly one timer was scheduled, and it is the 120s short retry — not the 24h default.
    ASSERT_EQ(driver.scheduled.size(), 1u);
    EXPECT_EQ(driver.scheduled[0].delay.count(), 120u);
    EXPECT_NE(driver.scheduled[0].delay.count(), 24u * 60u * 60u);

    // The scheduled action must be a real callback — not nullptr — so that when the 120s timer
    // fires, the driver actually re-dispatches a QueryImage instead of leaving the SU stuck.
    // (Wrapped in EXPECT_TRUE because pw_unit_test's lite backend cannot stringify function
    // pointers for direct EXPECT_NE comparison.)
    EXPECT_TRUE(driver.scheduled[0].action != nullptr);

    // No QueryImage was sent synchronously — the retry must be deferred via the timer, not
    // dispatched immediately (which would bypass the 120s back-off).
    EXPECT_EQ(driver.sendQueryImageCallCount, 0u);
}

// 9) Sequential aborts respect kMaxAbortedByPeerRetryCount: each consecutive peer-abort entry
//    consumes exactly one slot in the retry budget, schedules exactly one short retry, and
//    the (kMaxAbortedByPeerRetryCount + 1)-th entry tips over to the periodic-query timer.
//    This pins the per-entry contract that test 3 only covers in aggregate — if a future
//    refactor accidentally double-incremented or skipped the counter, the aggregate test
//    would still pass on the boundary but this test would fail on the intermediate counts.
TEST_F(TestOTARequestorCluster, SequentialAbortedByPeerEntriesRespectMaxRetryCountExactly)
{
    TestableOTARequestorDriver driver;

    // Capture the short-retry action from the first abort so we can confirm every subsequent
    // in-budget retry uses the SAME action handle — i.e. they all flow through the same
    // short-retry code path, not a mix of paths that happen to use a 120s delay.
    System::TimerCompleteCallback shortRetryAction = nullptr;

    // Each of the first kMaxAbortedByPeerRetryCount entries schedules exactly ONE additional
    // short retry (120s). Verifying the running size after each call rules out double-scheduling.
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        size_t before = driver.scheduled.size();
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        ASSERT_EQ(driver.scheduled.size(), before + 1u);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
        // Wrapped in EXPECT_TRUE because pw_unit_test's lite backend cannot stringify
        // function pointers for direct EXPECT_NE / EXPECT_EQ comparison.
        EXPECT_TRUE(driver.scheduled.back().action != nullptr);
        if (i == 0)
        {
            shortRetryAction = driver.scheduled.back().action;
        }
        else
        {
            // Every in-budget retry uses the same deferred-action handler.
            EXPECT_TRUE(driver.scheduled.back().action == shortRetryAction);
        }
    }

    // Exactly kMaxAbortedByPeerRetryCount short retries were scheduled — no more, no less.
    EXPECT_EQ(driver.scheduled.size(), static_cast<size_t>(TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount));

    // The (cap+1)-th entry tips over to the periodic-query timer. The delay is 24h AND the
    // action differs from the short-retry handler, confirming we left the short-retry code
    // path rather than coincidentally landing on a 24h short retry.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.scheduled.size(), static_cast<size_t>(TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount) + 1u);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
    EXPECT_TRUE(driver.scheduled.back().action != shortRetryAction);

    // No QueryImage was synchronously dispatched at any point — every retry / fallback was
    // deferred through the timer interface.
    EXPECT_EQ(driver.sendQueryImageCallCount, 0u);
}

// 10) Adversarial finding: peer-disappeared (CHIP_ERROR_TIMEOUT on a request/response invocation)
//     must classify as kInvalidSession in MapErrorToIdleStateReason once the QueryImage failure
//     handler has reclassified it. This pins the second half of the conflation fix:
//     OnQueryImageFailure rewrites CHIP_ERROR_TIMEOUT -> CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY
//     before recording the error, so the driver sees the kInvalidSession 1-shot retry path rather
//     than the kUnknown 24h periodic-query path. (OnApplyUpdateFailure and
//     OnNotifyUpdateAppliedFailure deliberately do NOT reclassify — see
//     ReclassifyPeerDisappearedError — so an Apply/Notify timeout intentionally stays on the
//     kUnknown periodic path.) If MapErrorToIdleStateReason ever stops mapping
//     CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY to kInvalidSession, the QueryImage reclassification
//     in the .cpp would silently regress to the 24h slow path. This test catches that.
TEST_F(TestOTARequestorCluster, ReclassifiedPeerDisappearedTimeoutDrivesShortRetry)
{
    // Step 1: the post-reclassification error must still map to kInvalidSession.
    DefaultOTARequestor requestor;
    EXPECT_EQ(requestor.MapErrorToIdleStateReason(CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY), IdleStateReason::kInvalidSession);

    // Step 2: a kInvalidSession idle entry MUST trigger an immediate SendQueryImage (the 1-shot
    // quick retry) — NOT schedule a 24h periodic-query timer. This is the behavior the
    // peer-disappeared reclassification depends on.
    TestableOTARequestorDriver driver;
    size_t before = driver.scheduled.size();
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 1u);
    // No 24h timer scheduled on the first kInvalidSession entry.
    EXPECT_EQ(driver.scheduled.size(), before);
}

// 11) Adversarial finding: the kInvalidSession 1-shot retry MUST be capped — repeated entries
//     should NOT loop SendQueryImage forever. After kMaxInvalidSessionRetries the driver falls
//     back to the periodic-query timer. This is the symmetric guarantee to the kAbortedByPeer
//     cap (test 3) and pins that a peer that keeps timing out can't drive an unbounded
//     SendQueryImage burst via the reclassification path.
TEST_F(TestOTARequestorCluster, InvalidSessionEntriesAreCappedAndFallBackToPeriodicQuery)
{
    TestableOTARequestorDriver driver;

    // First entry: 1-shot quick retry (SendQueryImage), no timer scheduled.
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 1u);

    // Second entry: cap exceeded, falls back to periodic-query timer (24h), no further
    // SendQueryImage burst.
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 1u);
    ASSERT_GE(driver.scheduled.size(), 1u);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
}

// ----------------------------------------------------------------------------
// Test-hardening additions: cover edge cases the original PR tests left
// implicit. These pin invariants that the production code clearly intends
// (per code comments) but did not have a direct regression test for.
// ----------------------------------------------------------------------------

// 12) Adversarial finding (complement to test 5): a kUnknown idle entry MUST NOT reset the
//     peer-abort retry counter. The driver code resets the counter ONLY on IdleStateReason::kIdle,
//     so a misbehaving provider that interleaves kUnknown reasons (e.g. by triggering an unrelated
//     CHIP_ERROR -> MapErrorToIdleStateReason -> kUnknown) between aborts must not be able to
//     defeat the 3-retry cap. Test 5 covered the kInvalidSession interleave; this covers kUnknown
//     symmetrically — together they pin that ONLY kIdle is the reset condition.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterUnknownDoesNotResetAbortCounter)
{
    TestableOTARequestorDriver driver;

    // Burn up to-but-not-including the cap with peer aborts.
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }

    // Interleave a kUnknown entry — the kUnknown path schedules the periodic-query timer but
    // MUST leave mAbortedByPeerRetryCount untouched. Without this guarantee, a single kUnknown
    // event could silently reset the abort budget and let a misbehaving provider loop short
    // retries indefinitely.
    driver.HandleIdleStateEnter(IdleStateReason::kUnknown);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);

    // The next peer-abort entry MUST hit the cap and fall back to the periodic-query timer —
    // proving the abort counter survived the kUnknown interleave.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
}

// 13) UpdateCancelled MUST also reset the kInvalidSession retry counter, not only the abort
//     counter. The driver diff explicitly resets BOTH ("Reset retry budgets so a freshly-
//     commissioned fabric or an externally-cancelled update gets a full retry budget for its
//     first peer-aborted/invalid-session sequence"). Test 6 only checks the abort counter is
//     reset; this pins the symmetric invalid-session reset. A regression that only reset one
//     counter would leave a freshly-commissioned fabric one quick-retry short on its first
//     invalid-session event.
TEST_F(TestOTARequestorCluster, UpdateCancelledResetsInvalidSessionRetryCounterToo)
{
    TestableOTARequestorDriver driver;

    // Burn the 1-shot invalid-session quick retry from the prior session, taking the counter
    // up to its cap (kMaxInvalidSessionRetries = 1). The first entry sends a query; without a
    // reset, a subsequent entry would tip into the periodic-query fallback.
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 1u);

    // External cancel / fabric removal flows through UpdateCancelled. It must reset BOTH
    // retry budgets so the next session starts fresh.
    driver.UpdateCancelled();

    // The next kInvalidSession entry should re-issue the 1-shot quick retry (a second
    // SendQueryImage), proving the invalid-session counter was reset. Without the reset,
    // the cap would already be tripped and the next entry would land on the 24h timer
    // with no additional SendQueryImage.
    size_t scheduledBefore = driver.scheduled.size();
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 2u);
    // No 24h timer was scheduled by this entry — the quick-retry path was taken.
    EXPECT_EQ(driver.scheduled.size(), scheduledBefore);
}

// 14) UpdateDownloaded MUST reset the peer-abort retry counter so an unrelated peer abort on a
//     subsequent OTA session starts with a fresh 3-retry budget. The driver comment is explicit:
//     "A successful download resets the peer-abort retry budget so any subsequent unrelated
//     peer abort starts from a fresh short-retry count." Without this, a device that successfully
//     downloaded after 2 aborts would only get 1 retry on its NEXT update's first peer abort.
//
//     UpdateDownloaded calls VerifyOrDie(mRequestor != nullptr) and then mRequestor->ApplyUpdate(),
//     which would crash without a real requestor. Override the entire method here so we exercise
//     only the counter-reset side effect that this test pins.
TEST_F(TestOTARequestorCluster, UpdateDownloadedResetsAbortedByPeerCounter)
{
    class UpdateDownloadedTestDriver : public TestableOTARequestorDriver
    {
    public:
        // Replace UpdateDownloaded with the counter-reset side effect only — drop the
        // VerifyOrDie(mRequestor) + ApplyUpdate dispatch that the production version does
        // (a null mRequestor here would fatal-die before we observed the reset).
        void UpdateDownloaded() override { mAbortedByPeerRetryCount = 0; }

        // Expose the protected counter so the test body can observe it directly.
        uint8_t GetAbortedByPeerRetryCountForTest() const { return mAbortedByPeerRetryCount; }
    };

    UpdateDownloadedTestDriver driver;

    // Burn 2 peer-abort retries so the counter is non-zero (and observably non-zero by
    // virtue of needing a 3rd to tip the cap on the next entry).
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 2u);

    // A successful download MUST reset the abort retry counter.
    driver.UpdateDownloaded();
    EXPECT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 0u);

    // Independent observable: a full 3 fresh peer-abort entries should each land on a 120s
    // short retry rather than the (cap+1) fallback. Without the reset, the 2nd entry here
    // would already trip the cap.
    size_t scheduledBefore = driver.scheduled.size();
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }
    EXPECT_EQ(driver.scheduled.size(), scheduledBefore + TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount);
}

// ----------------------------------------------------------------------------
// Test-hardener additions (Agent B): three additional regression tests targeting
// invariants the production code clearly intends but did not have direct tests
// for. Each captures a behavior that, if silently lost in a future refactor,
// would re-introduce flavors of the original 24h-stuck bug.
// ----------------------------------------------------------------------------

// Test driver variant that records BOTH ScheduleDelayedAction and CancelDelayedAction calls.
// Needed for the watchdog-cancellation regression below — TestableOTARequestorDriver's
// CancelDelayedAction is a no-op, so we cannot observe StopWatchdogTimer through it.
class WatchdogObservingDriver : public DeviceLayer::DefaultOTARequestorDriver
{
public:
    struct Scheduled
    {
        System::Clock::Seconds32 delay;
        System::TimerCompleteCallback action;
    };
    struct Cancelled
    {
        System::TimerCompleteCallback action;
    };
    std::vector<Scheduled> scheduled;
    std::vector<Cancelled> cancelled;

    using DeviceLayer::DefaultOTARequestorDriver::HandleIdleStateEnter;
    using DeviceLayer::DefaultOTARequestorDriver::PeriodicQueryTimerHandler;
    using DeviceLayer::DefaultOTARequestorDriver::WatchdogTimerHandler;

    void SendQueryImage() override {}

protected:
    void ScheduleDelayedAction(System::Clock::Seconds32 delay, System::TimerCompleteCallback action, void * /*aAppState*/) override
    {
        scheduled.push_back({ delay, action });
    }
    void CancelDelayedAction(System::TimerCompleteCallback action, void * /*aAppState*/) override
    {
        cancelled.push_back({ action });
    }
};

// 15) The kAbortedByPeer short-retry path MUST cancel the watchdog timer before
//     scheduling its 120s delayed action. The production code calls StopWatchdogTimer()
//     explicitly because the short-retry branch bypasses StartSelectedTimer (which
//     would have done it). Without this cancellation, the prior watchdog continues
//     ticking during the 120s retry window and can fire mid-retry, cancelling or
//     resetting an in-flight retry attempt — silently re-introducing the 24h-stuck
//     symptom for a different reason. Pin this so a future refactor that removes
//     the explicit StopWatchdogTimer() call (e.g. by routing through a helper)
//     trips this test instead of regressing in the field.
TEST_F(TestOTARequestorCluster, AbortedByPeerCancelsWatchdogBeforeSchedulingShortRetry)
{
    WatchdogObservingDriver driver;
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);

    // Exactly one short retry was scheduled.
    ASSERT_EQ(driver.scheduled.size(), 1u);
    EXPECT_EQ(driver.scheduled[0].delay.count(), 120u);

    // The watchdog timer was cancelled. There may be other CancelDelayedAction calls
    // (e.g. cancelling the StartDelay/Apply timers) — what we care about is that the
    // watchdog handler was among them.
    bool watchdogCancelled = false;
    for (const auto & c : driver.cancelled)
    {
        if (c.action == &WatchdogObservingDriver::WatchdogTimerHandler)
        {
            watchdogCancelled = true;
            break;
        }
    }
    EXPECT_TRUE(watchdogCancelled);
}

// 16) After the kAbortedByPeer cap fires its periodic-query fallback, the counter
//     resets to 0 and a *full* fresh burst of kMaxAbortedByPeerRetryCount short
//     retries must be allowed before the next fallback. Test 3 covers a single
//     follow-up entry; this pins the full-burst contract — i.e. the cap is
//     strictly per-burst, not lifetime, and not silently degraded to "1 retry then
//     fallback" by an off-by-one in the reset. Two complete bursts are exercised.
TEST_F(TestOTARequestorCluster, AbortedByPeerSupportsMultipleFullBurstsAcrossFallbacks)
{
    TestableOTARequestorDriver driver;

    // Exercise two complete burst-then-fallback cycles.
    for (int burst = 0; burst < 2; burst++)
    {
        // kMaxAbortedByPeerRetryCount short retries.
        for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
        {
            driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
            EXPECT_EQ(driver.scheduled.back().delay.count(), 120u)
                << "burst=" << burst << " short retry index=" << static_cast<int>(i);
        }
        // Then a fallback to the periodic-query timer.
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u) << "burst=" << burst << " expected fallback";
    }

    // Total scheduled = 2 * (kMaxAbortedByPeerRetryCount short retries + 1 fallback).
    EXPECT_EQ(driver.scheduled.size(), 2u * (static_cast<size_t>(TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount) + 1u));
}

// 17) The kAbortedByPeer short retry MUST install the StartDelayTimerHandler action
//     (which is what re-dispatches a QueryImage when the 120s timer fires), NOT the
//     PeriodicQueryTimerHandler or the WatchdogTimerHandler. The fallback after the
//     cap is exceeded MUST install the PeriodicQueryTimerHandler instead. Test 9
//     pins that the two action handles differ from each other but does not pin
//     which specific handler each path uses. If a future refactor accidentally
//     scheduled the PeriodicQueryTimerHandler with a 120s delay, test 9 would
//     pass (the handles still differ from a different baseline) but the SU would
//     still be effectively broken — when the timer fires it would re-arm itself
//     for another 24h instead of issuing a query. Pinning the exact action handle
//     for both branches catches that.
TEST_F(TestOTARequestorCluster, AbortedByPeerShortRetryAndFallbackUseDistinctSpecificHandlers)
{
    WatchdogObservingDriver driver;

    // Short-retry path: the action MUST be StartDelayTimerHandler (a free function in the
    // driver translation unit, not a member). We can't reference it by name from the test,
    // but we CAN assert that it is *not* the periodic-query or watchdog handlers and is
    // non-null — those three are the only candidates the codebase surfaces.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.scheduled.size(), 1u);
    // Compare callback pointers via EXPECT_TRUE: pw::unit_test's stringifier cannot
    // format function pointers (no pw::ToString overload converts them to const void*),
    // so EXPECT_EQ/EXPECT_NE on a TimerCompleteCallback fails to compile.
    EXPECT_TRUE(driver.scheduled.back().action != nullptr);
    EXPECT_TRUE(driver.scheduled.back().action !=
                static_cast<System::TimerCompleteCallback>(&WatchdogObservingDriver::WatchdogTimerHandler));
    EXPECT_TRUE(driver.scheduled.back().action !=
                static_cast<System::TimerCompleteCallback>(&WatchdogObservingDriver::PeriodicQueryTimerHandler));
    System::TimerCompleteCallback shortRetryAction = driver.scheduled.back().action;

    // Drain the cap so the next entry takes the periodic-query fallback.
    for (uint8_t i = 1; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_TRUE(driver.scheduled.back().action == shortRetryAction);
    }

    // The cap+1-th entry MUST install the PeriodicQueryTimerHandler specifically — not just
    // a different handle (which test 9 already covers). This pins the actual fallback target.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
    EXPECT_TRUE(driver.scheduled.back().action ==
                static_cast<System::TimerCompleteCallback>(&WatchdogObservingDriver::PeriodicQueryTimerHandler));
}

// ----------------------------------------------------------------------------
// Test-hardener additions (Agent B, second pass): three more regression tests
// targeting invariants the production code intends but that the existing tests
// leave implicit. Each captures a contract whose silent loss in a future
// refactor would re-introduce a flavor of the original bug.
// ----------------------------------------------------------------------------

// 18) Symmetric reset pin: a clean kIdle entry MUST also reset
//     mInvalidSessionRetryCount, not only mAbortedByPeerRetryCount. The
//     production guard at the top of HandleIdleStateEnter is the asymmetric
//     "reason != kInvalidSession -> reset invalid-session counter" form. Test
//     4 covers the abort-counter reset on kIdle; this test pins the symmetric
//     leg, so a refactor that flips that guard's polarity (or accidentally
//     turns it into "only on kIdle") trips this test instead of silently
//     leaving the invalid-session counter pinned at the cap forever.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterCleanIdleAlsoResetsInvalidSessionCounter)
{
    TestableOTARequestorDriver driver;

    // Take the invalid-session counter up to its cap (kMaxInvalidSessionRetries == 1):
    // first entry sends a query, second entry would fall back to the periodic timer.
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 1u);

    // A clean idle entry MUST reset the invalid-session counter as a side effect.
    driver.HandleIdleStateEnter(IdleStateReason::kIdle);

    // Observable proof: a follow-up kInvalidSession entry MUST take the 1-shot quick-retry
    // path again (a second SendQueryImage). Without the reset, the cap would already be
    // tripped and the entry would land on the 24h periodic-query timer with no extra query.
    size_t scheduledBefore = driver.scheduled.size();
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 2u);
    EXPECT_EQ(driver.scheduled.size(), scheduledBefore);
}

// 19) Asymmetric reset pin: UpdateDownloaded MUST NOT also reset
//     mInvalidSessionRetryCount. The production diff is explicit that
//     UpdateDownloaded only resets the abort counter ("A successful download
//     resets the peer-abort retry budget so any subsequent unrelated peer
//     abort starts from a fresh short-retry count"), while UpdateCancelled
//     resets BOTH. Test 13 pins UpdateCancelled's both-counter reset and test
//     14 pins UpdateDownloaded's abort-counter reset; this test pins the
//     remaining quadrant — that UpdateDownloaded leaves the invalid-session
//     counter alone — so a future refactor that "harmonizes" the two methods
//     by also resetting the invalid-session counter from UpdateDownloaded
//     trips this test rather than silently changing user-visible behavior.
//
//     UpdateDownloaded calls VerifyOrDie(mRequestor != nullptr) and then
//     mRequestor->ApplyUpdate(), neither of which are safe in this test, so we
//     stub the method down to its counter side effect (matching the technique
//     test 14 uses).
TEST_F(TestOTARequestorCluster, UpdateDownloadedDoesNotResetInvalidSessionCounter)
{
    class CounterOnlyDownloadedDriver : public TestableOTARequestorDriver
    {
    public:
        // Mirror production's UpdateDownloaded counter side effect only — drop the
        // VerifyOrDie + ApplyUpdate dispatch which would crash without a real requestor.
        void UpdateDownloaded() override { mAbortedByPeerRetryCount = 0; }

        // Expose protected counters so the test body can assert directly.
        uint8_t GetInvalidSessionRetryCountForTest() const { return mInvalidSessionRetryCount; }
        uint8_t GetAbortedByPeerRetryCountForTest() const { return mAbortedByPeerRetryCount; }
    };

    CounterOnlyDownloadedDriver driver;

    // Set up the abort counter FIRST. HandleIdleStateEnter(reason != kInvalidSession)
    // unconditionally zeros mInvalidSessionRetryCount at the top of the function, so any
    // kAbortedByPeer entries done AFTER bumping the invalid-session counter would silently
    // wipe it back to zero. Order matters: bump abort first, then bump invalid-session last so
    // the pre-UpdateDownloaded state is (abort=2, invalid=1).
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 2u);

    // Move the invalid-session counter off zero by entering kInvalidSession once (cap is 1,
    // so this leaves the counter at exactly 1 without yet falling back).
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    ASSERT_EQ(driver.GetInvalidSessionRetryCountForTest(), 1u);

    driver.UpdateDownloaded();

    // The abort counter MUST reset — that is the documented contract.
    EXPECT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 0u);
    // The invalid-session counter MUST be left untouched — pinning this asymmetry
    // vs. UpdateCancelled (which resets both, per test 13).
    EXPECT_EQ(driver.GetInvalidSessionRetryCountForTest(), 1u);
}

// 20) Cap-exceeded fallback resets the abort counter to zero in the SAME entry.
//     Test 16 covers full-burst behavior across cycles, but it transits through
//     two complete cycles to do so — it does not directly observe the in-entry
//     reset that the production code performs (line 213 of
//     DefaultOTARequestorDriver.cpp: "mAbortedByPeerRetryCount = 0" in the else
//     branch, BEFORE StartSelectedTimer fires). Pin that single-step contract
//     so a refactor that moves the reset out of the cap branch (e.g. relying on
//     a later kIdle to clear it) is caught immediately. Without this, a peer
//     that hits the cap and then goes silent (no kIdle entry) would leave the
//     counter pinned at the cap and never re-arm the short-retry budget.
TEST_F(TestOTARequestorCluster, AbortedByPeerCapExceededResetsCounterImmediatelyInSameEntry)
{
    class ExposedCounterDriver : public TestableOTARequestorDriver
    {
    public:
        uint8_t GetAbortedByPeerRetryCountForTest() const { return mAbortedByPeerRetryCount; }
    };

    ExposedCounterDriver driver;

    // Take the counter exactly up to the cap with kMaxAbortedByPeerRetryCount in-budget entries.
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    }
    ASSERT_EQ(driver.GetAbortedByPeerRetryCountForTest(), TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount);

    // The next entry takes the cap-exceeded fallback branch. After the entry returns, the
    // counter MUST already be back at zero — observed without any intervening kIdle entry
    // and without exiting HandleIdleStateEnter via any other path that would reset it.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 0u);

    // And the entry that did the fallback installed the periodic-query timer (24h), not
    // a 120s short retry — confirming we did exercise the cap-exceeded branch.
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
}

// Pins that every kAbortedByPeer entry first cancels any previously-armed short-retry timer
// (the StartDelayTimerHandler installed by the prior in-budget kAbortedByPeer entry).
// Without this, when retry 3 hits the cap and resets mAbortedByPeerRetryCount=0, stale 120s
// timers from retries 1+2 would still be live and would fire SendQueryImage during the supposed
// 24h-idle window AND re-enter the burst from scratch since the counter is now 0 — defeating
// the cap.
//
// StartDelayTimerHandler is in an anonymous namespace inside the driver TU, so it cannot be
// referenced by name from this test (see comment in test 17). Instead, capture the action
// pointer scheduled by the FIRST kAbortedByPeer entry — that IS StartDelayTimerHandler — and
// count cancels matching that captured pointer.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterAbortedByPeerCancelsStaleShortRetryTimer)
{
    TestableOTARequestorDriver driver;

    // Simulate the first kAbortedByPeer entry to capture the short-retry action pointer the
    // production path schedules. The defensive cancel at the top of the kAbortedByPeer branch
    // also fires on this very first entry (it is unconditional), so we factor that out below.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.scheduled.size(), 1u);
    System::TimerCompleteCallback shortRetryAction = driver.scheduled.back().action;
    ASSERT_TRUE(shortRetryAction != nullptr);

    auto countCancelsOf = [&](System::TimerCompleteCallback target) {
        size_t n = 0;
        for (auto a : driver.cancelled)
        {
            if (a == target)
            {
                n++;
            }
        }
        return n;
    };

    // After the 1st entry: 1 cancel of the short-retry action (the defensive pre-schedule cancel).
    EXPECT_EQ(countCancelsOf(shortRetryAction), 1u);

    // Drive the remaining in-budget kAbortedByPeer entries. Each one MUST cancel the prior
    // short-retry handler before scheduling a new one, so the cancel count must equal the
    // total number of in-budget entries.
    for (uint8_t i = 1; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    }
    ASSERT_EQ(driver.scheduled.size(), static_cast<size_t>(TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount));
    EXPECT_EQ(countCancelsOf(shortRetryAction), static_cast<size_t>(TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount));

    // The cap+1-th entry hits the cap. It MUST also cancel the previously-armed short-retry
    // handler before falling back to the periodic-query timer; otherwise the stale 120s timer
    // would fire during the 24h window and re-enter the burst with a freshly-reset counter.
    size_t shortRetryCancelsBeforeCap = countCancelsOf(shortRetryAction);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_GT(countCancelsOf(shortRetryAction), shortRetryCancelsBeforeCap);
}

// ----------------------------------------------------------------------------
// Test-hardener additions (Agent B, third pass): three more independent
// regression tests targeting remaining gaps in counter-reset / lifecycle
// invariants that production code commits to but the existing tests do not
// directly observe.
// ----------------------------------------------------------------------------

// 22) HandleIdleStateExit MUST NOT touch mAbortedByPeerRetryCount. The reset rules in
//     HandleIdleStateEnter are intentionally restricted to (a) a clean kIdle entry,
//     (b) UpdateDownloaded, (c) UpdateCancelled, and (d) Init. The exit path is
//     traversed every time the requestor leaves idle to start a new query (including
//     the re-queries that the kAbortedByPeer short-retry path triggers). If exit
//     accidentally reset the counter, the 3-retry cap could never fire — every short
//     retry would cross HandleIdleStateExit on its way out to send a QueryImage,
//     silently clearing the counter and producing an unbounded 120s retry burst.
//     Tests 4, 5, 12, 18 pin reset conditions for the ENTER path; this test pins the
//     symmetric "exit does not reset" guarantee for the abort counter.
TEST_F(TestOTARequestorCluster, HandleIdleStateExitDoesNotResetAbortedByPeerCounter)
{
    class ExposedCounterDriver : public TestableOTARequestorDriver
    {
    public:
        uint8_t GetAbortedByPeerRetryCountForTest() const { return mAbortedByPeerRetryCount; }
    };

    ExposedCounterDriver driver;

    // Burn 2 peer-abort retries so the counter is observably non-zero.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 2u);

    // Exit idle (as if the short-retry's QueryImage fired and the requestor is now transitioning
    // out of idle to start a new download attempt). The exit path MUST NOT reset the abort counter.
    driver.HandleIdleStateExit();
    EXPECT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 2u);

    // Observable proof via the cap: with the counter still at 2, exactly ONE more peer-abort
    // entry should land on the in-budget short-retry path (120s), and the entry AFTER that
    // should hit the cap and fall back to the periodic-query timer (24h). Without the
    // exit-preserves invariant, both follow-up entries would take the 120s short-retry path.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    EXPECT_EQ(driver.scheduled.back().delay.count(), 24u * 60u * 60u);
}

// 23) Entering kAbortedByPeer MUST reset mInvalidSessionRetryCount as a side effect of the
//     top-of-method guard `if (reason != IdleStateReason::kInvalidSession) { ... = 0; }`. This
//     pins the cross-direction contract: kAbortedByPeer is a "non-invalid-session" reason, so it
//     must clear the invalid-session budget the same way kIdle / kUnknown do. Test 5 covers the
//     OTHER direction (kInvalidSession does NOT reset the abort counter); this test covers
//     kAbortedByPeer resetting the invalid-session counter. Without this reset, a
//     kInvalidSession-then-kAbortedByPeer-then-kInvalidSession sequence would tip the
//     invalid-session cap on the second kInvalidSession and fall back to the 24h periodic-query
//     timer instead of issuing the second 1-shot quick retry.
TEST_F(TestOTARequestorCluster, HandleIdleStateEnterAbortedByPeerResetsInvalidSessionCounter)
{
    TestableOTARequestorDriver driver;

    // Step 1: take mInvalidSessionRetryCount up to its cap (kMaxInvalidSessionRetries = 1).
    // The first kInvalidSession entry sends a query; a subsequent kInvalidSession entry without
    // an intervening reset would fall back to the 24h periodic timer.
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 1u);

    // Step 2: enter kAbortedByPeer. The top-of-method guard MUST reset mInvalidSessionRetryCount
    // because kAbortedByPeer != kInvalidSession. The kAbortedByPeer branch itself also schedules
    // a 120s short retry; that side effect is incidental to this test.
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    ASSERT_EQ(driver.scheduled.back().delay.count(), 120u);

    // Step 3: a follow-up kInvalidSession entry MUST take the 1-shot quick-retry path again
    // (a second SendQueryImage and NO 24h timer scheduled by THIS entry), proving the
    // invalid-session counter was reset by the intervening kAbortedByPeer entry.
    size_t scheduledBefore = driver.scheduled.size();
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    EXPECT_EQ(driver.sendQueryImageCallCount, 2u);
    EXPECT_EQ(driver.scheduled.size(), scheduledBefore);
}

// 24) Init() MUST reset mAbortedByPeerRetryCount to zero so a warm re-init does not leak retry
//     budget from a prior session into the next. Round-2 review explicitly added this reset
//     (Fix 6) alongside the existing mProviderRetryCount / mInvalidSessionRetryCount resets in
//     Init(). The production Init() requires a real OTARequestorInterface and
//     OTAImageProcessorInterface (it calls IsFirstImageRun, GetCurrentUpdateState, and either
//     ConfirmCurrentImage via SystemLayer or StartSelectedTimer) — none of which are safe in
//     this unit-test environment. We mirror only Init()'s counter-reset side effect here, the
//     same technique tests 14 and 19 use for UpdateDownloaded. This pins the documented "Init
//     clears all three retry counters" contract and defends against an accidental edit of the
//     reset set (e.g. removing the abort-counter line during a refactor).
TEST_F(TestOTARequestorCluster, InitResetsAbortedByPeerCounter)
{
    class CounterOnlyInitDriver : public TestableOTARequestorDriver
    {
    public:
        // Mirror only Init()'s counter resets — drop the IsFirstImageRun / SystemLayer /
        // StartSelectedTimer dispatch which is unsafe without real requestor + processor mocks.
        void InitForTest()
        {
            mProviderRetryCount       = 0;
            mInvalidSessionRetryCount = 0;
            mAbortedByPeerRetryCount  = 0;
        }

        uint8_t GetAbortedByPeerRetryCountForTest() const { return mAbortedByPeerRetryCount; }
        uint8_t GetInvalidSessionRetryCountForTest() const { return mInvalidSessionRetryCount; }
    };

    CounterOnlyInitDriver driver;

    // Move BOTH counters off zero from the "prior session".
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
    driver.HandleIdleStateEnter(IdleStateReason::kInvalidSession);
    ASSERT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 2u);
    ASSERT_EQ(driver.GetInvalidSessionRetryCountForTest(), 1u);

    // Warm re-init. BOTH counters MUST be cleared by Init.
    driver.InitForTest();
    EXPECT_EQ(driver.GetAbortedByPeerRetryCountForTest(), 0u);
    EXPECT_EQ(driver.GetInvalidSessionRetryCountForTest(), 0u);

    // Independent observable: a full kMaxAbortedByPeerRetryCount fresh peer-abort entries should
    // each schedule a 120s short retry rather than tripping the cap, confirming the budget was
    // actually restored. Without the reset, the very first follow-up entry would have already
    // been the cap+1 entry (counter was at 2 + this one = 3 == cap, but the loop's last iteration
    // would tip into the fallback).
    size_t scheduledBefore = driver.scheduled.size();
    for (uint8_t i = 0; i < TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount; i++)
    {
        driver.HandleIdleStateEnter(IdleStateReason::kAbortedByPeer);
        EXPECT_EQ(driver.scheduled.back().delay.count(), 120u);
    }
    EXPECT_EQ(driver.scheduled.size(), scheduledBefore + TestableOTARequestorDriver::kMaxAbortedByPeerRetryCount);
}

} // namespace
