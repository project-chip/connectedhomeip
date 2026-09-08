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

constexpr EndpointId kProviderEndpoint  = 2;
constexpr EndpointId kRequestorEndpoint = 7;
const ScopedNodeId kCameraNode(0x1234, 1);
constexpr uint8_t kMaxSessions = 4;

// Intercepts the network boundary: records session requests instead of establishing CASE sessions,
// and captures the command payloads instead of sending them. The camera's behavior is simulated
// by invoking the interaction callbacks with the sender each send recorded as the request's.
class InterceptingWebRTCClient : public DefaultAvAnalysisWebRTCClient
{
public:
    using DefaultAvAnalysisWebRTCClient::CurrentRequest;
    using DefaultAvAnalysisWebRTCClient::Request;

    // The interaction callbacks, so the tests can play the camera's and the application's side
    using DefaultAvAnalysisWebRTCClient::OnAttributeData;
    using DefaultAvAnalysisWebRTCClient::OnDone;
    using DefaultAvAnalysisWebRTCClient::OnError;
    using DefaultAvAnalysisWebRTCClient::OnOfferReady;
    using DefaultAvAnalysisWebRTCClient::OnResponse;

    // Drives the request into the provider-check phase, as OnDeviceConnected would after CASE
    void EnterProviderCheck() { CurrentRequest().Advance(Request::Phase::kCheckingProvider); }

    // The CASE outcomes, so the tests can play the CASESessionManager's side. A session cannot be
    // fabricated here, so the connected case is driven through the continuation OnDeviceConnected
    // runs once it has held one.
    using DefaultAvAnalysisWebRTCClient::ContinueWithSession;
    using DefaultAvAnalysisWebRTCClient::OnDeviceConnectionFailure;

    // The sender every intercepted send records as the request's; never sends, an identity only
    CommandSender * Sender() { return &mSender; }

    // What every intercepted send returns; a failure returns before anything is recorded as sent
    CHIP_ERROR mSendResult = CHIP_NO_ERROR;

    int mConnectRequests = 0;
    ScopedNodeId mLastPeer;

    // The ProvideOffer payload the client would have sent
    int mSendAttempts          = 0;
    bool mSentSessionIdWasNull = false;
    std::string mSentSdp;
    Globals::StreamUsageEnum mSentUsage = Globals::StreamUsageEnum::kUnknownEnumValue;
    EndpointId mSentOriginatingEndpoint = kInvalidEndpointId;
    std::vector<uint16_t> mSentVideoStreams;
    bool mSentDeprecatedStreamIdAbsent = false;
    bool mSentAudioAbsent              = false;
    bool mSentIceAbsent                = false;

    // The EndSession payload the client would have sent
    int mEndSendAttempts                        = 0;
    uint16_t mSentEndSessionId                  = 0;
    Globals::WebRTCEndReasonEnum mSentEndReason = Globals::WebRTCEndReasonEnum::kUnknownEnumValue;

    // Drives a pending EndSession request past CASE, as OnDeviceConnected would
    CHIP_ERROR SendPendingEndSession() { return SendEndSession(); }

    // The ProvideICECandidates payload the client would have sent
    int mIceSendAttempts       = 0;
    uint16_t mSentIceSessionId = 0;
    std::vector<std::string> mSentCandidates;
    std::vector<std::optional<std::string>> mSentMids;
    std::vector<DataModel::Nullable<uint16_t>> mSentMLineIndexes;

    // Drives a pending ProvideICECandidates request past CASE, as OnDeviceConnected would
    CHIP_ERROR SendPendingICECandidates() { return SendProvideICECandidates(); }

protected:
    void EstablishSession(const ScopedNodeId & aCameraNode) override
    {
        mConnectRequests++;
        mLastPeer = aCameraNode;
    }

