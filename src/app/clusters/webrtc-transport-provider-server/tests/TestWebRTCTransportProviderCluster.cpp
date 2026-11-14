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

    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession, bool & outDeferredOffer) override
    {
        outSession.id             = args.sessionId;
        outSession.peerNodeID     = args.peerNodeId;
        outSession.peerEndpointID = args.originatingEndpointId;
        outSession.streamUsage    = args.streamUsage;
        outSession.SetFabricIndex(args.fabricIndex);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override
    {
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

} // namespace
