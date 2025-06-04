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

#include "WebRTCManager.h"

#include <app/dynamic_server/AccessControl.h>
#include <commands/interactive/InteractiveCommands.h>
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>

#include <cstdio>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace std::chrono_literals;

namespace {

// Constants
constexpr const char * kWebRTCDataChannelName = "urn:csa:matter:av-metadata";
constexpr int kVideoH264PayloadType = 96; // 96 is just the first value in the dynamic RTP payload‑type range (96‑127).
constexpr int kVideoBitRate         = 3000;

const char * GetPeerConnectionStateStr(rtc::PeerConnection::State state)
{
    switch (state)
    {
    case rtc::PeerConnection::State::New:
        return "New";

    case rtc::PeerConnection::State::Connecting:
        return "Connecting";

    case rtc::PeerConnection::State::Connected:
        return "Connected";

    case rtc::PeerConnection::State::Disconnected:
        return "Disconnected";

    case rtc::PeerConnection::State::Failed:
        return "Failed";

    case rtc::PeerConnection::State::Closed:
        return "Closed";
    }
    return "N/A";
}

const char * GetGatheringStateStr(rtc::PeerConnection::GatheringState state)
{
    switch (state)
    {
    case rtc::PeerConnection::GatheringState::New:
        return "New";

    case rtc::PeerConnection::GatheringState::InProgress:
        return "InProgress";

    case rtc::PeerConnection::GatheringState::Complete:
        return "Complete";
    }
    return "N/A";
}

} // namespace

WebRTCManager::WebRTCManager() : mWebRTCRequestorServer(kWebRTCRequesterDynamicEndpointId, mWebRTCRequestorDelegate) {}

WebRTCManager::~WebRTCManager()
{
    // Close the peer connection if they exist
    if (mPeerConnection)
    {
        mPeerConnection->close();
        mPeerConnection.reset();
    }
}

void WebRTCManager::Init()
{
    dynamic_server::InitAccessControl();
    mWebRTCRequestorServer.Init();
}

