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
    // Refused before any signaling: a session the camera granted but this node could not track would
    // be one nobody ends. Single flight means only this request can consume the slot found here.
    VerifyOrReturnError(FindFreeSession() != nullptr, CHIP_ERROR_NO_MEMORY);

    mRequest.BeginProvideOffer(aCameraNode, aWebRTCEndpoint, aVideoStreamId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::EndSession(const ScopedNodeId & aCameraNode, EndpointId aWebRTCEndpoint,
                                                     uint16_t aWebRTCSessionId, AvAnalysisWebRTCClient::Callback & aCallback)
{
    // Ending a session this client tracks, at the endpoint it was initiated on
    TrackedSession * session = FindTrackedSession(aCameraNode, aWebRTCSessionId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(session->providerEndpoint == aWebRTCEndpoint, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CanStartRequest());

    mRequest.BeginEndSession(aCameraNode, aWebRTCEndpoint, aWebRTCSessionId, aCallback);

    EstablishSession(aCameraNode);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendICECandidates(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId,
                                                            Span<const Globals::Structs::ICECandidateStruct::Type> aCandidates)
{
    // Candidates for a session this client tracks, routed to the camera that assigned it
    TrackedSession * session = FindTrackedSession(aCameraNode, aWebRTCSessionId);
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!aCandidates.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(CanStartRequest());

    mICECandidates.clear();
    mICECandidates.reserve(aCandidates.size());
    for (const auto & candidate : aCandidates)
    {
        BufferedICECandidate & buffered = mICECandidates.emplace_back();
        buffered.candidate.assign(candidate.candidate.data(), candidate.candidate.size());
        if (!candidate.SDPMid.IsNull())
        {
            buffered.sdpMid.emplace(candidate.SDPMid.Value().data(), candidate.SDPMid.Value().size());
        }
        buffered.sdpMLineIndex = candidate.SDPMLineIndex;
    }

    mRequest.BeginProvideICECandidates(session->cameraNode, session->providerEndpoint, aWebRTCSessionId);

    EstablishSession(mRequest.CameraNode());
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
        mICECandidates.clear();
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

CHIP_ERROR DefaultAvAnalysisWebRTCClient::InvokeOnHeldSession(CommandId aCommandId, const DataModel::EncodableToTLV & aRequest)
{
    VerifyOrReturnError(mRequest.HasSession(), CHIP_ERROR_INCORRECT_STATE);

    // A CommandSender may be destroyed at any time except from its own OnResponse or OnError, so
    // replacing a live one is only safe outside those; single flight means there is none here.
    VerifyOrReturnError(!mCommandSender, CHIP_ERROR_INCORRECT_STATE);

    auto session   = mRequest.Session();
    mCommandSender = Platform::MakeUnique<CommandSender>(this, &mRequest.ExchangeManager(), /* aIsTimedRequest = */ false,
                                                         /* aSuppressResponse = */ false, session.Value()->AllowsLargePayload());
    VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);

    // Recorded before sending: a send can dispatch the interaction's completion synchronously, and
    // that callback must already recognise this sender as ours.
    mRequest.SetInvokedSender(mCommandSender.get());
    mRequest.Advance(Request::Phase::kInvoking);

    CommandPathParams commandPath{ mRequest.WebRTCEndpoint(), WebRTCTransportProvider::Id, aCommandId,
                                   CommandPathFlags::kEndpointIdValid };
    CommandSender::AddRequestDataParameters addRequestDataParams;
    CHIP_ERROR err = mCommandSender->AddRequestData(commandPath, aRequest, addRequestDataParams);
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
    self->ContinueWithSession();
}

void DefaultAvAnalysisWebRTCClient::ContinueWithSession()
{
    if (mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        CHIP_ERROR endErr = SendEndSession();
        if (endErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AvAnalysisWebRTCClient: EndSession not sent: %" CHIP_ERROR_FORMAT, endErr.Format());
            FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
        }
        return;
    }

    if (mRequest.GetCommandType() == Request::CommandType::kProvideICECandidates)
    {
        CHIP_ERROR iceErr = SendProvideICECandidates();
        if (iceErr != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "AvAnalysisWebRTCClient: ProvideICECandidates not sent: %" CHIP_ERROR_FORMAT, iceErr.Format());
            FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
        }
        return;
    }

    // the provided endpoint must host WebRTCTransportProvider before a session may be initiated toward it
    mRequest.Advance(Request::Phase::kCheckingProvider);
    CHIP_ERROR err = SendProviderCheckRead();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: provider check not started: %" CHIP_ERROR_FORMAT, err.Format());
        FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
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
    // A ReadClient may be destroyed outside its callbacks and from its own OnDone, but not from any
    // other one, so a live one is never replaced; single flight means there is none here.
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
    VerifyOrReturn(DataModel::Decode(aData, serverList) == CHIP_NO_ERROR,
                   mRequest.SetProviderCheck(Request::ProviderCheck::kUnreadable));

    auto iter = serverList.begin();
    while (iter.Next())
    {
        if (iter.GetValue() == WebRTCTransportProvider::Id)
        {
            mRequest.SetProviderCheck(Request::ProviderCheck::kFound);
            return;
        }
    }
    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        mRequest.SetProviderCheck(Request::ProviderCheck::kUnreadable);
    }
}

void DefaultAvAnalysisWebRTCClient::OnError(CHIP_ERROR aError)
{
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kCheckingProvider));

    // A failed read says nothing about the endpoint; OnDone follows and fails the request
    ChipLogError(Zcl, "AvAnalysisWebRTCClient: provider check error: %" CHIP_ERROR_FORMAT, aError.Format());
    mRequest.SetProviderCheck(Request::ProviderCheck::kUnreadable);
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
    switch (mRequest.GetProviderCheck())
    {
    case Request::ProviderCheck::kUnreadable:
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: the ServerList of endpoint %u could not be read", mRequest.WebRTCEndpoint());
        FinishRequest(Status::Failure, mRequest.WebRTCSessionId());
        return;
    case Request::ProviderCheck::kNotFound:
        // no WebRTCTransportProvider on the endpoint the command named
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: no WebRTCTransportProvider on endpoint %u", mRequest.WebRTCEndpoint());
        FinishRequest(Status::NotFound, mRequest.WebRTCSessionId());
        return;
    case Request::ProviderCheck::kFound:
        break;
    }

    mRequest.Advance(Request::Phase::kCreatingOffer);
    CHIP_ERROR err = mPeerDelegate->CreateOffer(*this);
    // An offer delivered synchronously has moved the request on; only a request still awaiting one fails here
    if (err != CHIP_NO_ERROR && mRequest.InPhase(Request::Phase::kCreatingOffer))
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

