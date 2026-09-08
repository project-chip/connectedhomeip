/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <app/BufferedReadCallback.h>
#include <app/CASESessionManager.h>
#include <app/CommandSender.h>
#include <app/ReadClient.h>
#include <app/clusters/av-analysis-server/AvAnalysisWebRTCClient.h>
#include <app/clusters/webrtc-transport-requestor-server/WebRTCTransportRequestorCluster.h>
#include <app/data-model/EncodableToTLV.h>
#include <clusters/WebRTCTransportProvider/Commands.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <transport/SessionHolder.h>

#include <optional>
#include <string>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {

/**
 * The application side of the default WebRTC client: manages the peer connections whose signaling
 * the client performs. The methods carry the camera and the camera-assigned WebRTC session id,
 * under which pair the application keys its peer connections: ids are unique only within a camera.
 * A connection that never establishes is the application's to report as failed; the client sets no timeout.
 *
 * All methods are invoked on the Matter thread.
 */
class AvAnalysisWebRTCPeerDelegate
{
public:
    class OfferCallback
    {
    public:
        virtual ~OfferCallback() = default;

        /**
         * Delivers the SDP offer produced for a new session, or the error that prevented it
         * The SDP is copied before this call returns.
         */
        virtual void OnOfferReady(CHIP_ERROR aError, CharSpan aSdp) = 0;
    };

    virtual ~AvAnalysisWebRTCPeerDelegate() = default;

    /**
     * Creates the peer connection for a new session and produces its SDP offer, delivered through
     * aCallback. At most one offer is outstanding at a
     * time; its session id follows via OnSessionAssigned once the camera assigns one.
     */
    virtual CHIP_ERROR CreateOffer(OfferCallback & aCallback) = 0;

    /**
     * The offer most recently produced now has a camera-assigned session id: the application binds
     * the peer connection it created to this id.
     * Delivered after the request's OnSessionInitiated.
     */
    virtual void OnSessionAssigned(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) = 0;

    /**
     * The offer most recently asked for will not become a session (the camera refused it, the
     * exchange failed, the session could not be tracked, or the request was cancelled): the
     * application releases the peer connection it created for it. Also follows a CreateOffer that
     * returned an error, when there is nothing to release.
     */
    virtual void OnOfferAbandoned() = 0;

    /**
     * The session is over, the application releases the peer connection bound to this id.
     */
    virtual void OnSessionClosed(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId) = 0;
};

/**
 * AvAnalysisWebRTCClient: reaches the camera over a CASE session obtained from the
 * CASESessionManager and drives the WebRTCTransportProvider signaling with a CommandSender, with
 * the SDP offers produced by the application through AvAnalysisWebRTCPeerDelegate.
 *
 * One signaling interaction is in flight at a time; a request made while another is pending fails
 * with CHIP_ERROR_BUSY. Established sessions are tracked until they end, so their late outcomes
 * (active, failed, camera-ended) reach the callback that initiated them.
 */
class DefaultAvAnalysisWebRTCClient : public AvAnalysisWebRTCClient,
                                      public CommandSender::Callback,
                                      public ReadClient::Callback,
                                      public AvAnalysisWebRTCPeerDelegate::OfferCallback
{
public:
    DefaultAvAnalysisWebRTCClient() :
        mOnConnectedCallback(OnDeviceConnected, this), mOnConnectionFailureCallback(OnDeviceConnectionFailure, this)
    {}

    /**
     * @param aCASESessionManager Used to reach the camera node; must outlive this instance.
     * @param aPeerDelegate       Produces the SDP offers and manages the peer connections; must
     *                            outlive this instance.
     * @param aRequestorCluster   The node's WebRTCTransportRequestor server instance, registered by
     *                            the application; the client records the sessions it initiates
     *                            there (and removes them when they end) so the camera's inbound
     *                            commands validate and CurrentSessions stays truthful. Must outlive
     *                            this instance.
     * @param aMaxSessions        Sessions tracked concurrently; an Analysis Node passes its
     *                            MaxAnalysisStreamCount.
     */
    CHIP_ERROR Init(CASESessionManager * aCASESessionManager, AvAnalysisWebRTCPeerDelegate * aPeerDelegate,
                    WebRTCTransportRequestor::WebRTCTransportRequestorCluster * aRequestorCluster, uint8_t aMaxSessions);

    // AvAnalysisWebRTCClient
    CHIP_ERROR RequestSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aVideoStreamId,
                              AvAnalysisWebRTCClient::Callback & aCallback) override;
    CHIP_ERROR EndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId,
                          AvAnalysisWebRTCClient::Callback & aCallback) override;
    void Cancel() override;

    /**
     * Inbound session signals, forwarded by the application when its media layer sees the session's
     * connection established or failed, or when the camera's End arrives on its
     * WebRTCTransportRequestor cluster. A signal for a session this client does not track is ignored.
     * Invoked on the Matter thread. NotifyFailed and NotifyEnded remove the session from the requestor cluster.
     */
    void NotifyConnected(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId);
    void NotifyFailed(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId);
    void NotifyEnded(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId);

    /**
     * Sends this node's ICE candidates for a tracked session to its camera with ProvideICECandidates.
     * The candidates are copied; fails with CHIP_ERROR_BUSY while another request is in flight.
     */
    CHIP_ERROR SendICECandidates(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId,
                                 Span<const Globals::Structs::ICECandidateStruct::Type> aCandidates);

