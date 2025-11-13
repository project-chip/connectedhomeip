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

#include <app/clusters/ota-requestor/OTARequestorCluster.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/data-model-provider/tests/ReadTesting.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/data-model/Nullable.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/OtaSoftwareUpdateRequestor/ClusterId.h>
#include <clusters/OtaSoftwareUpdateRequestor/Commands.h>
#include <clusters/OtaSoftwareUpdateRequestor/Enums.h>
#include <clusters/OtaSoftwareUpdateRequestor/Events.h>
#include <clusters/OtaSoftwareUpdateRequestor/Metadata.h>
#include <clusters/OtaSoftwareUpdateRequestor/Structs.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

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

struct MockOtaRequestor : public OTARequestorInterface
{
public:
    ~MockOtaRequestor() = default;

    void Reset(void) override {}
    void
    HandleAnnounceOTAProvider(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                              const OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType & commandData) override
    {
        mLastAnnounceCommandPayload = commandData;
    }
    CHIP_ERROR TriggerImmediateQuery(FabricIndex fabricIndex = kUndefinedFabricIndex) override { return CHIP_NO_ERROR; }
    void TriggerImmediateQueryInternal() override {}
    void DownloadUpdate() override {}
    void DownloadUpdateDelayedOnUserConsent() override {}
    void ApplyUpdate() override {}
    void NotifyUpdateApplied() override {}
    CHIP_ERROR GetUpdateStateProgressAttribute(EndpointId endpointId, DataModel::Nullable<uint8_t> & progress) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetUpdateStateAttribute(EndpointId endpointId, OTAUpdateStateEnum & state) override { return CHIP_NO_ERROR; }
    OTAUpdateStateEnum GetCurrentUpdateState() override { return OTAUpdateStateEnum::kIdle; }
    DataModel::Nullable<uint8_t> GetCurrentUpdateStateProgress() override { return mUpdateStateProgress; }
    uint32_t GetTargetVersion() override { return 0; }
    void CancelImageUpdate() override {}
    CHIP_ERROR ClearDefaultOtaProviderList(FabricIndex fabricIndex) override { return CHIP_NO_ERROR; }
    void SetCurrentProviderLocation(ProviderLocationType providerLocation) override {}
    void SetMetadataForProvider(chip::ByteSpan metadataForProvider) override {}
    void GetProviderLocation(Optional<ProviderLocationType> & providerLocation) override {}
    CHIP_ERROR AddDefaultOtaProvider(const ProviderLocationType & providerLocation) override
    {
        return mProviderLocations.Add(providerLocation);
    }
    ProviderLocationList::Iterator GetDefaultOTAProviderListIterator() override { return mProviderLocations.Begin(); }
    CHIP_ERROR RegisterEventHandler(OTARequestorEventHandlerRegistration & eventHandler) override
    {
        return mEventHandlerRegistry.Register(eventHandler);
    }
    CHIP_ERROR UnregisterEventHandler(EndpointId endpointId) override { return mEventHandlerRegistry.Unregister(endpointId); }

    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType GetLastAnnounceCommandPayload() const
    {
        return mLastAnnounceCommandPayload;
    }

    void SetUpdateStateProgress(DataModel::Nullable<uint8_t> updateStateProgress) { mUpdateStateProgress = updateStateProgress; }

    OTARequestorEventHandler * GetEventHandler(EndpointId endpointId) { return mEventHandlerRegistry.Get(endpointId); }

private:
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType mLastAnnounceCommandPayload;
    ProviderLocationList mProviderLocations;
    DataModel::Nullable<uint8_t> mUpdateStateProgress;
    OTARequestorEventHandlerRegistry mEventHandlerRegistry;
};

struct TestOTARequestorCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestOTARequestorCluster, TestCreate)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    OTARequestorCluster clusterWithoutOtaRequestor(kTestEndpointId + 1, nullptr);
    EXPECT_EQ(clusterWithoutOtaRequestor.Startup(context.Get()), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorCluster, AttributeListTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(
        chip::Testing::IsAttributesListEqualTo(cluster,
                                               {
                                                   OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::kMetadataEntry,
                                                   OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::kMetadataEntry,
                                                   OtaSoftwareUpdateRequestor::Attributes::UpdateState::kMetadataEntry,
                                                   OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::kMetadataEntry,
                                               }));
}

