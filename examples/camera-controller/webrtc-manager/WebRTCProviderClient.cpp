/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "WebRTCProviderClient.h"
#include "WebRTCManager.h"

using namespace ::chip;
using namespace ::chip::app;
using WebRTCSessionStruct = chip::app::Clusters::Globals::Structs::WebRTCSessionStruct::Type;

namespace {

// Constants
constexpr uint32_t kSessionTimeoutSeconds       = 5;
constexpr uint32_t kDeferredOfferTimeoutSeconds = 30;

using ICEServerStruct = WebRTCProviderClient::ICEServerStruct;

// Points the optional fields of data at options. urlStorage and serverStorage hold the ICEServers list, whose strings point
// into options, so options must stay unchanged until the command is sent.
template <typename OfferData>
void SetOfferOptions(const WebRTCProviderClient::OfferOptions & options, std::vector<std::vector<CharSpan>> & urlStorage,
                     std::vector<ICEServerStruct> & serverStorage, OfferData & data)
{
    data.videoStreams.ClearValue();
    if (options.videoStreams.has_value())
    {
        data.videoStreams.SetValue(DataModel::List<const uint16_t>(options.videoStreams->data(), options.videoStreams->size()));
    }
    data.audioStreams.ClearValue();
    if (options.audioStreams.has_value())
    {
        data.audioStreams.SetValue(DataModel::List<const uint16_t>(options.audioStreams->data(), options.audioStreams->size()));
    }

    data.metadataEnabled.ClearValue();
    if (options.metadataEnabled.has_value())
    {
        data.metadataEnabled.SetValue(*options.metadataEnabled);
    }

    data.SFrameConfig.ClearValue();
    if (options.sFrameConfig.has_value())
    {
        Clusters::WebRTCTransportProvider::Structs::SFrameStruct::Type sFrameConfig;
        sFrameConfig.cipherSuite = options.sFrameConfig->cipherSuite;
        sFrameConfig.baseKey     = ByteSpan(options.sFrameConfig->baseKey.data(), options.sFrameConfig->baseKey.size());
        sFrameConfig.kid         = ByteSpan(options.sFrameConfig->kid.data(), options.sFrameConfig->kid.size());
        data.SFrameConfig.SetValue(sFrameConfig);
    }

    urlStorage.clear();
    serverStorage.clear();
    data.ICEServers.ClearValue();
    if (options.iceServers.has_value())
    {
        for (const auto & iceServer : *options.iceServers)
        {
            std::vector<CharSpan> urls;
            for (const auto & url : iceServer.urls)
            {
                urls.push_back(CharSpan(url.data(), url.size()));
            }
            urlStorage.push_back(std::move(urls));

            ICEServerStruct server;
            if (iceServer.username.has_value())
            {
                server.username.SetValue(CharSpan(iceServer.username->data(), iceServer.username->size()));
            }
            if (iceServer.credential.has_value())
            {
                server.credential.SetValue(CharSpan(iceServer.credential->data(), iceServer.credential->size()));
            }
            if (iceServer.caid.has_value())
            {
                server.caid.SetValue(*iceServer.caid);
            }
            serverStorage.push_back(server);
        }
        // urlStorage is complete, so each server's URL list can point into it.
        for (size_t i = 0; i < serverStorage.size(); ++i)
        {
            serverStorage[i].URLs = DataModel::List<const CharSpan>(urlStorage[i].data(), urlStorage[i].size());
        }
        data.ICEServers.SetValue(DataModel::List<const ICEServerStruct>(serverStorage.data(), serverStorage.size()));
    }

    data.ICETransportPolicy.ClearValue();
    if (options.iceTransportPolicy.has_value())
    {
        data.ICETransportPolicy.SetValue(CharSpan(options.iceTransportPolicy->data(), options.iceTransportPolicy->size()));
    }
}

// Sets a session's stream list and its deprecated single stream ID, as WebRTCSessionStruct defines them: the list holds
// every stream of the session (the requested list, or else the stream the provider reported) and the ID is its first entry.
void SetSessionStreams(const Optional<DataModel::List<const uint16_t>> & requestedStreams,
                       const Optional<DataModel::Nullable<uint16_t>> & reportedStreamID, uint16_t & singleStreamStorage,
                       Optional<DataModel::List<const uint16_t>> & sessionStreams, DataModel::Nullable<uint16_t> & sessionStreamID)
{
    if (requestedStreams.HasValue() && !requestedStreams.Value().empty())
    {
        sessionStreams = requestedStreams;
        sessionStreamID.SetNonNull(requestedStreams.Value()[0]);
    }
    else if (reportedStreamID.HasValue() && !reportedStreamID.Value().IsNull())
    {
        singleStreamStorage = reportedStreamID.Value().Value();
        sessionStreams.SetValue(DataModel::List<const uint16_t>(&singleStreamStorage, 1));
        sessionStreamID.SetNonNull(singleStreamStorage);
    }
}

} // namespace

