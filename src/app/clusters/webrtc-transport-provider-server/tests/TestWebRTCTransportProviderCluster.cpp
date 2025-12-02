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
#include <app/clusters/webrtc-transport-provider-server/WebRTCTransportProviderCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/Decode.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <clusters/WebRTCTransportProvider/Attributes.h>
#include <clusters/WebRTCTransportProvider/Commands.h>
#include <clusters/WebRTCTransportProvider/Enums.h>
#include <clusters/WebRTCTransportProvider/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
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
        mLastSolicitOfferArgs = args;
        if (mSolicitOfferResult == CHIP_NO_ERROR)
        {
            outSession.id          = args.sessionId;
            outSession.peerNodeID  = args.peerNodeId;
            outSession.fabricIndex = args.fabricIndex;
            outSession.streamUsage = args.streamUsage;
            outDeferredOffer       = mDeferredOffer;
        }
        return mSolicitOfferResult;
    }

    CHIP_ERROR HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession) override
    {
        mLastProvideOfferArgs = args;
        if (mProvideOfferResult == CHIP_NO_ERROR)
        {
            outSession.id          = args.sessionId;
            outSession.peerNodeID  = args.peerNodeId;
            outSession.fabricIndex = args.fabricIndex;
            outSession.streamUsage = args.streamUsage;
        }
        return mProvideOfferResult;
    }

    CHIP_ERROR HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer) override
    {
        mLastSessionId = sessionId;
        mLastSdpAnswer = sdpAnswer;
        return mProvideAnswerResult;
    }

    CHIP_ERROR HandleProvideICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates) override
    {
        mLastSessionId  = sessionId;
        mLastCandidates = candidates;
        return mProvideICECandidatesResult;
    }

    CHIP_ERROR HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode, DataModel::Nullable<uint16_t> videoStreamID,
                                DataModel::Nullable<uint16_t> audioStreamID) override
    {
        mLastSessionId     = sessionId;
        mLastEndReason     = reasonCode;
        mLastVideoStreamID = videoStreamID;
        mLastAudioStreamID = audioStreamID;
        return mEndSessionResult;
    }

    CHIP_ERROR ValidateStreamUsage(StreamUsageEnum streamUsage, Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                   Optional<DataModel::Nullable<uint16_t>> & audioStreamId) override
    {
        return mValidateStreamUsageResult;
    }

    CHIP_ERROR ValidateVideoStreamID(uint16_t videoStreamId) override { return mValidateVideoStreamIDResult; }

    CHIP_ERROR ValidateAudioStreamID(uint16_t audioStreamId) override { return mValidateAudioStreamIDResult; }

    CHIP_ERROR IsStreamUsageSupported(Globals::StreamUsageEnum streamUsage) override { return mIsStreamUsageSupportedResult; }

    CHIP_ERROR IsHardPrivacyModeActive(bool & isActive) override
    {
        isActive = mIsHardPrivacyModeActive;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftRecordingPrivacyModeActive(bool & isActive) override
    {
        isActive = mIsSoftRecordingPrivacyModeActive;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR IsSoftLivestreamPrivacyModeActive(bool & isActive) override
    {
        isActive = mIsSoftLivestreamPrivacyModeActive;
        return CHIP_NO_ERROR;
    }

    bool HasAllocatedVideoStreams() override { return mHasAllocatedVideoStreams; }

    bool HasAllocatedAudioStreams() override { return mHasAllocatedAudioStreams; }

    CHIP_ERROR ValidateSFrameConfig(uint16_t cipherSuite, size_t baseKeyLength) override { return mValidateSFrameConfigResult; }

    CHIP_ERROR IsUTCTimeNull(bool & isNull) override
    {
        isNull = mIsUTCTimeNull;
        return CHIP_NO_ERROR;
    }

    // Test setup methods
    void SetSolicitOfferResult(CHIP_ERROR result) { mSolicitOfferResult = result; }
    void SetProvideOfferResult(CHIP_ERROR result) { mProvideOfferResult = result; }
    void SetProvideAnswerResult(CHIP_ERROR result) { mProvideAnswerResult = result; }
    void SetProvideICECandidatesResult(CHIP_ERROR result) { mProvideICECandidatesResult = result; }
    void SetEndSessionResult(CHIP_ERROR result) { mEndSessionResult = result; }
    void SetDeferredOffer(bool deferred) { mDeferredOffer = deferred; }
    void SetValidateStreamUsageResult(CHIP_ERROR result) { mValidateStreamUsageResult = result; }
    void SetValidateVideoStreamIDResult(CHIP_ERROR result) { mValidateVideoStreamIDResult = result; }
    void SetValidateAudioStreamIDResult(CHIP_ERROR result) { mValidateAudioStreamIDResult = result; }
    void SetIsStreamUsageSupportedResult(CHIP_ERROR result) { mIsStreamUsageSupportedResult = result; }
    void SetValidateSFrameConfigResult(CHIP_ERROR result) { mValidateSFrameConfigResult = result; }
    void SetHardPrivacyModeActive(bool active) { mIsHardPrivacyModeActive = active; }
    void SetSoftRecordingPrivacyModeActive(bool active) { mIsSoftRecordingPrivacyModeActive = active; }
    void SetSoftLivestreamPrivacyModeActive(bool active) { mIsSoftLivestreamPrivacyModeActive = active; }
    void SetHasAllocatedVideoStreams(bool has) { mHasAllocatedVideoStreams = has; }
    void SetHasAllocatedAudioStreams(bool has) { mHasAllocatedAudioStreams = has; }
    void SetUTCTimeNull(bool isNull) { mIsUTCTimeNull = isNull; }

    // Getters for verification
    uint16_t GetLastSessionId() const { return mLastSessionId; }
    const OfferRequestArgs & GetLastSolicitOfferArgs() const { return mLastSolicitOfferArgs; }
    const ProvideOfferRequestArgs & GetLastProvideOfferArgs() const { return mLastProvideOfferArgs; }
    const std::string & GetLastSdpAnswer() const { return mLastSdpAnswer; }
    const std::vector<ICECandidateStruct> & GetLastCandidates() const { return mLastCandidates; }
    WebRTCEndReasonEnum GetLastEndReason() const { return mLastEndReason; }

private:
    uint16_t mLastSessionId = 0;
    OfferRequestArgs mLastSolicitOfferArgs;
    ProvideOfferRequestArgs mLastProvideOfferArgs;
    std::string mLastSdpAnswer;
    std::vector<ICECandidateStruct> mLastCandidates;
    WebRTCEndReasonEnum mLastEndReason = WebRTCEndReasonEnum::kUnknownEnumValue;
    DataModel::Nullable<uint16_t> mLastVideoStreamID;
    DataModel::Nullable<uint16_t> mLastAudioStreamID;

    CHIP_ERROR mSolicitOfferResult           = CHIP_NO_ERROR;
    CHIP_ERROR mProvideOfferResult           = CHIP_NO_ERROR;
    CHIP_ERROR mProvideAnswerResult          = CHIP_NO_ERROR;
    CHIP_ERROR mProvideICECandidatesResult   = CHIP_NO_ERROR;
    CHIP_ERROR mEndSessionResult             = CHIP_NO_ERROR;
    CHIP_ERROR mValidateStreamUsageResult    = CHIP_NO_ERROR;
    CHIP_ERROR mValidateVideoStreamIDResult  = CHIP_NO_ERROR;
    CHIP_ERROR mValidateAudioStreamIDResult  = CHIP_NO_ERROR;
    CHIP_ERROR mIsStreamUsageSupportedResult = CHIP_NO_ERROR;
    CHIP_ERROR mValidateSFrameConfigResult   = CHIP_NO_ERROR;

    bool mDeferredOffer                     = false;
    bool mIsHardPrivacyModeActive           = false;
    bool mIsSoftRecordingPrivacyModeActive  = false;
    bool mIsSoftLivestreamPrivacyModeActive = false;
    bool mHasAllocatedVideoStreams          = true;
    bool mHasAllocatedAudioStreams          = true;
    bool mIsUTCTimeNull                     = false;
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

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> builder;
    ASSERT_EQ(server.Attributes({ kTestEndpointId, WebRTCTransportProvider::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.AppendElements({
                  WebRTCTransportProvider::Attributes::CurrentSessions::kMetadataEntry,
              }),
              CHIP_NO_ERROR);
    ASSERT_EQ(expectedBuilder.ReferenceExisting(chip::app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);
}

TEST_F(TestWebRTCTransportProviderCluster, TestCommands)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> builder;
    ASSERT_EQ(server.AcceptedCommands({ kTestEndpointId, WebRTCTransportProvider::Id }, builder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedBuilder;
    ASSERT_EQ(expectedBuilder.AppendElements({
                  WebRTCTransportProvider::Commands::SolicitOffer::kMetadataEntry,
                  WebRTCTransportProvider::Commands::ProvideOffer::kMetadataEntry,
                  WebRTCTransportProvider::Commands::ProvideAnswer::kMetadataEntry,
                  WebRTCTransportProvider::Commands::ProvideICECandidates::kMetadataEntry,
                  WebRTCTransportProvider::Commands::EndSession::kMetadataEntry,
              }),
              CHIP_NO_ERROR);

    EXPECT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
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

TEST_F(TestWebRTCTransportProviderCluster, TestDelegateHandleSolicitOffer)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Setup test data
    Delegate::OfferRequestArgs args;
    args.sessionId   = 123;
    args.streamUsage = StreamUsageEnum::kLiveView;
    args.peerNodeId  = 0x1234ULL;
    args.fabricIndex = 1;

    WebRTCSessionStruct outSession;
    bool outDeferredOffer = false;

    // Test successful solicit offer
    mockDelegate.SetSolicitOfferResult(CHIP_NO_ERROR);
    mockDelegate.SetDeferredOffer(false);

    CHIP_ERROR result = mockDelegate.HandleSolicitOffer(args, outSession, outDeferredOffer);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(outSession.id, 123);
    EXPECT_EQ(outSession.streamUsage, StreamUsageEnum::kLiveView);
    EXPECT_FALSE(outDeferredOffer);

    // Test deferred offer
    mockDelegate.SetDeferredOffer(true);
    result = mockDelegate.HandleSolicitOffer(args, outSession, outDeferredOffer);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_TRUE(outDeferredOffer);

    // Test error case
    mockDelegate.SetSolicitOfferResult(CHIP_ERROR_NO_MEMORY);
    result = mockDelegate.HandleSolicitOffer(args, outSession, outDeferredOffer);
    EXPECT_EQ(result, CHIP_ERROR_NO_MEMORY);
}

TEST_F(TestWebRTCTransportProviderCluster, TestDelegateHandleProvideOffer)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Setup test data
    Delegate::ProvideOfferRequestArgs args;
    args.sessionId   = 456;
    args.streamUsage = StreamUsageEnum::kRecording;
    args.peerNodeId  = 0x5678ULL;
    args.fabricIndex = 1;
    args.sdp         = "test_sdp_offer";

    WebRTCSessionStruct outSession;

    // Test successful provide offer
    mockDelegate.SetProvideOfferResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleProvideOffer(args, outSession);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(outSession.id, 456);
    EXPECT_EQ(outSession.streamUsage, StreamUsageEnum::kRecording);
    EXPECT_EQ(mockDelegate.GetLastProvideOfferArgs().sdp, "test_sdp_offer");

    // Test error case
    mockDelegate.SetProvideOfferResult(CHIP_ERROR_INVALID_ARGUMENT);
    result = mockDelegate.HandleProvideOffer(args, outSession);
    EXPECT_EQ(result, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestWebRTCTransportProviderCluster, TestDelegateHandleProvideAnswer)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t sessionId        = 789;
    std::string testSdpAnswer = "test_sdp_answer";

    // Test successful answer handling
    mockDelegate.SetProvideAnswerResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleProvideAnswer(sessionId, testSdpAnswer);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), 789);
    EXPECT_EQ(mockDelegate.GetLastSdpAnswer(), testSdpAnswer);

    // Test error case
    mockDelegate.SetProvideAnswerResult(CHIP_ERROR_INTERNAL);
    result = mockDelegate.HandleProvideAnswer(sessionId, testSdpAnswer);
    EXPECT_EQ(result, CHIP_ERROR_INTERNAL);
}

TEST_F(TestWebRTCTransportProviderCluster, TestDelegateHandleProvideICECandidates)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t sessionId = 999;
    std::vector<ICECandidateStruct> testCandidates;

    ICECandidateStruct candidate1;
    candidate1.candidate = chip::CharSpan("candidate1", 10);
    testCandidates.push_back(candidate1);

    ICECandidateStruct candidate2;
    candidate2.candidate = chip::CharSpan("candidate2", 10);
    testCandidates.push_back(candidate2);

    // Test successful ICE candidates handling
    mockDelegate.SetProvideICECandidatesResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleProvideICECandidates(sessionId, testCandidates);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), 999);
    EXPECT_EQ(mockDelegate.GetLastCandidates().size(), 2u);

    // Test error case
    mockDelegate.SetProvideICECandidatesResult(CHIP_ERROR_INVALID_ARGUMENT);
    result = mockDelegate.HandleProvideICECandidates(sessionId, testCandidates);
    EXPECT_EQ(result, CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestWebRTCTransportProviderCluster, TestDelegateHandleEndSession)
{
    MockWebRTCTransportProviderDelegate mockDelegate;
    WebRTCTransportProviderCluster server(kTestEndpointId, mockDelegate);

    // Setup test data
    uint16_t sessionId             = 111;
    WebRTCEndReasonEnum testReason = WebRTCEndReasonEnum::kInviteTimeout;
    DataModel::Nullable<uint16_t> videoStreamID(1);
    DataModel::Nullable<uint16_t> audioStreamID(2);

    // Test successful end session handling
    mockDelegate.SetEndSessionResult(CHIP_NO_ERROR);

    CHIP_ERROR result = mockDelegate.HandleEndSession(sessionId, testReason, videoStreamID, audioStreamID);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastSessionId(), 111);
    EXPECT_EQ(mockDelegate.GetLastEndReason(), testReason);

    // Test different reason codes
    WebRTCEndReasonEnum testReason2 = WebRTCEndReasonEnum::kUserBusy;
    result                          = mockDelegate.HandleEndSession(sessionId, testReason2, videoStreamID, audioStreamID);
    EXPECT_EQ(result, CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetLastEndReason(), testReason2);

    // Test error case
    mockDelegate.SetEndSessionResult(CHIP_ERROR_TIMEOUT);
    result = mockDelegate.HandleEndSession(sessionId, testReason, videoStreamID, audioStreamID);
    EXPECT_EQ(result, CHIP_ERROR_TIMEOUT);
}

