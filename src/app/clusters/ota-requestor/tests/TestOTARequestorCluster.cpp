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

#include <app/clusters/ota-requestor/DefaultOTARequestorEventGenerator.h>
#include <app/clusters/ota-requestor/OTARequestorAttributes.h>
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

    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType GetLastAnnounceCommandPayload() const
    {
        return mLastAnnounceCommandPayload;
    }

private:
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType mLastAnnounceCommandPayload;
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorCluster, AttributeListTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsGeneratedCommandsListEqualTo(cluster, {}));
}

TEST_F(TestOTARequestorCluster, EventInfoTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType forwarded_payload =
        otaCommands.GetLastAnnounceCommandPayload();
    EXPECT_EQ(forwarded_payload.providerNodeID, static_cast<NodeId>(1234));
    EXPECT_EQ(forwarded_payload.vendorID, static_cast<VendorId>(4321));
    EXPECT_EQ(forwarded_payload.announcementReason, OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable);
    EXPECT_EQ(forwarded_payload.endpoint, 5);
}

TEST_F(TestOTARequestorCluster, AnnounceOtaProviderCommandInvalidMetadataTest)
{
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status, // NOLINT(bugprone-unchecked-optional-access)
              DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::ConstraintError));
}

TEST_F(TestOTARequestorCluster, ReadAttributesTest)
{
    chip::Testing::TestServerClusterContext context;
    MockOtaCommands otaCommands;
    OTARequestorAttributes attributes;
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();
    ASSERT_FALSE(eventsGenerator.GetNextEvent().has_value());

    VersionApplied::Type event{ 0x12345678u, 0xABCDu };
    EXPECT_EQ(cluster.GenerateVersionAppliedEvent(event), CHIP_NO_ERROR);
    auto generatedEvent = eventsGenerator.GetNextEvent();
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);

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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();
    ASSERT_FALSE(eventsGenerator.GetNextEvent().has_value());

    // Generate an event with null ProgressPercent and PlatformCode.
    DownloadError::Type event{ 0x12345678u, 0x123456789ABCDEF0u, DataModel::NullNullable, DataModel::NullNullable };
    EXPECT_EQ(cluster.GenerateDownloadErrorEvent(event), CHIP_NO_ERROR);
    auto generatedEvent = eventsGenerator.GetNextEvent();
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
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
    OTARequestorCluster cluster(kTestEndpointId, otaCommands, attributes);

    Testing::LogOnlyEvents & eventsGenerator = context.EventsGenerator();

    DownloadError::Type event{ 0x12345678u, 0x123456789ABCDEF0u, DataModel::NullNullable, DataModel::NullNullable };
    EXPECT_NE(cluster.GenerateDownloadErrorEvent(event), CHIP_NO_ERROR);
    EXPECT_FALSE(eventsGenerator.GetNextEvent().has_value());
}

} // namespace
