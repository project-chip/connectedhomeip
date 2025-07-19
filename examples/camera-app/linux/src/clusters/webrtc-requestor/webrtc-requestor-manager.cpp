/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "webrtc-requestor-manager.h"

#include <Options.h>
#include <app/server/Server.h>
#include <camera-av-stream-manager.h>
#include <controller/InvokeInteraction.h>
#include <iostream>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/logging/CHIPLogging.h>
#include <webrtc-transport.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportRequestor;

void WebRTCRequestorManager::Init(ScopedNodeId peerId, EndpointId endpointId)
{
    ChipLogProgress(Camera, "WebRTCRequestorManager::Init");

    // PeerId and Endpoint of the controller
    mPeerId                = peerId;
    mOriginatingEndpointId = endpointId;

    mPeerConnection = CreateWebRTCPeerConnection();

    mPeerConnection->SetCallbacks([this](const std::string & sdp, SDPType type) { this->OnLocalDescription(sdp, type); },
                                  [this](const std::string & candidate) { this->OnICECandidate(candidate); },
                                  [this](bool connected) { this->OnConnectionStateChanged(connected); },
                                  [this](std::shared_ptr<WebRTCTrack> track) { this->OnTrack(track); });
}

void WebRTCRequestorManager::OnLocalDescription(const std::string & sdp, SDPType type)
{
    mLocalSdp            = sdp;
    const char * typeStr = (type == SDPType::Offer) ? "offer" : "answer";
    ChipLogProgress(Camera, "Local Description (%s):", typeStr);
    ChipLogProgress(Camera, "%s", mLocalSdp.c_str());
}

void WebRTCRequestorManager::OnICECandidate(const std::string & candidate)
{
    mLocalCandidates.push_back(candidate);
    ChipLogProgress(Camera, "Local Candidate:");
    ChipLogProgress(Camera, "%s", candidate.c_str());
}

void WebRTCRequestorManager::OnConnectionStateChanged(bool connected)
{
    if (connected)
    {
        RegisterWebrtcTransport(mCurrentSessionId);
    }
}

void WebRTCRequestorManager::OnTrack(std::shared_ptr<WebRTCTrack> track)
{
    ChipLogProgress(Camera, "Remote track received for session %u", mCurrentSessionId);
}

