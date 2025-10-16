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
#include <app/clusters/testing/AttributeTesting.h>
#include <app/clusters/webrtc-transport-requestor-server/webrtc-transport-requestor-cluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/WebRTCTransportRequestor/Attributes.h>
#include <clusters/WebRTCTransportRequestor/Commands.h>
#include <clusters/WebRTCTransportRequestor/Enums.h>
#include <clusters/WebRTCTransportRequestor/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportRequestor;

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

static constexpr chip::EndpointId kTestEndpointId = 1;

// Mock delegate for testing
class MockWebRTCTransportRequestorDelegate : public Delegate
{
public:
    MockWebRTCTransportRequestorDelegate() : mLastSessionId(0), mLastEndReason(WebRTCEndReasonEnum::kUnknownEnumValue) {}

    CHIP_ERROR HandleOffer(uint16_t sessionId, const OfferArgs & args) override
    {
        mLastSessionId = sessionId;
        mLastOfferArgs = args;
        return mOfferResult;
    }

    CHIP_ERROR HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer) override
    {
        mLastSessionId = sessionId;
        mLastSdpAnswer = sdpAnswer;
        return mAnswerResult;
    }

    CHIP_ERROR HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override
    {
        mLastSessionId  = sessionId;
        mLastCandidates = candidates;
        return mICECandidatesResult;
    }

    CHIP_ERROR HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode) override
    {
        mLastSessionId = sessionId;
        mLastEndReason = reasonCode;
        return mEndResult;
    }

    // Test setup methods
    void SetOfferResult(CHIP_ERROR result) { mOfferResult = result; }
    void SetAnswerResult(CHIP_ERROR result) { mAnswerResult = result; }
    void SetICECandidatesResult(CHIP_ERROR result) { mICECandidatesResult = result; }
    void SetEndResult(CHIP_ERROR result) { mEndResult = result; }

    // Getters for verification
    uint16_t GetLastSessionId() const { return mLastSessionId; }
    const OfferArgs & GetLastOfferArgs() const { return mLastOfferArgs; }
    const std::string & GetLastSdpAnswer() const { return mLastSdpAnswer; }
    const std::vector<ICECandidateStruct> & GetLastCandidates() const { return mLastCandidates; }
    WebRTCEndReasonEnum GetLastEndReason() const { return mLastEndReason; }

private:
    uint16_t mLastSessionId;
    OfferArgs mLastOfferArgs;
    std::string mLastSdpAnswer;
    std::vector<ICECandidateStruct> mLastCandidates;
    WebRTCEndReasonEnum mLastEndReason;

    CHIP_ERROR mOfferResult         = CHIP_NO_ERROR;
    CHIP_ERROR mAnswerResult        = CHIP_NO_ERROR;
    CHIP_ERROR mICECandidatesResult = CHIP_NO_ERROR;
    CHIP_ERROR mEndResult           = CHIP_NO_ERROR;
};

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestWebRTCTransportRequestorCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestWebRTCTransportRequestorCluster, TestAttributes)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    ASSERT_EQ(server.Attributes({ kTestEndpointId, WebRTCTransportRequestor::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.AppendElements({
                  WebRTCTransportRequestor::Attributes::CurrentSessions::kMetadataEntry,
              }),
              CHIP_NO_ERROR);
    ASSERT_EQ(expectedBuilder.ReferenceExisting(chip::app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
}

TEST_F(TestWebRTCTransportRequestorCluster, TestCommands)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    ASSERT_EQ(server.AcceptedCommands({ kTestEndpointId, WebRTCTransportRequestor::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.AppendElements({
                  WebRTCTransportRequestor::Commands::Offer::kMetadataEntry,
                  WebRTCTransportRequestor::Commands::Answer::kMetadataEntry,
                  WebRTCTransportRequestor::Commands::ICECandidates::kMetadataEntry,
                  WebRTCTransportRequestor::Commands::End::kMetadataEntry,
              }),
              CHIP_NO_ERROR);

    EXPECT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
}

TEST_F(TestWebRTCTransportRequestorCluster, TestCurrentSessionsAttribute)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Initially, no sessions should exist
    auto sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());

    // Add a test session
    WebRTCSessionStruct testSession;
    testSession.id          = 1;
    testSession.peerNodeID  = chip::kUndefinedNodeId;
    testSession.streamUsage = StreamUsageEnum::kLiveView;

    auto result = server.UpsertSession(testSession);
    EXPECT_EQ(result, WebRTCTransportRequestorServer::UpsertResultEnum::kInserted);

    // Verify session was added
    sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 1u);
    EXPECT_EQ(sessions[0].id, 1);

    // Update the same session
    testSession.streamUsage = StreamUsageEnum::kRecording;
    result                  = server.UpsertSession(testSession);
    EXPECT_EQ(result, WebRTCTransportRequestorServer::UpsertResultEnum::kUpdated);

    // Verify session was updated, not duplicated
    sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 1u);
    EXPECT_EQ(sessions[0].streamUsage, StreamUsageEnum::kRecording);

    // Remove the session
    server.RemoveSession(1);
    sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportRequestorCluster, TestSessionManagement)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Test adding multiple sessions
    WebRTCSessionStruct session1;
    session1.id         = 1;
    session1.peerNodeID = 0x1234ULL;

    WebRTCSessionStruct session2;
    session2.id         = 2;
    session2.peerNodeID = 0x5678ULL;

    server.UpsertSession(session1);
    server.UpsertSession(session2);

    auto sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 2u);

    // Remove specific session
    server.RemoveSession(1);
    sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 1u);
    EXPECT_EQ(sessions[0].id, 2);

    // Remove non-existent session (should be no-op)
    server.RemoveSession(999);
    sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 1u);
}

