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

#include "webrtc-provider-manager.h"

#include <Options.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/logging/CHIPLogging.h>
#include <webrtc-transport.h>

#include <iostream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;

using namespace Camera;

void WebRTCProviderManager::Init()
{
    rtc::Configuration config;
    // config.iceServers.emplace_back("stun.l.google.com:19302");

    mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

    mPeerConnection->onLocalDescription([this](rtc::Description desc) {
        if (mState == State::SendingAnswer && desc.type() != rtc::Description::Type::Answer)
        {
            return;
        }

        mLocalSdp = std::string(desc);
        ChipLogProgress(Camera, "Local Description:");
        ChipLogProgress(Camera, "%s", mLocalSdp.c_str());

        switch (mState)
        {
        case State::SendingOffer:
            ScheduleOfferSend();
            break;
        case State::SendingAnswer:
            ScheduleAnswerSend();
            break;
        default:
            break;
        }
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        std::string candidateStr = std::string(candidate);
        mLocalCandidates.push_back(candidateStr);
        ChipLogProgress(Camera, "Local Candidate:");
        ChipLogProgress(Camera, "%s", candidateStr.c_str());
    });

    mPeerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        // Convert the enum to an integer or string as needed
        ChipLogProgress(Camera, "[State: %u]", static_cast<unsigned>(state));
        if (state == rtc::PeerConnection::State::Connected)
        {
            RegisterWebrtcTransport(mCurrentSessionId);
        }
    });

    mPeerConnection->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        ChipLogProgress(Camera, "[Gathering State: %u]", static_cast<unsigned>(state));
    });

    mPeerConnection->onDataChannel([&](std::shared_ptr<rtc::DataChannel> _dc) {
        ChipLogProgress(Camera, "[Got a DataChannel with label: %s]", _dc->label().c_str());
        mDataChannel = _dc;

        mDataChannel->onClosed([&]() { ChipLogProgress(Camera, "[DataChannel closed: %s]", mDataChannel->label().c_str()); });

        mDataChannel->onMessage([](auto data) {
            if (std::holds_alternative<std::string>(data))
            {
                ChipLogProgress(Camera, "[Received message: %s]", std::get<std::string>(data).c_str());
            }
        });
    });
}

void WebRTCProviderManager::CloseConnection()
{
    // Clean up all the Webrtc Transports
    mWebrtcTransportMap.clear();

    // Close the data channel and peer connection if they exist
    if (mDataChannel)
    {
        mDataChannel->close();
        mDataChannel.reset();
    }

    if (mPeerConnection)
    {
        mPeerConnection->close();
        mPeerConnection.reset();
    }
}

void WebRTCProviderManager::SetMediaController(MediaController * mediaController)
{
    mMediaController = mediaController;
}

CHIP_ERROR WebRTCProviderManager::HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                                     bool & outDeferredOffer)
{
    // Initialize a new WebRTC session from the SolicitOfferRequestArgs
    outSession.id          = args.sessionId;
    outSession.peerNodeID  = args.peerNodeId;
    outSession.streamUsage = args.streamUsage;
    outSession.fabricIndex = args.fabricIndex;

    // By spec, MetadataOptions SHALL be set to 0 and reserved for future use
    outSession.metadataOptions.ClearAll();

    // Resolve or allocate a VIDEO stream
    if (args.videoStreamId.HasValue())
    {
        if (args.videoStreamId.Value().IsNull())
        {
            // TODO: Automatically select the closest matching video stream for the StreamUsage requested by looking at the and the
            // server MAY allocate a new video stream if there are available resources.
        }
        else
        {
            outSession.videoStreamID = args.videoStreamId.Value();
        }
    }
    else
    {
        outSession.videoStreamID.SetNull();
    }

    // Resolve or allocate an AUDIO stream
    if (args.audioStreamId.HasValue())
    {
        if (args.audioStreamId.Value().IsNull())
        {
            // TODO: Automatically select the closest matching audio stream for the StreamUsage requested and the server MAY
            // allocate a new audio stream if there are available resources.
        }
        else
        {
            outSession.audioStreamID = args.audioStreamId.Value();
        }
    }
    else
    {
        outSession.audioStreamID.SetNull();
    }

    mPeerId                = ScopedNodeId(args.peerNodeId, args.fabricIndex);
    mOriginatingEndpointId = args.originatingEndpointId;
    mCurrentSessionId      = args.sessionId;

    outDeferredOffer = LinuxDeviceOptions::GetInstance().cameraDeferredOffer;

    MoveToState(State::SendingOffer);

    if (!mDataChannel)
    {
        mDataChannel = mPeerConnection->createDataChannel("matter-av");
    }

    mPeerConnection->createOffer();

    return CHIP_NO_ERROR;
}

