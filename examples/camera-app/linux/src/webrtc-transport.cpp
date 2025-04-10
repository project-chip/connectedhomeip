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

#include <lib/support/logging/CHIPLogging.h>
#include <transport/webrtc-transport.h>

WebrtcTransport::WebrtcTransport(uint16_t sessionID, uint64_t nodeID, std::shared_ptr<rtc::PeerConnection> mPeerConnection)
{
    ChipLogProgress(Camera, "WebrtcTransport created for sessionID: %u", sessionID);
    sessionID            = sessionID;
    nodeID               = nodeID;
    peerConnection       = mPeerConnection;
    timestamp            = 0;
    const rtc::SSRC ssrc = 42;
    media                = rtc::Description::Video("video", rtc::Description::Direction::SendOnly);
    media.addH264Codec(96);
    media.setBitrate(3000);
    media.addSSRC(ssrc, "video-send");
    // TODO: role actpass in remote answer description, need to set as Active/Passive based on local offer description
    // track           = peerConnection->addTrack(media);
    auto rtpConfig  = std::make_shared<rtc::RtpPacketizationConfig>(ssrc, "video-send", 96, rtc::H264RtpPacketizer::ClockRate);
    auto packetizer = std::make_shared<rtc::H264RtpPacketizer>(rtpConfig);
    // track->setMediaHandler(packetizer);
}

WebrtcTransport::~WebrtcTransport()
{
    ChipLogProgress(Camera, "WebrtcTransport destroyed for sessionID: %u", sessionID);
}

void WebrtcTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    // ChipLogProgress(Camera, "Sending video data of size: %u bytes", (int)size);
    auto * b           = reinterpret_cast<const std::byte *>(data);
    rtc::binary sample = {};
    sample.assign(b, b + size);
    int sampleDuration_us = 1000 * 1000 / 30;
    timestamp += sampleDuration_us;
    rtc::FrameInfo frameInfo(timestamp);
    frameInfo.payloadType = 96;
    // track->sendFrame(sample, frameInfo);
}

// Dummy implementation of SendAudio method
void WebrtcTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    // Placeholder for actual WebRTC implementation to send audio data
}

// Dummy implementation of SendAudioVideo method
void WebrtcTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    // Placeholder for actual WebRTC implementation to send synchronized audio/video data
}

// Dummy implementation of CanSendVideo method
bool WebrtcTransport::CanSendVideo()
{
    return mCanSendVideo;
}

// Dummy implementation of CanSendAudio method
bool WebrtcTransport::CanSendAudio()
{
    return mCanSendAudio;
}
