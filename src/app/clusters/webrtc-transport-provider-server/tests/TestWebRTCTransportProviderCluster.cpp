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
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WebRTCTransportProvider/Attributes.h>
#include <clusters/WebRTCTransportProvider/Commands.h>
#include <clusters/WebRTCTransportProvider/Enums.h>
#include <clusters/WebRTCTransportProvider/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cstring>
#include <vector>

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
        // Retain the configuration to verify it stays valid after the handler returns.
        if (args.sFrameConfig.HasValue())
        {
            mCapturedSFrameConfig.SetValue(args.sFrameConfig.Value());
        }
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

    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override { return CHIP_NO_ERROR; }

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

    Optional<SFrameConfigStorage> mCapturedSFrameConfig;
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
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    ASSERT_TRUE(IsAttributesListEqualTo(server,
                                        { WebRTCTransportProvider::Attributes::CurrentSessions::kMetadataEntry,
                                          WebRTCTransportProvider::Attributes::SupportedSFrameCipherSuites::kMetadataEntry }));
}

TEST_F(TestWebRTCTransportProviderCluster, TestCommands)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

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
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Initially, no sessions should exist
    auto sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportProviderCluster, TestSessionManagement)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

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
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
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
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
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
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Test reading unsupported attribute
    uint32_t dummyValue;
    auto status = tester.ReadAttribute(0xFFFF /* Invalid attribute ID */, dummyValue);
    EXPECT_FALSE(status.IsSuccess());
    EXPECT_EQ(status.GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::UnsupportedAttribute);

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWebRTCTransportProviderCluster, TestSFrameConfigIsDeepCopied)
{
    TestServerClusterContext context;
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);
    ASSERT_EQ(server.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(server);

    // Mutable source buffers, scribbled over after the command returns to prove
    // the delegate retained a deep copy rather than aliasing the input.
    uint8_t senderKid[]      = { 0x01, 0x02, 0x03 };
    uint8_t senderBaseKey[]  = { 0xAA, 0xBB, 0xCC, 0xDD };
    uint8_t receiveKid[]     = { 0x0A, 0x0B };
    uint8_t receiveBaseKey[] = { 0x11, 0x22, 0x33, 0x44, 0x55 };

    Globals::Structs::SFrameKeyStruct::Type senderKey;
    senderKey.kid     = ByteSpan(senderKid);
    senderKey.baseKey = ByteSpan(senderBaseKey);

    std::vector<Globals::Structs::SFrameKeyStruct::Type> receiveKeys(1);
    receiveKeys[0].kid     = ByteSpan(receiveKid);
    receiveKeys[0].baseKey = ByteSpan(receiveBaseKey);

    Globals::Structs::SFrameStruct::Type sframeConfig;
    sframeConfig.audioCipherSuite = 1;
    sframeConfig.videoCipherSuite = 2;
    sframeConfig.senderKey        = senderKey;
    sframeConfig.receiveKeys =
        DataModel::List<const Globals::Structs::SFrameKeyStruct::Type>(receiveKeys.data(), receiveKeys.size());
    sframeConfig.ratchetBits = 5;

    uint16_t videoStreamId = 7;
    Commands::SolicitOffer::Type request;
    request.streamUsage = StreamUsageEnum::kLiveView;
    request.videoStreams.SetValue(DataModel::List<const uint16_t>(&videoStreamId, 1));
    request.SFrameConfig.SetValue(sframeConfig);

    auto result = tester.Invoke(Commands::SolicitOffer::Id, request);
    ASSERT_TRUE(result.status.has_value() && result.status->IsSuccess());

    // Clobber the original input buffers now that the command has returned.
    memset(senderKid, 0xFF, sizeof(senderKid));
    memset(senderBaseKey, 0xFF, sizeof(senderBaseKey));
    memset(receiveKid, 0xFF, sizeof(receiveKid));
    memset(receiveBaseKey, 0xFF, sizeof(receiveBaseKey));

    ASSERT_TRUE(mockDelegate.mCapturedSFrameConfig.HasValue());
    const auto & captured = mockDelegate.mCapturedSFrameConfig.Value();
    EXPECT_EQ(captured.audioCipherSuite, 1);
    EXPECT_EQ(captured.videoCipherSuite, 2);
    EXPECT_EQ(captured.ratchetBits, 5);

    const uint8_t expectedSenderKid[]      = { 0x01, 0x02, 0x03 };
    const uint8_t expectedSenderBaseKey[]  = { 0xAA, 0xBB, 0xCC, 0xDD };
    const uint8_t expectedReceiveKid[]     = { 0x0A, 0x0B };
    const uint8_t expectedReceiveBaseKey[] = { 0x11, 0x22, 0x33, 0x44, 0x55 };

    EXPECT_TRUE(captured.senderKey.kid.data_equal(ByteSpan(expectedSenderKid)));
    EXPECT_TRUE(captured.senderKey.baseKey.data_equal(ByteSpan(expectedSenderBaseKey)));

    ASSERT_EQ(captured.receiveKeys.size(), 1u);
    EXPECT_TRUE(captured.receiveKeys[0].kid.data_equal(ByteSpan(expectedReceiveKid)));
    EXPECT_TRUE(captured.receiveKeys[0].baseKey.data_equal(ByteSpan(expectedReceiveBaseKey)));

    server.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