    CHIP_ERROR SendProvideOffer() override
    {
        mSendAttempts++;
        ReturnErrorOnFailure(mSendResult);

        uint16_t videoStream = 0;
        WebRTCTransportProvider::Commands::ProvideOffer::Type request;
        ReturnErrorOnFailure(BuildProvideOffer(request, videoStream));
        mSentSessionIdWasNull    = request.webRTCSessionID.IsNull();
        mSentSdp                 = std::string(request.sdp.data(), request.sdp.size());
        mSentUsage               = request.streamUsage;
        mSentOriginatingEndpoint = request.originatingEndpointID;
        mSentVideoStreams.clear();
        if (request.videoStreams.HasValue())
        {
            for (uint16_t stream : request.videoStreams.Value())
            {
                mSentVideoStreams.push_back(stream);
            }
        }
        mSentDeprecatedStreamIdAbsent = !request.videoStreamID.HasValue();
        mSentAudioAbsent              = !request.audioStreamID.HasValue() && !request.audioStreams.HasValue();
        mSentIceAbsent                = !request.ICEServers.HasValue() && !request.ICETransportPolicy.HasValue();

        MarkSent();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendEndSession() override
    {
        mEndSendAttempts++;
        ReturnErrorOnFailure(mSendResult);

        WebRTCTransportProvider::Commands::EndSession::Type request;
        ReturnErrorOnFailure(BuildEndSession(request));
        mSentEndSessionId = request.webRTCSessionID;
        mSentEndReason    = request.reason;

        MarkSent();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendProvideICECandidates() override
    {
        mIceSendAttempts++;
        ReturnErrorOnFailure(mSendResult);

        std::vector<Globals::Structs::ICECandidateStruct::Type> storage;
        WebRTCTransportProvider::Commands::ProvideICECandidates::Type request;
        ReturnErrorOnFailure(BuildProvideICECandidates(request, storage));
        mSentIceSessionId = request.webRTCSessionID;
        mSentCandidates.clear();
        mSentMids.clear();
        mSentMLineIndexes.clear();
        for (const auto & candidate : request.ICECandidates)
        {
            mSentCandidates.emplace_back(candidate.candidate.data(), candidate.candidate.size());
            mSentMids.emplace_back(
                candidate.SDPMid.IsNull()
                    ? std::nullopt
                    : std::optional<std::string>(std::string(candidate.SDPMid.Value().data(), candidate.SDPMid.Value().size())));
            mSentMLineIndexes.push_back(candidate.SDPMLineIndex);
        }

        MarkSent();
        return CHIP_NO_ERROR;
    }

private:
    // As InvokeOnHeldSession records a send: the camera's answer arrives through the sender callbacks
    void MarkSent()
    {
        CurrentRequest().SetInvokedSender(&mSender);
        CurrentRequest().Advance(Request::Phase::kInvoking);
    }

    CommandSender mSender{ nullptr, nullptr };
};

constexpr uint16_t kVideoStreamId = 42;

class RecordingCallback : public AvAnalysisWebRTCClient::Callback
{
public:
    int mInitiatedCount = 0;
    int mActiveCount    = 0;
    int mFailedCount    = 0;
    int mEndedCount     = 0;
    // A status no completion delivers, so a Success can only come from one
    Status mLastStatus    = Status::InvalidAction;
    uint16_t mLastSession = 0;

    void OnSessionInitiated(Status aStatus, uint16_t aWebRTCSessionId) override
    {
        mInitiatedCount++;
        mLastStatus  = aStatus;
        mLastSession = aWebRTCSessionId;
    }
    void OnSessionActive(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override
    {
        mActiveCount++;
        mLastCamera  = aCameraNode;
        mLastSession = aWebRTCSessionId;
    }
    void OnSessionFailed(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override
    {
        mFailedCount++;
        mLastCamera  = aCameraNode;
        mLastSession = aWebRTCSessionId;
    }
    ScopedNodeId mLastCamera;
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
        if (mDeliverOfferSynchronously)
        {
            aCallback.OnOfferReady(CHIP_NO_ERROR, "v=0 synchronous offer"_span);
        }
        return mCreateOfferResult;
    }

    CHIP_ERROR mCreateOfferResult   = CHIP_NO_ERROR;
    bool mDeliverOfferSynchronously = false;
    void OnSessionAssigned(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override
    {
        mSessionsAssigned++;
        mLastAssigned = aWebRTCSessionId;

        // The contract: the request that produced the offer is over
        if (mClient != nullptr)
        {
            mRequestInFlightAtAssign = mClient->CurrentRequest().InFlight();
            if (mSendCandidatesOnAssign)
            {
                Globals::Structs::ICECandidateStruct::Type candidate;
                candidate.candidate = "candidate:1 1 UDP 2122252543 192.168.1.10 5000 typ host"_span;
                mSendResultAtAssign = mClient->SendICECandidates(
                    aCameraNode, aWebRTCSessionId, Span<const Globals::Structs::ICECandidateStruct::Type>(&candidate, 1));
            }
        }
    }

    InterceptingWebRTCClient * mClient = nullptr;
    bool mRequestInFlightAtAssign      = true;
    bool mSendCandidatesOnAssign       = false;
    CHIP_ERROR mSendResultAtAssign     = CHIP_ERROR_INTERNAL;
    void OnOfferAbandoned() override { mOffersAbandoned++; }
    void OnSessionClosed(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) override
    {
        mSessionsClosed++;
        mLastClosedCamera = aCameraNode;
        mLastClosed       = aWebRTCSessionId;

        // An application whose peer connection reports its own close synchronously
        if (mReportFailureOnClose && mClient != nullptr)
        {
            mClient->NotifyFailed(aCameraNode, aWebRTCSessionId);
        }
    }
    ScopedNodeId mLastClosedCamera;
    bool mReportFailureOnClose = false;

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

ConcreteDataAttributePath ServerListPath(EndpointId aEndpoint)
{
    return ConcreteDataAttributePath(aEndpoint, Descriptor::Id, Descriptor::Attributes::ServerList::Id);
}

// Feeds a cluster-id list report for aPath with aStatus into the client, as the read would deliver it
void FeedClusterList(InterceptingWebRTCClient & aClient, const ConcreteDataAttributePath & aPath, Span<const ClusterId> aClusters,
                     const StatusIB & aStatus)
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
    aClient.OnAttributeData(aPath, &reader, aStatus);
}

// A successful ServerList report for aEndpoint
void FeedServerList(InterceptingWebRTCClient & aClient, EndpointId aEndpoint, Span<const ClusterId> aClusters)
{
    FeedClusterList(aClient, ServerListPath(aEndpoint), aClusters, StatusIB());
}

struct TestDefaultAvAnalysisWebRTCClient : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override
    {
        ASSERT_EQ(mClient.Init(&mCASESessionManager, &mPeerDelegate, &mRequestorCluster, kMaxSessions), CHIP_NO_ERROR);
        mPeerDelegate.mClient = &mClient;
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
    void FeedOfferResponse(uint16_t aWebRTCSessionId) { FeedOfferResponse(aWebRTCSessionId, kVideoStreamId); }

    void FeedOfferResponse(uint16_t aWebRTCSessionId, std::optional<uint16_t> aVideoStreamId)
    {
        FeedOfferResponseFrom(mClient.Sender(), aWebRTCSessionId, aVideoStreamId);
    }

    // The response as delivered by aSender, which need not be the request's own
    void FeedOfferResponseFrom(CommandSender * aSender, uint16_t aWebRTCSessionId, std::optional<uint16_t> aVideoStreamId)
    {
        using Fields = WebRTCTransportProvider::Commands::ProvideOfferResponse::Fields;

        uint8_t buffer[64];
        TLV::TLVWriter writer;
        writer.Init(buffer);
        TLV::TLVType containerType;
        ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType), CHIP_NO_ERROR);
        ASSERT_EQ(writer.Put(TLV::ContextTag(Fields::kWebRTCSessionID), aWebRTCSessionId), CHIP_NO_ERROR);
        if (aVideoStreamId.has_value())
        {
            ASSERT_EQ(writer.Put(TLV::ContextTag(Fields::kVideoStreamID), *aVideoStreamId), CHIP_NO_ERROR);
        }
        ASSERT_EQ(writer.EndContainer(containerType), CHIP_NO_ERROR);

        TLV::TLVReader reader;
        reader.Init(buffer, writer.GetLengthWritten());
        ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);

        ConcreteCommandPath responsePath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                         WebRTCTransportProvider::Commands::ProvideOfferResponse::Id);
        mClient.OnResponse(aSender, responsePath, StatusIB(), &reader);
    }

