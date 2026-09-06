/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/av-analysis-server/DefaultAvAnalysisWebRTCClient.h>
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/WebRTCTransportProvider/Ids.h>
#include <lib/core/TLV.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace Protocols::InteractionModel;

constexpr EndpointId kProviderEndpoint = 2;
const ScopedNodeId kCameraNode(0x1234, 1);
constexpr uint8_t kMaxSessions = 4;

// Intercepts the network boundary: records session requests instead of establishing CASE sessions,
// and captures the ProvideOffer payload instead of sending it. The camera's behavior is simulated
// by invoking the public callback methods.
class InterceptingWebRTCClient : public DefaultAvAnalysisWebRTCClient
{
public:
    using DefaultAvAnalysisWebRTCClient::CurrentRequest;
    using DefaultAvAnalysisWebRTCClient::HandleServerListReport;

    // Drives the request into the provider-check phase, as OnDeviceConnected would after CASE
    void EnterProviderCheck() { CurrentRequest().Advance(Request::Phase::kCheckingProvider); }

    int mConnectRequests = 0;
    ScopedNodeId mLastPeer;

    // The ProvideOffer payload the client would have sent
    int mSendAttempts = 0;
    bool mSentSessionIdWasNull;
    std::string mSentSdp;
    Globals::StreamUsageEnum mSentUsage;
    EndpointId mSentOriginatingEndpoint;
    Optional<DataModel::Nullable<uint16_t>> mSentVideoStreamId;
    bool mSentAudioAbsent;
    bool mSentIceAbsent;

    // The EndSession payload the client would have sent
    int mEndSendAttempts = 0;
    uint16_t mSentEndSessionId;
    Globals::WebRTCEndReasonEnum mSentEndReason;

    // Drives a pending EndSession request past CASE, as OnDeviceConnected would
    CHIP_ERROR SendPendingEndSession() { return SendEndSession(); }

protected:
    void EstablishSession(const ScopedNodeId & aCameraNode) override
    {
        mConnectRequests++;
        mLastPeer = aCameraNode;
    }

    CHIP_ERROR SendProvideOffer() override
    {
        mSendAttempts++;

        WebRTCTransportProvider::Commands::ProvideOffer::Type request;
        ReturnErrorOnFailure(BuildProvideOffer(request));
        mSentSessionIdWasNull    = request.webRTCSessionID.IsNull();
        mSentSdp                 = std::string(request.sdp.data(), request.sdp.size());
        mSentUsage               = request.streamUsage;
        mSentOriginatingEndpoint = request.originatingEndpointID;
        mSentVideoStreamId       = request.videoStreamID;
        mSentAudioAbsent         = !request.audioStreamID.HasValue();
        mSentIceAbsent           = !request.ICEServers.HasValue() && !request.ICETransportPolicy.HasValue();

        // camera's answer arrives via the sender callbacks
        CurrentRequest().Advance(Request::Phase::kInvoking);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendEndSession() override
    {
        mEndSendAttempts++;

        WebRTCTransportProvider::Commands::EndSession::Type request;
        ReturnErrorOnFailure(BuildEndSession(request));
        mSentEndSessionId = request.webRTCSessionID;
        mSentEndReason    = request.reason;

        CurrentRequest().Advance(Request::Phase::kInvoking);
        return CHIP_NO_ERROR;
    }
};

constexpr uint16_t kVideoStreamId = 42;

class RecordingCallback : public AvAnalysisWebRTCClient::Callback
{
public:
    int mInitiatedCount   = 0;
    int mActiveCount      = 0;
    int mFailedCount      = 0;
    int mEndedCount       = 0;
    Status mLastStatus    = Status::Success;
    uint16_t mLastSession = 0;