void WebRTCProviderClient::Init(const ScopedNodeId & peerId, EndpointId endpointId,
                                Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorCluster * requestorServer)
{
    mPeerId          = peerId;
    mEndpointId      = endpointId;
    mRequestorServer = requestorServer;

    ChipLogProgress(Camera, "WebRTCProviderClient: Initialized with PeerId=0x" ChipLogFormatX64 ", endpoint=%u",
                    ChipLogValueX64(peerId.GetNodeId()), static_cast<unsigned>(endpointId));
}

CHIP_ERROR WebRTCProviderClient::SolicitOffer(StreamUsageEnum streamUsage, EndpointId originatingEndpointId,
                                              Optional<DataModel::Nullable<uint16_t>> videoStreamId,
                                              Optional<DataModel::Nullable<uint16_t>> audioStreamId, const OfferOptions & options)
{
    ChipLogProgress(Camera, "Sending SolicitOffer to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    if (mState != State::Idle)
    {
        ChipLogError(Camera, "Operation NOT POSSIBLE: another sync is in progress");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Store the command type
    mCommandType = CommandType::kSolicitOffer;

    // Stash data in class members so the CommandSender can safely reference them async
    mSolicitOfferData.streamUsage           = streamUsage;
    mSolicitOfferData.originatingEndpointID = originatingEndpointId;
    mSolicitOfferData.videoStreamID         = videoStreamId;
    mSolicitOfferData.audioStreamID         = audioStreamId;
    mOfferOptions                           = options;
    SetOfferOptions(mOfferOptions, mICEServerURLs, mICEServers, mSolicitOfferData);

    // Store the streamUsage from the original command so we can build the WebRTCSessionStruct when the response arrives.
    mCurrentStreamUsage = streamUsage;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Connecting);

    // WebRTC ProvideOffer requires a large payload session establishment
    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                           TransportPayloadCapability::kLargePayload);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderClient::ProvideOffer(DataModel::Nullable<uint16_t> webRTCSessionId, std::string sdp,
                                              StreamUsageEnum streamUsage, EndpointId originatingEndpointId,
                                              Optional<DataModel::Nullable<uint16_t>> videoStreamId,
                                              Optional<DataModel::Nullable<uint16_t>> audioStreamId, const OfferOptions & options)
{
    ChipLogProgress(Camera, "Sending ProvideOffer to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    if (mState != State::Idle)
    {
        ChipLogError(Camera, "Operation NOT POSSIBLE: another sync is in progress");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Store the command type
    mCommandType = CommandType::kProvideOffer;

    // Stash data in class members so the CommandSender can safely reference them async
    mSdpString                              = sdp;
    mProvideOfferData.webRTCSessionID       = webRTCSessionId;
    mProvideOfferData.sdp                   = CharSpan::fromCharString(mSdpString.c_str());
    mProvideOfferData.streamUsage           = streamUsage;
    mProvideOfferData.originatingEndpointID = originatingEndpointId;
    mProvideOfferData.videoStreamID         = videoStreamId;
    mProvideOfferData.audioStreamID         = audioStreamId;
    mOfferOptions                           = options;
    SetOfferOptions(mOfferOptions, mICEServerURLs, mICEServers, mProvideOfferData);

    // Store the streamUsage from the original command so we can build the WebRTCSessionStruct when the response arrives.
    mCurrentStreamUsage = streamUsage;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Connecting);

    // WebRTC ProvideOffer requires a large payload session establishment
    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                           TransportPayloadCapability::kLargePayload);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderClient::ProvideAnswer(uint16_t webRTCSessionId, const std::string & sdp)
{
    ChipLogProgress(Camera, "Sending ProvideAnswer to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    if (mState != State::Idle)
    {
        ChipLogError(Camera, "Operation NOT POSSIBLE: another sync is in progress");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Store the command type
    mCommandType = CommandType::kProvideAnswer;

    // Stash data in class members so the CommandSender can safely reference them async
    mProvideAnswerData.webRTCSessionID = webRTCSessionId;
    mProvideAnswerData.sdp             = CharSpan::fromCharString(sdp.c_str());
    ;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Connecting);

    // WebRTC ProvideOffer requires a large payload session establishment
    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                           TransportPayloadCapability::kLargePayload);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderClient::ProvideICECandidates(uint16_t webRTCSessionId, const std::vector<ICECandidateInfo> & iceCandidates)
{
    ChipLogProgress(Camera, "Sending ProvideICECandidates to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    if (mState != State::Idle)
    {
        ChipLogError(Camera, "Operation NOT POSSIBLE: another sync is in progress");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Store the command type
    mCommandType = CommandType::kProvideICECandidates;

    // Store ICE Candidates for lifetime management
    mClientICECandidates = iceCandidates;
    mICECandidateStructList.clear();

    for (const auto & candidateInfo : mClientICECandidates)
    {
        ICECandidateStruct iceCandidate;
        iceCandidate.candidate = CharSpan(candidateInfo.candidate.data(), candidateInfo.candidate.size());

        // Set SDPMid if available
        if (!candidateInfo.mid.empty())
        {
            iceCandidate.SDPMid.SetNonNull(CharSpan(candidateInfo.mid.data(), candidateInfo.mid.size()));
        }
        else
        {
            iceCandidate.SDPMid.SetNull();
        }

        // Set SDPMLineIndex if valid
        if (candidateInfo.mlineIndex >= 0)
        {
            iceCandidate.SDPMLineIndex.SetNonNull(static_cast<uint16_t>(candidateInfo.mlineIndex));
        }
        else
        {
            iceCandidate.SDPMLineIndex.SetNull();
        }

        mICECandidateStructList.push_back(iceCandidate);
    }

    // Stash data in class members so the CommandSender can safely reference them async
    mProvideICECandidatesData.webRTCSessionID = webRTCSessionId;
    mProvideICECandidatesData.ICECandidates =
        chip::app::DataModel::List<const ICECandidateStruct>(mICECandidateStructList.data(), mICECandidateStructList.size());

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Connecting);

    // WebRTC ProvideOffer requires a large payload session establishment
    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                           TransportPayloadCapability::kLargePayload);

    return CHIP_NO_ERROR;
}

void WebRTCProviderClient::HandleOfferReceived(uint16_t webRTCSessionId)
{
    ChipLogProgress(Camera, "Offer command received for WebRTC session ID: %u", webRTCSessionId);

    DeviceLayer::SystemLayer().CancelTimer(OnSessionEstablishTimeout, this);
    MoveToState(State::Idle);
}

void WebRTCProviderClient::HandleAnswerReceived(uint16_t webRTCSessionId)
{
    ChipLogProgress(Camera, "Answer command received for WebRTC session ID: %u", webRTCSessionId);

    DeviceLayer::SystemLayer().CancelTimer(OnSessionEstablishTimeout, this);
    MoveToState(State::Idle);
}

void WebRTCProviderClient::OnResponse(CommandSender * client, const ConcreteCommandPath & path, const StatusIB & status,
                                      TLV::TLVReader * data)
{
    ChipLogProgress(Camera, "WebRTCProviderClient: OnResponse received for cluster: 0x%" PRIx32 " command: 0x%" PRIx32,
                    path.mClusterId, path.mCommandId);

    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Camera, "Response Failure: %s", ErrorStr(error));
        return;
    }

    // Handle different command responses
    if (path.mClusterId == Clusters::WebRTCTransportProvider::Id)
    {
        switch (path.mCommandId)
        {
        case Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::Id:
            ChipLogDetail(Camera, "Processing SolicitOfferResponse");
            if (data == nullptr)
            {
                ChipLogError(Camera, "Response failure: data pointer is null");
                return;
            }

            HandleSolicitOfferResponse(*data);
            break;

        case Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::Id:
            ChipLogDetail(Camera, "Processing ProvideOfferResponse");
            if (data == nullptr)
            {
                ChipLogError(Camera, "Response failure: data pointer is null");
                return;
            }
            HandleProvideOfferResponse(*data);
            break;

        default:
            ChipLogDetail(Camera, "Unexpected command ID: 0x%" PRIx32, path.mCommandId);
            break;
        }
    }
    else
    {
        ChipLogDetail(Camera, "Unexpected cluster ID: 0x%" PRIx32, path.mClusterId);
    }
}

void WebRTCProviderClient::OnError(const CommandSender * client, CHIP_ERROR error)
{
    ChipLogError(Camera, "WebRTCProviderClient: OnError for command %u: %" CHIP_ERROR_FORMAT, static_cast<unsigned>(mCommandType),
                 error.Format());
}

void WebRTCProviderClient::OnDone(CommandSender * client)
{
    ChipLogProgress(Camera, "WebRTCProviderClient: OnDone for command %u.", static_cast<unsigned>(mCommandType));

    // Reset command type, free up the CommandSender
    mCommandType = CommandType::kUndefined;
    mCommandSender.reset();

    if (mState == State::AwaitingResponse || mState == State::Connecting)
    {
        MoveToState(State::Idle);
    }
}

void WebRTCProviderClient::MoveToState(const State targetState)
{
    mState = targetState;
    ChipLogProgress(Camera, "WebRTCProviderClient moving to [ %s ]", GetStateStr());
}

const char * WebRTCProviderClient::GetStateStr() const
{
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::Connecting:
        return "Connecting";

    case State::AwaitingResponse:
        return "AwaitingResponse";

    case State::AwaitingOffer:
        return "AwaitingOffer";

    case State::AwaitingAnswer:
        return "AwaitingAnswer";
    }
    return "N/A";
}

