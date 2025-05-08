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
#include <arpa/inet.h>
#include <commands/interactive/InteractiveCommands.h>
#include <crypto/RandUtils.h>
#include <cstdio>
#include <lib/support/StringBuilder.h>
#include <nlohmann/json.hpp>
#include <string>

using namespace chip;
using namespace chip::app;
using namespace std::chrono_literals;

namespace {

// Constants
constexpr const char * kWebRTCDataChannelName = "urn:csa:matter:av-metadata";

} // namespace

using nlohmann::json;

WebRTCManager::WebRTCManager() : mWebRTCRequestorServer(kWebRTCRequesterDynamicEndpointId, mWebRTCRequestorDelegate) {}

WebRTCManager::~WebRTCManager()
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

    // Schedule the ProvideAnswer() call to run asynchronously.
    DeviceLayer::SystemLayer().ScheduleLambda([this, sessionId]() { ProvideAnswer(sessionId); });

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

    VerifyOrReturnError(initializeSocket(), CHIP_ERROR_INTERNAL);

    // Create the peer connection
    rtc::Configuration config;

    // directly configure stun servers for now
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");
    config.iceServers.emplace_back("stun:stun1.l.google.com:19302");

    mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

    mPeerConnection->onLocalDescription([this](rtc::Description desc) {
        mLocalDescription = std::string(desc);
        ChipLogProgress(Camera, "Local Description:");
        json offer = { { "offer", mLocalDescription } };
        ChipLogProgress(Camera, "%s", offer.dump().c_str());
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        std::string candidateStr = candidate.candidate();
        mLocalCandidates.push_back(candidateStr);
        ChipLogProgress(Camera, "Local Candidate:");
        ChipLogProgress(Camera, "%s", candidateStr.c_str());
    });

    mPeerConnection->onStateChange(
        [](rtc::PeerConnection::State state) { ChipLogProgress(Camera, "[PeerConnection State: %d]", static_cast<int>(state)); });

    mPeerConnection->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
        ChipLogProgress(Camera, "[Gathering State: %d]", static_cast<int>(state));
        if (state == rtc::PeerConnection::GatheringState::Complete)
        {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < mLocalCandidates.size(); i++)
            {
                oss << "\"" << mLocalCandidates[i] << "\"";
                if (i != mLocalCandidates.size() - 1)
                    oss << ", ";
            }
            oss << "]";
            ChipLogProgress(Camera, "IceCandidates: %s", oss.str().c_str());
        }
    });

    // Add a media track so that controller can receive video
    // TODO get track configuration from allocated streams
    // use fixed values for now
    mMedia = rtc::Description::Video("video", rtc::Description::Direction::RecvOnly);
    mMedia.addH264Codec(VIDEO_H264_CODEC);
    mMedia.setBitrate(VIDEO_BITRATE);
    mTrack = mPeerConnection->addTrack(mMedia);

    mDepacketizer = std::make_shared<rtc::H264RtpDepacketizer>();
    mTrack->setMediaHandler(mDepacketizer);
    mTrack->onFrame([this](rtc::binary message, rtc::FrameInfo frameInfo) {
        // send H264 frames to sock so that a client can pick it up to dispaly it.
        if (sendto(sock, reinterpret_cast<const char *>(message.data()), int(message.size()), 0,
                   reinterpret_cast<const struct sockaddr *>(&socket_address), sizeof(socket_address)) == -1)
        {
            ChipLogError(Camera, "Failed to send video frame");
        }
    });

    // Create a data channel for this offerer
    mDataChannel = mPeerConnection->createDataChannel(kWebRTCDataChannelName);

    if (mDataChannel)
    {
        mDataChannel->onOpen(
            [&]() { ChipLogProgress(Camera, "[DataChannel open: %s]", mDataChannel ? mDataChannel->label().c_str() : "unknown"); });

        mDataChannel->onClosed([&]() {
            ChipLogProgress(Camera, "[DataChannel closed: %s]", mDataChannel ? mDataChannel->label().c_str() : "unknown");
        });

        mDataChannel->onMessage([](auto data) {
            if (std::holds_alternative<std::string>(data))
            {
                ChipLogProgress(Camera, "[Received: %s]", std::get<std::string>(data).c_str());
            }
        });
    }

    return CHIP_NO_ERROR;
}

bool WebRTCManager::initializeSocket()
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        ChipLogError(Camera, "Socket creation failed");
        return false;
    }
    socket_address                 = {};
    socket_address.sin_family      = AF_INET;
    socket_address.sin_addr.s_addr = inet_addr(LOCALHOST_IP);
    socket_address.sin_port        = htons(VIDEO_STREAM_DEST_PORT);
    // Since this is an outbound only socket, no need to bind
    return true;
}

CHIP_ERROR WebRTCManager::ProvideOffer(DataModel::Nullable<uint16_t> sessionId, StreamUsageEnum streamUsage)
{
    ChipLogProgress(Camera, "Sending ProvideOffer command to the peer device");

    mPeerConnection->setLocalDescription();

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

CHIP_ERROR WebRTCManager::ProvideAnswer(uint16_t sessionId)
{
    ChipLogProgress(Camera, "Sending ProvideAnswer command to the peer device");
    std::string answer(mPeerConnection->createAnswer());

    if (answer.empty())
    {
        ChipLogError(Camera, "No answer SDP to send");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    CHIP_ERROR err = mWebRTCProviderClient.ProvideAnswer(sessionId, answer);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send Answer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR WebRTCManager::ProvideICECandidates()
{
    ChipLogProgress(Camera, "Sending ProvideICECandidates command to the peer device");

    if (mLocalCandidates.empty())
    {
        ChipLogError(Camera, "No local ICE candidates to send");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = mWebRTCProviderClient.ProvideICECandidates(mLocalCandidates);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send ICE candidates: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

void WebRTCManager::HandleWebRTCProviderResponse(const ConcreteCommandPath & path, const StatusIB & status, TLV::TLVReader * data)
{
    Instance().mWebRTCProviderClient.HandleWebRTCProviderResponse(path, status, data);
}