    void OnSessionInitiated(Status aStatus, uint16_t aWebRTCSessionId) override
    {
        mInitiatedCount++;
        mLastStatus  = aStatus;
        mLastSession = aWebRTCSessionId;
    }
    void OnSessionActive(uint16_t aWebRTCSessionId) override
    {
        mActiveCount++;
        mLastSession = aWebRTCSessionId;
    }
    void OnSessionFailed(uint16_t aWebRTCSessionId) override
    {
        mFailedCount++;
        mLastSession = aWebRTCSessionId;
    }
    void OnSessionEnded(Status aStatus, uint16_t aWebRTCSessionId) override
    {
        mEndedCount++;
        mLastStatus  = aStatus;
        mLastSession = aWebRTCSessionId;
    }
};

// Hands back a canned SDP synchronously; records the session lifecycle notifications
class FakePeerDelegate : public AvAnalysisWebRTCPeerDelegate
{
public:
    int mOffersRequested   = 0;
    int mOffersAbandoned   = 0;
    int mSessionsAssigned  = 0;
    int mSessionsClosed    = 0;
    uint16_t mLastAssigned = 0;
    uint16_t mLastClosed   = 0;

    CHIP_ERROR CreateOffer(OfferCallback & aCallback) override
    {
        mOffersRequested++;
        mLastOfferCallback = &aCallback;
        return mCreateOfferResult;
    }

    CHIP_ERROR mCreateOfferResult = CHIP_NO_ERROR;
    void OnSessionAssigned(uint16_t aWebRTCSessionId) override
    {
        mSessionsAssigned++;
        mLastAssigned = aWebRTCSessionId;
    }
    void OnOfferAbandoned() override { mOffersAbandoned++; }
    void OnSessionClosed(uint16_t aWebRTCSessionId) override
    {
        mSessionsClosed++;
        mLastClosed = aWebRTCSessionId;
    }

    OfferCallback * mLastOfferCallback = nullptr;
};

// The requestor cluster the client records sessions into
class StubRequestorDelegate : public WebRTCTransportRequestor::Delegate
{
public:
    CHIP_ERROR HandleOffer(const WebRTCTransportRequestor::WebRTCSessionStruct &, const OfferArgs &) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR HandleAnswer(const WebRTCTransportRequestor::WebRTCSessionStruct &, const std::string &) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR HandleICECandidates(const WebRTCTransportRequestor::WebRTCSessionStruct &,
                                   const std::vector<WebRTCTransportRequestor::ICECandidateStruct> &) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR HandleEnd(const WebRTCTransportRequestor::WebRTCSessionStruct &,
                         WebRTCTransportRequestor::WebRTCEndReasonEnum) override
    {
        return CHIP_NO_ERROR;
    }
};

// Feeds a crafted ServerList report for the provider endpoint into the client
void FeedServerList(InterceptingWebRTCClient & aClient, EndpointId aEndpoint, Span<const ClusterId> aClusters)
{
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    TLV::TLVType outer;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer), CHIP_NO_ERROR);
    for (ClusterId cluster : aClusters)
    {
        ASSERT_EQ(writer.Put(TLV::AnonymousTag(), cluster), CHIP_NO_ERROR);
    }
    ASSERT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    aClient.HandleServerListReport(ConcreteDataAttributePath(aEndpoint, Descriptor::Id, Descriptor::Attributes::ServerList::Id),
                                   reader);
}