TEST_F(TestOTARequestorCluster, AcceptedCommandsTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(
        chip::Testing::IsAcceptedCommandsListEqualTo(cluster,
                                                     {
                                                         OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::kMetadataEntry,
                                                     }));
}

TEST_F(TestOTARequestorCluster, GeneratedCommandsTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(chip::Testing::IsGeneratedCommandsListEqualTo(cluster, {}));
}

TEST_F(TestOTARequestorCluster, EventInfoTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
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
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID     = 1234;
    payload.vendorID           = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint           = 5;

    auto result = tester.Invoke(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload);
    EXPECT_EQ(result.status, std::nullopt);

    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::DecodableType forwarded_payload =
        otaRequestor.GetLastAnnounceCommandPayload();
    EXPECT_EQ(forwarded_payload.providerNodeID, static_cast<NodeId>(1234));
    EXPECT_EQ(forwarded_payload.vendorID, static_cast<VendorId>(4321));
    EXPECT_EQ(forwarded_payload.announcementReason, OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable);
    EXPECT_EQ(forwarded_payload.endpoint, 5);
}

TEST_F(TestOTARequestorCluster, AnnounceOtaProviderCommandInvalidMetadataTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID     = 1234;
    payload.vendorID           = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint           = 5;
    // The maximum allowed metadata length is 512 bytes, so send 513.
    uint8_t bytes[513]      = { '\0' };
    payload.metadataForNode = MakeOptional(ByteSpan(bytes));

    auto result = tester.Invoke(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status.value(), DataModel::ActionReturnStatus(Protocols::InteractionModel::Status::InvalidCommand));
}

TEST_F(TestOTARequestorCluster, ReadAttributesTest)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    OTARequestorCluster clusterUpdateImpossible(kTestEndpointId + 1, &otaRequestor, false);
    EXPECT_EQ(clusterUpdateImpossible.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);
    chip::Test::ClusterTester testerUpdateImpossible(clusterUpdateImpossible);

    // Read and verify DefaultOTAProviders.
    using DecodableProviderLocation = OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType;
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type provider;
    provider.providerNodeID = 1234u;
    provider.endpoint       = 8;
    provider.fabricIndex    = 2;
    otaRequestor.AddDefaultOtaProvider(provider);
    DataModel::DecodableList<DecodableProviderLocation> defaultOtaProviders;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id, defaultOtaProviders),
              CHIP_NO_ERROR);
    size_t defaultOtaProvidersSize;
    defaultOtaProviders.ComputeSize(&defaultOtaProvidersSize);
    EXPECT_EQ(defaultOtaProvidersSize, 1u);
    DataModel::DecodableList<DecodableProviderLocation>::Iterator defaultOtaProvidersIterator = defaultOtaProviders.begin();
    EXPECT_TRUE(defaultOtaProvidersIterator.Next());
    const DecodableProviderLocation & decodedProvider = defaultOtaProvidersIterator.GetValue();
    EXPECT_EQ(decodedProvider.providerNodeID, 1234u);
    EXPECT_EQ(decodedProvider.endpoint, 8);
    EXPECT_EQ(decodedProvider.fabricIndex, 2);

    // Read and verify UpdatePossible.
    bool updatePossible;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id, updatePossible), CHIP_NO_ERROR);
    EXPECT_TRUE(updatePossible);

    EXPECT_EQ(testerUpdateImpossible.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id, updatePossible),
              CHIP_NO_ERROR);
    EXPECT_FALSE(updatePossible);

    // Read and verify UpdateState.
    OtaSoftwareUpdateRequestor::UpdateStateEnum updateState;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id, updateState), CHIP_NO_ERROR);
    EXPECT_EQ(updateState, OtaSoftwareUpdateRequestor::UpdateStateEnum::kIdle);

    // Read and verify UpdateStateProgress.
    DataModel::Nullable<uint8_t> updateStateProgress;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id, updateStateProgress),
              CHIP_NO_ERROR);
    EXPECT_TRUE(updateStateProgress.IsNull());

    // Verify a non-null value as well.
    otaRequestor.SetUpdateStateProgress(85);
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id, updateStateProgress),
              CHIP_NO_ERROR);
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