    // The full successful offer exchange, ending with the session tracked under aWebRTCSessionId
    void EstablishSessionWithId(uint16_t aWebRTCSessionId) { EstablishSessionWithId(aWebRTCSessionId, mCallback); }
    void EstablishSessionWithId(uint16_t aWebRTCSessionId, RecordingCallback & aCallback)
    {
        EstablishSessionWithId(aWebRTCSessionId, aCallback, kCameraNode);
    }

    // The full exchange toward a specific camera, for sessions of several cameras side by side
    void EstablishSessionWithId(uint16_t aWebRTCSessionId, RecordingCallback & aCallback, const ScopedNodeId & aCameraNode)
    {
        ASSERT_EQ(mClient.RequestSession(aCameraNode, kProviderEndpoint, kVideoStreamId, aCallback), CHIP_NO_ERROR);
        mClient.EnterProviderCheck();
        const ClusterId kProviderList[] = { WebRTCTransportProvider::Id };
        FeedServerList(mClient, kProviderEndpoint, Span<const ClusterId>(kProviderList));
        mClient.OnDone(static_cast<ReadClient *>(nullptr));
        ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
        mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
        FeedOfferResponse(aWebRTCSessionId);
        mClient.OnDone(mClient.Sender());
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
        mClient.OnResponse(mClient.Sender(), responsePath, StatusIB(aStatus), nullptr);
    }

    // Two candidates as the application would hand them over: one with a mid, one without
    static std::vector<Globals::Structs::ICECandidateStruct::Type> TwoCandidates()
    {
        std::vector<Globals::Structs::ICECandidateStruct::Type> candidates(2);
        candidates[0].candidate = "candidate:1 1 UDP 2122252543 192.168.1.10 5000 typ host"_span;
        candidates[0].SDPMid.SetNonNull("video"_span);
        candidates[0].SDPMLineIndex.SetNonNull(static_cast<uint16_t>(0));
        candidates[1].candidate = "candidate:2 1 UDP 1686052607 203.0.113.5 5000 typ srflx"_span;
        candidates[1].SDPMid.SetNull();
        candidates[1].SDPMLineIndex.SetNull();
        return candidates;
    }

    // Requests a candidate send for a tracked session and sends the command, up to the camera's answer
    void DriveToICECandidatesSent(uint16_t aWebRTCSessionId)
    {
        auto candidates = TwoCandidates();
        ASSERT_EQ(
            mClient.SendICECandidates(kCameraNode, aWebRTCSessionId,
                                      Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
            CHIP_NO_ERROR);
        ASSERT_EQ(mClient.SendPendingICECandidates(), CHIP_NO_ERROR);
    }

    // InterceptingWebRTCClient overrides EstablishSession.
    CASESessionManager mCASESessionManager;
    InterceptingWebRTCClient mClient;
    RecordingCallback mCallback;
    FakePeerDelegate mPeerDelegate;
    StubRequestorDelegate mRequestorDelegate;
    // Not endpoint 1: the offer's OriginatingEndpointID must come from this registration and not
    // from a plausible default
    WebRTCTransportRequestor::WebRTCTransportRequestorCluster mRequestorCluster{ kRequestorEndpoint, mRequestorDelegate };
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

TEST_F(TestDefaultAvAnalysisWebRTCClient, ACameraThatCannotBeReachedFailsTheRequest)
{
    ASSERT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);

    mClient.OnDeviceConnectionFailure(&mClient, kCameraNode, CHIP_ERROR_TIMEOUT);

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    // The request is over rather than stuck: the client takes the next one
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ACameraThatCannotBeReachedFailsAnEndSession)
{
    EstablishSessionWithId(55);
    ASSERT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);