CHIP_ERROR WebRTCProviderClient::SendCommandForType(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle,
                                                    CommandType commandType)
{
    ChipLogProgress(Camera, "Sending command with Endpoint ID: %d, Command Type: %d", mEndpointId, static_cast<int>(commandType));

    switch (commandType)
    {
    case CommandType::kSolicitOffer:
        return SendCommand(exchangeMgr, sessionHandle, Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Id,
                           mSolicitOfferData);

    case CommandType::kProvideAnswer:
        return SendCommand(exchangeMgr, sessionHandle, Clusters::WebRTCTransportProvider::Commands::ProvideAnswer::Id,
                           mProvideAnswerData);

    case CommandType::kProvideOffer:
        return SendCommand(exchangeMgr, sessionHandle, Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id,
                           mProvideOfferData);

    case CommandType::kProvideICECandidates:
        return SendCommand(exchangeMgr, sessionHandle, Clusters::WebRTCTransportProvider::Commands::ProvideICECandidates::Id,
                           mProvideICECandidatesData);

    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void WebRTCProviderClient::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                             const SessionHandle & sessionHandle)
{
    WebRTCProviderClient * self = reinterpret_cast<WebRTCProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnected: context is null"));

    ChipLogProgress(Camera, "CASE session established, sending WebRTCTransportProvider command...");
    CHIP_ERROR sendErr = self->SendCommandForType(exchangeMgr, sessionHandle, self->mCommandType);
    if (sendErr != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "SendCommandForType failed: %" CHIP_ERROR_FORMAT, sendErr.Format());
        self->MoveToState(State::Idle);
    }
    else
    {
        self->MoveToState(State::AwaitingResponse);
    }
}

void WebRTCProviderClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    WebRTCProviderClient * self = reinterpret_cast<WebRTCProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectionFailure: context is null"));
    self->OnDone(nullptr);
}

void WebRTCProviderClient::OnSessionEstablishTimeout(chip::System::Layer * systemLayer, void * appState)
{
    WebRTCProviderClient * self = reinterpret_cast<WebRTCProviderClient *>(appState);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnSessionEstablishTimeout: context is null"));

    if (self->mCurrentSessionId != 0)
    {
        self->mRequestorServer->RemoveSession(self->mCurrentSessionId, self->mPeerId.GetNodeId(), self->mPeerId.GetFabricIndex());
        self->mCurrentSessionId = 0;
    }

    ChipLogError(Camera, "WebRTC Session establishment has timed out!");
    self->MoveToState(State::Idle);
}

void WebRTCProviderClient::HandleSolicitOfferResponse(TLV::TLVReader & data)
{
    ChipLogProgress(Camera, "WebRTCProviderClient::HandleSolicitOfferResponse.");

    Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    VerifyOrReturn(error == CHIP_NO_ERROR,
                   ChipLogError(Camera, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format()));

    // Create a new session record and populate fields from the decoded command response and current secure session info
    WebRTCSessionStruct session;
    session.id             = value.webRTCSessionID;
    session.peerNodeID     = mPeerId.GetNodeId();
    session.fabricIndex    = mPeerId.GetFabricIndex();
    session.peerEndpointID = mEndpointId;
    session.streamUsage    = mCurrentStreamUsage;

    session.metadataEnabled = mSolicitOfferData.metadataEnabled.ValueOr(false);

    mCurrentSessionId = value.webRTCSessionID;

    // UpsertSession copies the stream lists, so the single-stream storage only needs to outlive that call.
    uint16_t videoStream = 0;
    uint16_t audioStream = 0;
    SetSessionStreams(mSolicitOfferData.videoStreams, value.videoStreamID, videoStream, session.videoStreams,
                      session.videoStreamID);
    SetSessionStreams(mSolicitOfferData.audioStreams, value.audioStreamID, audioStream, session.audioStreams,
                      session.audioStreamID);

    // The provider reports VideoStreamID / AudioStreamID only when the request used them, so if DeferredOffer == FALSE at
    // least one must be valid in that case. Requests using VideoStreams / AudioStreams get a response without stream IDs.
    bool requestUsedStreamIDs = mSolicitOfferData.videoStreamID.HasValue() || mSolicitOfferData.audioStreamID.HasValue();
    if (!value.deferredOffer && requestUsedStreamIDs && session.videoStreamID.IsNull() && session.audioStreamID.IsNull())
    {
        ChipLogError(Camera, "Provider reported DeferredOffer=FALSE but did not supply a valid Video or Audio stream ID");
        return;
    }

    VerifyOrReturn(mRequestorServer != nullptr, ChipLogError(Camera, "WebRTCProviderClient is not initialized"));

    // Insert or update the Requestor cluster's CurrentSessions.
    mRequestorServer->UpsertSession(session);

    if (value.deferredOffer)
    {
        ChipLogProgress(Camera, "DeferredOffer=TRUE -- there will be a larger than normal amount of time to receive Offer command");

        // Longer timeout because the provider is in low-power standby
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kDeferredOfferTimeoutSeconds),
                                                                       OnSessionEstablishTimeout, this);
    }
    else
    {
        // Normal (shorter) timeout for an imminent Offer round-trip
        TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kSessionTimeoutSeconds),
                                                                       OnSessionEstablishTimeout, this);
    }

    MoveToState(State::AwaitingOffer);
}