TEST_F(TestOTARequestorCluster, StateTransitionEvent)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnStateTransition(OtaSoftwareUpdateRequestor::UpdateStateEnum::kQuerying,
                              OtaSoftwareUpdateRequestor::UpdateStateEnum::kIdle,
                              OtaSoftwareUpdateRequestor::ChangeReasonEnum::kFailure, 1000u);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id,
                                OtaSoftwareUpdateRequestor::Events::StateTransition::Id));
    OtaSoftwareUpdateRequestor::Events::StateTransition::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.previousState, OtaSoftwareUpdateRequestor::UpdateStateEnum::kQuerying);
    EXPECT_EQ(decodedEvent.newState, OtaSoftwareUpdateRequestor::UpdateStateEnum::kIdle);
    EXPECT_EQ(decodedEvent.reason, OtaSoftwareUpdateRequestor::ChangeReasonEnum::kFailure);
    EXPECT_EQ(decodedEvent.targetSoftwareVersion, DataModel::MakeNullable(1000u));
}

TEST_F(TestOTARequestorCluster, StateTransitionEventWithNoOtaRequestor)
{
    chip::Test::TestServerClusterContext context;
    OTARequestorCluster cluster(kTestEndpointId, nullptr);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnStateTransition(OtaSoftwareUpdateRequestor::UpdateStateEnum::kQuerying,
                              OtaSoftwareUpdateRequestor::UpdateStateEnum::kIdle,
                              OtaSoftwareUpdateRequestor::ChangeReasonEnum::kFailure, 1000u);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id,
                                OtaSoftwareUpdateRequestor::Events::StateTransition::Id));
    OtaSoftwareUpdateRequestor::Events::StateTransition::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.previousState, OtaSoftwareUpdateRequestor::UpdateStateEnum::kQuerying);
    EXPECT_EQ(decodedEvent.newState, OtaSoftwareUpdateRequestor::UpdateStateEnum::kIdle);
    EXPECT_EQ(decodedEvent.reason, OtaSoftwareUpdateRequestor::ChangeReasonEnum::kFailure);
    EXPECT_EQ(decodedEvent.targetSoftwareVersion, DataModel::MakeNullable(1000u));
}

TEST_F(TestOTARequestorCluster, NoStateTransitionEventWithIdenticalStates)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnStateTransition(OtaSoftwareUpdateRequestor::UpdateStateEnum::kApplying,
                              OtaSoftwareUpdateRequestor::UpdateStateEnum::kApplying,
                              OtaSoftwareUpdateRequestor::ChangeReasonEnum::kTimeOut, 2000u);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_FALSE(event.has_value());
}

TEST_F(TestOTARequestorCluster, VersionAppliedEvent)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnVersionApplied(3000u, 1234u);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(
        event->eventOptions.mPath,
        ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, OtaSoftwareUpdateRequestor::Events::VersionApplied::Id));
    OtaSoftwareUpdateRequestor::Events::VersionApplied::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 3000u);
    EXPECT_EQ(decodedEvent.productID, 1234u);
}

TEST_F(TestOTARequestorCluster, VersionAppliedEventWithNoOtaRequestor)
{
    chip::Test::TestServerClusterContext context;
    OTARequestorCluster cluster(kTestEndpointId, nullptr);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnVersionApplied(3000u, 1234u);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(
        event->eventOptions.mPath,
        ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, OtaSoftwareUpdateRequestor::Events::VersionApplied::Id));
    OtaSoftwareUpdateRequestor::Events::VersionApplied::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 3000u);
    EXPECT_EQ(decodedEvent.productID, 1234u);
}

TEST_F(TestOTARequestorCluster, DownloadErrorEvents)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnDownloadError(4000u, 10000u, 82u, 1234567890l);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(
        event->eventOptions.mPath,
        ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, OtaSoftwareUpdateRequestor::Events::DownloadError::Id));
    OtaSoftwareUpdateRequestor::Events::DownloadError::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 4000u);
    EXPECT_EQ(decodedEvent.bytesDownloaded, 10000u);
    EXPECT_EQ(decodedEvent.progressPercent, DataModel::MakeNullable(static_cast<uint8_t>(82u)));
    EXPECT_EQ(decodedEvent.platformCode, DataModel::MakeNullable(1234567890l));

    cluster.OnDownloadError(5000u, 12000u, DataModel::NullNullable, DataModel::NullNullable);
    event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(
        event->eventOptions.mPath,
        ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, OtaSoftwareUpdateRequestor::Events::DownloadError::Id));
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 5000u);
    EXPECT_EQ(decodedEvent.bytesDownloaded, 12000u);
    EXPECT_TRUE(decodedEvent.progressPercent.IsNull());
    EXPECT_TRUE(decodedEvent.platformCode.IsNull());
}

