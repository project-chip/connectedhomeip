/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "WebRTCClient.h"

#include <arpa/inet.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace webrtc {

constexpr int kVideoH264PayloadType = 96; // 96 is just the first value in the dynamic RTP payload‑type range (96‑127).
constexpr int kVideoBitRate         = 3000;

constexpr const char * kStreamDestIp    = "127.0.0.1";
constexpr uint16_t kVideoStreamDestPort = 5000;

// Constants for Audio
constexpr int kAudioBitRate             = 64000;
constexpr int kOpusPayloadType          = 111;
constexpr uint16_t kAudioStreamDestPort = 5001;

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
    return "Invalid";
};

WebRTCClient::WebRTCClient()
{
    mPeerConnection = nullptr;
}

WebRTCClient::~WebRTCClient()
{
    if (mPeerConnection == nullptr)
        return;
    Disconnect();
    delete mPeerConnection;
}

CHIP_ERROR WebRTCClient::CreatePeerConnection(const std::string & stunUrl)
{
    rtc::InitLogger(rtc::LogLevel::None);

    if (mPeerConnection != nullptr)
    {
        ChipLogError(NotSpecified, "PeerConnection exists already!");
        return CHIP_ERROR_ALREADY_INITIALIZED;
    }
    rtc::Configuration config;
    if (!stunUrl.empty())
    {
        config.iceServers.emplace_back(stunUrl);
    }
    else
    {
        ChipLogError(NotSpecified, "No STUN server URL provided");
    }
    mPeerConnection = new rtc::PeerConnection(config);
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to create PeerConnection");
        return CHIP_ERROR_NO_MEMORY;
    }

    mPeerConnection->onLocalDescription([this](rtc::Description desc) {
        std::string localDescription = std::string(desc);
        if (mLocalDescriptionCallback)
            mLocalDescriptionCallback(localDescription.c_str(), desc.typeString());
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        std::string candidateStr = std::string(candidate);
        mLocalCandidates.push_back(candidateStr);
        if (mIceCandidateCallback)
            mIceCandidateCallback(candidate.candidate(), candidate.mid());
    });

    mPeerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        if (mStateChangeCallback)
            mStateChangeCallback(GetPeerConnectionStateStr(state));
        if (state == rtc::PeerConnection::State::Disconnected || state == rtc::PeerConnection::State::Failed ||
            state == rtc::PeerConnection::State::Closed)
        {
            CloseRTPSocket();
        }
    });

    mPeerConnection->onGatheringStateChange([this](rtc::PeerConnection::GatheringState state) {
        if (state == rtc::PeerConnection::GatheringState::Complete)
        {
            if (mGatheringCompleteCallback)
                mGatheringCompleteCallback();
        }
    });

    // Create UDP socket for RTP forwarding
    mRTPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mRTPSocket == -1)
    {
        ChipLogError(Camera, "Failed to create RTP socket: %s", strerror(errno));
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
    addr.sin_addr.s_addr = inet_addr(kStreamDestIp);
    addr.sin_port        = htons(kVideoStreamDestPort);

    rtc::Description::Video media("video", rtc::Description::Direction::RecvOnly);
    media.addH264Codec(kVideoH264PayloadType);
    media.setBitrate(kVideoBitRate);
    mTrack = mPeerConnection->addTrack(media);

    auto depacketizer = std::make_shared<rtc::H264RtpDepacketizer>();
    mTrack->setMediaHandler(depacketizer);

    mTrack->onFrame([this, addr](rtc::binary message, rtc::FrameInfo frameInfo) {
        // send H264 frames to sock so that a client can pick it up to dispaly it.
        sendto(mRTPSocket, reinterpret_cast<const char *>(message.data()), size_t(message.size()), 0,
               reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr));
    });

    // For Audio
    sockaddr_in audioAddr     = {};
    audioAddr.sin_family      = AF_INET;
    audioAddr.sin_addr.s_addr = inet_addr(kStreamDestIp);
    audioAddr.sin_port        = htons(kAudioStreamDestPort);

    rtc::Description::Audio audioMedia("audio", rtc::Description::Direction::RecvOnly);
    audioMedia.addOpusCodec(kOpusPayloadType);
    audioMedia.setBitrate(kAudioBitRate);
    mAudioTrack = mPeerConnection->addTrack(audioMedia);

    auto audioSession = std::make_shared<rtc::RtcpReceivingSession>();
    mAudioTrack->setMediaHandler(audioSession);

    mAudioTrack->onMessage(
        [this, audioAddr](rtc::binary message) {
            // send audio RTP packets to sock so that a client can pick it up to play it.
            sendto(mAudioRTPSocket, reinterpret_cast<const char *>(message.data()), static_cast<size_t>(message.size()), 0,
                   reinterpret_cast<const struct sockaddr *>(&audioAddr), sizeof(audioAddr));
        },
        nullptr);

    return CHIP_NO_ERROR;
}

void WebRTCClient::CreateOffer()
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->setLocalDescription();
}

void WebRTCClient::CreateAnswer()
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->setLocalDescription();
}

void WebRTCClient::SetRemoteDescription(const std::string & sdp, const std::string & type)
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->setRemoteDescription(rtc::Description(sdp, type));
}

void WebRTCClient::AddIceCandidate(const std::string & candidate, const std::string & mid)
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    mPeerConnection->addRemoteCandidate(rtc::Candidate(candidate, mid));
}

void WebRTCClient::CloseRTPSocket()
{
    if (mRTPSocket != -1)
    {
        ChipLogProgress(Camera, "Closing RTP socket");
        close(mRTPSocket);
        mRTPSocket = -1;
    }
}

void WebRTCClient::Disconnect()
{
    ChipLogProgress(Camera, "Disconnecting WebRTC session");

    // Close the peer connection
    if (mPeerConnection)
    {
        mPeerConnection->close();
    }

    // Close the RTP socket
    CloseRTPSocket();

    // Reset track
    mTrack.reset();
    mAudioTrack.reset();

    // Clear local states
    mLocalDescription.clear();
    mLocalCandidates.clear();
}

const char * WebRTCClient::GetLocalSessionDescriptionInternal()
{
    if (mPeerConnection == nullptr)
    {
        return "";
    }

    auto desc = mPeerConnection->localDescription();
    if (desc.has_value())
    {
        mLocalDescription = desc.value();
    }

    return mLocalDescription.c_str();
}

const char * WebRTCClient::GetPeerConnectionState()
{
    if (mPeerConnection == nullptr)
    {
        return "Invalid";
    }

    return GetPeerConnectionStateStr(mPeerConnection->state());
}

void WebRTCClient::OnLocalDescription(std::function<void(const std::string &, const std::string &)> callback)
{
    mLocalDescriptionCallback = callback;
}

void WebRTCClient::OnIceCandidate(std::function<void(const std::string &, const std::string &)> callback)
{
    mIceCandidateCallback = callback;
}

void WebRTCClient::OnGatheringComplete(std::function<void()> callback)
{
    mGatheringCompleteCallback = callback;
}

void WebRTCClient::OnStateChange(std::function<void(const char *)> callback)
{
    mStateChangeCallback = callback;
}

} // namespace webrtc
} // namespace chip