void WebRTCProviderManager::RegisterWebrtcTransport(uint16_t sessionId)
{
    if (mWebrtcTransportMap.find(sessionId) == mWebrtcTransportMap.end())
    {
        return;
    }

    if (mMediaController == nullptr)
    {
        ChipLogProgress(Camera, "mMediaController is null. Failed to Register WebRTC Transport");
        return;
    }
    mMediaController->RegisterTransport(mWebrtcTransportMap[sessionId].get(), mVideoStreamID, mAudioStreamID);
}

CHIP_ERROR WebRTCProviderManager::HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession)
{
    ChipLogProgress(Camera, "HandleProvideOffer called");

    // Initialize a new WebRTC session from the SolicitOfferRequestArgs
    outSession.id          = args.sessionId;
    outSession.peerNodeID  = args.peerNodeId;
    outSession.streamUsage = args.streamUsage;
    outSession.fabricIndex = args.fabricIndex;

    // By spec, MetadataOptions SHALL be set to 0 and reserved for future use
    outSession.metadataOptions.ClearAll();

    // Resolve or allocate a VIDEO stream
    if (args.videoStreamId.HasValue())
    {
        if (args.videoStreamId.Value().IsNull())
        {
            // TODO: Automatically select the closest matching video stream for the StreamUsage requested by looking at the and the
            // server MAY allocate a new video stream if there are available resources.
        }
        else
        {
            outSession.videoStreamID = args.videoStreamId.Value();
            mVideoStreamID           = args.videoStreamId.Value().Value();
        }
    }
    else
    {
        outSession.videoStreamID.SetNull();
    }

    // Resolve or allocate an AUDIO stream
    if (args.audioStreamId.HasValue())
    {
        if (args.audioStreamId.Value().IsNull())
        {
            // TODO: Automatically select the closest matching audio stream for the StreamUsage requested and the server MAY
            // allocate a new audio stream if there are available resources.
        }
        else
        {
            outSession.audioStreamID = args.audioStreamId.Value();
            mAudioStreamID           = args.audioStreamId.Value().Value();
        }
    }
    else
    {
        outSession.audioStreamID.SetNull();
    }

    // Process the SDP Offer, begin the ICE Candidate gathering phase, create the SDP Answer, and invoke Answer.
    mPeerId                = ScopedNodeId(args.peerNodeId, args.fabricIndex);
    mOriginatingEndpointId = args.originatingEndpointId;
    mCurrentSessionId      = args.sessionId;

    if (mWebrtcTransportMap.find(args.sessionId) == mWebrtcTransportMap.end())
    {
        mWebrtcTransportMap[args.sessionId] =
            std::unique_ptr<WebrtcTransport>(new WebrtcTransport(args.sessionId, mPeerId.GetNodeId(), mPeerConnection));
    }

    MoveToState(State::SendingAnswer);
    rtc::Description remoteOffer(args.sdp, rtc::Description::Type::Offer);
    mPeerConnection->setRemoteDescription(remoteOffer);
    mPeerConnection->createAnswer();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    ChipLogProgress(Camera, "HandleProvideAnswer called with sessionId: %u", sessionId);

    // Check if the provided sessionId matches your current session
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
        ChipLogError(Camera, "Cannot set remote description: mPeerConnection is null for session ID %u", sessionId);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mPeerConnection->setRemoteDescription(sdpAnswer);

    MoveToState(State::SendingICECandidates);
    ScheduleICECandidatesSend();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideICECandidates(uint16_t sessionId, const std::vector<std::string> & candidates)
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
        ChipLogProgress(Camera, "Applying candidate: %s", candidate.c_str());
        mPeerConnection->addRemoteCandidate(candidate);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode,
                                                   DataModel::Nullable<uint16_t> videoStreamID,
                                                   DataModel::Nullable<uint16_t> audioStreamID)
{
    if (mWebrtcTransportMap.find(sessionId) != mWebrtcTransportMap.end())
    {
        ChipLogProgress(Camera, "Delete Webrtc Transport for the session: %u", sessionId);
        mWebrtcTransportMap.erase(sessionId);
    }

    if (mCurrentSessionId == sessionId)
    {
        mCurrentSessionId      = 0;
        mOriginatingEndpointId = 0;
        mPeerId                = ScopedNodeId();
        mLocalSdp.clear();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR
WebRTCProviderManager::ValidateStreamUsage(StreamUsageEnum streamUsage,
                                           const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                           const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    // TODO: Validates the requested stream usage against the camera's resource management and stream priority policies.
    return CHIP_NO_ERROR;
}

void WebRTCProviderManager::MoveToState(const State targetState)
{
    mState = targetState;
    ChipLogProgress(Camera, "WebRTCProviderManager moving to [ %s ]", GetStateStr());
}

const char * WebRTCProviderManager::GetStateStr() const
{
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::SendingOffer:
        return "SendingOffer";

    case State::SendingAnswer:
        return "SendingAnswer";

    case State::SendingICECandidates:
        return "SendingICECandidates";
    }
    return "N/A";
}

void WebRTCProviderManager::ScheduleOfferSend()
{
    ChipLogProgress(Camera, "ScheduleOfferSend called.");

    DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        ChipLogProgress(Camera, "Sending Offer command to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

        mCommandType = CommandType::kOffer;

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::ScheduleAnswerSend()
{
    ChipLogProgress(Camera, "ScheduleAnswerSend called.");

    DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        ChipLogProgress(Camera, "Sending Answer command to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

        mCommandType = CommandType::kAnswer;

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::ScheduleICECandidatesSend()
{
    ChipLogProgress(Camera, "ScheduleICECandidatesSend called.");

    DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        ChipLogProgress(Camera, "Sending ICECandidates command to node " ChipLogFormatX64, ChipLogValueX64(mPeerId.GetNodeId()));

        mCommandType = CommandType::kICECandidates;

        // Attempt to find or establish a CASE session to the target PeerId.
        CASESessionManager * caseSessionMgr = Server::GetInstance().GetCASESessionManager();
        VerifyOrDie(caseSessionMgr != nullptr);

        // WebRTC Answer requires a large payload session establishment.
        caseSessionMgr->FindOrEstablishSession(mPeerId, &mOnConnectedCallback, &mOnConnectionFailureCallback,
                                               TransportPayloadCapability::kLargePayload);
    });
}

void WebRTCProviderManager::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                              const SessionHandle & sessionHandle)
{
    WebRTCProviderManager * self = reinterpret_cast<WebRTCProviderManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnected:: context is null"));

    ChipLogProgress(Camera, "CASE session established, sending command with Command Type: %d...",
                    static_cast<int>(self->mCommandType));

    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (self->mCommandType)
    {
    case CommandType::kOffer:
        err = self->SendOfferCommand(exchangeMgr, sessionHandle);
        self->MoveToState(State::Idle);
        break;

    case CommandType::kAnswer:
        err = self->SendAnswerCommand(exchangeMgr, sessionHandle);
        self->MoveToState(State::Idle);
        break;

    case CommandType::kICECandidates:
        err = self->SendICECandidatesCommand(exchangeMgr, sessionHandle);
        self->MoveToState(State::Idle);
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

void WebRTCProviderManager::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);
    WebRTCProviderManager * self = reinterpret_cast<WebRTCProviderManager *>(context);
    VerifyOrReturn(self != nullptr, ChipLogError(Camera, "OnDeviceConnectionFailure: context is null"));
}

CHIP_ERROR WebRTCProviderManager::SendOfferCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "Offer command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) { ChipLogError(Camera, "Offer command failed: %" CHIP_ERROR_FORMAT, error.Format()); };

    uint16_t sessionId = mCurrentSessionId;
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ModifyWebRTCOfferSessionId, sessionId++);

    // Build the command
    WebRTCTransportRequestor::Commands::Offer::Type command;
    command.webRTCSessionID = sessionId;
    command.sdp             = CharSpan::fromCharString(mLocalSdp.c_str());

    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, mOriginatingEndpointId, command, onSuccess, onFailure);
}

