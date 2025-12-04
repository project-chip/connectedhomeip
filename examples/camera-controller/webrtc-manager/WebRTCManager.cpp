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

#include <commands/interactive/InteractiveCommands.h>
#include <controller/webrtc/WebRTCTransportRequestorManager.h>
#include <controller/webrtc/access_control/WebRTCAccessControl.h>
#include <crypto/RandUtils.h>
#include <lib/support/StringBuilder.h>

#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

using namespace chip;
using namespace chip::app;
using namespace std::chrono_literals;

namespace {

// Constants
constexpr int kVideoH264PayloadType = 96; // 96 is just the first value in the dynamic RTP payload‑type range (96‑127).
constexpr int kVideoBitRate         = 3000;

constexpr const char * kStreamGstDestIp    = "127.0.0.1";
constexpr uint16_t kVideoStreamGstDestPort = 5000;

// Constants for Audio
constexpr int kAudioBitRate                = 64000;
constexpr int kOpusPayloadType             = 111;
constexpr uint16_t kAudioStreamGstDestPort = 5001;

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

WebRTCManager::WebRTCManager() {}

WebRTCManager::~WebRTCManager()
{
    Disconnect();
}

void WebRTCManager::Init()
{
    Controller::AccessControl::InitAccessControl(kWebRTCRequesterDynamicEndpointId);

    mWebRTCRegisteredServerCluster.Create(kWebRTCRequesterDynamicEndpointId, mWebRTCRequestorDelegate);
    CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Register(mWebRTCRegisteredServerCluster.Registration());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to register WebRTCTransportRequestor on endpoint %u: %" CHIP_ERROR_FORMAT,
                     kWebRTCRequesterDynamicEndpointId, err.Format());
    }
}