    mClient.OnDeviceConnectionFailure(&mClient, kCameraNode, CHIP_ERROR_TIMEOUT);

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    // This node is done with the session whatever became of the EndSession
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AConnectionFailureOutsideTheConnectingPhaseIsIgnored)
{
    DriveToOffer(); // the request is awaiting the application's offer, no longer connecting

    mClient.OnDeviceConnectionFailure(&mClient, kCameraNode, CHIP_ERROR_TIMEOUT);

    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_TRUE(mClient.CurrentRequest().InPhase(InterceptingWebRTCClient::Request::Phase::kCreatingOffer));
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AHeldSessionSendsTheCommandTheRequestIsFor)
{
    // An offer request checks the provider before anything is sent
    ASSERT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
    mClient.ContinueWithSession();
    EXPECT_EQ(mClient.mSendAttempts, 0);
    EXPECT_EQ(mClient.mEndSendAttempts, 0);
    EXPECT_EQ(mClient.mIceSendAttempts, 0);
    // No session is really held, so the read cannot start and the request fails
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);

    // An EndSession request sends EndSession
    EstablishSessionWithId(55);
    const int offersBefore = mClient.mSendAttempts;
    ASSERT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
    mClient.ContinueWithSession();
    EXPECT_EQ(mClient.mEndSendAttempts, 1);
    EXPECT_EQ(mClient.mSentEndSessionId, 55);
    EXPECT_EQ(mClient.mIceSendAttempts, 0);
    EXPECT_EQ(mClient.mSendAttempts, offersBefore);
    FeedEndSessionStatus(Status::Success);
    mClient.OnDone(mClient.Sender());

    // A candidate request sends ProvideICECandidates
    EstablishSessionWithId(56);
    auto candidates = TwoCandidates();
    ASSERT_EQ(mClient.SendICECandidates(
                  kCameraNode, 56, Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
              CHIP_NO_ERROR);
    mClient.ContinueWithSession();
    EXPECT_EQ(mClient.mIceSendAttempts, 1);
    EXPECT_EQ(mClient.mSentIceSessionId, 56);
    EXPECT_EQ(mClient.mEndSendAttempts, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnEndSessionTheClientCannotSendFailsTheRequest)
{
    EstablishSessionWithId(55);
    ASSERT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);

    mClient.mSendResult = CHIP_ERROR_INTERNAL;
    mClient.ContinueWithSession();

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    // Completed: the client accepts a new request again
    mClient.mSendResult = CHIP_NO_ERROR;
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CandidatesTheClientCannotSendKeepTheSession)
{
    EstablishSessionWithId(55);
    auto candidates = TwoCandidates();
    ASSERT_EQ(mClient.SendICECandidates(
                  kCameraNode, 55, Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
              CHIP_NO_ERROR);

    mClient.mSendResult = CHIP_ERROR_INTERNAL;
    mClient.ContinueWithSession();

    // Nobody awaits a candidate send, and the session it was about survives
    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    mClient.mSendResult = CHIP_NO_ERROR;
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, EndingAnUntrackedSessionIsRejected)
{
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mClient.mConnectRequests, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, EndingASessionAtAnotherEndpointIsRejected)
{
    EstablishSessionWithId(55);
    const int connectsBefore = mClient.mConnectRequests;

    // The session was initiated on kProviderEndpoint; EndSession must go there
    EXPECT_EQ(mClient.EndSession(kCameraNode, static_cast<EndpointId>(kProviderEndpoint + 1), 55, mCallback),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mClient.mConnectRequests, connectsBefore);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
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

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUndecodableServerListFailsTheRequestRatherThanNotFound)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // A report for the right path whose payload is not a list
    uint8_t buffer[8];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    ASSERT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(7)), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    mClient.OnAttributeData(ServerListPath(kProviderEndpoint), &reader, StatusIB());
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    // Nothing is known about the endpoint, so the cluster is not reported absent
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AServerListThatCannotBeIteratedFailsTheRequestRatherThanNotFound)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // A list whose second element is not a cluster id: iteration stops before the list is known
    uint8_t buffer[32];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    TLV::TLVType outer;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, outer), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Put(TLV::AnonymousTag(), Descriptor::Id), CHIP_NO_ERROR);
    ASSERT_EQ(writer.PutString(TLV::AnonymousTag(), "x"), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(outer), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    mClient.OnAttributeData(ServerListPath(kProviderEndpoint), &reader, StatusIB());
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnErrorStatusForTheServerListIsNotFound)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // The camera has no such endpoint: whatever the report carries, its status says the list is not to be had
    const ClusterId kProviderList[] = { WebRTCTransportProvider::Id };
    FeedClusterList(mClient, ServerListPath(kProviderEndpoint), Span<const ClusterId>(kProviderList),
                    StatusIB(Status::UnsupportedEndpoint));
    mClient.OnAttributeData(ServerListPath(kProviderEndpoint), nullptr, StatusIB(Status::UnsupportedEndpoint));
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AServerListReportOutsideTheProviderCheckIsIgnored)
{
    // CASE is still being established: a report cannot be this request's
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    const ClusterId kProviderList[] = { WebRTCTransportProvider::Id };
    FeedServerList(mClient, kProviderEndpoint, Span<const ClusterId>(kProviderList));

    // The check that follows starts with nothing known
    mClient.EnterProviderCheck();
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);
    EXPECT_EQ(mPeerDelegate.mOffersRequested, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AProviderListReportedUnderAnotherPathIsNotTheProviderCheck)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // The provider is listed, but under the ClientList attribute of the right endpoint
    const ClusterId kProviderList[] = { WebRTCTransportProvider::Id };
    FeedClusterList(mClient, ConcreteDataAttributePath(kProviderEndpoint, Descriptor::Id, Descriptor::Attributes::ClientList::Id),
                    Span<const ClusterId>(kProviderList), StatusIB());
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ReadCallbacksOutsideTheProviderCheckAreIgnored)
{
    // An EndSession is in flight: no read of ours is
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    mClient.OnError(CHIP_ERROR_ACCESS_DENIED);
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    // The request is untouched, still awaiting the camera's answer
    EXPECT_EQ(mClient.CurrentRequest().GetPhase(), InterceptingWebRTCClient::Request::Phase::kInvoking);
    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);

    FeedEndSessionStatus(Status::Success);
    mClient.OnDone(mClient.Sender());
    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AFailedServerListReadFailsTheRequestRatherThanNotFound)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.EnterProviderCheck();

    // The read itself fails (for instance the camera denies access to the Descriptor cluster)
    mClient.OnError(CHIP_ERROR_ACCESS_DENIED);
    mClient.OnDone(static_cast<ReadClient *>(nullptr));

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CancelSilentlyAbandonsTheRequest)
{
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, 42, mCallback), CHIP_NO_ERROR);
    mClient.Cancel();

    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 0); // No offer was asked for yet
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
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1); // Whatever the refusal left behind is released

    // Completed: the client accepts a new request again
    mPeerDelegate.mCreateOfferResult = CHIP_NO_ERROR;
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferDeliveredBeforeCreateOfferReturnsAnErrorStands)
{
    // The application hands the offer over from within CreateOffer, then returns an error anyway
    mPeerDelegate.mDeliverOfferSynchronously = true;
    mPeerDelegate.mCreateOfferResult         = CHIP_ERROR_INTERNAL;
    DriveToOffer();

    // The offer is on the wire: the late error must not fail the request behind it
    EXPECT_EQ(mClient.mSendAttempts, 1);
    EXPECT_EQ(mClient.CurrentRequest().GetPhase(), InterceptingWebRTCClient::Request::Phase::kInvoking);
    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 0);

    // The camera's answer concludes it
    FeedOfferResponse(55);
    mClient.OnDone(mClient.Sender());
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastSession, 55);
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