CHIP_ERROR WebRTCProviderManager::SendAnswerCommand(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "Answer command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) { ChipLogError(Camera, "Answer command failed: %" CHIP_ERROR_FORMAT, error.Format()); };

    uint16_t sessionId = mCurrentSessionId;
    CHIP_FAULT_INJECT(chip::FaultInjection::kFault_ModifyWebRTCAnswerSessionId, sessionId++);

    // Build the command
    WebRTCTransportRequestor::Commands::Answer::Type command;
    command.webRTCSessionID = sessionId;
    command.sdp             = CharSpan::fromCharString(mLocalSdp.c_str());

    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, mOriginatingEndpointId, command, onSuccess, onFailure);
}

CHIP_ERROR WebRTCProviderManager::SendICECandidatesCommand(Messaging::ExchangeManager & exchangeMgr,
                                                           const SessionHandle & sessionHandle)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(Camera, "ICECandidates command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(Camera, "ICECandidates command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    // Build the command
    WebRTCTransportRequestor::Commands::ICECandidates::Type command;

    if (mLocalCandidates.empty())
    {
        ChipLogError(Camera, "No local ICE candidates to send");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Convert mLocalCandidates (std::vector<std::string>) into a list of CharSpans.
    std::vector<chip::CharSpan> candidateSpans;
    candidateSpans.reserve(mLocalCandidates.size());
    for (const auto & candidate : mLocalCandidates)
    {
        candidateSpans.push_back(chip::CharSpan(candidate.c_str(), static_cast<uint16_t>(candidate.size())));
    }

    auto ICECandidates = chip::app::DataModel::List<const chip::CharSpan>(candidateSpans.data(), candidateSpans.size());

    command.webRTCSessionID = mCurrentSessionId;
    command.ICECandidates   = ICECandidates;

    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle, mOriginatingEndpointId, command, onSuccess, onFailure);
}