CHIP_ERROR DefaultAvAnalysisWebRTCClient::BuildProvideOffer(WebRTCTransportProvider::Commands::ProvideOffer::Type & aRequest,
                                                            uint16_t & aVideoStream) const
{
    aVideoStream = mRequest.VideoStreamId();

    aRequest.webRTCSessionID.SetNull(); // a new session, to be assigned by the camera
    aRequest.sdp         = CharSpan(mOfferSdp.data(), mOfferSdp.size());
    aRequest.streamUsage = Globals::StreamUsageEnum::kAnalysis;
    // The requestor cluster's registered path is where the camera's answering commands must land
    aRequest.originatingEndpointID = mRequestorCluster->GetPaths().front().mEndpointId;
    aRequest.videoStreams          = MakeOptional(DataModel::List<const uint16_t>(&aVideoStream, 1));
    // No audio for analysis; ICE servers and transport policy are the camera's defaults
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendProvideOffer()
{
    uint16_t videoStream = 0;
    WebRTCTransportProvider::Commands::ProvideOffer::Type request;
    ReturnErrorOnFailure(BuildProvideOffer(request, videoStream));
    return InvokeOnHeldSession(WebRTCTransportProvider::Commands::ProvideOffer::Id,
                               DataModel::EncodableType<WebRTCTransportProvider::Commands::ProvideOffer::Type>(request));
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
    return InvokeOnHeldSession(WebRTCTransportProvider::Commands::EndSession::Id,
                               DataModel::EncodableType<WebRTCTransportProvider::Commands::EndSession::Type>(request));
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::BuildProvideICECandidates(
    WebRTCTransportProvider::Commands::ProvideICECandidates::Type & aRequest,
    std::vector<Globals::Structs::ICECandidateStruct::Type> & aCandidates) const
{
    VerifyOrReturnError(!mICECandidates.empty(), CHIP_ERROR_INCORRECT_STATE);

    aCandidates.clear();
    aCandidates.reserve(mICECandidates.size());
    for (const BufferedICECandidate & buffered : mICECandidates)
    {
        Globals::Structs::ICECandidateStruct::Type & candidate = aCandidates.emplace_back();
        candidate.candidate                                    = CharSpan(buffered.candidate.data(), buffered.candidate.size());
        if (buffered.sdpMid.has_value())
        {
            candidate.SDPMid.SetNonNull(CharSpan(buffered.sdpMid->data(), buffered.sdpMid->size()));
        }
        candidate.SDPMLineIndex = buffered.sdpMLineIndex;
    }

    aRequest.webRTCSessionID = mRequest.WebRTCSessionId();
    aRequest.ICECandidates =
        DataModel::List<const Globals::Structs::ICECandidateStruct::Type>(aCandidates.data(), aCandidates.size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::SendProvideICECandidates()
{
    // Encoded by AddRequestData within InvokeOnHeldSession, so the list storage need only live this long
    std::vector<Globals::Structs::ICECandidateStruct::Type> candidates;
    WebRTCTransportProvider::Commands::ProvideICECandidates::Type request;
    ReturnErrorOnFailure(BuildProvideICECandidates(request, candidates));
    return InvokeOnHeldSession(WebRTCTransportProvider::Commands::ProvideICECandidates::Id,
                               DataModel::EncodableType<WebRTCTransportProvider::Commands::ProvideICECandidates::Type>(request));
}

void DefaultAvAnalysisWebRTCClient::OnResponse(CommandSender * apCommandSender, const ConcreteCommandPath & aPath,
                                               const StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: response for an interaction already finished with"));
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kInvoking),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: a second response for a command already responded to"));

    if (mRequest.GetCommandType() == Request::CommandType::kProvideICECandidates ||
        mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        // Answered with a status alone. Only a SUCCESS status reaches OnResponse; the CommandSender
        // delivers every error status through OnError instead.
        VerifyOrReturn(aStatusIB.IsSuccess());
        mRequest.Advance(Request::Phase::kResponded);
        return;
    }

    VerifyOrReturn(aStatusIB.IsSuccess() && apData != nullptr,
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: ProvideOffer response carried no data"));
    VerifyOrReturn(aPath.mEndpointId == mRequest.WebRTCEndpoint() && aPath.mClusterId == WebRTCTransportProvider::Id &&
                       aPath.mCommandId == WebRTCTransportProvider::Commands::ProvideOfferResponse::Id,
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: unexpected response path"));

    WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType response;
    VerifyOrReturn(DataModel::Decode(*apData, response) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: ProvideOfferResponse did not decode"));

    mRequest.SetOfferResponse(response.webRTCSessionID);
    mRequest.Advance(Request::Phase::kResponded);
}

void DefaultAvAnalysisWebRTCClient::OnError(const CommandSender * apCommandSender, CHIP_ERROR aError)
{
    VerifyOrReturn(mRequest.WasInvokedBy(apCommandSender),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: error for an interaction already finished with"));
    VerifyOrReturn(mRequest.InPhase(Request::Phase::kInvoking),
                   ChipLogError(Zcl, "AvAnalysisWebRTCClient: error after the command was already responded to"));

    // A camera status response, or Failure for a transport error such as a timeout. Recorded only:
    // the sender is still alive here, so completion waits for OnDone
    Status status = Status::Failure;
    if (aError.IsIMStatus())
    {
        status = StatusIB(aError).mStatus;
    }
    mRequest.Fail(status);
}

void DefaultAvAnalysisWebRTCClient::OnDone(CommandSender * apCommandSender)
{
    const bool isOurs = mRequest.WasInvokedBy(apCommandSender);

    // The one place a CommandSender may be destroyed, and every command outcome is delivered from
    // here, so the client is free to accept a request started from the completion.
    if (mCommandSender.get() == apCommandSender)
    {
        mCommandSender.reset();
    }

    VerifyOrReturn(isOurs);

    const uint16_t webRTCSessionId = mRequest.WebRTCSessionId();
    const ScopedNodeId cameraNode  = mRequest.CameraNode();

    if (mRequest.InPhase(Request::Phase::kFailed))
    {
        FinishRequest(mRequest.FailureStatus(), webRTCSessionId);
        return;
    }

    // Still kInvoking: the camera's response was rejected by OnResponse, so the command has no
    // usable outcome
    if (!mRequest.InPhase(Request::Phase::kResponded))
    {
        FinishRequest(Status::Failure, webRTCSessionId);
        return;
    }

    if (mRequest.GetCommandType() == Request::CommandType::kProvideICECandidates ||
        mRequest.GetCommandType() == Request::CommandType::kEndSession)
    {
        FinishRequest(Status::Success, webRTCSessionId);
        return;
    }

    CHIP_ERROR err = RegisterSession(webRTCSessionId);
    if (err != CHIP_NO_ERROR)
    {
        // A slot was free when the request started and only this path takes one, so the request
        // fails rather than leaving the camera holding a session nobody tracks.
        ChipLogError(Zcl, "AvAnalysisWebRTCClient: session %u not tracked: %" CHIP_ERROR_FORMAT, webRTCSessionId, err.Format());
        FinishRequest(Status::ResourceExhausted, webRTCSessionId);
        return;
    }

    FinishRequest(Status::Success, webRTCSessionId);

    // Last, once the request is over and the callback knows the session: the application binds the
    // peer connection it created for this offer to the assigned id
    mPeerDelegate->OnSessionAssigned(cameraNode, webRTCSessionId);
}

CHIP_ERROR DefaultAvAnalysisWebRTCClient::RegisterSession(uint16_t aWebRTCSessionId)
{
    // A camera restarts its ids from 0, so one may come back for a session we still track because
    // its End never reached us; that session died with the camera, so its stream is failed first
    if (FindTrackedSession(mRequest.CameraNode(), aWebRTCSessionId) != nullptr)
    {
        FailTrackedSession(mRequest.CameraNode(), aWebRTCSessionId);
    }

    TrackedSession * slot = FindFreeSession();
    VerifyOrReturnError(slot != nullptr, CHIP_ERROR_NO_MEMORY);

    // Tracked session
    slot->inUse            = true;
    slot->webRTCSessionId  = aWebRTCSessionId;
    slot->callback         = mRequest.PeekCallback();
    slot->cameraNode       = mRequest.CameraNode();
    slot->providerEndpoint = mRequest.WebRTCEndpoint();
    slot->videoStreamId    = mRequest.VideoStreamId();

    // The requestor cluster is the receiving end of this session
    Globals::Structs::WebRTCSessionStruct::Type session;
    session.id             = aWebRTCSessionId;
    session.peerNodeID     = mRequest.CameraNode().GetNodeId();
    session.fabricIndex    = mRequest.CameraNode().GetFabricIndex();
    session.peerEndpointID = mRequest.WebRTCEndpoint();
    session.streamUsage    = Globals::StreamUsageEnum::kAnalysis;
    session.videoStreams   = MakeOptional(DataModel::List<const uint16_t>(&slot->videoStreamId, 1));
    session.videoStreamID  = DataModel::MakeNullable(slot->videoStreamId);
    session.audioStreamID.SetNull();
    mRequestorCluster->UpsertSession(session);
    return CHIP_NO_ERROR;
}

void DefaultAvAnalysisWebRTCClient::ReleaseSession(TrackedSession & aSession)
{
    const ScopedNodeId cameraNode  = aSession.cameraNode;
    const uint16_t webRTCSessionId = aSession.webRTCSessionId;

    mRequestorCluster->RemoveSession(webRTCSessionId, cameraNode.GetNodeId(), cameraNode.GetFabricIndex());

    aSession = TrackedSession{};

    mPeerDelegate->OnSessionClosed(cameraNode, webRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::NotifyConnected(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId)
{
    TrackedSession * session = FindTrackedSession(aCameraNode, aWebRTCSessionId);
    VerifyOrReturn(
        session != nullptr,
        ChipLogDetail(Zcl, "AvAnalysisWebRTCClient: connection of untracked session %u of " ChipLogFormatScopedNodeId " ignored",
                      aWebRTCSessionId, ChipLogValueScopedNodeId(aCameraNode)));

    session->callback->OnSessionActive(aCameraNode, aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::NotifyFailed(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId)
{
    FailTrackedSession(aCameraNode, aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::NotifyEnded(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId)
{
    // An End this node did not ask for: to the AV Analysis cluster the session has failed
    FailTrackedSession(aCameraNode, aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::FailTrackedSession(const ScopedNodeId & aCameraNode, uint16_t aWebRTCSessionId)
{
    TrackedSession * session = FindTrackedSession(aCameraNode, aWebRTCSessionId);
    VerifyOrReturn(session != nullptr,
                   ChipLogDetail(Zcl,
                                 "AvAnalysisWebRTCClient: end of untracked session %u of " ChipLogFormatScopedNodeId " ignored",
                                 aWebRTCSessionId, ChipLogValueScopedNodeId(aCameraNode)));

    // Released before the callback learns of it, so the callback finds the session already gone
    AvAnalysisWebRTCClient::Callback * callback = session->callback;
    ReleaseSession(*session);
    callback->OnSessionFailed(aCameraNode, aWebRTCSessionId);
}

void DefaultAvAnalysisWebRTCClient::FinishRequest(Status aStatus, uint16_t aWebRTCSessionId)
{
    VerifyOrReturn(mRequest.InFlight());

    const Request::CommandType completed        = mRequest.GetCommandType();
    const ScopedNodeId cameraNode               = mRequest.CameraNode();
    AvAnalysisWebRTCClient::Callback * callback = mRequest.TakeCallback();
    // An offer request failing after asking for the offer leaves the application a peer connection
    // that will never get a session
    const bool offerAbandoned = mRequest.OfferRequested() && aStatus != Status::Success;
    mRequest.Reset();
    mOfferSdp.clear();
    mICECandidates.clear();

    if (offerAbandoned)
    {
        mPeerDelegate->OnOfferAbandoned();
    }

    if (completed == Request::CommandType::kProvideICECandidates)
    {
        // Nobody awaits this outcome; a camera left without our candidates fails the connection,
        // which the application reports through NotifyFailed
        if (aStatus != Status::Success)
        {
            ChipLogError(Zcl,
                         "AvAnalysisWebRTCClient: ProvideICECandidates for session %u of " ChipLogFormatScopedNodeId
                         " failed with status 0x%02x",
                         aWebRTCSessionId, ChipLogValueScopedNodeId(cameraNode), to_underlying(aStatus));
        }
        return;
    }

    VerifyOrReturn(callback != nullptr);

    if (completed == Request::CommandType::kProvideOffer)
    {
        callback->OnSessionInitiated(aStatus, aWebRTCSessionId);
        return;
    }

    // Whatever became of the EndSession, this node is done with the session: a stream whose
    // EndSession failed goes to Failure, whose only exit is a new session, so nothing could ever
    // end this one again. Its peer connection closes with it; a camera that still holds the
    // session ends it on its side once the peer is gone.
    TrackedSession * session = FindTrackedSession(cameraNode, aWebRTCSessionId);
    if (session != nullptr)
    {
        ReleaseSession(*session);
    }
    callback->OnSessionEnded(aStatus, aWebRTCSessionId);
}

DefaultAvAnalysisWebRTCClient::TrackedSession * DefaultAvAnalysisWebRTCClient::FindFreeSession()
{
    for (uint8_t i = 0; i < mMaxSessions; i++)
    {
        if (!mSessions[i].inUse)
        {
            return &mSessions[i];
        }
    }
    return nullptr;
}

DefaultAvAnalysisWebRTCClient::TrackedSession * DefaultAvAnalysisWebRTCClient::FindTrackedSession(const ScopedNodeId & aCameraNode,
                                                                                                  uint16_t aWebRTCSessionId)
{
    for (uint8_t i = 0; i < mMaxSessions; i++)
    {
        if (mSessions[i].inUse && mSessions[i].cameraNode == aCameraNode && mSessions[i].webRTCSessionId == aWebRTCSessionId)
        {
            return &mSessions[i];
        }
    }
    return nullptr;
}

} // namespace Clusters
} // namespace app
} // namespace chip
