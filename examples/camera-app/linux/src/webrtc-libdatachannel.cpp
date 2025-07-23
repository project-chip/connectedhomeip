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

#include "webrtc-abstract.h"
#include <lib/support/logging/CHIPLogging.h>
#include <rtc/rtc.hpp>

namespace {

// Constants
constexpr int kVideoH264PayloadType = 96;
constexpr int kVideoBitRate         = 3000;

rtc::Description::Type SDPTypeToRtcType(SDPType type)
{
    switch (type)
    {
    case SDPType::Offer:
        return rtc::Description::Type::Offer;
    case SDPType::Answer:
        return rtc::Description::Type::Answer;
    case SDPType::Pranswer:
        return rtc::Description::Type::Pranswer;
    case SDPType::Rollback:
        return rtc::Description::Type::Rollback;
    default:
        return rtc::Description::Type::Offer;
    }
}

SDPType RtcTypeToSDPType(rtc::Description::Type type)
{
    switch (type)
    {
    case rtc::Description::Type::Offer:
        return SDPType::Offer;
    case rtc::Description::Type::Answer:
        return SDPType::Answer;
    case rtc::Description::Type::Pranswer:
        return SDPType::Pranswer;
    case rtc::Description::Type::Rollback:
        return SDPType::Rollback;
    default:
        return SDPType::Offer;
    }
}

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

class LibDataChannelTrack : public WebRTCTrack
{
public:
    LibDataChannelTrack(std::shared_ptr<rtc::Track> track) : mTrack(track) {}

    void SendData(const char * data, size_t size) override
    {
        if (mTrack && mTrack->isOpen())
        {
            mTrack->send(reinterpret_cast<const std::byte *>(data), size);
        }
        else
        {
            ChipLogError(Camera, "Track is closed");
        }
    }

    bool IsReady() override { return mTrack != nullptr && mTrack->isOpen(); }

    std::string GetType() override
    {
        if (mTrack)
        {
            auto description = mTrack->description();
            return std::string(description.type());
        }
        return "";
    }

private:
    std::shared_ptr<rtc::Track> mTrack;
};

class LibDataChannelPeerConnection : public WebRTCPeerConnection
{
public:
    LibDataChannelPeerConnection()
    {
        rtc::Configuration config;
        // config.iceServers.emplace_back("stun.l.google.com:19302");
        mPeerConnection = std::make_shared<rtc::PeerConnection>(config);
    }

    void SetCallbacks(OnLocalDescriptionCallback onLocalDescription, OnICECandidateCallback onICECandidate,
                      OnConnectionStateCallback onConnectionState, OnTrackCallback onTrack) override
    {
        mPeerConnection->onLocalDescription(
            [onLocalDescription](rtc::Description desc) { onLocalDescription(std::string(desc), RtcTypeToSDPType(desc.type())); });

        mPeerConnection->onLocalCandidate([onICECandidate](rtc::Candidate candidate) { onICECandidate(std::string(candidate)); });

        mPeerConnection->onStateChange([onConnectionState](rtc::PeerConnection::State state) {
            ChipLogProgress(Camera, "[PeerConnection State: %s]", GetPeerConnectionStateStr(state));
            if (state == rtc::PeerConnection::State::Connected)
            {
                onConnectionState(true);
            }
        });

        mPeerConnection->onGatheringStateChange([](rtc::PeerConnection::GatheringState state) {
            ChipLogProgress(Camera, "[PeerConnection Gathering State: %s]", GetGatheringStateStr(state));
        });

        mPeerConnection->onTrack(
            [onTrack](std::shared_ptr<rtc::Track> track) { onTrack(std::make_shared<LibDataChannelTrack>(track)); });
    }

    void Close() override
    {
        if (mPeerConnection)
        {
            mPeerConnection->close();
        }
    }

    void CreateOffer() override { mPeerConnection->setLocalDescription(); }

    void CreateAnswer() override { mPeerConnection->createAnswer(); }

    void SetRemoteDescription(const std::string & sdp, SDPType type) override
    {
        rtc::Description::Type rtcType = SDPTypeToRtcType(type);
        mPeerConnection->setRemoteDescription(rtc::Description(sdp, rtcType));
    }

    void AddRemoteCandidate(const std::string & candidate, const std::string & mid) override
    {
        if (mid.empty())
        {
            mPeerConnection->addRemoteCandidate(rtc::Candidate(candidate));
        }
        else
        {
            mPeerConnection->addRemoteCandidate(rtc::Candidate(candidate, mid));
        }
    }

    std::shared_ptr<WebRTCTrack> AddTrack(MediaType mediaType) override
    {
        if (mediaType == MediaType::Video)
        {
            rtc::Description::Video media("video", rtc::Description::Direction::SendOnly);
            media.addH264Codec(kVideoH264PayloadType);
            media.setBitrate(kVideoBitRate);
            auto track = mPeerConnection->addTrack(media);
            return std::make_shared<LibDataChannelTrack>(track);
        }

        // TODO: Add audio track support
        ChipLogProgress(Camera, "Audio track support is not yet implemented");
        return nullptr;
    }

private:
    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
};

} // namespace

std::shared_ptr<WebRTCPeerConnection> CreateWebRTCPeerConnection()
{
    return std::make_shared<LibDataChannelPeerConnection>();
}