CHIP_ERROR WebRTCRequestorManager::HandleOffer(uint16_t sessionId, const OfferArgs & args)
{
    ChipLogProgress(Camera, "WebRTCRequestorManager::HandleOffer");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR WebRTCRequestorManager::HandleAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    ChipLogProgress(Camera, "WebRTCRequestorManager::HandleAnswer");
    if (sessionId != mCurrentSessionId)
    {
        ChipLogError(Camera, "Session ID %u does not match the current session ID %u", sessionId, mCurrentSessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (sdpAnswer.empty())
    {
        ChipLogError(Camera, "Provided SDP Answer is empty for session ID %u", sessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (!mPeerConnection)
    {
        ChipLogError(Camera, "Cannot process ICE candidates: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mPeerConnection->SetRemoteDescription(sdpAnswer, SDPType::Answer);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCRequestorManager::HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates)
{
    ChipLogProgress(Camera, "HandleProvideICECandidates called with sessionId: %u", sessionId);

    // Check if the provided sessionId matches your current session
    if (sessionId != mCurrentSessionId)
    {
        ChipLogError(Camera, "Session ID %u does not match the current session ID %u", sessionId, mCurrentSessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!mPeerConnection)
    {
        ChipLogError(Camera, "Cannot process ICE candidates: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (candidates.empty())
    {
        ChipLogError(Camera, "Candidate list is empty. At least one candidate is expected.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & candidate : candidates)
    {
        ChipLogProgress(Camera, "Applying candidate: %s",
                        std::string(candidate.candidate.begin(), candidate.candidate.end()).c_str());
        std::string mid =
            candidate.SDPMid.IsNull() ? "" : std::string(candidate.SDPMid.Value().begin(), candidate.SDPMid.Value().end());
        mPeerConnection->AddRemoteCandidate(std::string(candidate.candidate.begin(), candidate.candidate.end()), mid);
    }

    // Schedule sending Ice Candidates when remote candidates are received. This keeps the exchange simple
    ScheduleProvideICECandidatesSend();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCRequestorManager::HandleEnd(uint16_t sessionId, WebRTCEndReasonEnum reasonCode)
{
    ChipLogProgress(Camera, "WebRTCRequestorManager::HandleEnd");
    auto it = mWebrtcTransportMap.find(sessionId);
    if (it != mWebrtcTransportMap.end())
    {
        ChipLogProgress(Camera, "Delete Webrtc Transport for the session: %u", sessionId);

        ReleaseAudioVideoStreams();

        mMediaController->UnregisterTransport(it->second.get());
        mWebrtcTransportMap.erase(it);
    }

    if (mPeerConnection)
    {
        ChipLogProgress(Camera, "Closing peer connection: %u", sessionId);
        mPeerConnection->Close();
        mPeerConnection.reset();
    }

    if (mCurrentSessionId == sessionId)
    {
        mCurrentSessionId = 0;
        mPeerId           = ScopedNodeId();
        mLocalSdp.clear();
        mLocalCandidates.clear();
    }

    return CHIP_NO_ERROR;
}

void WebRTCRequestorManager::SetMediaController(MediaController * mediaController)
{
    mMediaController = mediaController;
}

void WebRTCRequestorManager::CloseConnection()
{
    ChipLogProgress(Camera, "Closing Connection");

    // Close the peer connection if they exist
    if (mPeerConnection)
    {
        mPeerConnection->Close();
        mPeerConnection.reset();
    }
}

void WebRTCRequestorManager::ScheduleProvideOfferSend()
{
    ChipLogProgress(Camera, "ScheduleProvideOfferSend");
    mVideoTrack = mPeerConnection->AddTrack(MediaType::Video);

    ChipLogProgress(Camera, "Generate and set the SDP");
    mPeerConnection->CreateOffer();

    DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        ChipLogProgress(Camera, "Sending Offer command to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));
        mCommandType = CommandType::kProvideOffer;

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);
        // WebRTC ProvideOffer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCRequestorManager::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                               const SessionHandle & sessionHandle)
{
    WebRTCRequestorManager * self = reinterpret_cast<WebRTCRequestorManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnected:: context is null"));

    ChipLogProgress(Camera, "CASE session established, sending command with Command Type: %d...",
                    static_cast<int>(self->mCommandType));

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (self->mCommandType)
    {
    case CommandType::kProvideOffer:
        err = self->SendProvideOfferCommand(exchangeMgr, sessionHandle);
        break;

    case CommandType::kProvideICECandidates:
        err = self->SendProvideICECandidatesCommand(exchangeMgr, sessionHandle);
        break;

    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "OnDeviceConnected::SendCommand failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void WebRTCRequestorManager::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    WebRTCRequestorManager * self = reinterpret_cast<WebRTCRequestorManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectionFailure: context is null"));
}

CHIP_ERROR WebRTCRequestorManager::SendProvideOfferCommand(Messaging::ExchangeManager & exchangeMgr,
                                                           const SessionHandle & sessionHandle)
{
    auto onSuccess = [this](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(Camera, "ProvideOffer Response Failure: %s", ErrorStr(error));
            return;
        }

        VerifyOrReturn(commandPath.mClusterId == Clusters::WebRTCTransportProvider::Id,
                       ChipLogError(Camera, "Unexpected cluster ID: 0x%" PRIx32, commandPath.mClusterId));

        HandleProvideOfferResponse(dataResponse);
        ChipLogProgress(Camera, "Offer command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) { ChipLogError(Camera, "Offer command failed: %" CHIP_ERROR_FORMAT, error.Format()); };

    // Build the command
    WebRTCTransportProvider::Commands::ProvideOffer::Type command;
    command.webRTCSessionID       = NullOptional;
    command.sdp                   = CharSpan::fromCharString(mLocalSdp.c_str());
    command.streamUsage           = WebRTCTransportProvider::StreamUsageEnum::kLiveView;
    command.originatingEndpointID = static_cast<chip::EndpointId>(1);
    // ICE info are sent during the ICE candidate exchange phase of this flow.
    command.ICEServers         = NullOptional;
    command.ICETransportPolicy = NullOptional;

    command.videoStreamID = NullOptional;
    command.audioStreamID = NullOptional;

    VerifyOrReturnError(mCameraDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // update stream ids with the first alllocated streams if available.
    for (const VideoStream & stream : mCameraDevice->GetCameraHALInterface().GetAvailableVideoStreams())
    {
        if (stream.isAllocated)
        {
            command.videoStreamID = MakeOptional(DataModel::MakeNullable(stream.videoStreamParams.videoStreamID));
            break;
        }
    }

    for (const AudioStream & stream : mCameraDevice->GetCameraHALInterface().GetAvailableAudioStreams())
    {
        if (stream.isAllocated)
        {
            command.audioStreamID = MakeOptional(DataModel::MakeNullable(stream.audioStreamParams.audioStreamID));
            break;
        }
    }

    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, mOriginatingEndpointId, command, onSuccess, onFailure);
}

void WebRTCRequestorManager::HandleProvideOfferResponse(
    const WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType & value)
{
    ChipLogProgress(Camera, "WebRTCRequestorManager::HandleProvideOfferResponse");

    // Create a new session record and populate fields from the decoded command response and current secure session info
    WebRTCSessionStruct session;
    session.id             = value.webRTCSessionID;
    session.peerNodeID     = mPeerId.GetNodeId();
    session.fabricIndex    = mPeerId.GetFabricIndex();
    session.peerEndpointID = mOriginatingEndpointId;
    session.streamUsage    = WebRTCTransportProvider::StreamUsageEnum::kLiveView;

    // Populate optional fields for video/audio stream IDs if present; set them to Null otherwise
    if (value.videoStreamID.HasValue())
    {
        session.videoStreamID = value.videoStreamID.Value();
        mVideoStreamID        = session.videoStreamID.IsNull() ? 0 : session.videoStreamID.Value();
    }
    else
    {
        session.videoStreamID = DataModel::MakeNullable<uint16_t>();
        mVideoStreamID        = 0;
    }

    if (value.audioStreamID.HasValue())
    {
        session.audioStreamID = value.audioStreamID.Value();
        mAudioStreamID        = session.audioStreamID.IsNull() ? 0 : session.audioStreamID.Value();
    }
    else
    {
        session.audioStreamID = DataModel::MakeNullable<uint16_t>();
        mAudioStreamID        = 0;
    }

    // Insert or update the Requestor cluster's CurrentSessions.
    if (mWebRTCRequestorServer != nullptr)
        mWebRTCRequestorServer->UpsertSession(session);

    mCurrentSessionId = value.webRTCSessionID;

    if (mWebrtcTransportMap.find(mCurrentSessionId) == mWebrtcTransportMap.end())
    {
        mWebrtcTransportMap[mCurrentSessionId] =
            std::unique_ptr<WebrtcTransport>(new WebrtcTransport(mCurrentSessionId, mPeerId.GetNodeId(), mPeerConnection));
    }
    AcquireAudioVideoStreams();
}

void WebRTCRequestorManager::ScheduleProvideICECandidatesSend()
{
    ChipLogProgress(Camera, "ScheduleProvideICECandidatesSend called.");

    DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        ChipLogProgress(Camera, "Sending ICECandidates command to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

        mCommandType = CommandType::kProvideICECandidates;

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC ProvideICECandidates requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

CHIP_ERROR WebRTCRequestorManager::SendProvideICECandidatesCommand(Messaging::ExchangeManager & exchangeMgr,
                                                                   const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "ICECandidates command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(Camera, "ICECandidates command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    // Build the command
    WebRTCTransportProvider::Commands::ProvideICECandidates::Type command;

    if (mLocalCandidates.empty())
    {
        ChipLogError(Camera, "No local ICE candidates to send");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    std::vector<ICECandidateStruct> iceCandidateStructList;
    for (const auto & candidate : mLocalCandidates)
    {
        ICECandidateStruct iceCandidate = { CharSpan::fromCharString(candidate.c_str()) };
        iceCandidateStructList.push_back(iceCandidate);
    }

    command.webRTCSessionID = mCurrentSessionId;
    command.ICECandidates = DataModel::List<const ICECandidateStruct>(iceCandidateStructList.data(), iceCandidateStructList.size());

    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, mOriginatingEndpointId, command, onSuccess, onFailure);
}

CHIP_ERROR WebRTCRequestorManager::AcquireAudioVideoStreams()
{
    VerifyOrReturnError(mCameraDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mCameraDevice->GetCameraAVStreamMgmtDelegate().OnTransportAcquireAudioVideoStreams(mAudioStreamID, mVideoStreamID);
}

CHIP_ERROR WebRTCRequestorManager::ReleaseAudioVideoStreams()
{
    VerifyOrReturnError(mCameraDevice != nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mCameraDevice->GetCameraAVStreamMgmtDelegate().OnTransportReleaseAudioVideoStreams(mAudioStreamID, mVideoStreamID);
}

void WebRTCRequestorManager::RegisterWebrtcTransport(uint16_t sessionId)
{
    auto it = mWebrtcTransportMap.find(sessionId);
    if (it == mWebrtcTransportMap.end())
    {
        return;
    }

    if (mMediaController == nullptr)
    {
        ChipLogProgress(Camera, "mMediaController is null. Failed to Register WebRTC Transport");
        return;
    }

    auto & transport = it->second;

    // Set the Video track on the transport
    if (mVideoTrack)
    {
        transport->SetVideoTrack(mVideoTrack);
    }

    // Set the Audio track on the transport
    if (mAudioTrack)
    {
        transport->SetAudioTrack(mAudioTrack);
    }

    mMediaController->RegisterTransport(transport.get(), mVideoStreamID, mAudioStreamID);
}
