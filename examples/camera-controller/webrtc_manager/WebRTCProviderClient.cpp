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

void WebRTCProviderClient::Init(const ScopedNodeId & peerId, EndpointId endpointId,
                                Clusters::WebRTCTransportRequestor::WebRTCTransportRequestorServer * requestorServer)
{
    mPeerId          = peerId;
    mEndpointId      = endpointId;
    mRequestorServer = requestorServer;

    ChipLogProgress(Camera, "WebRTCProviderClient: Initialized with PeerId=0x" ChipLogFormatX64 ", endpoint=%u",
                    ChipLogValueX64(peerId.GetNodeId()), static_cast<unsigned>(endpointId));
}

CHIP_ERROR WebRTCProviderClient::ProvideOffer(
    DataModel::Nullable<uint16_t> webRTCSessionID, std::string sdp, Clusters::WebRTCTransportProvider::StreamUsageEnum streamUsage,
    EndpointId originatingEndpointID, Optional<DataModel::Nullable<uint16_t>> videoStreamID,
    Optional<DataModel::Nullable<uint16_t>> audioStreamID,
    Optional<DataModel::List<const Clusters::WebRTCTransportProvider::Structs::ICEServerStruct::Type>> ICEServers,
    Optional<chip::CharSpan> ICETransportPolicy)
{
    ChipLogProgress(Camera, "Sending ProvideOffer to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    // Store the command type
    mCommandType = CommandType::kProvideOffer;

    // Stash data in class members so the CommandSender can safely reference them async
    mSdpString                              = sdp;
    mProvideOfferData.webRTCSessionID       = webRTCSessionID;
    mProvideOfferData.sdp                   = CharSpan::fromCharString(mSdpString.c_str());
    mProvideOfferData.streamUsage           = streamUsage;
    mProvideOfferData.originatingEndpointID = originatingEndpointID;
    mProvideOfferData.videoStreamID         = videoStreamID;
    mProvideOfferData.audioStreamID         = audioStreamID;
    mProvideOfferData.ICEServers            = ICEServers;
    mProvideOfferData.ICETransportPolicy    = ICETransportPolicy;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderClient::ProvideICECandidates(uint16_t webRTCSessionID, DataModel::List<const chip::CharSpan> ICECandidates)
{
    ChipLogProgress(Camera, "Sending ProvideICECandidates to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

    // Store the command type
    mCommandType = CommandType::kProvideICECandidates;

    // Stash data in class members so the CommandSender can safely reference them async
    mProvideICECandidatesData.webRTCSessionID = webRTCSessionID;
    mProvideICECandidatesData.ICECandidates   = ICECandidates;

    // Attempt to find or establish a CASE session to the target PeerId.
    InteractionModelEngine * engine     = InteractionModelEngine::GetInstance();
    CASESessionManager * caseSessionMgr = engine->GetCASESessionManager();
    VerifyOrReturnError(caseSessionMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback);

    return CHIP_NO_ERROR;
}

void WebRTCProviderClient::OnResponse(CommandSender * client, const ConcreteCommandPath & path, const StatusIB & status,
                                      TLV::TLVReader * data)
{
    ChipLogProgress(Camera, "WebRTCProviderClient: OnResponse.");

    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        ChipLogError(Camera, "Response Failure: %s", ErrorStr(error));
        return;
    }

    if (path.mClusterId == Clusters::WebRTCTransportProvider::Id &&
        path.mCommandId == Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::Id)
    {
        if (data == nullptr)
        {
            ChipLogError(NotSpecified, "Response Failure: data is null");
            return;
        }

        HandleProvideOfferResponse(*data);
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
}

CHIP_ERROR WebRTCProviderClient::SendCommandForType(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle,
                                                    CommandType commandType)
{
    ChipLogProgress(Camera, "Sending command with Endpoint ID: %d, Command Type: %d", mEndpointId, static_cast<int>(commandType));

    switch (commandType)
    {
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
    }
}

void WebRTCProviderClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    WebRTCProviderClient * self = reinterpret_cast<WebRTCProviderClient *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectionFailure: context is null"));
    self->OnDone(nullptr);
}

void WebRTCProviderClient::HandleProvideOfferResponse(TLV::TLVReader & data)
{
    ChipLogProgress(NotSpecified, "WebRTCProviderClient::HandleProvideOfferResponse.");

    Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType value;
    CHIP_ERROR error = app::DataModel::Decode(data, value);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to decode command response value. Error: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Create a new session record and populate fields from the decoded command response and current secure session info
    Clusters::WebRTCTransportProvider::Structs::WebRTCSessionStruct::Type session;
    session.id             = value.webRTCSessionID;
    session.peerNodeID     = mPeerId.GetNodeId();
    session.fabricIndex    = mPeerId.GetFabricIndex();
    session.peerEndpointID = mEndpointId;

    //TODO:: spec needs to clarify how to set streamUsage here

    // Populate optional fields for video/audio stream IDs if present; set them to Null otherwise
    if (value.videoStreamID.HasValue())
    {
        session.videoStreamID = value.videoStreamID.Value();
    }
    else
    {
        session.videoStreamID.SetNull();
    }

    if (value.audioStreamID.HasValue())
    {
        session.audioStreamID = value.audioStreamID.Value();
    }
    else
    {
        session.audioStreamID.SetNull();
    }

    if (mRequestorServer == nullptr)
    {
        ChipLogError(NotSpecified, "WebRTCProviderClient is not initialized");
        return;
    }

    // Insert or update the Requestor cluster's CurrentSessions.
    mRequestorServer->UpsertSession(session);
}
