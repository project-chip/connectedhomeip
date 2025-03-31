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

#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/logging/CHIPLogging.h>

#include <iostream>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WebRTCTransportProvider;

using namespace Camera;

void WebRTCProviderManager::CloseConnection()
{
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

CHIP_ERROR WebRTCProviderManager::HandleSolicitOffer(const OfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                                     bool & outDeferredOffer)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideOffer(const ProvideOfferRequestArgs & args, WebRTCSessionStruct & outSession,
                                                     const chip::ScopedNodeId & peerId, EndpointId originatingEndpointId)
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

    // Process the SDP Offer, begin the ICE Candidate gathering phase, create the SDP Answer, and invoke Answer.
    CloseConnection();

    mPeerId                = peerId;
    mOriginatingEndpointId = originatingEndpointId;
    mCurrentSessionId      = args.sessionId;

    rtc::Configuration config;
    // config.iceServers.emplace_back("stun.l.google.com:19302");

    mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

    mPeerConnection->onLocalDescription([this](rtc::Description description) {
        mSdpAnswer = std::string(description);
        ChipLogProgress(NotSpecified, "Local Description:");
        ChipLogProgress(NotSpecified, "%s", mSdpAnswer.c_str());

        ScheduleAnswerSend();
    });

    mPeerConnection->onLocalCandidate([](rtc::Candidate candidate) {
        ChipLogProgress(NotSpecified, "Local Candidate:");
        ChipLogProgress(NotSpecified, "%s", std::string(candidate).c_str());
    });

    mPeerConnection->onStateChange([](rtc::PeerConnection::State state) {
        // Convert the enum to an integer or string as needed
        ChipLogProgress(NotSpecified, "[State: %u]", static_cast<unsigned>(state));
    });

    mPeerConnection->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        ChipLogProgress(NotSpecified, "[Gathering State: %u]", static_cast<unsigned>(state));
    });

    mPeerConnection->onDataChannel([&](std::shared_ptr<rtc::DataChannel> _dc) {
        ChipLogProgress(NotSpecified, "[Got a DataChannel with label: %s]", _dc->label().c_str());
        mDataChannel = _dc;

        mDataChannel->onClosed([&]() { ChipLogProgress(NotSpecified, "[DataChannel closed: %s]", mDataChannel->label().c_str()); });

        mDataChannel->onMessage([](auto data) {
            if (std::holds_alternative<std::string>(data))
            {
                ChipLogProgress(NotSpecified, "[Received message: %s]", std::get<std::string>(data).c_str());
            }
        });
    });

    mPeerConnection->setRemoteDescription(args.sdp);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideAnswer(uint16_t sessionId, const std::string & sdpAnswer)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR WebRTCProviderManager::HandleProvideICECandidates(uint16_t sessionId, const std::vector<std::string> & candidates)
{
    ChipLogProgress(NotSpecified, "HandleProvideICECandidates called with sessionId: %u", sessionId);

    // Check if the provided sessionId matches your current session
    if (sessionId != mCurrentSessionId)
    {
        ChipLogError(NotSpecified, "Session ID %u does not match the current session ID %u", sessionId, mCurrentSessionId);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!mPeerConnection)
    {
        ChipLogError(NotSpecified, "Cannot process ICE candidates: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (candidates.empty())
    {
        ChipLogError(NotSpecified, "Candidate list is empty. At least one candidate is expected.");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (const auto & candidate : candidates)
    {
        ChipLogProgress(NotSpecified, "Applying candidate: %s", candidate.c_str());
        mPeerConnection->addRemoteCandidate(candidate);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCProviderManager::HandleEndSession(uint16_t sessionId, WebRTCEndReasonEnum reasonCode,
                                                   DataModel::Nullable<uint16_t> videoStreamID,
                                                   DataModel::Nullable<uint16_t> audioStreamID)
{
    if (mCurrentSessionId == sessionId)
    {
        mCurrentSessionId      = 0;
        mOriginatingEndpointId = 0;
        mOriginatingEndpointId = 0;
        mPeerId                = ScopedNodeId();
        mSdpAnswer.clear();
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

void WebRTCProviderManager::ScheduleAnswerSend()
{
    DeviceLayer::SystemLayer().ScheduleLambda([this]() {
        CHIP_ERROR err = SendAnswerCommand(mCurrentSessionId, mSdpAnswer, mPeerId, mOriginatingEndpointId);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Answer failed: %" CHIP_ERROR_FORMAT, err.Format());
        }

        mOriginatingEndpointId = 0;
        mOriginatingEndpointId = 0;
        mPeerId                = ScopedNodeId();
    });
}

CHIP_ERROR WebRTCProviderManager::SendAnswerCommand(uint16_t sessionId, const std::string & sdpAnswer, const ScopedNodeId & peerId,
                                                    EndpointId originatingEndpointId)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Answer command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Answer command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    // TODO:#38183 Use CASESessionManager instead of sessionHandle to send Answer Command
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();
    auto sessionHandle                       = exchangeMgr.GetSessionManager()->FindSecureSessionForNode(peerId);
    if (!sessionHandle.HasValue())
    {
        ChipLogError(NotSpecified, "No secure session found for PeerId, cannot send Answer");
        return CHIP_ERROR_NOT_CONNECTED;
    }

    // Build the command
    WebRTCTransportRequestor::Commands::Answer::Type command;
    command.webRTCSessionID = sessionId;
    command.sdp             = CharSpan::fromCharString(sdpAnswer.c_str());

    // Now invoke the command using the found session handle
    return Controller::InvokeCommandRequest(&exchangeMgr, sessionHandle.Value(), originatingEndpointId, command, onSuccess,
                                            onFailure);
}