struct TestDefaultAvAnalysisWebRTCClient : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        ASSERT_EQ(mClient.Init(&mCASESessionManager, &mPeerDelegate, &mRequestorCluster, kMaxSessions), CHIP_NO_ERROR);
    }

    // Drives a request up to the application's turn: CASE up, provider present, offer asked for
    void DriveToOffer() { DriveToOffer(mCallback); }
    void DriveToOffer(RecordingCallback & aCallback)
    {
        ASSERT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, aCallback), CHIP_NO_ERROR);
        mClient.EnterProviderCheck();
        const ClusterId kProviderList[] = { WebRTCTransportProvider::Id };
        FeedServerList(mClient, kProviderEndpoint, Span<const ClusterId>(kProviderList));
        mClient.OnDone(static_cast<ReadClient *>(nullptr));
    }

    // Feeds the camera's ProvideOfferResponse{sessionId, videoStreamId} into the client
    void FeedOfferResponse(uint16_t aWebRTCSessionId)
    {
        using Fields = WebRTCTransportProvider::Commands::ProvideOfferResponse::Fields;

        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        TLV::TLVType containerType;
        ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType), CHIP_NO_ERROR);
        ASSERT_EQ(writer.Put(TLV::ContextTag(Fields::kWebRTCSessionID), aWebRTCSessionId), CHIP_NO_ERROR);
        ASSERT_EQ(writer.Put(TLV::ContextTag(Fields::kVideoStreamID), kVideoStreamId), CHIP_NO_ERROR);
        ASSERT_EQ(writer.EndContainer(containerType), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);

        ConcreteCommandPath responsePath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                         WebRTCTransportProvider::Commands::ProvideOfferResponse::Id);
        mClient.OnResponse(nullptr, responsePath, StatusIB(), &reader);
    }

    // The full successful offer exchange, ending with the session tracked under aWebRTCSessionId
    void EstablishSessionWithId(uint16_t aWebRTCSessionId) { EstablishSessionWithId(aWebRTCSessionId, mCallback); }
    void EstablishSessionWithId(uint16_t aWebRTCSessionId, RecordingCallback & aCallback)
    {
        DriveToOffer(aCallback);
        ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
        mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
        FeedOfferResponse(aWebRTCSessionId);
        mClient.OnDone(static_cast<CommandSender *>(nullptr));
    }

    // Requests the end of a tracked session and sends the command, up to the camera's answer
    void DriveToEndSessionSent(uint16_t aWebRTCSessionId)
    {
        ASSERT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, aWebRTCSessionId, mCallback), CHIP_NO_ERROR);
        ASSERT_EQ(mClient.SendPendingEndSession(), CHIP_NO_ERROR);
    }

    // Feeds the camera's status-only answer to EndSession into the client
    void FeedEndSessionStatus(Status aStatus)
    {
        ConcreteCommandPath responsePath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                         WebRTCTransportProvider::Commands::EndSession::Id);
        mClient.OnResponse(nullptr, responsePath, StatusIB(aStatus), nullptr);
    }

    // InterceptingWebRTCClient overrides EstablishSession.
    CASESessionManager mCASESessionManager;
    InterceptingWebRTCClient mClient;
    RecordingCallback mCallback;
    FakePeerDelegate mPeerDelegate;
    StubRequestorDelegate mRequestorDelegate;
    WebRTCTransportRequestor::WebRTCTransportRequestorCluster mRequestorCluster{ 1, mRequestorDelegate };
};

