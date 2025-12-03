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
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>
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
using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

static constexpr chip::EndpointId kTestEndpointId = 1;

// Minimal mock delegate for testing
class MockWebRTCTransportRequestorDelegate : public Delegate
{
public:
    CHIP_ERROR HandleOffer(const WebRTCSessionStruct & session, const OfferArgs & args) override { return CHIP_NO_ERROR; }

    CHIP_ERROR HandleAnswer(const WebRTCSessionStruct & session, const std::string & sdpAnswer) override { return CHIP_NO_ERROR; }

    CHIP_ERROR HandleICECandidates(const WebRTCSessionStruct & session, const std::vector<ICECandidateStruct> & candidates) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HandleEnd(const WebRTCSessionStruct & session, WebRTCEndReasonEnum reasonCode) override { return CHIP_NO_ERROR; }
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

    ASSERT_TRUE(IsAttributesListEqualTo(server, { WebRTCTransportRequestor::Attributes::CurrentSessions::kMetadataEntry }));
}

TEST_F(TestWebRTCTransportRequestorCluster, TestCommands)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(server,
                                              {
                                                  WebRTCTransportRequestor::Commands::Offer::kMetadataEntry,
                                                  WebRTCTransportRequestor::Commands::Answer::kMetadataEntry,
                                                  WebRTCTransportRequestor::Commands::ICECandidates::kMetadataEntry,
                                                  WebRTCTransportRequestor::Commands::End::kMetadataEntry,
                                              }));
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
    testSession.peerNodeID  = 1;
    testSession.fabricIndex = 1;
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
    server.RemoveSession(1, testSession.peerNodeID, testSession.fabricIndex);
    sessions = server.GetCurrentSessions();
    EXPECT_TRUE(sessions.empty());
}

TEST_F(TestWebRTCTransportRequestorCluster, TestSessionManagement)
{
    MockWebRTCTransportRequestorDelegate mockDelegate;
    WebRTCTransportRequestorServer server(kTestEndpointId, mockDelegate);

    // Test adding multiple sessions
    WebRTCSessionStruct session1;
    session1.id          = 1;
    session1.peerNodeID  = 0x1234ULL;
    session1.fabricIndex = 1;

    WebRTCSessionStruct session2;
    session2.id          = 2;
    session2.peerNodeID  = 0x5678ULL;
    session2.fabricIndex = 1;

    server.UpsertSession(session1);
    server.UpsertSession(session2);

    auto sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 2u);

    // Remove specific session
    server.RemoveSession(1, session1.peerNodeID, session1.fabricIndex);
    sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 1u);
    EXPECT_EQ(sessions[0].id, 2);

    // Remove non-existent session (should be no-op)
    server.RemoveSession(999, 0, 0);
    sessions = server.GetCurrentSessions();
    EXPECT_EQ(sessions.size(), 1u);
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