TEST_F(TestWebRTCTransportRequestorCluster, TestDelegateHandleOffer)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t testSessionId = 123;
    std::string testSdp    = "test_sdp_offer";

    // Test successful offer handling
    mockDelegate.SetOfferResult(CHIP_NO_ERROR);

    Delegate::OfferArgs offerArgs;
    offerArgs.sdp        = testSdp;
    offerArgs.peerNodeId = 0x1234ULL; // Use ULL suffix for uint64_t/NodeId

    CHIP_ERROR result = mockDelegate.HandleOffer(testSessionId, offerArgs);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), testSessionId);
    EXPECT_EQ(mockDelegate.GetLastOfferArgs().sdp, testSdp);
    EXPECT_EQ(mockDelegate.GetLastOfferArgs().peerNodeId, 0x1234ULL);

    // Test error case
    mockDelegate.SetOfferResult(CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    result = mockDelegate.HandleOffer(testSessionId, offerArgs);
    EXPECT_EQ(result, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestWebRTCTransportRequestorCluster, TestDelegateHandleAnswer)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t testSessionId    = 456;
    std::string testSdpAnswer = "test_sdp_answer";

    // Test successful answer handling
    mockDelegate.SetAnswerResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleAnswer(testSessionId, testSdpAnswer);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), testSessionId);
    EXPECT_EQ(mockDelegate.GetLastSdpAnswer(), testSdpAnswer);

    // Test error case
    mockDelegate.SetAnswerResult(CHIP_ERROR_INTERNAL);
    result = mockDelegate.HandleAnswer(testSessionId, testSdpAnswer);
    EXPECT_EQ(result, CHIP_ERROR_INTERNAL);
}

TEST_F(TestWebRTCTransportRequestorCluster, TestDelegateHandleICECandidates)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t testSessionId = 789;
    std::vector<ICECandidateStruct> testCandidates;

    ICECandidateStruct candidate1;
    candidate1.candidate = chip::CharSpan("candidate1", 10);
    testCandidates.push_back(candidate1);

    ICECandidateStruct candidate2;
    candidate2.candidate = chip::CharSpan("candidate2", 10);
    testCandidates.push_back(candidate2);

    // Test successful ICE candidates handling
    mockDelegate.SetICECandidatesResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleICECandidates(testSessionId, testCandidates);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), testSessionId);
    EXPECT_EQ(mockDelegate.GetLastCandidates().size(), 2u);

    // Test error case
    mockDelegate.SetICECandidatesResult(CHIP_ERROR_INVALID_ARGUMENT);
    result = mockDelegate.HandleICECandidates(testSessionId, testCandidates);
    EXPECT_EQ(result, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestWebRTCTransportRequestorCluster, TestDelegateHandleEnd)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t testSessionId         = 999;
    WebRTCEndReasonEnum testReason = WebRTCEndReasonEnum::kInviteTimeout;

    // Test successful end handling
    mockDelegate.SetEndResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleEnd(testSessionId, testReason);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), testSessionId);
    EXPECT_EQ(mockDelegate.GetLastEndReason(), testReason);

    // Test different reason codes
    WebRTCEndReasonEnum testReason2 = WebRTCEndReasonEnum::kUserBusy;
    result                          = mockDelegate.HandleEnd(testSessionId, testReason2);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastEndReason(), testReason2);

    // Test error case
    mockDelegate.SetEndResult(CHIP_ERROR_TIMEOUT);
    result = mockDelegate.HandleEnd(testSessionId, testReason);
    EXPECT_EQ(result, CHIP_ERROR_TIMEOUT);
}