TEST_F(TestDefaultAvAnalysisWebRTCClient, InitArgumentValidation)
{
    DefaultAvAnalysisWebRTCClient client;
    EXPECT_EQ(client.Init(nullptr, &mPeerDelegate, &mRequestorCluster, kMaxSessions), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager, nullptr, &mRequestorCluster, kMaxSessions), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager, &mPeerDelegate, nullptr, kMaxSessions), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager, &mPeerDelegate, &mRequestorCluster, 0), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(client.Init(&mCASESessionManager, &mPeerDelegate, &mRequestorCluster, kMaxSessions), CHIP_NO_ERROR);
    EXPECT_EQ(client.Init(&mCASESessionManager, &mPeerDelegate, &mRequestorCluster, kMaxSessions), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, RequestStartsSessionEstablishment)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mConnectRequests, 1);
    EXPECT_EQ(mClient.mLastPeer, kCameraNode);
    EXPECT_EQ(mCallback.mInitiatedCount, 0); // Nothing completed yet
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, SecondRequestWhilePendingIsBusy)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_ERROR_BUSY);
    EXPECT_EQ(mClient.mConnectRequests, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, EndingAnUntrackedSessionIsRejected)
{
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mClient.mConnectRequests, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, MissingProviderClusterIsNotFound)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // The endpoint's ServerList lacks WebRTCTransportProvider
    const ClusterId kOtherClusters[] = { Descriptor::Id, 0x0557 };
    FeedServerList(mClient, kProviderEndpoint, Span<const ClusterId>(kOtherClusters));
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);

    // Completed: the client accepts a new request again
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ReportsForOtherPathsDoNotSatisfyTheProviderCheck)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // The provider exists, but on a different endpoint than the command named
    const ClusterId kProviderList[] = { WebRTCTransportProvider::Id };
    FeedServerList(mClient, static_cast<EndpointId>(kProviderEndpoint + 1), Span<const ClusterId>(kProviderList));
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CancelSilentlyAbandonsTheRequest)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.Cancel();

    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ProviderFoundRequestsAnOfferFromTheApplication)
{
    DriveToOffer();

    EXPECT_EQ(mPeerDelegate.mOffersRequested, 1);
    EXPECT_EQ(mClient.mSendAttempts, 0);     // No SDP yet, nothing to send
    EXPECT_EQ(mCallback.mInitiatedCount, 0); // Awaiting the application
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, OfferHookRefusalFailsTheRequest)
{
    mPeerDelegate.mCreateOfferResult = CHIP_ERROR_INTERNAL;
    DriveToOffer();

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);

    // Completed: the client accepts a new request again
    mPeerDelegate.mCreateOfferResult = CHIP_NO_ERROR;
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ApplicationOfferFailureFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);

    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_ERROR_INTERNAL, CharSpan());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mClient.mSendAttempts, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, StaleOfferDeliveryIsIgnored)
{
    mClient.OnOfferReady(CHIP_NO_ERROR, "v=0 stale offer"_span);

    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mClient.mSendAttempts, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, OfferedSessionIsEstablishedAndRegistered)
{
    EstablishSessionWithId(55);

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastSession, 55);

    // The application was told which id its pending peer connection now has
    EXPECT_EQ(mPeerDelegate.mSessionsAssigned, 1);
    EXPECT_EQ(mPeerDelegate.mLastAssigned, 55);

    // The requestor cluster validates the camera's inbound commands against this record
    auto sessions = mRequestorCluster.GetCurrentSessions();
    ASSERT_EQ(sessions.size(), 1u);
    EXPECT_EQ(sessions[0].id, 55);
    EXPECT_EQ(sessions[0].peerNodeID, kCameraNode.GetNodeId());
    EXPECT_EQ(sessions[0].fabricIndex, kCameraNode.GetFabricIndex());
    EXPECT_EQ(sessions[0].peerEndpointID, kProviderEndpoint);
    EXPECT_EQ(sessions[0].streamUsage, Globals::StreamUsageEnum::kAnalysis);
    ASSERT_FALSE(sessions[0].videoStreamID.IsNull());
    EXPECT_EQ(sessions[0].videoStreamID.Value(), kVideoStreamId);

    // Tracked: ending this session is now a legitimate request
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, OfferPayloadFollowsTheNormalFlow)
{
    EstablishSessionWithId(55);

    ASSERT_EQ(mClient.mSendAttempts, 1);
    EXPECT_TRUE(mClient.mSentSessionIdWasNull); // A new session is the camera's to assign
    EXPECT_EQ(mClient.mSentSdp, "v=0 test offer");
    EXPECT_EQ(mClient.mSentUsage, Globals::StreamUsageEnum::kAnalysis);
    EXPECT_EQ(mClient.mSentOriginatingEndpoint, 1); // Where the requestor cluster is registered
    ASSERT_TRUE(mClient.mSentVideoStreamId.HasValue());
    ASSERT_FALSE(mClient.mSentVideoStreamId.Value().IsNull());
    EXPECT_EQ(mClient.mSentVideoStreamId.Value().Value(), kVideoStreamId);
    EXPECT_TRUE(mClient.mSentAudioAbsent); // No audio for analysis
    EXPECT_TRUE(mClient.mSentIceAbsent);   // ICE configuration is the camera's default
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CameraErrorStatusIsPropagatedVerbatim)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // The camera refuses the offer with a specific status
    mClient.OnError(static_cast<CommandSender *>(nullptr), StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
    EXPECT_EQ(mPeerDelegate.mSessionsAssigned, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUnansweredExchangeFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // The exchange ends with neither a response nor an error
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, SessionSlotExhaustionIsResourceExhausted)
{
    for (uint16_t id = 101; id < 101 + kMaxSessions; id++)
    {
        EstablishSessionWithId(id);
    }
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);

    // The camera grants one more session than this node can track
    EstablishSessionWithId(200);

    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
    EXPECT_EQ(mPeerDelegate.mSessionsAssigned, kMaxSessions);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), static_cast<size_t>(kMaxSessions));
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, EndSessionPayloadNamesTheSessionWithUserHangup)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    ASSERT_EQ(mClient.mEndSendAttempts, 1);
    EXPECT_EQ(mClient.mSentEndSessionId, 55);
    EXPECT_EQ(mClient.mSentEndReason, Globals::WebRTCEndReasonEnum::kUserHangup);
    EXPECT_EQ(mCallback.mEndedCount, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ConfirmedEndReleasesTheSessionEverywhere)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    FeedEndSessionStatus(Status::Success);
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastSession, 55);

    // Gone from the requestor cluster, released by the application, no longer tracked
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mPeerDelegate.mLastClosed, 55);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_ERROR_INVALID_ARGUMENT);

    // The slot is reusable
    EstablishSessionWithId(56);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CameraErrorOnEndSessionIsPropagatedAndKeepsTheSession)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    // The camera refuses
    mClient.OnError(static_cast<CommandSender *>(nullptr), StatusIB(Status::NotFound).ToChipError());
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);

    // Nothing released: the session is still tracked and still recorded
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUnansweredEndSessionExchangeFailsAndKeepsTheSession)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnsweredSessionIsReportedActiveAndStaysTracked)
{
    EstablishSessionWithId(55);

    mClient.NotifyAnswered(55);

    EXPECT_EQ(mCallback.mActiveCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
    EXPECT_EQ(mCallback.mFailedCount, 0);

    // Active is not over: still recorded, still held by the application, still endable
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CameraEndedSessionIsReportedFailedAndReleased)
{
    EstablishSessionWithId(55);

    // The camera's End arrives on the requestor cluster, not as the outcome of our EndSession
    mClient.NotifyEnded(55);

    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
    EXPECT_EQ(mCallback.mEndedCount, 0);

    // Released everywhere: no record, no peer connection, no longer endable
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mPeerDelegate.mLastClosed, 55);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, MediaFailureIsReportedFailedAndReleased)
{
    EstablishSessionWithId(55);

    mClient.NotifyFailed(55);

    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);

    // The slot is reusable
    EstablishSessionWithId(56);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, SignalsForUntrackedSessionsAreIgnored)
{
    EstablishSessionWithId(55);

    mClient.NotifyAnswered(99);
    mClient.NotifyFailed(99);
    mClient.NotifyEnded(99);

    EXPECT_EQ(mCallback.mActiveCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 0);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, SignalsRouteToTheCallbackThatInitiatedTheSession)
{
    RecordingCallback otherCallback;
    EstablishSessionWithId(55);
    EstablishSessionWithId(56, otherCallback);
    ASSERT_EQ(otherCallback.mInitiatedCount, 1);

    mClient.NotifyAnswered(56);
    mClient.NotifyEnded(55);

    EXPECT_EQ(otherCallback.mActiveCount, 1);
    EXPECT_EQ(otherCallback.mFailedCount, 0);
    EXPECT_EQ(mCallback.mActiveCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnEstablishedSessionIsNotAnAbandonedOffer)
{
    EstablishSessionWithId(55);

    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferTheCameraRefusesIsAbandoned)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
    mClient.OnError(static_cast<CommandSender *>(nullptr), StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferLeftUnansweredIsAbandoned)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
    mClient.OnDone(static_cast<CommandSender *>(nullptr));

    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferTheApplicationCouldNotProduceIsAbandoned)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_ERROR_INTERNAL, CharSpan());

    // The application may have created the peer connection before failing to describe it
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUntrackableSessionAbandonsItsOffer)
{
    for (uint16_t id = 101; id < 101 + kMaxSessions; id++)
    {
        EstablishSessionWithId(id);
    }
    EstablishSessionWithId(200);

    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ARequestFailingBeforeTheOfferAbandonsNothing)
{
    // No WebRTCTransportProvider on the endpoint: the application was never asked for an offer
    ASSERT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);
    EXPECT_EQ(mPeerDelegate.mOffersRequested, 0);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CancelAbandonsThePendingOffer)
{
    DriveToOffer();
    ASSERT_EQ(mPeerDelegate.mOffersRequested, 1);

    mClient.Cancel();

    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
    EXPECT_EQ(mCallback.mInitiatedCount, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CancelReleasesEveryTrackedSessionSilently)
{
    EstablishSessionWithId(55);
    EstablishSessionWithId(56);

    mClient.Cancel();

    // Released everywhere, with no outcome delivered
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 2);
    EXPECT_EQ(mCallback.mFailedCount, 0);
    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_ERROR_INVALID_ARGUMENT);

    // Late signals for the forgotten sessions are ignored
    mClient.NotifyAnswered(55);
    mClient.NotifyEnded(56);
    EXPECT_EQ(mCallback.mActiveCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 0);

    // The client is fully reusable
    EstablishSessionWithId(57);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

} // namespace
