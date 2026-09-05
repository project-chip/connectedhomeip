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
#include <clusters/WebRTCTransportProvider/Commands.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ScopedMemoryBuffer.h>
#include <transport/SessionHolder.h>

#include <string>

namespace chip {
namespace app {
namespace Clusters {

/**
 * The application side of the default WebRTC client: manages the peer connections whose signaling
 * the client performs. The methods carry the camera-assigned
 * WebRTC session id, under which the application keys its peer connections.
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
     */
    virtual void OnSessionAssigned(uint16_t aWebRTCSessionId) = 0;

    /**
     * The session is over (deactivated, failed, or cancelled): the application releases the peer
     * connection bound to this id.
     */
    virtual void OnSessionClosed(uint16_t aWebRTCSessionId) = 0;
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
     * Inbound session signals, forwarded by the application when the camera's Answer or End
     * arrives on its WebRTCTransportRequestor cluster, or its media layer observes a failure.
     * A signal for a session this client does not track is ignored.
     */
    void NotifyAnswered(uint16_t aWebRTCSessionId);
    void NotifyFailed(uint16_t aWebRTCSessionId);
    void NotifyEnded(uint16_t aWebRTCSessionId);

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

protected:
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
        };

        enum class CommandType : uint8_t
        {
            kProvideOffer,
            kEndSession,
        };

        Phase GetPhase() const { return mPhase; }
        bool InPhase(Phase aPhase) const { return mPhase == aPhase; }
        bool InFlight() const { return mPhase != Phase::kIdle; }
        void Advance(Phase aPhase) { mPhase = aPhase; }

        CommandType GetCommandType() const { return mCommandType; }
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
            Begin(CommandType::kProvideOffer, aCameraNode, aWebRTCEndpoint, aCallback);
            mVideoStreamId = aVideoStreamId;
        }

        void BeginEndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint, uint16_t aWebRTCSessionId,
                             AvAnalysisWebRTCClient::Callback & aCallback)
        {
            Begin(CommandType::kEndSession, aCameraNode, aWebRTCEndpoint, aCallback);
            mWebRTCSessionId = aWebRTCSessionId;
        }

        /**
         * Records the decoded ProvideOfferResponse: the camera-assigned session id and the video
         * stream the camera selected.
         */
        void SetOfferResponse(uint16_t aWebRTCSessionId, const DataModel::Nullable<uint16_t> & aVideoStreamId)
        {
            mWebRTCSessionId       = aWebRTCSessionId;
            mResponseVideoStreamId = aVideoStreamId;
        }
        const DataModel::Nullable<uint16_t> & ResponseVideoStreamId() const { return mResponseVideoStreamId; }

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

        void SetProviderFound(bool aFound) { mProviderFound = aFound; }
        bool ProviderFound() const { return mProviderFound; }

        void Reset() { *this = Request{}; }

    private:
        void Begin(CommandType aCommandType, const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                   AvAnalysisWebRTCClient::Callback & aCallback)
        {
            Reset();
            mPhase          = Phase::kConnecting;
            mCommandType    = aCommandType;
            mCameraNode     = aCameraNode;
            mWebRTCEndpoint = aWebRTCEndpoint;
            mCallback       = &aCallback;
        }

        // Request machinery
        Phase mPhase                                 = Phase::kIdle;
        CommandType mCommandType                     = CommandType::kProvideOffer;
        AvAnalysisWebRTCClient::Callback * mCallback = nullptr;
        CommandSender * mInvokedSender               = nullptr;
        bool mProviderFound                          = false;

        // Sending the command
        SessionHolder mSessionHolder;
        Messaging::ExchangeManager * mExchangeMgr = nullptr;
        EndpointId mWebRTCEndpoint                = kInvalidEndpointId;
        uint16_t mVideoStreamId                   = 0;

        // The session record the requestor cluster gets on success
        ScopedNodeId mCameraNode;
        uint16_t mWebRTCSessionId = 0;
        DataModel::Nullable<uint16_t> mResponseVideoStreamId;
    };

    /**
     * The request in flight.
     */
    Request & CurrentRequest() { return mRequest; }

    /**
     * Provider-check report decoder, dispatched from OnAttributeData.
     */
    void HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData);

    /**
     * Fills a ProvideOffer request from the pending request and the buffered SDP, per the WebRTC
     * Normal Flow: a null session id asks for a new session, StreamUsage is Analysis, and the
     * originating endpoint is where our WebRTCTransportRequestor cluster is registered. The sdp
     * span references mOfferSdp and is only valid while it is unchanged.
     */
    CHIP_ERROR BuildProvideOffer(WebRTCTransportProvider::Commands::ProvideOffer::Type & aRequest) const;

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
    virtual CHIP_ERROR SendEndSession();

