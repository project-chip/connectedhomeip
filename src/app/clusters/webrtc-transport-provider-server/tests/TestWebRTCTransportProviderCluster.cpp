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

using ICEServerDecodableStruct = chip::app::Clusters::Globals::Structs::ICEServerStruct::DecodableType;
using WebRTCSessionStruct      = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;
using ICECandidateStruct       = chip::app::Clusters::Globals::Structs::ICECandidateStruct::Type;
using StreamUsageEnum          = chip::app::Clusters::Globals::StreamUsageEnum;
using WebRTCEndReasonEnum      = chip::app::Clusters::Globals::WebRTCEndReasonEnum;

static constexpr chip::EndpointId kTestEndpointId = 1;

// Mock delegate for testing WebRTCTransportProvider
class MockWebRTCTransportProviderDelegate : public Delegate
{
public:
    MockWebRTCTransportProviderDelegate() = default;

    CHIP_ERROR HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession, bool & outDeferredOffer) override
    {
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

    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode, DataModel::Nullable<uint16_t> videoStreamID,
                                DataModel::Nullable<uint16_t> audioStreamID) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                   Optional<DataModel::Nullable<uint16_t>> & audioStreamId) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override { return CHIP_NO_ERROR; }

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
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestWebRTCTransportProviderCluster, TestAttributes)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    ASSERT_TRUE(Testing::IsAttributesListEqualTo(server, { WebRTCTransportProvider::Attributes::CurrentSessions::kMetadataEntry }));
}

TEST_F(TestWebRTCTransportProviderCluster, TestCommands)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    ASSERT_TRUE(Testing::IsAcceptedCommandsListEqualTo(server,
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
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Create a mock attribute request for CurrentSessions
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = WebRTCTransportProvider::Id;
    request.path.mAttributeId = WebRTCTransportProvider::Attributes::CurrentSessions::Id;

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

TEST_F(TestWebRTCTransportProviderCluster, TestReadClusterRevisionAttribute)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Create a mock attribute request for ClusterRevision
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = WebRTCTransportProvider::Id;
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

TEST_F(TestWebRTCTransportProviderCluster, TestReadUnsupportedAttribute)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Create a mock attribute request for an unsupported attribute
    chip::app::DataModel::ReadAttributeRequest request;
    request.path.mEndpointId  = kTestEndpointId;
    request.path.mClusterId   = WebRTCTransportProvider::Id;
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
