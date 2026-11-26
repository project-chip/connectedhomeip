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

#include <app/CommandHandler.h>
#include <app/clusters/webrtc-transport-provider-server/WebRTCTransportProviderCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/FabricTestFixture.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WebRTCTransportProvider/Attributes.h>
#include <clusters/WebRTCTransportProvider/Commands.h>
#include <clusters/WebRTCTransportProvider/Enums.h>
#include <clusters/WebRTCTransportProvider/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;
using namespace chip::Testing;

using chip::app::ClusterShutdownType;

using ICEServerDecodableStruct = Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = Clusters::Globals::WebRTCEndReasonEnum;

static constexpr EndpointId kTestEndpointId = 1;

// Mock delegate for testing WebRTCTransportProvider
class MockWebRTCTransportProviderDelegate : public Delegate
{
public:
    MockWebRTCTransportProviderDelegate() = default;

    bool populateStreamListsFromTempBuffers = false;
    bool overwriteStreamListsOnProvideOffer = true;
    std::vector<uint16_t> nextVideoStreams;
    std::vector<uint16_t> nextAudioStreams;
    std::unique_ptr<std::vector<uint16_t>> tempVideoBuffer;
    std::unique_ptr<std::vector<uint16_t>> tempAudioBuffer;

    void ReleaseTempBuffers()
    {
        tempVideoBuffer.reset();
        tempAudioBuffer.reset();
    }

    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession, bool & outDeferredOffer) override
    {
        outSession.id             = args.sessionId;
        outSession.peerNodeID     = args.peerNodeId;
        outSession.peerEndpointID = args.originatingEndpointId;
        outSession.streamUsage    = args.streamUsage;
        outSession.SetFabricIndex(args.fabricIndex);
        if (populateStreamListsFromTempBuffers)
        {
            tempVideoBuffer = std::make_unique<std::vector<uint16_t>>(nextVideoStreams);
            tempAudioBuffer = std::make_unique<std::vector<uint16_t>>(nextAudioStreams);
            if (!tempVideoBuffer->empty())
            {
                outSession.videoStreams.SetValue(DataModel::List<const uint16_t>(tempVideoBuffer->data(), tempVideoBuffer->size()));
            }
            if (!tempAudioBuffer->empty())
            {
                outSession.audioStreams.SetValue(DataModel::List<const uint16_t>(tempAudioBuffer->data(), tempAudioBuffer->size()));
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override
    {
        outSession.id             = args.sessionId;
        outSession.peerNodeID     = args.peerNodeId;
        outSession.peerEndpointID = args.originatingEndpointId;
        outSession.streamUsage    = args.streamUsage;
        outSession.SetFabricIndex(args.fabricIndex);
        if (populateStreamListsFromTempBuffers && overwriteStreamListsOnProvideOffer)
        {
            tempVideoBuffer = std::make_unique<std::vector<uint16_t>>(nextVideoStreams);
            tempAudioBuffer = std::make_unique<std::vector<uint16_t>>(nextAudioStreams);
            if (!tempVideoBuffer->empty())
            {
                outSession.videoStreams.SetValue(DataModel::List<const uint16_t>(tempVideoBuffer->data(), tempVideoBuffer->size()));
            }
            else
            {
                outSession.videoStreams.ClearValue();
            }
            if (!tempAudioBuffer->empty())
            {
                outSession.audioStreams.SetValue(DataModel::List<const uint16_t>(tempAudioBuffer->data(), tempAudioBuffer->size()));
            }
            else
            {
                outSession.audioStreams.ClearValue();
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override { return CHIP_NO_ERROR; }

    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override
    {
        return CHIP_NO_ERROR;
    }

    std::vector<uint16_t> endedSessionIds;

    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override
    {
        endedSessionIds.push_back(sessionId);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<std::vector<uint16_t>> & videoStreams,
                                   Optional<std::vector<uint16_t>> & audioStreams) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateVideoStreams(const std::vector<uint16_t> & videoStreams) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateAudioStreams(const std::vector<uint16_t> & audioStreams) override { return CHIP_NO_ERROR; }

    CHIP_ERROR IsStreamUsageSupported(Globals::StreamUsageEnum streamUsage) override { return CHIP_NO_ERROR; }

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override
    {
        isActive = false;
        return CHIP_NO_ERROR;
    }

    bool HasAllocatedVideoStreams() override { return true; }

    bool HasAllocatedAudioStreams() override { return true; }

    CHIP_ERROR ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength) override { return CHIP_NO_ERROR; }

    CHIP_ERROR IsUTCTimeNull(bool & isNull) override
    {
        isNull = false;
        return CHIP_NO_ERROR;
    }
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestWebRTCTransportProviderCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

TEST_F(TestWebRTCTransportProviderCluster, TestAttributes)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);

    ASSERT_TRUE(IsAttributesListEqualTo(server, { WebRTCTransportProvider::Attributes::CurrentSessions::kMetadataEntry }));
}

TEST_F(TestWebRTCTransportProviderCluster, TestCommands)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(server,
                                              {
                                                  WebRTCTransportProvider::Commands::SolicitOffer::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::ProvideOffer::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::ProvideAnswer::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::ProvideICECandidates::kMetadataEntry,
                                                  WebRTCTransportProvider::Commands::EndSession::kMetadataEntry,
                                              }));
}

TEST_F(TestWebRTCTransportProviderCluster, TestCurrentSessionsAttribute)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);

    // Initially, no sessions should exist
    auto sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportProviderCluster, TestSessionManagement)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);

    // Verify initial state
    auto sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());

    // Test that RemoveSession on non-existent session is safe
    server.RemoveSession(999);
    sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportProviderCluster, TestReadCurrentSessionsAttribute)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading empty sessions
    WebRTCTransportProvider::Attributes::CurrentSessions::TypeInfo::DecodableType sessions;
    auto status = tester.ReadAttribute(WebRTCTransportProvider::Attributes::CurrentSessions::Id, sessions);
    EXPECT_TRUE(status.IsSuccess());

    auto iter = sessions.begin();
    EXPECT_FALSE(iter.Next()); // Should be empty

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWebRTCTransportProviderCluster, TestReadClusterRevisionAttribute)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading cluster revision
    Globals::Attributes::ClusterRevision::TypeInfo::DecodableType clusterRevision = 0;
    auto status = tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, clusterRevision);
    EXPECT_TRUE(status.IsSuccess());
    EXPECT_EQ(clusterRevision, WebRTCTransportProvider::kRevision);

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWebRTCTransportProviderCluster, TestReadUnsupportedAttribute)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading unsupported attribute
    uint32_t dummyValue;
    auto status = tester.ReadAttribute(0xFFFF /* Invalid attribute ID */, dummyValue);
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedAttribute);

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Removing a fabric ends only that fabric's sessions; other fabrics' sessions survive.
TEST_F(TestWebRTCTransportProviderCluster, TestFabricRemovedTearsDownSessions)
{
    TestServerClusterContext context;
    FabricTestFixture fabricHelper{ &context.StorageDelegate() };

    FabricIndex fabric1 = kTestFabricIndex;
    ASSERT_EQ(fabricHelper.SetUpTestFabric(fabric1), CHIP_NO_ERROR);
    FabricIndex fabric2 = static_cast<FabricIndex>(kTestFabricIndex + 1);
    ASSERT_EQ(fabricHelper.AddAdditionalTestFabric(fabric2), CHIP_NO_ERROR);

    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricHelper.GetFabricTable());
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    Commands::SolicitOffer::Type solicitRequest;
    solicitRequest.streamUsage   = StreamUsageEnum::kLiveView;
    solicitRequest.videoStreamID = Optional<DataModel::Nullable<uint16_t>>(1);

    tester.SetFabricIndex(fabric1);
    auto result = tester.Invoke(Commands::SolicitOffer::Id, solicitRequest);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_EQ(server.GetCurrentSessions().size(), 1u);
    uint16_t session1 = server.GetCurrentSessions()[0].id;

    tester.SetFabricIndex(fabric2);
    auto result2 = tester.Invoke(Commands::SolicitOffer::Id, solicitRequest);
    ASSERT_TRUE(result2.IsSuccess());
    ASSERT_EQ(server.GetCurrentSessions().size(), 2u);
    uint16_t session2 = server.GetCurrentSessions()[1].id;

    // Removing fabric1 fires OnFabricRemoved: fabric1's session ends, fabric2's remains.
    EXPECT_EQ(fabricHelper.GetFabricTable().Delete(fabric1), CHIP_NO_ERROR);

    ASSERT_EQ(mockDelegate.endedSessionIds.size(), 1u);
    EXPECT_EQ(mockDelegate.endedSessionIds[0], session1);

    ASSERT_EQ(server.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(server.GetCurrentSessions()[0].id, session2);
    EXPECT_EQ(server.GetCurrentSessions()[0].GetFabricIndex(), fabric2);

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
    ASSERT_EQ(fabricHelper.TearDownTestFabric(fabric2), CHIP_NO_ERROR);
}