TEST_F(TestOTARequestorCluster, DownloadErrorEventsWithNoOtaRequestor)
{
    chip::Test::TestServerClusterContext context;
    OTARequestorCluster cluster(kTestEndpointId, nullptr);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & eventsGenerator = context.EventsGenerator();

    cluster.OnDownloadError(4000u, 10000u, 82u, 1234567890l);
    auto event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(
        event->eventOptions.mPath,
        ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, OtaSoftwareUpdateRequestor::Events::DownloadError::Id));
    OtaSoftwareUpdateRequestor::Events::DownloadError::DecodableType decodedEvent;
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 4000u);
    EXPECT_EQ(decodedEvent.bytesDownloaded, 10000u);
    EXPECT_EQ(decodedEvent.progressPercent, DataModel::MakeNullable(static_cast<uint8_t>(82u)));
    EXPECT_EQ(decodedEvent.platformCode, DataModel::MakeNullable(1234567890l));

    cluster.OnDownloadError(5000u, 12000u, DataModel::NullNullable, DataModel::NullNullable);
    event = eventsGenerator.GetNextEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(
        event->eventOptions.mPath,
        ConcreteEventPath(kTestEndpointId, OtaSoftwareUpdateRequestor::Id, OtaSoftwareUpdateRequestor::Events::DownloadError::Id));
    ASSERT_EQ(event->GetEventData(decodedEvent), CHIP_NO_ERROR);
    EXPECT_EQ(decodedEvent.softwareVersion, 5000u);
    EXPECT_EQ(decodedEvent.bytesDownloaded, 12000u);
    EXPECT_TRUE(decodedEvent.progressPercent.IsNull());
    EXPECT_TRUE(decodedEvent.platformCode.IsNull());
}

TEST_F(TestOTARequestorCluster, RegistersAsEventHandler)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);

    EXPECT_EQ(otaRequestor.GetEventHandler(kTestEndpointId), nullptr);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(otaRequestor.GetEventHandler(kTestEndpointId), &cluster);

    cluster.Shutdown();
    EXPECT_EQ(otaRequestor.GetEventHandler(kTestEndpointId), nullptr);
}

TEST_F(TestOTARequestorCluster, ReadsWithNoRequestorInterfaceReturnErrors)
{
    chip::Test::TestServerClusterContext context;
    OTARequestorCluster cluster(kTestEndpointId, nullptr);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);

    // Read and verify that DefaultOTAProviders returns an error.
    using DecodableProviderLocation = OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType;
    DataModel::DecodableList<DecodableProviderLocation> defaultOtaProviders;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders::Id, defaultOtaProviders),
              CHIP_ERROR_INTERNAL);

    // UpdatePossible shouldn't be affected.
    bool updatePossible;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id, updatePossible), CHIP_NO_ERROR);
    EXPECT_TRUE(updatePossible);

    // Read and verify that UpdateState returns an error.
    OtaSoftwareUpdateRequestor::UpdateStateEnum updateState;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id, updateState), CHIP_ERROR_INTERNAL);

    // Read and verify that UpdateStateProgress returns an error.
    DataModel::Nullable<uint8_t> updateStateProgress;
    EXPECT_EQ(tester.ReadAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id, updateStateProgress),
              CHIP_ERROR_INTERNAL);

    // FeatureMap shouldn't be affected.
    uint32_t featureMap;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    // ClusterRevision shouldn't be affected.
    uint16_t clusterRevision;
    EXPECT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, 1u);

    // Non-existent attribute should be handled the same.
    uint32_t nonExistentAttribute;
    EXPECT_NE(tester.ReadAttribute(0xFFFF, nonExistentAttribute), CHIP_NO_ERROR);
}