CHIP_ERROR WebRTCManager::HandleOffer(const WebRTCSessionStruct & session, const WebRTCRequestorDelegate::OfferArgs & args)
{
    ChipLogProgress(Camera, "WebRTCManager::HandleOffer");

    mWebRTCProviderClient.HandleOfferReceived(session.id);

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
    mPendingSessionId = session.id;

    // Schedule the ProvideAnswer() call to run with a small delay to ensure the response is sent first
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(300),
        [](chip::System::Layer * systemLayer, void * appState) {
            auto * self = static_cast<WebRTCManager *>(appState);
            TEMPORARY_RETURN_IGNORED self->ProvideAnswer(self->mPendingSessionId, self->mLocalDescription);
        },
        this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::HandleAnswer(const WebRTCSessionStruct & session, const std::string & sdp)
{
    ChipLogProgress(Camera, "WebRTCManager::HandleAnswer");

    mWebRTCProviderClient.HandleAnswerReceived(session.id);

    if (!mPeerConnection)
    {
        ChipLogError(Camera, "Cannot set remote description: mPeerConnection is null");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    rtc::Description answerDesc(sdp, rtc::Description::Type::Answer);
    mPeerConnection->setRemoteDescription(answerDesc);

    // Store sessionId for the delayed callback
    mPendingSessionId = session.id;

    // Schedule the ProvideICECandidates() call to run with a small delay to ensure the response is sent first
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(
        chip::System::Clock::Milliseconds32(300),
        [](chip::System::Layer * systemLayer, void * appState) {
            auto * self = static_cast<WebRTCManager *>(appState);
            TEMPORARY_RETURN_IGNORED self->ProvideICECandidates(self->mPendingSessionId);
        },
        this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::HandleICECandidates(const WebRTCSessionStruct & session,
                                              const std::vector<ICECandidateStruct> & candidates)
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

void WebRTCManager::CloseRTPSocket()
{
    if (mRTPSocket != -1)
    {
        ChipLogProgress(Camera, "Closing RTP socket");
        close(mRTPSocket);
        mRTPSocket = -1;
    }
}

void WebRTCManager::Disconnect()
{
    ChipLogProgress(Camera, "Disconnecting WebRTC session");

    // Close the peer connection
    if (mPeerConnection)
    {
        mPeerConnection->close();
        mPeerConnection.reset();
    }

    // Close the RTP socket
    CloseRTPSocket();

    // Reset track
    mTrack.reset();
    mAudioTrack.reset();

    // Clear state
    mCurrentVideoStreamId = 0;
    mPendingSessionId     = 0;
    mLocalDescription.clear();
    mLocalCandidates.clear();
}

CHIP_ERROR WebRTCManager::Connnect(Controller::DeviceCommissioner & commissioner, NodeId nodeId, EndpointId endpointId)
{
    ChipLogProgress(Camera, "Attempting to establish WebRTC connection to node 0x" ChipLogFormatX64 " on endpoint 0x%x",
                    ChipLogValueX64(nodeId), endpointId);

    // Clean up any existing connection first
    Disconnect();

    FabricIndex fabricIndex       = commissioner.GetFabricIndex();
    const FabricInfo * fabricInfo = commissioner.GetFabricTable()->FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint64_t fabricId = fabricInfo->GetFabricId();
    ChipLogProgress(Camera, "Commissioner is on Fabric ID 0x" ChipLogFormatX64, ChipLogValueX64(fabricId));

    chip::ScopedNodeId peerId(nodeId, fabricIndex);

    mWebRTCProviderClient.Init(peerId, endpointId, &mWebRTCRegisteredServerCluster.Cluster());

    rtc::InitLogger(rtc::LogLevel::Warning);

    // Create the peer connection
    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");
    mPeerConnection = std::make_shared<rtc::PeerConnection>(config);

    mPeerConnection->onLocalDescription([this](rtc::Description desc) {
        mLocalDescription = std::string(desc);
        ChipLogProgress(Camera, "Local Description:");
        ChipLogProgress(Camera, "%s", mLocalDescription.c_str());

        // Extract any candidates embedded in the SDP description
        std::vector<rtc::Candidate> candidates = desc.candidates();
        ChipLogProgress(Camera, "Extracted %lu candidates from SDP description", candidates.size());

        for (const auto & candidate : candidates)
        {
            ICECandidateInfo candidateInfo;
            candidateInfo.candidate  = std::string(candidate);
            candidateInfo.mid        = candidate.mid();
            candidateInfo.mlineIndex = -1; // libdatachannel doesn't provide mlineIndex

            ChipLogProgress(Camera, "[From SDP] Candidate: %s, mid: %s", candidateInfo.candidate.c_str(),
                            candidateInfo.mid.c_str());

            mLocalCandidates.push_back(candidateInfo);
        }
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        ICECandidateInfo candidateInfo;
        candidateInfo.candidate = std::string(candidate);
        candidateInfo.mid       = candidate.mid();

        // Note: libdatachannel doesn't directly provide mlineIndex, so we use -1 to indicate it is not present.
        candidateInfo.mlineIndex = -1;

        ChipLogProgress(Camera, "Local Candidate:");
        ChipLogProgress(Camera, "%s", candidateInfo.candidate.c_str());
        ChipLogProgress(Camera, "  mid: %s, mlineIndex: %d", candidateInfo.mid.c_str(), candidateInfo.mlineIndex);

        mLocalCandidates.push_back(candidateInfo);
    });

    mPeerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        ChipLogProgress(Camera, "[PeerConnection State: %s]", GetPeerConnectionStateStr(state));

        // Check if the session is now established (connected)
        if (state == rtc::PeerConnection::State::Connected)
        {
            // Call the callback to notify DeviceManager
            if (mSessionEstablishedCallback && mCurrentVideoStreamId != 0)
            {
                mSessionEstablishedCallback(mCurrentVideoStreamId);
            }
        }
        else if (state == rtc::PeerConnection::State::Failed || state == rtc::PeerConnection::State::Closed)
        {
            // Limit the clearup to Failed and Closed only to avoid prematurely ending sessions.
            Disconnect();
        }
    });

    mPeerConnection->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
        ChipLogProgress(Camera, "[PeerConnection Gathering State: %s]", GetGatheringStateStr(state));
    });

    // Create UDP socket for RTP forwarding
    mRTPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mRTPSocket == -1)
    {
        ChipLogError(Camera, "Failed to create RTP Video socket: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    mAudioRTPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mAudioRTPSocket == -1)
    {
        ChipLogError(Camera, "Failed to create RTP Audio socket: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    sockaddr_in addr     = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(kStreamGstDestIp);
    addr.sin_port        = htons(kVideoStreamGstDestPort);

    rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
    media.addH264Codec(kVideoH264PayloadType);
    media.setBitrate(kVideoBitRate);
    mTrack = mPeerConnection->addTrack(media);

    auto session = std::make_shared<rtc::RtcpReceivingSession>();
    mTrack->setMediaHandler(session);

    mTrack->onMessage(
        [this, addr](rtc::binary message) {
            // This is an RTP packet
            sendto(mRTPSocket, reinterpret_cast<const char *>(message.data()), size_t(message.size()), 0,
                   reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr));
        },
        nullptr);

    // For Audio
    sockaddr_in audioAddr     = {};
    audioAddr.sin_family      = AF_INET;
    audioAddr.sin_addr.s_addr = inet_addr(kStreamGstDestIp);
    audioAddr.sin_port        = htons(kAudioStreamGstDestPort);

    rtc::Description::Audio audioMedia("audio", rtc::Description::Direction::RecvOnly);
    audioMedia.addOpusCodec(kOpusPayloadType);
    audioMedia.setBitrate(kAudioBitRate);
    mAudioTrack = mPeerConnection->addTrack(audioMedia);

    auto audioSession = std::make_shared<rtc::RtcpReceivingSession>();
    mAudioTrack->setMediaHandler(audioSession);

    mAudioTrack->onMessage(
        [this, audioAddr](rtc::binary message) {
            // This is an RTP Audio packet
            sendto(mAudioRTPSocket, reinterpret_cast<const char *>(message.data()), static_cast<size_t>(message.size()), 0,
                   reinterpret_cast<const struct sockaddr *>(&audioAddr), sizeof(audioAddr));
        },
        nullptr);

    ChipLogProgress(Camera, "Generate and set the SDP");
    mPeerConnection->setLocalDescription();

    return CHIP_NO_ERROR;
}

CHIP_ERROR WebRTCManager::ProvideOffer(DataModel::Nullable<uint16_t> sessionId, StreamUsageEnum streamUsage,
                                       Optional<app::DataModel::Nullable<uint16_t>> videoStreamId,
                                       Optional<app::DataModel::Nullable<uint16_t>> audioStreamId)
{
    ChipLogProgress(Camera, "Sending ProvideOffer command to the peer device");

    if (mLocalDescription.empty())
    {
        ChipLogError(Camera, "No local SDP to send");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // At least one of Video Stream ID and Audio Stream ID has to be present
    if (!videoStreamId.HasValue() && !audioStreamId.HasValue())
    {
        ChipLogError(Zcl, "One of VideoStreamID or AudioStreamID must be present");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store the stream ID for the callback
    if (videoStreamId.HasValue() && !videoStreamId.Value().IsNull())
    {
        mCurrentVideoStreamId = videoStreamId.Value().Value();
        ChipLogProgress(Camera, "Tracking stream ID %u for WebRTC session", mCurrentVideoStreamId);
    }

    CHIP_ERROR err = mWebRTCProviderClient.ProvideOffer(sessionId, mLocalDescription, streamUsage,
                                                        kWebRTCRequesterDynamicEndpointId, videoStreamId, audioStreamId);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send ProvideOffer: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR WebRTCManager::SolicitOffer(StreamUsageEnum streamUsage, Optional<app::DataModel::Nullable<uint16_t>> videoStreamId,
                                       Optional<app::DataModel::Nullable<uint16_t>> audioStreamId)
{
    ChipLogProgress(Camera, "Sending SolicitOffer command to the peer device");

    // At least one of Video Stream ID and Audio Stream ID has to be present
    if (!videoStreamId.HasValue() && !audioStreamId.HasValue())
    {
        ChipLogError(Zcl, "One of VideoStreamID or AudioStreamID must be present");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err =
        mWebRTCProviderClient.SolicitOffer(streamUsage, kWebRTCRequesterDynamicEndpointId, videoStreamId, audioStreamId);

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

    // Make a copy of candidates to send to avoid race condition with onLocalCandidate callback
    // which can asynchronously add new candidates while we're sending
    std::vector<ICECandidateInfo> candidatesToSend = mLocalCandidates;
    size_t candidateCount                          = candidatesToSend.size();

    CHIP_ERROR err = mWebRTCProviderClient.ProvideICECandidates(sessionId, candidatesToSend);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Camera, "Failed to send ICE candidates: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ChipLogProgress(Camera, "Sent %lu ICE candidate(s)", candidateCount);

        // Remove only the candidates that were successfully sent
        // New candidates may have arrived during transmission, so we remove from the front
        if (mLocalCandidates.size() >= candidateCount)
        {
            mLocalCandidates.erase(mLocalCandidates.begin(), mLocalCandidates.begin() + candidateCount);
            if (!mLocalCandidates.empty())
            {
                ChipLogProgress(Camera, "%lu new candidate(s) arrived during transmission, keeping for next batch",
                                mLocalCandidates.size());
            }
        }
        else
        {
            ChipLogProgress(Camera, "Sent %lu ICE candidate(s), clearing list", mLocalCandidates.size());
            mLocalCandidates.clear();
        }
    }

    return err;
}