TEST_F(TestDefaultAvAnalysisWebRTCClient, SessionAssignmentFollowsTheCompletedRequest)
{
    // The application flushes its candidates the moment it learns the session id
    mPeerDelegate.mSendCandidatesOnAssign = true;
    EstablishSessionWithId(55);

    // By then the offer request was over and its callback delivered, so the send was accepted
    ASSERT_EQ(mPeerDelegate.mSessionsAssigned, 1);
    EXPECT_FALSE(mPeerDelegate.mRequestInFlightAtAssign);
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mPeerDelegate.mSendResultAtAssign, CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mConnectRequests, 2); // the offer, then the candidates

    // The candidate send is the request now in flight
    ASSERT_EQ(mClient.SendPendingICECandidates(), CHIP_NO_ERROR);
    EXPECT_EQ(mClient.mSentIceSessionId, 55);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, OfferPayloadFollowsTheNormalFlow)
{
    EstablishSessionWithId(55);

    ASSERT_EQ(mClient.mSendAttempts, 1);
    EXPECT_TRUE(mClient.mSentSessionIdWasNull); // A new session is the camera's to assign
    EXPECT_EQ(mClient.mSentSdp, "v=0 test offer");
    EXPECT_EQ(mClient.mSentUsage, Globals::StreamUsageEnum::kAnalysis);
    EXPECT_EQ(mClient.mSentOriginatingEndpoint, kRequestorEndpoint); // Where the requestor cluster is registered
    ASSERT_EQ(mClient.mSentVideoStreams.size(), 1u);
    EXPECT_EQ(mClient.mSentVideoStreams[0], kVideoStreamId);
    EXPECT_TRUE(mClient.mSentDeprecatedStreamIdAbsent);
    EXPECT_TRUE(mClient.mSentAudioAbsent); // No audio for analysis
    EXPECT_TRUE(mClient.mSentIceAbsent);   // ICE configuration is the camera's default
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CameraErrorStatusIsPropagatedVerbatim)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // The camera refuses the offer with a specific status
    mClient.OnError(mClient.Sender(), StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
    EXPECT_EQ(mPeerDelegate.mSessionsAssigned, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ATransportErrorIsReportedAsFailure)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // No camera status behind the error: the exchange timed out. Recorded as a failure at once
    mClient.OnError(mClient.Sender(), CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(mClient.CurrentRequest().GetPhase(), InterceptingWebRTCClient::Request::Phase::kFailed);
    EXPECT_EQ(mClient.CurrentRequest().FailureStatus(), Status::Failure);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferTheClientCannotSendFailsTheRequest)
{
    mClient.mSendResult = CHIP_ERROR_NO_MEMORY;
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // Nothing left this node, so no OnDone will come: the request fails right here
    EXPECT_EQ(mClient.mSendAttempts, 1);
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);

    // Completed: the client accepts a new request again
    mClient.mSendResult = CHIP_NO_ERROR;
    EstablishSessionWithId(55);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CallbacksOfAnotherSenderAreIgnored)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // A sender this request never used speaks up: none of it is this request's outcome
    CommandSender stranger{ nullptr, nullptr };
    FeedOfferResponseFrom(&stranger, 99, kVideoStreamId);
    mClient.OnError(&stranger, StatusIB(Status::ResourceExhausted).ToChipError());
    mClient.OnDone(&stranger);

    EXPECT_EQ(mClient.CurrentRequest().GetPhase(), InterceptingWebRTCClient::Request::Phase::kInvoking);
    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);

    // The request's own sender concludes it
    FeedOfferResponse(55);
    mClient.OnDone(mClient.Sender());
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastSession, 55);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ACameraErrorIsDeliveredOnlyWhenTheExchangeCloses)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // The error arrives while the sender is still alive: recorded, not yet delivered
    mClient.OnError(mClient.Sender(), StatusIB(Status::ResourceExhausted).ToChipError());
    EXPECT_EQ(mClient.CurrentRequest().GetPhase(), InterceptingWebRTCClient::Request::Phase::kFailed);
    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 0);

    // The exchange closes: the recorded status is delivered and the offer abandoned
    mClient.OnDone(mClient.Sender());
    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::ResourceExhausted);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUnansweredExchangeFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // The exchange ends with neither a response nor an error
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ARequestWithNoFreeSessionSlotIsRefusedUpFront)
{
    for (uint16_t id = 101; id < 101 + kMaxSessions; id++)
    {
        EstablishSessionWithId(id);
    }
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    const int connectsBefore = mClient.mConnectRequests;

    // Nothing is signaled toward the camera, so no session can be granted that this node cannot track
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(mClient.mConnectRequests, connectsBefore);
    EXPECT_EQ(mCallback.mInitiatedCount, kMaxSessions);
    EXPECT_EQ(mPeerDelegate.mOffersRequested, kMaxSessions);

    // A released session frees a slot for the next request
    mClient.NotifyFailed(kCameraNode, 101);
    EstablishSessionWithId(200);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
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
    mClient.OnDone(mClient.Sender());

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

TEST_F(TestDefaultAvAnalysisWebRTCClient, CameraErrorOnEndSessionIsPropagatedAndTheSessionIsStillReleased)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    // The camera refuses
    mClient.OnError(mClient.Sender(), StatusIB(Status::NotFound).ToChipError());
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::NotFound);

    // The stream goes to Failure, from which nothing could end this session again: released
    // everywhere rather than left occupying a slot
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mPeerDelegate.mLastClosed, 55);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_ERROR_INVALID_ARGUMENT);

    // The slot is reusable by the re-activation that follows
    EstablishSessionWithId(56);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnErrorStatusInTheEndSessionResponseFailsIt)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    // A status-only response that is not SUCCESS is no answer the request can use
    FeedEndSessionStatus(Status::Failure);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, EndingASessionWhileAnotherRequestIsInFlightIsBusy)
{
    EstablishSessionWithId(55);
    EstablishSessionWithId(56);
    ASSERT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);

    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 56, mCallback), CHIP_ERROR_BUSY);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 2u); // Neither session was touched
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnEndSessionConcludingAfterTheCamerasEndReleasesOnce)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    // The camera ends the session on its own before answering our EndSession
    mClient.NotifyEnded(kCameraNode, 55);
    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);

    // Our EndSession is then answered: reported, with nothing left to release
    FeedEndSessionStatus(Status::Success);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUnansweredEndSessionExchangeFailsAndStillReleasesTheSession)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mEndedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ConnectedSessionIsReportedActiveAndStaysTracked)
{
    EstablishSessionWithId(55);

    mClient.NotifyConnected(kCameraNode, 55);

    EXPECT_EQ(mCallback.mActiveCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
    EXPECT_EQ(mCallback.mFailedCount, 0);

    // Active is not over: still recorded, still held by the application, still endable
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AFailureReportedFromWithinOnSessionClosedReleasesOnce)
{
    mPeerDelegate.mReportFailureOnClose = true;
    EstablishSessionWithId(55);

    // The camera's End releases the session; the application's close handler reports the same
    // session failed while the release is in progress
    mClient.NotifyEnded(kCameraNode, 55);

    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AReissuedSessionIdFailsTheSessionStillTrackedUnderIt)
{
    // A camera restarted and grants id 0 again while we still track its earlier session 0
    EstablishSessionWithId(0);
    EstablishSessionWithId(0);

    // The earlier session is failed and released; the new one is the only one tracked
    EXPECT_EQ(mCallback.mInitiatedCount, 2);
    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mPeerDelegate.mSessionsAssigned, 2);
    ASSERT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 0, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnErrorAfterTheResponseDoesNotUndoIt)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
    FeedOfferResponse(55);

    // A stray error IB after the successful response is ignored
    mClient.OnError(mClient.Sender(), StatusIB(Status::Failure).ToChipError());
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    EXPECT_EQ(mCallback.mLastSession, 55);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AResponseFromAnotherEndpointFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    using Fields = WebRTCTransportProvider::Commands::ProvideOfferResponse::Fields;
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType containerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Put(TLV::ContextTag(Fields::kWebRTCSessionID), static_cast<uint16_t>(55)), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(containerType), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    ConcreteCommandPath otherEndpoint(static_cast<EndpointId>(kProviderEndpoint + 1), WebRTCTransportProvider::Id,
                                      WebRTCTransportProvider::Commands::ProvideOfferResponse::Id);
    mClient.OnResponse(mClient.Sender(), otherEndpoint, StatusIB(), &reader);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CameraEndedSessionIsReportedFailedAndReleased)
{
    EstablishSessionWithId(55);

    // The camera's End arrives on the requestor cluster, not as the outcome of our EndSession
    mClient.NotifyEnded(kCameraNode, 55);

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

    mClient.NotifyFailed(kCameraNode, 55);

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

    mClient.NotifyConnected(kCameraNode, 99);
    mClient.NotifyFailed(kCameraNode, 99);
    mClient.NotifyEnded(kCameraNode, 99);

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

    mClient.NotifyConnected(kCameraNode, 56);
    mClient.NotifyEnded(kCameraNode, 55);

    EXPECT_EQ(otherCallback.mActiveCount, 1);
    EXPECT_EQ(otherCallback.mFailedCount, 0);
    EXPECT_EQ(mCallback.mActiveCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, SessionsOfDifferentCamerasSharingAnIdAreToldApart)
{
    // Session ids are assigned per camera: two cameras both hand out id 0
    const ScopedNodeId kOtherCamera(0x5678, 1);
    RecordingCallback otherCallback;
    EstablishSessionWithId(0, mCallback, kCameraNode);
    EstablishSessionWithId(0, otherCallback, kOtherCamera);
    ASSERT_EQ(mRequestorCluster.GetCurrentSessions().size(), 2u);

    // The other camera's session fails: only it is released and reported
    mClient.NotifyFailed(kOtherCamera, 0);

    EXPECT_EQ(otherCallback.mFailedCount, 1);
    EXPECT_EQ(otherCallback.mLastCamera, kOtherCamera);
    EXPECT_EQ(mCallback.mFailedCount, 0);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mPeerDelegate.mLastClosedCamera, kOtherCamera);
    ASSERT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions()[0].peerNodeID, kCameraNode.GetNodeId());

    // The first camera's session is intact: connected, and still endable under its own camera
    mClient.NotifyConnected(kCameraNode, 0);
    EXPECT_EQ(mCallback.mActiveCount, 1);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 0, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnEstablishedSessionIsNotAnAbandonedOffer)
{
    EstablishSessionWithId(55);

    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferLeftUnansweredIsAbandoned)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
    mClient.OnDone(mClient.Sender());

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
    mClient.NotifyConnected(kCameraNode, 55);
    mClient.NotifyEnded(kCameraNode, 56);
    EXPECT_EQ(mCallback.mActiveCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 0);

    // The client is fully reusable
    EstablishSessionWithId(57);
    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ARequestBeforeInitIsRefused)
{
    InterceptingWebRTCClient uninitialised;
    EXPECT_EQ(uninitialised.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(uninitialised.mConnectRequests, 0);

    // With no session table nothing is tracked, so the session-bound entry points find nothing to
    // act on and walk no memory
    RecordingCallback uninitialisedCallback;
    EXPECT_NE(uninitialised.EndSession(kCameraNode, kProviderEndpoint, 55, uninitialisedCallback), CHIP_NO_ERROR);
    auto candidates = TwoCandidates();
    EXPECT_NE(uninitialised.SendICECandidates(
                  kCameraNode, 55, Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
              CHIP_NO_ERROR);
    uninitialised.NotifyConnected(kCameraNode, 55);
    uninitialised.NotifyFailed(kCameraNode, 55);
    uninitialised.NotifyEnded(kCameraNode, 55);
    uninitialised.Cancel();

    EXPECT_EQ(uninitialisedCallback.mInitiatedCount, 0);
    EXPECT_EQ(uninitialisedCallback.mActiveCount, 0);
    EXPECT_EQ(uninitialisedCallback.mFailedCount, 0);
    EXPECT_EQ(uninitialisedCallback.mEndedCount, 0);
    EXPECT_EQ(uninitialised.mConnectRequests, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnEmptyOfferFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);

    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, CharSpan());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mClient.mSendAttempts, 0);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, TheRequestorRecordCarriesTheOfferedVideoStreamId)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    FeedOfferResponse(55, std::nullopt);
    mClient.OnDone(mClient.Sender());

    auto sessions = mRequestorCluster.GetCurrentSessions();
    ASSERT_EQ(sessions.size(), 1u);
    ASSERT_FALSE(sessions[0].videoStreamID.IsNull());
    EXPECT_EQ(sessions[0].videoStreamID.Value(), kVideoStreamId);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, TheResponsesDeprecatedVideoStreamIdIsIgnored)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    FeedOfferResponse(55, static_cast<uint16_t>(kVideoStreamId + 1));
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mLastStatus, Status::Success);
    auto sessions = mRequestorCluster.GetCurrentSessions();
    ASSERT_EQ(sessions.size(), 1u);
    ASSERT_FALSE(sessions[0].videoStreamID.IsNull());
    EXPECT_EQ(sessions[0].videoStreamID.Value(), kVideoStreamId);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ASecondOfferResponseIsIgnored)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    FeedOfferResponse(55);
    FeedOfferResponse(56); // rejected: the command was already responded to
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastSession, 55);
    EXPECT_EQ(mPeerDelegate.mLastAssigned, 55);
    ASSERT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions()[0].id, 55);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnOfferResponseWithoutDataFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    ConcreteCommandPath responsePath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                     WebRTCTransportProvider::Commands::ProvideOfferResponse::Id);
    mClient.OnResponse(mClient.Sender(), responsePath, StatusIB(), nullptr);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AResponseOnTheWrongCommandFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // A well-formed ProvideOfferResponse payload arriving under another command id
    using Fields = WebRTCTransportProvider::Commands::ProvideOfferResponse::Fields;
    uint8_t buffer[64];
    TLV::TLVWriter writer;
    writer.Init(buffer);
    TLV::TLVType containerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, containerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Put(TLV::ContextTag(Fields::kWebRTCSessionID), static_cast<uint16_t>(55)), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(containerType), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    ConcreteCommandPath wrongPath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                  WebRTCTransportProvider::Commands::SolicitOfferResponse::Id);
    mClient.OnResponse(mClient.Sender(), wrongPath, StatusIB(), &reader);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUndecodableOfferResponseFailsTheRequest)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);

    // A scalar where a structure is expected
    uint8_t buffer[8];
    TLV::TLVWriter writer;
    writer.Init(buffer, sizeof(buffer));
    ASSERT_EQ(writer.Put(TLV::AnonymousTag(), static_cast<uint32_t>(7)), CHIP_NO_ERROR);
    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(), CHIP_NO_ERROR);
    ConcreteCommandPath responsePath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                     WebRTCTransportProvider::Commands::ProvideOfferResponse::Id);
    mClient.OnResponse(mClient.Sender(), responsePath, StatusIB(), &reader);
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mInitiatedCount, 1);
    EXPECT_EQ(mCallback.mLastStatus, Status::Failure);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CancelDuringTheOfferExchangeAbandonsTheOffer)
{
    DriveToOffer();
    ASSERT_NE(mPeerDelegate.mLastOfferCallback, nullptr);
    mPeerDelegate.mLastOfferCallback->OnOfferReady(CHIP_NO_ERROR, "v=0 test offer"_span);
    ASSERT_EQ(mClient.mSendAttempts, 1);

    mClient.Cancel();

    EXPECT_EQ(mCallback.mInitiatedCount, 0);
    EXPECT_EQ(mPeerDelegate.mOffersAbandoned, 1);
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CancelDuringAnEndSessionDeliversNothingAndForgetsTheSession)
{
    EstablishSessionWithId(55);
    DriveToEndSessionSent(55);

    mClient.Cancel();

    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 0u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 1);
    EXPECT_EQ(mClient.RequestSession(kCameraNode, kProviderEndpoint, kVideoStreamId, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnUnansweredCandidateSendKeepsTheSession)
{
    EstablishSessionWithId(55);
    DriveToICECandidatesSent(55);

    // The exchange closes with no response at all
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mFailedCount, 0);
    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ICECandidatesForAnUntrackedSessionAreRejected)
{
    auto candidates = TwoCandidates();
    EXPECT_EQ(mClient.SendICECandidates(
                  kCameraNode, 99, Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
              CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(mClient.mConnectRequests, 0);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AnEmptyCandidateListIsRejected)
{
    EstablishSessionWithId(55);

    EXPECT_EQ(mClient.SendICECandidates(kCameraNode, 55, Span<const Globals::Structs::ICECandidateStruct::Type>()),
              CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ICECandidatesAreCopiedAndSentToTheSessionsCamera)
{
    EstablishSessionWithId(55);
    const int connectsBefore = mClient.mConnectRequests;

    {
        // The caller's list dies before the send; the client must have copied it
        auto candidates = TwoCandidates();
        ASSERT_EQ(
            mClient.SendICECandidates(kCameraNode, 55,
                                      Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
            CHIP_NO_ERROR);
    }
    ASSERT_EQ(mClient.SendPendingICECandidates(), CHIP_NO_ERROR);

    // Routed through the tracked session's camera
    EXPECT_EQ(mClient.mConnectRequests, connectsBefore + 1);
    EXPECT_EQ(mClient.mLastPeer, kCameraNode);

    ASSERT_EQ(mClient.mIceSendAttempts, 1);
    EXPECT_EQ(mClient.mSentIceSessionId, 55);
    ASSERT_EQ(mClient.mSentCandidates.size(), 2u);
    EXPECT_EQ(mClient.mSentCandidates[0], "candidate:1 1 UDP 2122252543 192.168.1.10 5000 typ host");
    EXPECT_EQ(mClient.mSentMids[0].value_or(""), "video");
    ASSERT_FALSE(mClient.mSentMLineIndexes[0].IsNull());
    EXPECT_EQ(mClient.mSentMLineIndexes[0].Value(), 0);
    EXPECT_EQ(mClient.mSentCandidates[1], "candidate:2 1 UDP 1686052607 203.0.113.5 5000 typ srflx");
    EXPECT_FALSE(mClient.mSentMids[1].has_value());
    EXPECT_TRUE(mClient.mSentMLineIndexes[1].IsNull());
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ICECandidatesAreRoutedToTheSessionsOwnCamera)
{
    // Two cameras, each with a session 0
    const ScopedNodeId kOtherCamera(0x5678, 1);
    RecordingCallback otherCallback;
    EstablishSessionWithId(0, mCallback, kCameraNode);
    EstablishSessionWithId(0, otherCallback, kOtherCamera);

    auto candidates = TwoCandidates();
    ASSERT_EQ(mClient.SendICECandidates(
                  kOtherCamera, 0, Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
              CHIP_NO_ERROR);

    // The send reaches the camera that assigned the session, not the first camera with that id
    EXPECT_EQ(mClient.mLastPeer, kOtherCamera);
    ASSERT_EQ(mClient.SendPendingICECandidates(), CHIP_NO_ERROR);
    ASSERT_EQ(mClient.mIceSendAttempts, 1);
    EXPECT_EQ(mClient.mSentIceSessionId, 0);
    EXPECT_EQ(mClient.mSentCandidates.size(), 2u);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, AConfirmedCandidateSendCompletesSilentlyAndFreesTheClient)
{
    EstablishSessionWithId(55);
    DriveToICECandidatesSent(55);
    const int initiatedBefore = mCallback.mInitiatedCount;

    ConcreteCommandPath responsePath(kProviderEndpoint, WebRTCTransportProvider::Id,
                                     WebRTCTransportProvider::Commands::ProvideICECandidates::Id);
    mClient.OnResponse(mClient.Sender(), responsePath, StatusIB(), nullptr);
    mClient.OnDone(mClient.Sender());

    // No callback is owed for candidates
    EXPECT_EQ(mCallback.mInitiatedCount, initiatedBefore);
    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 0);

    // The session is untouched and the client accepts the next request
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, ACameraErrorOnCandidatesIsLoggedOnlyAndKeepsTheSession)
{
    EstablishSessionWithId(55);
    DriveToICECandidatesSent(55);

    mClient.OnError(mClient.Sender(), StatusIB(Status::NotFound).ToChipError());
    mClient.OnDone(mClient.Sender());

    EXPECT_EQ(mCallback.mEndedCount, 0);
    EXPECT_EQ(mCallback.mFailedCount, 0);
    EXPECT_EQ(mRequestorCluster.GetCurrentSessions().size(), 1u);
    EXPECT_EQ(mPeerDelegate.mSessionsClosed, 0);
    EXPECT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);
}

TEST_F(TestDefaultAvAnalysisWebRTCClient, CandidatesWhileAnotherRequestIsInFlightAreBusy)
{
    EstablishSessionWithId(55);
    ASSERT_EQ(mClient.EndSession(kCameraNode, kProviderEndpoint, 55, mCallback), CHIP_NO_ERROR);

    auto candidates = TwoCandidates();
    EXPECT_EQ(mClient.SendICECandidates(
                  kCameraNode, 55, Span<const Globals::Structs::ICECandidateStruct::Type>(candidates.data(), candidates.size())),
              CHIP_ERROR_BUSY);
}

} // namespace
