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

WebRTCClient::WebRTCClient()
{
    mPeerConnection          = nullptr;
    mTransportProviderClient = std::make_unique<WebRTCTransportProviderClient>();
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
        mLocalDescription = std::string(desc);
        if (mLocalDescriptionCallback)
            mLocalDescriptionCallback(mLocalDescription.c_str(), desc.typeString());
    });

    mPeerConnection->onLocalCandidate([this](rtc::Candidate candidate) {
        std::string candidateStr = std::string(candidate);
        mLocalCandidates.push_back(candidateStr);
        if (mIceCandidateCallback)
            mIceCandidateCallback(candidate.candidate(), candidate.mid());
    });

    mPeerConnection->onStateChange([this](rtc::PeerConnection::State state) {
        if (mStateChangeCallback)
            mStateChangeCallback(static_cast<int>(state));
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

            auto desc = mPeerConnection->localDescription();
            // Update local description to include ice candidates since gathering is complete
            if (desc.has_value())
                mLocalDescription = desc.value();
        }
    });

    // Create UDP socket for RTP forwarding
    mRTPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mRTPSocket == -1)
    {
        ChipLogError(Camera, "Failed to create RTP socket: %s", strerror(errno));
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

    // Clear local states
    mLocalDescription.clear();
    mLocalCandidates.clear();
}

const char * WebRTCClient::GetLocalDescription()
{
    return mLocalDescription.c_str();
}

int WebRTCClient::GetPeerConnectionState()
{
    if (mPeerConnection == nullptr)
    {
        return -1; // Invalid state
    }

    return static_cast<int>(mPeerConnection->state());
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

void WebRTCClient::OnStateChange(std::function<void(int)> callback)
{
    mStateChangeCallback = callback;
}

void WebRTCClient::WebRTCProviderClientInit(uint32_t nodeId, uint8_t fabricIndex, uint16_t endpoint)
{
    mTransportProviderClient->Init(nodeId, fabricIndex, endpoint);
}

PyChipError WebRTCClient::SendCommand(void * appContext, uint16_t endpointId, uint32_t clusterId, uint32_t commandId,
                                      const uint8_t * payload, size_t length)
{
    return mTransportProviderClient->SendCommand(appContext, endpointId, clusterId, commandId, payload, length);
}

void WebRTCClient::WebRTCProviderClientInitCallbacks(OnCommandSenderResponseCallback onCommandSenderResponseCallback,
                                                     OnCommandSenderErrorCallback onCommandSenderErrorCallback,
                                                     OnCommandSenderDoneCallback onCommandSenderDoneCallback)
{
    mTransportProviderClient->InitCallbacks(onCommandSenderResponseCallback, onCommandSenderErrorCallback,
                                            onCommandSenderDoneCallback);
}

} // namespace webrtc
} // namespace chip