protected:
    // The interaction callbacks are the SDK's to invoke, not the application's

    // CommandSender::Callback
    void OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath, const StatusIB & aStatusIB,
                    TLV::TLVReader * apData) override;
    void OnError(const CommandSender * apCommandSender, CHIP_ERROR aError) override;
    void OnDone(CommandSender * apCommandSender) override;

    // ReadClient::Callback (provider-presence check)
    void OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData, const StatusIB & aStatus) override;
    void OnError(CHIP_ERROR aError) override;
    void OnDone(ReadClient * apReadClient) override;

    // AvAnalysisWebRTCPeerDelegate::OfferCallback
    void OnOfferReady(CHIP_ERROR aError, CharSpan aSdp) override;

    /**
     * Starts CASE session establishment toward the camera; the connected/connection-failure
     * callbacks continue the pending request. The default body is the expected behavior; virtual
     * only so unit tests can intercept the network boundary.
     */
    virtual void EstablishSession(const ScopedNodeId & aCameraNode)
    {
        mCASESessionManager->FindOrEstablishSession(aCameraNode, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                                    TransportPayloadCapability::kLargePayload);
    }

    /**
     * One signaling request, from the first CASE attempt to the command's response. The phase says
     * which step is outstanding and therefore which callback is legitimate; exactly one request
     * exists at a time.
     */
    class Request
    {
    public:
        enum class Phase : uint8_t
        {
            kIdle,             // No request in flight
            kConnecting,       // Awaiting a CASE session with the camera
            kCheckingProvider, // Reading the provided endpoint's ServerList for WebRTCTransportProvider
            kCreatingOffer,    // Awaiting the application's SDP offer
            kInvoking,         // Command sent, awaiting the camera's response
            kResponded,        // Camera responded successfully; completion awaits the exchange closing (OnDone)
            kFailed,           // Command failed (camera error status or transport error); completion awaits OnDone
        };

        enum class CommandType : uint8_t
        {
            kProvideOffer,
            kProvideICECandidates,
            kEndSession,
        };

        Phase GetPhase() const { return mPhase; }
        bool InPhase(Phase aPhase) const { return mPhase == aPhase; }
        bool InFlight() const { return mPhase != Phase::kIdle; }
        void Advance(Phase aPhase) { mPhase = aPhase; }

        CommandType GetCommandType() const { return mCommandType; }
        // True once an offer request has asked the application for an offer
        bool OfferRequested() const
        {
            return mCommandType == CommandType::kProvideOffer &&
                (mPhase == Phase::kCreatingOffer || mPhase == Phase::kInvoking || mPhase == Phase::kResponded ||
                 mPhase == Phase::kFailed);
        }
        const ScopedNodeId & CameraNode() const { return mCameraNode; }
        EndpointId WebRTCEndpoint() const { return mWebRTCEndpoint; }
        uint16_t VideoStreamId() const { return mVideoStreamId; }
        // The WebRTC session this request is about
        uint16_t WebRTCSessionId() const { return mWebRTCSessionId; }

        /**
         * Start a request in kConnecting; the callback is delivered exactly once, by TakeCallback().
         */
        void BeginProvideOffer(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aVideoStreamId,
                               AvAnalysisWebRTCClient::Callback & aCallback)
        {
            Begin(CommandType::kProvideOffer, aCameraNode, aWebRTCEndpoint, &aCallback);
            mVideoStreamId = aVideoStreamId;
        }

        // No callback, the outcome of a candidate send is logged only
        void BeginProvideICECandidates(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId)
        {
            Begin(CommandType::kProvideICECandidates, aCameraNode, aWebRTCEndpoint, nullptr);
            mWebRTCSessionId = aWebRTCSessionId;
        }

        void BeginEndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId,
                             AvAnalysisWebRTCClient::Callback & aCallback)
        {
            Begin(CommandType::kEndSession, aCameraNode, aWebRTCEndpoint, &aCallback);
            mWebRTCSessionId = aWebRTCSessionId;
        }

        /**
         * Records the decoded ProvideOfferResponse: the camera-assigned session id. The session
         * carries the VideoStreams this request asked for, so the response names no stream.
         */
        void SetOfferResponse(uint16_t aWebRTCSessionId) { mWebRTCSessionId = aWebRTCSessionId; }

        /**
         * Returns the callback owed the outcome, or nullptr when it was already delivered (or the
         * request was cancelled). Delivering is therefore exactly-once by construction.
         */
        AvAnalysisWebRTCClient::Callback * TakeCallback()
        {
            auto * callback = mCallback;
            mCallback       = nullptr;
            return callback;
        }

        /**
         * The callback owed the outcome, without consuming it (to register a session on success).
         */
        AvAnalysisWebRTCClient::Callback * PeekCallback() const { return mCallback; }

        // Session held from kConnecting until the request ends
        void HoldSession(const SessionHandle & aSession, Messaging::ExchangeManager & aExchangeMgr)
        {
            mSessionHolder.Grab(aSession);
            mExchangeMgr = &aExchangeMgr;
        }
        bool HasSession() const { return mSessionHolder && mExchangeMgr != nullptr; }
        // By value: SessionHolder::Get() builds a fresh Optional, so handing out a reference into it
        // would dangle as soon as the caller's full expression ends.
        Optional<SessionHandle> Session() const { return mSessionHolder.Get(); }
        Messaging::ExchangeManager & ExchangeManager() const { return *mExchangeMgr; }

        // The sender this request invoked with. A CommandSender callback that does not
        // carry this pointer belongs to an interaction this request has already finished with.
        void SetInvokedSender(CommandSender * aSender) { mInvokedSender = aSender; }
        bool WasInvokedBy(const CommandSender * aSender) const { return mInvokedSender == aSender; }

        // What the provider check learnt about the named endpoint
        enum class ProviderCheck : uint8_t
        {
            kNotFound,   // The ServerList was read and WebRTCTransportProvider is not in it (or no report came)
            kFound,      // WebRTCTransportProvider is in the ServerList
            kUnreadable, // The read failed or its report did not decode: nothing is known about the endpoint
        };
        void SetProviderCheck(ProviderCheck aResult) { mProviderCheck = aResult; }
        ProviderCheck GetProviderCheck() const { return mProviderCheck; }

        // Records the command's failure; completion is delivered once the exchange closes
        void Fail(Protocols::InteractionModel::Status aStatus)
        {
            mFailureStatus = aStatus;
            mPhase         = Phase::kFailed;
        }
        Protocols::InteractionModel::Status FailureStatus() const { return mFailureStatus; }

        void Reset() { *this = Request{}; }

    private:
        void Begin(CommandType aCommandType, const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                   AvAnalysisWebRTCClient::Callback * aCallback)
        {
            Reset();
            mPhase          = Phase::kConnecting;
            mCommandType    = aCommandType;
            mCameraNode     = aCameraNode;
            mWebRTCEndpoint = aWebRTCEndpoint;
            mCallback       = aCallback;
        }

        // Request machinery
        Phase mPhase                                       = Phase::kIdle;
        CommandType mCommandType                           = CommandType::kProvideOffer;
        AvAnalysisWebRTCClient::Callback * mCallback       = nullptr;
        CommandSender * mInvokedSender                     = nullptr;
        ProviderCheck mProviderCheck                       = ProviderCheck::kNotFound;
        Protocols::InteractionModel::Status mFailureStatus = Protocols::InteractionModel::Status::Failure;

        // Sending the command
        SessionHolder mSessionHolder;
        Messaging::ExchangeManager * mExchangeMgr = nullptr;
        EndpointId mWebRTCEndpoint                = kInvalidEndpointId;
        uint16_t mVideoStreamId                   = 0;

        // The session record the requestor cluster gets on success
        ScopedNodeId mCameraNode;
        uint16_t mWebRTCSessionId = 0;
    };

    /**
     * The request in flight.
     */
    Request & CurrentRequest() { return mRequest; }

    /**
     * Fills a ProvideOffer request from the pending request and the buffered SDP, per the WebRTC
     * Normal Flow: a null session id asks for a new session, StreamUsage is Analysis, and the
     * originating endpoint is where our WebRTCTransportRequestor cluster is registered. The sdp
     * span references mOfferSdp and is only valid while it is unchanged. The VideoStreams list
     * references aVideoStream, which the caller keeps alive until the request is encoded.
     */
    CHIP_ERROR BuildProvideOffer(WebRTCTransportProvider::Commands::ProvideOffer::Type & aRequest, uint16_t & aVideoStream) const;

    /**
     * Fills a ProvideICECandidates request for the session this request is about, from the
     * buffered candidates. The list references aCandidates, which the caller keeps alive until the
     * request is encoded.
     */
    CHIP_ERROR BuildProvideICECandidates(WebRTCTransportProvider::Commands::ProvideICECandidates::Type & aRequest,
                                         std::vector<Globals::Structs::ICECandidateStruct::Type> & aCandidates) const;

    /**
     * Fills an EndSession request for the session this request is about, with the Reason the
     * AV Analysis cluster prescribes for DeactivateAnalysisStream: UserHangup.
     */
    CHIP_ERROR BuildEndSession(WebRTCTransportProvider::Commands::EndSession::Type & aRequest) const;

    /**
     * Send the built command on the held session. The default bodies are the expected behavior;
     * virtual only so unit tests can intercept the network boundary.
     */
    virtual CHIP_ERROR SendProvideOffer();
    virtual CHIP_ERROR SendProvideICECandidates();
    virtual CHIP_ERROR SendEndSession();