private:
    // A session the camera assigned, tracked until it ends: routes its late signals to the
    // callback that initiated it, and (for outbound sends about it) back to the camera.
    struct TrackedSession
    {
        uint16_t webRTCSessionId                     = 0;
        AvAnalysisWebRTCClient::Callback * mCallback = nullptr;
        ScopedNodeId cameraNode;
        EndpointId providerEndpoint = kInvalidEndpointId;
        bool inUse                  = false;
    };

    // Common preconditions of RequestSession/EndSession, checked before Request::Begin* runs
    CHIP_ERROR CanStartRequest() const;
    CHIP_ERROR SendProviderCheckRead();
    void ResetReadClient();
    void OnProviderCheckComplete();
    // Tracks the camera-assigned session, records it on the requestor cluster, and hands it to the
    // peer delegate; the offer request's success completion.
    CHIP_ERROR RegisterSession(uint16_t aWebRTCSessionId);
    // The session is over on both nodes, so it leaves the requestor cluster, the peer
    // delegate releases its connection, and the slot is free again.
    void ReleaseSession(TrackedSession & aSession);
    void FinishRequest(Protocols::InteractionModel::Status aStatus, uint16_t aWebRTCSessionId);
    TrackedSession * FindTrackedSession(uint16_t aWebRTCSessionId);

    /**
     * Invokes aRequest on the held session with a fresh CommandSender, recording it as the
     * request's sender and advancing to kInvoking; the command's outcome arrives through the
     * CommandSender callbacks. A send that never left reclaims the sender, since its OnDone will
     * never come.
     */
    template <typename RequestType>
    CHIP_ERROR InvokeOnHeldSession(const RequestType & aRequest)
    {
        VerifyOrReturnError(mRequest.HasSession(), CHIP_ERROR_INCORRECT_STATE);

        // A CommandSender may only be destroyed from its own OnDone, so it must not be replaced
        // while a previous one's callbacks can still fire.
        VerifyOrReturnError(!mCommandSender, CHIP_ERROR_INCORRECT_STATE);

        auto session = mRequest.Session();
        mCommandSender =
            Platform::MakeUnique<CommandSender>(this, &mRequest.ExchangeManager(), /* aIsTimedRequest = */ false,
                                                /* aSuppressResponse = */ false, session.Value()->AllowsLargePayload());
        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

        // Recorded before sending: a send can dispatch the interaction's completion synchronously,
        // and that callback must already recognise this sender as ours.
        mRequest.SetInvokedSender(mCommandSender.get());
        mRequest.Advance(Request::Phase::kInvoking);

        CommandPathParams commandPath{ mRequest.WebRTCEndpoint(), WebRTCTransportProvider::Id, RequestType::GetCommandId(),
                                       CommandPathFlags::kEndpointIdValid };
        CHIP_ERROR err = mCommandSender->AddRequestData(commandPath, aRequest);
        if (err == CHIP_NO_ERROR)
        {
            err = mCommandSender->SendCommandRequest(session.Value());
        }
        if (err != CHIP_NO_ERROR)
        {
            mRequest.SetInvokedSender(nullptr);
            mCommandSender.reset();
        }
        return err;
    }

    static void OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error);

    CASESessionManager * mCASESessionManager                                      = nullptr;
    AvAnalysisWebRTCPeerDelegate * mPeerDelegate                                  = nullptr;
    WebRTCTransportRequestor::WebRTCTransportRequestorCluster * mRequestorCluster = nullptr;
    Platform::ScopedMemoryBuffer<TrackedSession> mSessions;
    uint8_t mMaxSessions = 0;

    Request mRequest;

    std::string mOfferSdp;

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