TEST_F(TestOTARequestorCluster, CommandsWithNoRequestorInterfaceReturnErrors)
{
    chip::Test::TestServerClusterContext context;
    OTARequestorCluster cluster(kTestEndpointId, nullptr);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    chip::Test::ClusterTester tester(cluster);
    OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Type payload;
    payload.providerNodeID     = 1234;
    payload.vendorID           = static_cast<VendorId>(4321);
    payload.announcementReason = OtaSoftwareUpdateRequestor::AnnouncementReasonEnum::kUpdateAvailable;
    payload.endpoint           = 5;

    auto result = tester.Invoke(OtaSoftwareUpdateRequestor::Commands::AnnounceOTAProvider::Id, payload);
    ASSERT_TRUE(result.status.has_value());
    EXPECT_TRUE(result.status->IsError());
}

TEST_F(TestOTARequestorCluster, WriteDefaultProvidersList)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    auto & changeListener = context.ChangeListener();
    changeListener.DirtyList().clear();

    // Write the default OTA providers list.
    namespace DefaultOtaProviders = OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders;
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type provider;
    provider.providerNodeID = 1234u;
    provider.endpoint       = 8;
    // This comes from the subject descriptor set in WriteOperation's constructor, as ClusterTester creates
    // a WriteOperation and doesn't set the subject descriptor.
    provider.fabricIndex = chip::app::Testing::kDenySubjectDescriptor.fabricIndex;
    DataModel::List<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> payload(&provider, 1u);
    std::optional<DataModel::ActionReturnStatus> result = tester.WriteAttribute(DefaultOtaProviders::Id, payload);
    ASSERT_TRUE(result.has_value());
    ASSERT_TRUE(result->IsSuccess());

    // Verify the data was written correctly.
    auto iterator = otaRequestor.GetDefaultOTAProviderListIterator();
    ASSERT_TRUE(iterator.Next());
    EXPECT_EQ(iterator.GetValue(), provider);
    EXPECT_FALSE(iterator.Next());

    // Verify the attribute was reported as changed.
    ASSERT_EQ(changeListener.DirtyList().size(), 1u);
    EXPECT_EQ(changeListener.DirtyList()[0].mEndpointId, kTestEndpointId);
    EXPECT_EQ(changeListener.DirtyList()[0].mClusterId, OtaSoftwareUpdateRequestor::Id);
    EXPECT_EQ(changeListener.DirtyList()[0].mAttributeId, DefaultOtaProviders::Id);
}

TEST_F(TestOTARequestorCluster, WriteDefaultProvidersListWithNoInterfaceReturnsErrors)
{
    chip::Test::TestServerClusterContext context;
    OTARequestorCluster cluster(kTestEndpointId, nullptr);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    auto & changeListener = context.ChangeListener();
    changeListener.DirtyList().clear();

    // Write the default OTA providers list.
    namespace DefaultOtaProviders = OtaSoftwareUpdateRequestor::Attributes::DefaultOTAProviders;
    OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type provider;
    provider.providerNodeID = 1234u;
    provider.endpoint       = 8;
    // This comes from the subject descriptor set in WriteOperation's constructor, as ClusterTester creates
    // a WriteOperation and doesn't set the subject descriptor
    provider.fabricIndex = chip::app::Testing::kDenySubjectDescriptor.fabricIndex;
    DataModel::List<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::Type> payload(&provider, 1u);
    std::optional<DataModel::ActionReturnStatus> result = tester.WriteAttribute(DefaultOtaProviders::Id, payload);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->IsError());
    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

TEST_F(TestOTARequestorCluster, WritingReadOnlyAttributesReturnsUnsupportedWrite)
{
    chip::Test::TestServerClusterContext context;
    MockOtaRequestor otaRequestor;
    OTARequestorCluster cluster(kTestEndpointId, &otaRequestor);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);
    chip::Test::ClusterTester tester(cluster);

    auto & changeListener = context.ChangeListener();
    changeListener.DirtyList().clear();

    std::optional<DataModel::ActionReturnStatus> result =
        tester.WriteAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdatePossible::Id, false);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateState::Id,
                                   OtaSoftwareUpdateRequestor::UpdateStateEnum::kDelayedOnApply);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(OtaSoftwareUpdateRequestor::Attributes::UpdateStateProgress::Id, DataModel::MakeNullable(50));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(Globals::Attributes::FeatureMap::Id, static_cast<uint32_t>(10));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    result = tester.WriteAttribute(Globals::Attributes::ClusterRevision::Id, static_cast<uint16_t>(5));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, Protocols::InteractionModel::Status::UnsupportedAttribute);

    EXPECT_EQ(changeListener.DirtyList().size(), 0u);
}

} // namespace
