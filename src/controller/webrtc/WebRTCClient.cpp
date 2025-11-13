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

// Forward declaration of utils used to extract information from sdp
std::string ExtractMidFromSdp(const std::string & sdp, const std::string & mediaType);
int ExtractDynamicPayloadType(const std::string & sdp, const std::string & type, const std::string & mediaType,
                              const std::string & codec);
const char * GetPeerConnectionStateStr(rtc::PeerConnection::State state);

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

    // Create UDP sockets for RTP forwarding
    mVideoRTPSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (mVideoRTPSocket == -1)
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

    return CHIP_NO_ERROR;
}

void WebRTCClient::addVideoTrack(std::string mid, int payloadType)
{
    if (mVideoTrack != nullptr)
    {
        ChipLogProgress(Camera, "Video track already added");
        return;
    }

    std::string vMid = mid.empty() ? kVideoMid : mid;
    ChipLogProgress(Camera, "Adding Video Track with mid=%s and payload type=%d", vMid.c_str(), payloadType);
    rtc::Description::Video video(vMid, rtc::Description::Direction::RecvOnly);
    video.addH264Codec(payloadType);
    video.setBitrate(kVideoBitRate);
    mVideoTrack = mPeerConnection->addTrack(video);

    auto videoSession      = std::make_shared<rtc::RtcpReceivingSession>();
    auto videoDepacketizer = std::make_shared<rtc::H264RtpDepacketizer>(rtc::NalUnit::Separator::StartSequence);
    videoDepacketizer->addToChain(videoSession);
    mVideoTrack->setMediaHandler(videoDepacketizer);

    sockaddr_in addr     = {};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(kStreamDestIp);
    addr.sin_port        = htons(kVideoStreamDestPort);

    mVideoTrack->onFrame([this, addr](rtc::binary message, rtc::FrameInfo frameInfo) {
        // send H264 frames to sock so that a client can pick it up to dispaly it.
        sendto(this->mVideoRTPSocket, reinterpret_cast<const char *>(message.data()), size_t(message.size()), 0,
               reinterpret_cast<const struct sockaddr *>(&addr), sizeof(addr));
    });
}

void WebRTCClient::addAudioTrack(std::string mid, int payloadType)
{
    if (mAudioTrack != nullptr)
    {
        ChipLogProgress(Camera, "Audio track already added");
        return;
    }

    std::string aMid = mid.empty() ? kAudioMid : mid;
    ChipLogProgress(Camera, "Adding Audio Track with mid=%s and payload type=%d", aMid.c_str(), payloadType);
    rtc::Description::Audio audioMedia(aMid, rtc::Description::Direction::RecvOnly);
    audioMedia.addOpusCodec(payloadType);
    audioMedia.setBitrate(kAudioBitRate);
    mAudioTrack = mPeerConnection->addTrack(audioMedia);

    auto audioSession = std::make_shared<rtc::RtcpReceivingSession>();
    mAudioTrack->setMediaHandler(audioSession);

    sockaddr_in audioAddr     = {};
    audioAddr.sin_family      = AF_INET;
    audioAddr.sin_addr.s_addr = inet_addr(kStreamDestIp);
    audioAddr.sin_port        = htons(kAudioStreamDestPort);

    mAudioTrack->onMessage(
        [this, audioAddr](rtc::binary message) {
            // send audio RTP packets to sock so that a client can pick it up to play it.
            sendto(this->mAudioRTPSocket, reinterpret_cast<const char *>(message.data()), static_cast<size_t>(message.size()), 0,
                   reinterpret_cast<const struct sockaddr *>(&audioAddr), sizeof(audioAddr));
        },
        nullptr);
}

void WebRTCClient::CreateOffer()
{
    if (mPeerConnection == nullptr)
    {
        ChipLogError(NotSpecified, "Peerconnection is null");
        return;
    }

    // Controller is the offerer. Add tracks with the default values
    addVideoTrack();
    addAudioTrack();

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

    if (type == "offer")
    {
        // Controller is the answerer. Extract values from offer SDP and add tracks accordingly
        std::string videoMid = ExtractMidFromSdp(sdp, "video");
        int videoPayloadType = ExtractDynamicPayloadType(sdp, type, "video", "H264");
        videoPayloadType     = videoPayloadType == -1 ? kVideoH264PayloadType : videoPayloadType;
        addVideoTrack(videoMid, videoPayloadType);

        std::string audioMid = ExtractMidFromSdp(sdp, "audio");
        int audioPayloadType = ExtractDynamicPayloadType(sdp, type, "audio", "opus");
        audioPayloadType     = audioPayloadType == -1 ? kOpusPayloadType : audioPayloadType;
        addAudioTrack(audioMid, audioPayloadType);
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
    ChipLogProgress(Camera, "Closing RTP sockets");
    if (mVideoRTPSocket != -1)
    {
        close(mVideoRTPSocket);
        mVideoRTPSocket = -1;
    }

    if (mAudioRTPSocket != -1)
    {
        close(mAudioRTPSocket);
        mAudioRTPSocket = -1;
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
    mVideoTrack.reset();
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

std::string ExtractMidFromSdp(const std::string & sdp, const std::string & mediaType)
{
    if (sdp.empty() || mediaType.empty())
    {
        ChipLogError(Camera, "ExtractMidFromSdp: empty SDP or media type");
        return "";
    }

    const std::string mediaPrefix = "m=" + mediaType;
    const std::string midPrefix   = "a=mid:";

    std::istringstream stream(sdp);
    std::string line;
    bool inTargetBlock = false;

    while (std::getline(stream, line))
    {
        // Trim possible Windows carriage return
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (inTargetBlock)
        {
            if (line.rfind(midPrefix, 0) == 0) // line starts with "a=mid:"
                return line.substr(midPrefix.length());

            if (line.rfind("m=", 0) == 0) // next media block – stop searching
                break;
        }
        else if (line.rfind(mediaPrefix, 0) == 0) // found the desired media block
        {
            inTargetBlock = true;
        }
    }

    // No MID found for the requested media type
    return "";
}

int ExtractDynamicPayloadType(const std::string & sdp, const std::string & type, const std::string & mediaType,
                              const std::string & codec)
{
    rtc::Description desc(sdp, type);
    for (int mid = 0; mid < desc.mediaCount(); mid++)
    {
        auto media = desc.media(mid);
        if (!std::holds_alternative<rtc::Description::Media *>(media))
            continue;

        rtc::Description::Media * mediaDesc = std::get<rtc::Description::Media *>(media);

        if (mediaDesc == nullptr)
        {
            ChipLogError(Camera, "Media Description is null at index=%d", mid);
            continue;
        }
        if (mediaDesc->type() != mediaType)
        {
            continue;
        }

        for (int pt : mediaDesc->payloadTypes())
        {
            auto * map = mediaDesc->rtpMap(pt);
            if (map == nullptr)
            {
                ChipLogError(Camera, "No RTP map found for payload type: %d", pt);
                continue;
            }
            if (map->format == codec)
            {
                ChipLogProgress(Camera, "%s codec has payload type: %d", codec.c_str(), pt);
                return pt;
            }
        }
    }
    ChipLogError(Camera, "Payload type for codec %s not found", codec.c_str());
    return -1;
}

} // namespace webrtc
} // namespace chip