TEST_F(TestWebRTCTransportProviderCluster, TestValidationMethods)
{
    MockWebRTCTransportProviderDelegate mockDelegate;

    // Test ValidateStreamUsage
    mockDelegate.SetValidateStreamUsageResult(CHIP_NO_ERROR);
    Optional<DataModel::Nullable<uint16_t>> videoId;
    Optional<DataModel::Nullable<uint16_t>> audioId;
    EXPECT_EQ(mockDelegate.ValidateStreamUsage(StreamUsageEnum::kLiveView, videoId, audioId), CHIP_NO_ERROR);

    mockDelegate.SetValidateStreamUsageResult(CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mockDelegate.ValidateStreamUsage(StreamUsageEnum::kRecording, videoId, audioId), CHIP_ERROR_INVALID_ARGUMENT);

    // Test ValidateVideoStreamID
    mockDelegate.SetValidateVideoStreamIDResult(CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.ValidateVideoStreamID(1), CHIP_NO_ERROR);

    mockDelegate.SetValidateVideoStreamIDResult(CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mockDelegate.ValidateVideoStreamID(999), CHIP_ERROR_NOT_FOUND);

    // Test ValidateAudioStreamID
    mockDelegate.SetValidateAudioStreamIDResult(CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.ValidateAudioStreamID(1), CHIP_NO_ERROR);

    mockDelegate.SetValidateAudioStreamIDResult(CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mockDelegate.ValidateAudioStreamID(999), CHIP_ERROR_NOT_FOUND);

    // Test IsStreamUsageSupported
    mockDelegate.SetIsStreamUsageSupportedResult(CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.IsStreamUsageSupported(StreamUsageEnum::kLiveView), CHIP_NO_ERROR);

    mockDelegate.SetIsStreamUsageSupportedResult(CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(mockDelegate.IsStreamUsageSupported(StreamUsageEnum::kUnknownEnumValue), CHIP_ERROR_NOT_FOUND);

    // Test ValidateSFrameConfig
    mockDelegate.SetValidateSFrameConfigResult(CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.ValidateSFrameConfig(1, 16), CHIP_NO_ERROR);

    mockDelegate.SetValidateSFrameConfigResult(CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mockDelegate.ValidateSFrameConfig(999, 100), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestWebRTCTransportProviderCluster, TestPrivacyModeMethods)
{
    MockWebRTCTransportProviderDelegate mockDelegate;

    // Test HardPrivacyMode
    mockDelegate.SetHardPrivacyModeActive(false);
    bool isActive = false;
    EXPECT_EQ(mockDelegate.IsHardPrivacyModeActive(isActive), CHIP_NO_ERROR);
    EXPECT_FALSE(isActive);

    mockDelegate.SetHardPrivacyModeActive(true);
    EXPECT_EQ(mockDelegate.IsHardPrivacyModeActive(isActive), CHIP_NO_ERROR);
    EXPECT_TRUE(isActive);

    // Test SoftRecordingPrivacyMode
    mockDelegate.SetSoftRecordingPrivacyModeActive(false);
    EXPECT_EQ(mockDelegate.IsSoftRecordingPrivacyModeActive(isActive), CHIP_NO_ERROR);
    EXPECT_FALSE(isActive);

    mockDelegate.SetSoftRecordingPrivacyModeActive(true);
    EXPECT_EQ(mockDelegate.IsSoftRecordingPrivacyModeActive(isActive), CHIP_NO_ERROR);
    EXPECT_TRUE(isActive);

    // Test SoftLivestreamPrivacyMode
    mockDelegate.SetSoftLivestreamPrivacyModeActive(false);
    EXPECT_EQ(mockDelegate.IsSoftLivestreamPrivacyModeActive(isActive), CHIP_NO_ERROR);
    EXPECT_FALSE(isActive);

    mockDelegate.SetSoftLivestreamPrivacyModeActive(true);
    EXPECT_EQ(mockDelegate.IsSoftLivestreamPrivacyModeActive(isActive), CHIP_NO_ERROR);
    EXPECT_TRUE(isActive);
}

TEST_F(TestWebRTCTransportProviderCluster, TestStreamAvailabilityMethods)
{
    MockWebRTCTransportProviderDelegate mockDelegate;

    // Test HasAllocatedVideoStreams
    mockDelegate.SetHasAllocatedVideoStreams(true);
    EXPECT_TRUE(mockDelegate.HasAllocatedVideoStreams());

    mockDelegate.SetHasAllocatedVideoStreams(false);
    EXPECT_FALSE(mockDelegate.HasAllocatedVideoStreams());

    // Test HasAllocatedAudioStreams
    mockDelegate.SetHasAllocatedAudioStreams(true);
    EXPECT_TRUE(mockDelegate.HasAllocatedAudioStreams());

    mockDelegate.SetHasAllocatedAudioStreams(false);
    EXPECT_FALSE(mockDelegate.HasAllocatedAudioStreams());
}

TEST_F(TestWebRTCTransportProviderCluster, TestUTCTimeMethods)
{
    MockWebRTCTransportProviderDelegate mockDelegate;

    // Test IsUTCTimeNull
    mockDelegate.SetUTCTimeNull(false);
    bool isNull = true;
    EXPECT_EQ(mockDelegate.IsUTCTimeNull(isNull), CHIP_NO_ERROR);
    EXPECT_FALSE(isNull);

    mockDelegate.SetUTCTimeNull(true);
    EXPECT_EQ(mockDelegate.IsUTCTimeNull(isNull), CHIP_NO_ERROR);
    EXPECT_TRUE(isNull);
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