private:
    // A session the camera assigned, tracked until it ends: routes its late signals to the
    // callback that initiated it, and (for outbound sends about it) back to the camera.
    struct TrackedSession
    {
        uint16_t webRTCSessionId                    = 0;
        AvAnalysisWebRTCClient::Callback * callback = nullptr;
        ScopedNodeId cameraNode;
        EndpointId providerEndpoint = kInvalidEndpointId;
        bool inUse                  = false;
    };

    // Common preconditions of RequestSession/EndSession, checked before Request::Begin* runs
    CHIP_ERROR CanStartRequest() const;
    CHIP_ERROR SendProviderCheckRead();
    void ResetReadClient();
    void HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData);
    void OnProviderCheckComplete();
    // Tracks the camera-assigned session on the requestor cluster; a stale session under the same key is failed first
    CHIP_ERROR RegisterSession(uint16_t aWebRTCSessionId);
    // Frees the slot, then removes the session from the requestor cluster and releases the peer connection
    void ReleaseSession(TrackedSession & aSession);
    // Routes NotifyFailed/NotifyEnded: releases the tracked session, then reports OnSessionFailed
    void FailTrackedSession(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId);
    void FinishRequest(Protocols::InteractionModel::Status aStatus, uint16_t aWebRTCSessionId);
    // Session ids are unique only within a camera, so a session is identified by both
    TrackedSession * FindTrackedSession(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId);
    TrackedSession * FindFreeSession();

    /**
     * Invokes the command aCommandId of WebRTCTransportProvider with the encodable aRequest on the
     * held session with a fresh CommandSender, recording it as the request's sender and advancing
     * to kInvoking; the command's outcome arrives through the CommandSender callbacks.
     */
    CHIP_ERROR InvokeOnHeldSession(CommandId aCommandId, const DataModel::EncodableToTLV & aRequest);

    static void OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    CASESessionManager * mCASESessionManager                                      = nullptr;
    AvAnalysisWebRTCPeerDelegate * mPeerDelegate                                  = nullptr;
    WebRTCTransportRequestor::WebRTCTransportRequestorCluster * mRequestorCluster = nullptr;
    Platform::ScopedMemoryBuffer<TrackedSession> mSessions;
    uint8_t mMaxSessions = 0;

    Request mRequest;

    // The payload of the request in flight, buffered for the asynchronous send
    std::string mOfferSdp;
    struct BufferedICECandidate
    {
        std::string candidate;
        std::optional<std::string> sdpMid; // absent encodes a null SDPMid
        DataModel::Nullable<uint16_t> sdpMLineIndex;
    };
    std::vector<BufferedICECandidate> mICECandidates;

    // mReadCallback is declared before mReadClient on purpose: the ReadClient holds the callback by
    // reference, and members are destroyed in reverse declaration order, so the callback outlives it.
    Platform::UniquePtr<BufferedReadCallback> mReadCallback;
    Platform::UniquePtr<ReadClient> mReadClient;
    Platform::UniquePtr<CommandSender> mCommandSender;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectionFailureCallback;
};

} // namespace Clusters
} // namespace app
} // namespace chip
