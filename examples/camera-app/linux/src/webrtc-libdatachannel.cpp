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
constexpr int kSSRC                 = 42;
constexpr int kMaxFragmentSize      = 1188; // 1200 (max packet size) - 12 (RTP header size)
constexpr int kAudioBitRate         = 64000;
constexpr int kOpusPayloadType      = 111;
constexpr int kAudioSSRC            = 43;

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
    // Initialize libdatachannel's RTP packetizer for H.264
    void InitH264Packetizer()
    {
        // 90 kHz clock for H.264
        mRtpCfg      = std::make_shared<rtc::RtpPacketizationConfig>(kSSRC, "videosrc", kVideoH264PayloadType,
                                                                rtc::H264RtpPacketizer::ClockRate);
        mRtpCfg->mid = mTrack->description().mid();

        // Setting MTU size to 1200 as default size used in the libdatachannel is 1400
        // Pick separator:
        // - StartSequence : for Annex-B (00 00 01 / 00 00 00 01)
        // - Length : for 4-byte length-prefixed NAL units
        mPacketizer = std::make_shared<rtc::H264RtpPacketizer>(rtc::NalUnit::Separator::StartSequence, mRtpCfg, kMaxFragmentSize);

        // RTCP helpers (recommended)
        mSr   = std::make_shared<rtc::RtcpSrReporter>(mRtpCfg);
        mNack = std::make_shared<rtc::RtcpNackResponder>();
        mPacketizer->addToChain(mSr);
        mPacketizer->addToChain(mNack);

        // Attach handler chain to the sending track
        mTrack->setMediaHandler(mPacketizer);
    }

    void InitOpusPacketizer()
    {
        mRtpCfgAudio = std::make_shared<rtc::RtpPacketizationConfig>(kAudioSSRC, "mic", kOpusPayloadType,
                                                                     rtc::OpusRtpPacketizer::DefaultClockRate);

        mRtpCfgAudio->mid = mTrack->description().mid();

        mOpusPacketizer = std::make_shared<rtc::OpusRtpPacketizer>(mRtpCfgAudio);
        mOpusSr         = std::make_shared<rtc::RtcpSrReporter>(mRtpCfgAudio);
        mOpusNack       = std::make_shared<rtc::RtcpNackResponder>();
        mOpusPacketizer->addToChain(mOpusSr);
        mOpusPacketizer->addToChain(mOpusNack);

        mTrack->setMediaHandler(mOpusPacketizer);
    }

    void SendData(const char * data, size_t size) override
    {
        if (mTrack && mTrack->isOpen())
        {
            const std::string kind = mTrack->description().type();
            if (kind == "video" && !mVideoInitDone)
            {
                InitH264Packetizer();
                mVideoInitDone = true;
            }
            else if (kind == "audio" && !mAudioInitDone)
            {
                InitOpusPacketizer();
                mAudioInitDone = true;
            }
            // Feed RAW H.264 access unit. Packetizer does NAL split, FU-A/STAP-A, RTP headers, marker bit, SR/NACK.
            rtc::binary frame(size);
            std::memcpy(frame.data(), data, size);
            mTrack->send(std::move(frame));
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
    // Lazy-init state
    bool mAudioInitDone = false;
    bool mVideoInitDone = false;

    std::shared_ptr<rtc::Track> mTrack;

    // For Video
    std::shared_ptr<rtc::RtpPacketizationConfig> mRtpCfg;
    std::shared_ptr<rtc::H264RtpPacketizer> mPacketizer;
    std::shared_ptr<rtc::RtcpSrReporter> mSr;
    std::shared_ptr<rtc::RtcpNackResponder> mNack;

    // For audio
    std::shared_ptr<rtc::RtpPacketizationConfig> mRtpCfgAudio;
    std::shared_ptr<rtc::OpusRtpPacketizer> mOpusPacketizer;
    std::shared_ptr<rtc::RtcpSrReporter> mOpusSr;
    std::shared_ptr<rtc::RtcpNackResponder> mOpusNack;
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
            rtc::Description::Video vMedia("video", rtc::Description::Direction::SendOnly);
            vMedia.addH264Codec(kVideoH264PayloadType);
            vMedia.setBitrate(kVideoBitRate);
            vMedia.addSSRC(kSSRC, "video-stream", "stream1", "video-stream");
            auto track = mPeerConnection->addTrack(vMedia);
            return std::make_shared<LibDataChannelTrack>(track);
        }

        if (mediaType == MediaType::Audio)
        {
            rtc::Description::Audio aMedia("audio", rtc::Description::Direction::SendOnly);
            aMedia.addOpusCodec(kOpusPayloadType);
            aMedia.setBitrate(kAudioBitRate);
            aMedia.addSSRC(kAudioSSRC, "audio-stream", "stream1", "audio-stream");
            auto track = mPeerConnection->addTrack(aMedia);
            return std::make_shared<LibDataChannelTrack>(track);
        }
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