TEST_F(TestWebRTCTransportRequestorCluster, TestReadCurrentSessionsAttribute)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Create a mock attribute request for CurrentSessions
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = WebRTCTransportRequestor::Id;
    request.path.mAttributeId = WebRTCTransportRequestor::Attributes::CurrentSessions::Id;

    // Create a buffer for encoding
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
    ASSERT_TRUE(buffer.Alloc(1024));
    chip::TLV::TLVWriter writer;
    writer.Init(buffer.Get(), buffer.AllocatedSize());

    // Create AttributeReportIBs::Builder for the encoder
    chip::app::AttributeReportIBs::Builder attributeReportIBsBuilder;
    chip::TLV::TLVWriter reportWriter;
    reportWriter.Init(buffer.Get(), buffer.AllocatedSize());
    CHIP_ERROR err = attributeReportIBsBuilder.Init(&reportWriter);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    chip::app::AttributeValueEncoder encoder(attributeReportIBsBuilder, chip::Access::SubjectDescriptor{}, request.path,
                                             0 /* dataVersion */);

    // Test reading empty sessions
    auto status = server.ReadAttribute(request, encoder);
    EXPECT_TRUE(status.IsSuccess());
}

TEST_F(TestWebRTCTransportRequestorCluster, TestReadClusterRevisionAttribute)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Create a mock attribute request for ClusterRevision
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = WebRTCTransportRequestor::Id;
    request.path.mAttributeId = chip::app::Clusters::Globals::Attributes::ClusterRevision::Id;

    // Create a buffer for encoding
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
    ASSERT_TRUE(buffer.Alloc(1024));

    // Create AttributeReportIBs::Builder for the encoder
    chip::app::AttributeReportIBs::Builder attributeReportIBsBuilder;
    chip::TLV::TLVWriter reportWriter;
    reportWriter.Init(buffer.Get(), buffer.AllocatedSize());
    CHIP_ERROR err = attributeReportIBsBuilder.Init(&reportWriter);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    chip::app::AttributeValueEncoder encoder(attributeReportIBsBuilder, chip::Access::SubjectDescriptor{}, request.path,
                                             0 /* dataVersion */);

    // Test reading cluster revision
    auto status = server.ReadAttribute(request, encoder);
    EXPECT_TRUE(status.IsSuccess());
}

TEST_F(TestWebRTCTransportRequestorCluster, TestReadUnsupportedAttribute)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Create a mock attribute request for an unsupported attribute
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = WebRTCTransportRequestor::Id;
    request.path.mAttributeId = 0xFFFF; // Invalid attribute ID

    // Create a buffer for encoding
    chip::Platform::ScopedMemoryBufferWithSize<uint8_t> buffer;
    ASSERT_TRUE(buffer.Alloc(1024));

    // Create AttributeReportIBs::Builder for the encoder
    chip::app::AttributeReportIBs::Builder attributeReportIBsBuilder;
    chip::TLV::TLVWriter reportWriter;
    reportWriter.Init(buffer.Get(), buffer.AllocatedSize());
    CHIP_ERROR err = attributeReportIBsBuilder.Init(&reportWriter);
    ASSERT_EQ(err, CHIP_NO_ERROR);

    chip::app::AttributeValueEncoder encoder(attributeReportIBsBuilder, chip::Access::SubjectDescriptor{}, request.path,
                                             0 /* dataVersion */);

    // Test reading unsupported attribute
    auto status = server.ReadAttribute(request, encoder);
    EXPECT_EQ(status, chip::Protocols::InteractionModel::Status::UnsupportedAttribute);
}

} // namespace