CHIP_ERROR WebRTCManager::HandleOffer(uint16_t sessionId, const WebRTCRequestorDelegate::OfferArgs & args)
{
    ChipLogProgress(Camera, "WebRTCManager::HandleOffer");

    mWebRTCProviderClient.HandleOfferReceived(sessionId);

    if (!mPeerConnection)
    {
        ChipLogError(Camera, "Cannot set remote description: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    mPeerConnection->setRemoteDescription(rtc::Description{ args.sdp, "offer" });

    if (mLocalDescription.empty())
    {
        ChipLogError(Camera, "No local SDP to send");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Store sessionId for the delayed callback
    mPendingSessionId = sessionId;

    // Schedule the ProvideAnswer() call to run with a small delay to ensure the response is sent first
    DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(300),
        [](chip::System::Layer * systemLayer, void * appState) {
            auto * self = static_cast<WebRTCManager *>(appState);
            self->ProvideAnswer(self->mPendingSessionId, self->mLocalDescription);
        },
        this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::HandleAnswer(uint16_t sessionId, const std::string & sdp)
{
    ChipLogProgress(Camera, "WebRTCManager::HandleAnswer");

    mWebRTCProviderClient.HandleAnswerReceived(sessionId);

    if (!mPeerConnection)
    {
        ChipLogError(Camera, "Cannot set remote description: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    rtc::Description answerDesc(sdp, rtc::Description::Type::Answer);
    mPeerConnection->setRemoteDescription(answerDesc);

    // Store sessionId for the delayed callback
    mPendingSessionId = sessionId;

    // Schedule the ProvideICECandidates() call to run with a small delay to ensure the response is sent first
    DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(300),
        [](chip::System::Layer * systemLayer, void * appState) {
            auto * self = static_cast<WebRTCManager *>(appState);
            self->ProvideICECandidates(self->mPendingSessionId);
        },
        this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::HandleICECandidates(uint16_t sessionId, const std::vector<ICECandidateStruct> & candidates)
{
    ChipLogProgress(Camera, "WebRTCManager::HandleICECandidates");

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
        if (candidate.SDPMid.IsNull())
        {
            mPeerConnection->addRemoteCandidate(
                rtc::Candidate(std::string(candidate.candidate.begin(), candidate.candidate.end())));
        }
        else
        {
            mPeerConnection->addRemoteCandidate(
                rtc::Candidate(std::string(candidate.candidate.begin(), candidate.candidate.end()),
                               std::string(candidate.SDPMid.Value().begin(), candidate.SDPMid.Value().end())));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::Connnect(Controller::DeviceCommissioner & commissioner, NodeId nodeId, EndpointId endpointId)
{
    ChipLogProgress(Camera, "Attempting to establish WebRTC connection to node 0x" ChipLogFormatX64 " on endpoint 0x%x",
                    ChipLogValueX64(nodeId), endpointId);

    FabricIndex fabricIndex       = commissioner.GetFabricIndex();
    const FabricInfo * fabricInfo = commissioner.GetFabricTable()->FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint64_t fabricId = fabricInfo->GetFabricId();
    ChipLogProgress(Camera, "Commissioner is on Fabric ID 0x" ChipLogFormatX64, ChipLogValueX64(fabricId));

    chip::ScopedNodeId peerId(nodeId, fabricIndex);

    mWebRTCProviderClient.Init(peerId, endpointId, &mWebRTCRequestorServer);

    rtc::InitLogger(rtc::LogLevel::Warning);

    // Create the peer connection
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");
    mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

    mPeerConnection->onLocalDescription([this](rtc::Description desc) {
        mLocalDescription = std::string(desc);
        ChipLogProgress(Camera, "Local Description:");
        ChipLogProgress(Camera, "%s", mLocalDescription.c_str());
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        std::string candidateStr = std::string(candidate);
        mLocalCandidates.push_back(candidateStr);
        ChipLogProgress(Camera, "Local Candidate:");
        ChipLogProgress(Camera, "%s", candidateStr.c_str());
    });

    mPeerConnection->onStateChange([](rtc::PeerConnection::State state) {
        ChipLogProgress(Camera, "[PeerConnection State: %s]", GetPeerConnectionStateStr(state));
    });

    mPeerConnection->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        ChipLogProgress(Camera, "[PeerConnection Gathering State: %s]", GetGatheringStateStr(state));
    });

    rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
    media.addH264Codec(kVideoH264PayloadType);
    media.setBitrate(kVideoBitRate);
    mTrack = mPeerConnection->addTrack(media);

    ChipLogProgress(Camera, "Generate and set the SDP");
    mPeerConnection->setLocalDescription();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::ProvideOffer(DataModel::Nullable<uint16_t> sessionId, StreamUsageEnum streamUsage)
{
    ChipLogProgress(Camera, "Sending ProvideOffer command to the peer device");

    if (mLocalDescription.empty())
    {
        ChipLogError(Camera, "No local SDP to send");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err =
        mWebRTCProviderClient.ProvideOffer(sessionId, mLocalDescription, streamUsage, kWebRTCRequesterDynamicEndpointId,
                                           MakeOptional(DataModel::NullNullable), // "Null" for video
                                           MakeOptional(DataModel::NullNullable)  // "Null" for audio
        );

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send ProvideOffer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR WebRTCManager::SolicitOffer(StreamUsageEnum streamUsage)
{
    ChipLogProgress(Camera, "Sending SolicitOffer command to the peer device");

    CHIP_ERROR err = mWebRTCProviderClient.SolicitOffer(streamUsage, kWebRTCRequesterDynamicEndpointId,
                                                        MakeOptional(DataModel::NullNullable), // "Null" for video
                                                        MakeOptional(DataModel::NullNullable)  // "Null" for audio
    );

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send SolicitOffer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR WebRTCManager::ProvideAnswer(uint16_t sessionId, const std::string & sdp)
{
    ChipLogProgress(Camera, "Sending ProvideAnswer command to the peer device");

    CHIP_ERROR err = mWebRTCProviderClient.ProvideAnswer(sessionId, sdp);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send Answer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR WebRTCManager::ProvideICECandidates(uint16_t sessionId)
{
    ChipLogProgress(Camera, "Sending ProvideICECandidates command to the peer device");

    if (mLocalCandidates.empty())
    {
        ChipLogError(Camera, "No local ICE candidates to send");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = mWebRTCProviderClient.ProvideICECandidates(sessionId, mLocalCandidates);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send ICE candidates: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}
