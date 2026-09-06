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

#include <app/clusters/av-analysis-server/DefaultAvAnalysisWebRTCClient.h>

#include <app/AttributePathParams.h>
#include <app/InteractionModelEngine.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/Ids.h>
#include <clusters/WebRTCTransportProvider/Ids.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

CHIP_ERROR DefaultAvAnalysisWebRTCClient::Init(CASESessionManager * aCASESessionManager,
                                               AvAnalysisWebRTCPeerDelegate * aPeerDelegate,
                                               WebRTCTransportRequestor::WebRTCTransportRequestorCluster * aRequestorCluster,
                                               uint8_t aMaxSessions)
{
    VerifyOrReturnError(aCASESessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aPeerDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aRequestorCluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aMaxSessions > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mSessions, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mSessions.Calloc(aMaxSessions), CHIP_ERROR_NO_MEMORY);

    mCASESessionManager = aCASESessionManager;
    mPeerDelegate       = aPeerDelegate;
    mRequestorCluster   = aRequestorCluster;
    mMaxSessions        = aMaxSessions;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::RequestSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                                                         uint16_t aVideoStreamId, AvAnalysisWebRTCClient::Callback & aCallback)
{
    ReturnErrorOnFailure(CanStartRequest());

    mRequest.BeginProvideOffer(aCameraNode, aWebRTCEndpoint, aVideoStreamId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::EndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                                                     uint16_t aWebRTCSessionId, AvAnalysisWebRTCClient::Callback & aCallback)
{
    // Ending a session this client tracks
    VerifyOrReturnError(FindTrackedSession(aWebRTCSessionId) != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CanStartRequest());

    mRequest.BeginEndSession(aCameraNode, aWebRTCEndpoint, aWebRTCSessionId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisWebRTCClient::Cancel()
{
    if (mRequest.InFlight())
    {
        // Deregister from a session establishment still in progress
        mOnConnectedCallback.Cancel();
        mOnConnectionFailureCallback.Cancel();

        // Abort a provider check or command exchange still in progress; callbacks die with them
        ResetReadClient();
        mCommandSender.reset();

        // Forget the request without delivering a completion; the application drops the peer
        // connection it may have created for it
        const bool offerRequested = mRequest.OfferRequested();
        mRequest.Reset();
        mOfferSdp.clear();
        if (offerRequested)
        {
            mPeerDelegate->OnOfferAbandoned();
        }
    }

    // Forget the initiated sessions without delivering their outcomes
    for (uint8_t i = 0; i < mMaxSessions; i++)
    {
        if (mSessions[i].inUse)
        {
            ReleaseSession(mSessions[i]);
        }
    }
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::CanStartRequest() const
{
    VerifyOrReturnError(mCASESessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mRequest.InFlight(), CHIP_ERROR_BUSY);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisWebRTCClient::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                      const SessionHandle & sessionHandle)
{
    auto * self = static_cast<DefaultAvAnalysisWebRTCClient *>(context);
    VerifyOrReturn(self->mRequest.InPhase(Request::Phase::kConnecting),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected session establishment"));

    self->mRequest.HoldSession(sessionHandle, exchangeMgr);

    if (self->mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        CHIP_ERROR endErr = self->SendEndSession();
        if (endErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AvAnalysisWebRTCClient: EndSession not sent: %" CHIP_ERROR_FORMAT, endErr.Format());
            self->FinishRequest(Status::Failure, self->mRequest.WebRTCSessionId());
        }
        return;
    }

    // the provided endpoint must host WebRTCTransportProvider before a session may be initiated toward it
    self->mRequest.Advance(Request::Phase::kCheckingProvider);
    CHIP_ERROR err = self->SendProviderCheckRead();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: provider check not started: %" CHIP_ERROR_FORMAT, err.Format());
        self->FinishRequest(Status::Failure, self->mRequest.WebRTCSessionId());
    }
}

void DefaultAvAnalysisWebRTCClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    auto * self = static_cast<DefaultAvAnalysisWebRTCClient *>(context);
    VerifyOrReturn(self->mRequest.InPhase(Request::Phase::kConnecting));

    ChipLogError(Zcl, "AvAnalysisWebRTCClient: could not reach camera node: %" CHIP_ERROR_FORMAT, error.Format());
    self->FinishRequest(Status::Failure, self->mRequest.WebRTCSessionId());
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendProviderCheckRead()
{
    VerifyOrReturnError(mRequest.HasSession(), CHIP_ERROR_INCORRECT_STATE);
    // A ReadClient may only be destroyed from its OnDone
    VerifyOrReturnError(!mReadClient, CHIP_ERROR_INCORRECT_STATE);

    AttributePathParams readPath(mRequest.WebRTCEndpoint(), Descriptor::Id, Descriptor::Attributes::ServerList::Id);
    ReadPrepareParams readParams(mRequest.Session().Value());
    readParams.mpAttributePathParamsList    = &readPath;
    readParams.mAttributePathParamsListSize = 1;

    mReadCallback = Platform::MakeUnique<BufferedReadCallback>(*this);
    VerifyOrReturnError(mReadCallback != nullptr, CHIP_ERROR_NO_MEMORY);

    mReadClient = Platform::MakeUnique<ReadClient>(InteractionModelEngine::GetInstance(), &mRequest.ExchangeManager(),
                                                   *mReadCallback, ReadClient::InteractionType::Read);
    if (mReadClient == nullptr)
    {
        mReadCallback.reset();
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = mReadClient->SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        ResetReadClient();
    }
    return err;
}

void DefaultAvAnalysisWebRTCClient::ResetReadClient()
{
    // The ReadClient holds the buffered callback by reference, so it goes first
    mReadClient.reset();
    mReadCallback.reset();
}

void DefaultAvAnalysisWebRTCClient::OnAttributeData(const ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                                    const StatusIB & aStatus)
{
    VerifyOrReturn(aStatus.IsSuccess() && apData != nullptr);
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCheckingProvider));

    HandleServerListReport(aPath, *apData);
}

void DefaultAvAnalysisWebRTCClient::HandleServerListReport(const ConcreteDataAttributePath & aPath, TLV::TLVReader & aData)
{
    VerifyOrReturn(aPath.mClusterId == Descriptor::Id && aPath.mAttributeId == Descriptor::Attributes::ServerList::Id &&
                   aPath.mEndpointId == mRequest.WebRTCEndpoint());

    DataModel::DecodableList<ClusterId> serverList;
    VerifyOrReturn(DataModel::Decode(aData, serverList) == CHIP_NO_ERROR);

    auto iter = serverList.begin();
    while (iter.Next())
    {
        if (iter.GetValue() == WebRTCTransportProvider::Id)
        {
            mRequest.SetProviderFound(true);
            return;
        }
    }
}

void DefaultAvAnalysisWebRTCClient::OnError(CHIP_ERROR aError)
{
    ChipLogError(Zcl, "AvAnalysisWebRTCClient: provider check error: %" CHIP_ERROR_FORMAT, aError.Format());
}

void DefaultAvAnalysisWebRTCClient::OnDone(ReadClient * apReadClient)
{
    ResetReadClient();

    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCheckingProvider),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected read completion"));

    OnProviderCheckComplete();
}

void DefaultAvAnalysisWebRTCClient::OnProviderCheckComplete()
{
    if (!mRequest.ProviderFound())
    {
        // no WebRTCTransportProvider on the endpoint the command named
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: no WebRTCTransportProvider on endpoint %u", mRequest.WebRTCEndpoint());
        FinishRequest(Status::NotFound, mRequest.WebRTCSessionId());
        return;
    }

    mRequest.Advance(Request::Phase::kCreatingOffer);
    CHIP_ERROR err = mPeerDelegate->CreateOffer(*this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: no SDP offer forthcoming: %" CHIP_ERROR_FORMAT, err.Format());
        FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
    }
}

void DefaultAvAnalysisWebRTCClient::OnOfferReady(CHIP_ERROR aError, CharSpan aSdp)
{
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCreatingOffer),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected offer delivery"));

    if (aError != CHIP_NO_ERROR || aSdp.empty())
    {
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: the application produced no SDP offer: %" CHIP_ERROR_FORMAT, aError.Format());
        FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
        return;
    }

    mOfferSdp.assign(aSdp.data(), aSdp.size());

    CHIP_ERROR err = SendProvideOffer();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: ProvideOffer not sent: %" CHIP_ERROR_FORMAT, err.Format());
        FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
    }
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::BuildProvideOffer(WebRTCTransportProvider::Commands::ProvideOffer::Type & aRequest) const
{
    // The requestor cluster's registered path is where the camera's answering commands must land
    VerifyOrReturnError(!mRequestorCluster->GetPaths().empty(), CHIP_ERROR_INCORRECT_STATE);

    aRequest.webRTCSessionID.SetNull(); // a new session, to be assigned by the camera
    aRequest.sdp                   = CharSpan(mOfferSdp.data(), mOfferSdp.size());
    aRequest.streamUsage           = Globals::StreamUsageEnum::kAnalysis;
    aRequest.originatingEndpointID = mRequestorCluster->GetPaths().front().mEndpointId;
    aRequest.videoStreamID         = MakeOptional(DataModel::MakeNullable(mRequest.VideoStreamId()));
    // No audio for analysis; ICE servers and transport policy are the camera's defaults
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendProvideOffer()
{
    WebRTCTransportProvider::Commands::ProvideOffer::Type request;
    ReturnErrorOnFailure(BuildProvideOffer(request));
    return InvokeOnHeldSession(request);
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::BuildEndSession(WebRTCTransportProvider::Commands::EndSession::Type & aRequest) const
{
    aRequest.webRTCSessionID = mRequest.WebRTCSessionId();
    aRequest.reason          = Globals::WebRTCEndReasonEnum::kUserHangup;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendEndSession()
{
    WebRTCTransportProvider::Commands::EndSession::Type request;
    ReturnErrorOnFailure(BuildEndSession(request));
    return InvokeOnHeldSession(request);
}

void DefaultAvAnalysisWebRTCClient::OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                               const StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: response for an interaction already finished with"));
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kInvoking),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: a second response for a command already responded to"));

    if (mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        // EndSession is answered with a status alone; anything but SUCCESS arrives through OnError
        VerifyOrReturn(aStatusIB.IsSuccess());
        mRequest.Advance(Request::Phase::kResponded);
        return;
    }

    VerifyOrReturn(aStatusIB.IsSuccess() && apData != nullptr,
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: ProvideOffer response carried no data"));
    VerifyOrReturn(aPath.mClusterId == WebRTCTransportProvider::Id &&
                       aPath.mCommandId == WebRTCTransportProvider::Commands::ProvideOfferResponse::Id,
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected response command"));

    WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType response;
    VerifyOrReturn(DataModel::Decode(*apData, response) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: ProvideOfferResponse did not decode"));

    mRequest.SetOfferResponse(response.webRTCSessionID,
                              response.videoStreamID.HasValue() ? response.videoStreamID.Value() : DataModel::NullNullable);
    mRequest.Advance(Request::Phase::kResponded);
}

void DefaultAvAnalysisWebRTCClient::OnError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: error for an interaction already finished with"));

    // A camera status response
    Status status = Status::Failure;
    if (aError.IsIMStatus())
    {
        status = StatusIB(aError).mStatus;
    }
    FinishRequest(status, mRequest.WebRTCSessionId());
}

void DefaultAvAnalysisWebRTCClient::OnDone(CommandSender * apCommandSender)
{
    const bool isOurs = mRequest.WasInvokedBy(apCommandSender);

    // OnDone needs to destroy the sender, and doing it before delivering any
    // completion leaves the client free to accept a request started from that completion.
    if (mCommandSender.get() == apCommandSender)
    {
        mCommandSender.reset();
    }

    VerifyOrReturn(isOurs);

    const uint16_t webRTCSessionId = mRequest.WebRTCSessionId();

    if (mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        // Confirmed: the camera has released the session too, so it leaves this node entirely
        if (mRequest.InPhase(Request::Phase::kResponded))
        {
            TrackedSession * session = FindTrackedSession(webRTCSessionId);
            if (session != nullptr)
            {
                ReleaseSession(*session);
            }
            FinishRequest(Status::Success, webRTCSessionId);
            return;
        }
        // Unanswered (OnError already finished the request, making this a no-op, or the exchange
        // timed out): the session stays tracked; the camera may still hold it
        FinishRequest(Status::Failure, webRTCSessionId);
        return;
    }

    // The exchange ended without the offer being responded to (OnError already finished the request,
    // making this a no-op, or the exchange timed out with nothing delivered at all)
    if (!mRequest.InPhase(Request::Phase::kResponded))
    {
        FinishRequest(Status::Failure, webRTCSessionId);
        return;
    }

    CHIP_ERROR err = RegisterSession(webRTCSessionId);
    if (err != CHIP_NO_ERROR)
    {
        // The camera holds a session this node cannot track.
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: session %u not tracked: %" CHIP_ERROR_FORMAT, webRTCSessionId, err.Format());
        FinishRequest(Status::ResourceExhausted, webRTCSessionId);
        return;
    }

    FinishRequest(Status::Success, webRTCSessionId);
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::RegisterSession(uint16_t aWebRTCSessionId)
{
    TrackedSession * slot = nullptr;
    for (uint8_t i = 0; i < mMaxSessions; i++)
    {
        if (!mSessions[i].inUse)
        {
            slot = &mSessions[i];
            break;
        }
    }
    VerifyOrReturnError(slot != nullptr, CHIP_ERROR_NO_MEMORY);

    // Tracked session
    slot->inUse            = true;
    slot->webRTCSessionId  = aWebRTCSessionId;
    slot->mCallback        = mRequest.PeekCallback();
    slot->cameraNode       = mRequest.CameraNode();
    slot->providerEndpoint = mRequest.WebRTCEndpoint();

    // The requestor cluster is the receiving end of this session
    Globals::Structs::WebRTCSessionStruct::Type session;
    session.id             = aWebRTCSessionId;
    session.peerNodeID     = mRequest.CameraNode().GetNodeId();
    session.fabricIndex    = mRequest.CameraNode().GetFabricIndex();
    session.peerEndpointID = mRequest.WebRTCEndpoint();
    session.streamUsage    = Globals::StreamUsageEnum::kAnalysis;
    session.videoStreamID  = mRequest.ResponseVideoStreamId();
    session.audioStreamID.SetNull();
    mRequestorCluster->UpsertSession(session);

    // The application binds the peer connection it created for this offer to the assigned id
    mPeerDelegate->OnSessionAssigned(aWebRTCSessionId);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisWebRTCClient::ReleaseSession(TrackedSession & aSession)
{
    mRequestorCluster->RemoveSession(aSession.webRTCSessionId, aSession.cameraNode.GetNodeId(),
                                     aSession.cameraNode.GetFabricIndex());
    mPeerDelegate->OnSessionClosed(aSession.webRTCSessionId);
    aSession = TrackedSession{};
}

void DefaultAvAnalysisWebRTCClient::NotifyAnswered(uint16_t aWebRTCSessionId)
{
    TrackedSession * session = FindTrackedSession(aWebRTCSessionId);
    VerifyOrReturn(session != nullptr,
                   ChipLogProgress(Zcl, "AvAnalysisWebRTCClient: answer for untracked session %u ignored", aWebRTCSessionId));

    // The Answer completes the signaling flow; the session stays tracked until it ends
    session->mCallback->OnSessionActive(aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::NotifyFailed(uint16_t aWebRTCSessionId)
{
    FailTrackedSession(aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::NotifyEnded(uint16_t aWebRTCSessionId)
{
    // An End this node did not ask for: to the AV Analysis cluster the session has failed
    FailTrackedSession(aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::FailTrackedSession(uint16_t aWebRTCSessionId)
{
    TrackedSession * session = FindTrackedSession(aWebRTCSessionId);
    VerifyOrReturn(session != nullptr,
                   ChipLogProgress(Zcl, "AvAnalysisWebRTCClient: end of untracked session %u ignored", aWebRTCSessionId));

    // Released before the callback learns of it, so the callback finds the session already gone
    AvAnalysisWebRTCClient::Callback * callback = session->mCallback;
    ReleaseSession(*session);
    callback->OnSessionFailed(aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::FinishRequest(Status aStatus, uint16_t aWebRTCSessionId)
{
    // TakeCallback() nulls the callback, so a second completion for the same request is a no-op
    AvAnalysisWebRTCClient::Callback * callback = mRequest.TakeCallback();
    VerifyOrReturn(callback != nullptr);

    const Request::CommandType completed = mRequest.GetCommandType();
    // An offer request failing after asking for the offer leaves the application a peer connection
    // that will never get a session
    const bool offerAbandoned = mRequest.OfferRequested() && aStatus != Status::Success;
    mRequest.Reset();
    mOfferSdp.clear();

    if (offerAbandoned)
    {
        mPeerDelegate->OnOfferAbandoned();
    }

    if (completed == Request::CommandType::kProvideOffer)
    {
        callback->OnSessionInitiated(aStatus, aWebRTCSessionId);
    }
    else
    {
        callback->OnSessionEnded(aStatus, aWebRTCSessionId);
    }
}

DefaultAvAnalysisWebRTCClient::TrackedSession * DefaultAvAnalysisWebRTCClient::FindTrackedSession(uint16_t aWebRTCSessionId)
{
    for (uint8_t i = 0; i < mMaxSessions; i++)
    {
        if (mSessions[i].inUse && mSessions[i].webRTCSessionId == aWebRTCSessionId)
        {
            return &mSessions[i];
        }
    }
    return nullptr;
}

} // namespace Clusters
} // namespace app
} // namespace chip