TEST_F(TestWebRTCTransportProviderCluster, TestCurrentSessionsOwnsStreamLists)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    mockDelegate.populateStreamListsFromTempBuffers = true;

    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);
    ASSERT_SUCCESS(server.Startup(context.Get()));

    ClusterTester tester(server);

    mockDelegate.nextVideoStreams = { 10, 20 };
    mockDelegate.nextAudioStreams = { 30, 40 };

    Commands::SolicitOffer::Type solicitRequest;
    solicitRequest.streamUsage   = StreamUsageEnum::kLiveView;
    solicitRequest.videoStreamID = Optional<DataModel::Nullable<uint16_t>>(10);
    ASSERT_TRUE(tester.Invoke(Commands::SolicitOffer::Id, solicitRequest).IsSuccess());
    mockDelegate.ReleaseTempBuffers();

    mockDelegate.nextVideoStreams = { 50 };
    mockDelegate.nextAudioStreams = { 60, 70 };

    Commands::ProvideOffer::Type provideRequest;
    provideRequest.webRTCSessionID.SetNull();
    provideRequest.sdp           = "v=0"_span;
    provideRequest.streamUsage   = StreamUsageEnum::kLiveView;
    provideRequest.videoStreamID = Optional<DataModel::Nullable<uint16_t>>(50);
    ASSERT_TRUE(tester.Invoke(Commands::ProvideOffer::Id, provideRequest).IsSuccess());
    mockDelegate.ReleaseTempBuffers();

    // Read CurrentSessions via attribute encoding after delegate temporary buffers are freed.
    WebRTCTransportProvider::Attributes::CurrentSessions::TypeInfo::DecodableType decodedSessions;
    ASSERT_TRUE(tester.ReadAttribute(WebRTCTransportProvider::Attributes::CurrentSessions::Id, decodedSessions).IsSuccess());

    auto decodeList = [](const DataModel::DecodableList<uint16_t> & list) {
        std::vector<uint16_t> values;
        auto iter = list.begin();
        while (iter.Next())
        {
            values.push_back(iter.GetValue());
        }
        EXPECT_SUCCESS(iter.GetStatus());
        return values;
    };

    std::vector<std::vector<uint16_t>> readVideos;
    std::vector<std::vector<uint16_t>> readAudios;
    auto iter = decodedSessions.begin();
    while (iter.Next())
    {
        const auto & session = iter.GetValue();
        ASSERT_TRUE(session.videoStreams.HasValue());
        ASSERT_TRUE(session.audioStreams.HasValue());
        readVideos.push_back(decodeList(session.videoStreams.Value()));
        readAudios.push_back(decodeList(session.audioStreams.Value()));
    }
    ASSERT_SUCCESS(iter.GetStatus());

    ASSERT_EQ(readVideos.size(), 2u);
    EXPECT_EQ(readVideos[0], (std::vector<uint16_t>{ 10, 20 }));
    EXPECT_EQ(readAudios[0], (std::vector<uint16_t>{ 30, 40 }));
    EXPECT_EQ(readVideos[1], (std::vector<uint16_t>{ 50 }));
    EXPECT_EQ(readAudios[1], (std::vector<uint16_t>{ 60, 70 }));

    // GetCurrentSessions() returns owned copies that stay valid after RemoveSession().
    auto snapshot = server.GetCurrentSessions();
    ASSERT_EQ(snapshot.size(), 2u);
    server.RemoveSession(snapshot[0].id);

    ASSERT_TRUE(snapshot[0].videoStreams.HasValue());
    ASSERT_TRUE(snapshot[0].audioStreams.HasValue());
    EXPECT_EQ(std::vector<uint16_t>(snapshot[0].videoStreams.Value().begin(), snapshot[0].videoStreams.Value().end()),
              (std::vector<uint16_t>{ 10, 20 }));
    EXPECT_EQ(std::vector<uint16_t>(snapshot[0].audioStreams.Value().begin(), snapshot[0].audioStreams.Value().end()),
              (std::vector<uint16_t>{ 30, 40 }));

    // Remaining session in the cluster stays valid after erase shifts elements.
    auto remaining = server.GetCurrentSessions();
    ASSERT_EQ(remaining.size(), 1u);
    ASSERT_TRUE(remaining[0].videoStreams.HasValue());
    EXPECT_EQ(std::vector<uint16_t>(remaining[0].videoStreams.Value().begin(), remaining[0].videoStreams.Value().end()),
              (std::vector<uint16_t>{ 50 }));

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWebRTCTransportProviderCluster, TestReOfferPreservesAndUpdatesOwnedStreamLists)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    mockDelegate.populateStreamListsFromTempBuffers = true;

    FabricTable fabricTable;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate, fabricTable);
    ASSERT_SUCCESS(server.Startup(context.Get()));

    ClusterTester tester(server);

    mockDelegate.nextVideoStreams = { 1, 2 };
    mockDelegate.nextAudioStreams = { 3 };

    Commands::ProvideOffer::Type provideRequest;
    provideRequest.webRTCSessionID.SetNull();
    provideRequest.sdp           = "v=0"_span;
    provideRequest.streamUsage   = StreamUsageEnum::kLiveView;
    provideRequest.videoStreamID = Optional<DataModel::Nullable<uint16_t>>(1);
    ASSERT_TRUE(tester.Invoke(Commands::ProvideOffer::Id, provideRequest).IsSuccess());
    mockDelegate.ReleaseTempBuffers();

    ASSERT_EQ(server.GetCurrentSessions().size(), 1u);
    const uint16_t sessionId = server.GetCurrentSessions()[0].id;

    // Re-offer leaving outSession stream lists untouched exercises self-aliasing in UpsertSession.
    mockDelegate.overwriteStreamListsOnProvideOffer = false;
    provideRequest.webRTCSessionID.SetNonNull(sessionId);
    ASSERT_TRUE(tester.Invoke(Commands::ProvideOffer::Id, provideRequest).IsSuccess());

    auto afterPreserve = server.GetCurrentSessions();
    ASSERT_EQ(afterPreserve.size(), 1u);
    ASSERT_TRUE(afterPreserve[0].videoStreams.HasValue());
    ASSERT_TRUE(afterPreserve[0].audioStreams.HasValue());
    EXPECT_EQ(std::vector<uint16_t>(afterPreserve[0].videoStreams.Value().begin(), afterPreserve[0].videoStreams.Value().end()),
              (std::vector<uint16_t>{ 1, 2 }));
    EXPECT_EQ(std::vector<uint16_t>(afterPreserve[0].audioStreams.Value().begin(), afterPreserve[0].audioStreams.Value().end()),
              (std::vector<uint16_t>{ 3 }));

    // Re-offer replacing stream lists updates owned storage cleanly.
    mockDelegate.overwriteStreamListsOnProvideOffer = true;
    mockDelegate.nextVideoStreams                   = { 8, 9 };
    mockDelegate.nextAudioStreams                   = {};
    ASSERT_TRUE(tester.Invoke(Commands::ProvideOffer::Id, provideRequest).IsSuccess());
    mockDelegate.ReleaseTempBuffers();

    auto afterUpdate = server.GetCurrentSessions();
    ASSERT_EQ(afterUpdate.size(), 1u);
    ASSERT_TRUE(afterUpdate[0].videoStreams.HasValue());
    EXPECT_EQ(std::vector<uint16_t>(afterUpdate[0].videoStreams.Value().begin(), afterUpdate[0].videoStreams.Value().end()),
              (std::vector<uint16_t>{ 8, 9 }));
    EXPECT_FALSE(afterUpdate[0].audioStreams.HasValue());

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