void WebRTCProviderClient::HandleProvideOfferResponse(TLV::TLVReader & data)
{
    ChipLogProgress(Camera, "WebRTCProviderClient::HandleProvideOfferResponse.");

    Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    VerifyOrReturn(error == CHIP_NO_ERROR,
                   ChipLogError(Camera, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format()));

    // Create a new session record and populate fields from the decoded command response and current secure session info
    WebRTCSessionStruct session;
    session.id             = value.webRTCSessionID;
    session.peerNodeID     = mPeerId.GetNodeId();
    session.fabricIndex    = mPeerId.GetFabricIndex();
    session.peerEndpointID = mEndpointId;
    session.streamUsage    = mCurrentStreamUsage;

    session.metadataEnabled = mProvideOfferData.metadataEnabled.ValueOr(false);

    mCurrentSessionId = value.webRTCSessionID;

    // UpsertSession copies the stream lists, so the single-stream storage only needs to outlive that call.
    uint16_t videoStream = 0;
    uint16_t audioStream = 0;
    SetSessionStreams(mProvideOfferData.videoStreams, value.videoStreamID, videoStream, session.videoStreams,
                      session.videoStreamID);
    SetSessionStreams(mProvideOfferData.audioStreams, value.audioStreamID, audioStream, session.audioStreams,
                      session.audioStreamID);

    if (mRequestorServer == nullptr)
    {
        ChipLogError(Camera, "WebRTCProviderClient is not initialized");
        return;
    }

    // A re-offer only changes the SDP: StreamUsage, the streams and MetadataEnabled apply to new sessions only, so an
    // existing session record stays as it is.
    bool isReOfferOfKnownSession = false;
    if (!mProvideOfferData.webRTCSessionID.IsNull())
    {
        for (const auto & existing : mRequestorServer->GetCurrentSessions())
        {
            isReOfferOfKnownSession = isReOfferOfKnownSession ||
                (existing.id == session.id && existing.peerNodeID == session.peerNodeID &&
                 existing.GetFabricIndex() == session.GetFabricIndex());
        }
    }

    // Insert or update the Requestor cluster's CurrentSessions.
    if (!isReOfferOfKnownSession)
    {
        mRequestorServer->UpsertSession(session);
    }

    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(kSessionTimeoutSeconds),
                                                                   OnSessionEstablishTimeout, this);

    MoveToState(State::AwaitingAnswer);
}
