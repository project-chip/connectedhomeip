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
#include <webrtc-transport.h>

// Default payload types for codecs used in this example app
const int kH264CodecPayloadType = 96;
const int kOpusCodecPayloadType = 111;
// Random ssrc values for video and audio streams, each media track should have a unique SSRC value to send/receive RTP payload
const int kVideoSSRC = 42;
const int kAudioSSRC = 43;

WebrtcTransport::WebrtcTransport(uint16_t sessionID, uint64_t nodeID, std::shared_ptr<rtc::PeerConnection> peerConnection)
{
    ChipLogProgress(Camera, "WebrtcTransport created for sessionID: %u", sessionID);
    mSessionID            = sessionID;
    mNodeID               = nodeID;
    mPeerConnection       = peerConnection;
    mVideoSampleTimestamp = 0;
    mAudioSampleTimestamp = 0;

    // TODO: Get the codec details from AV stream management
    // Create video track
    auto videoDesc = rtc::Description::Video();
    videoDesc.addSSRC(kVideoSSRC, "video-send");
    videoDesc.addH264Codec(kH264CodecPayloadType);
    mVideoTrack = mPeerConnection->addTrack(videoDesc);

    // Create audio track
    auto audioDesc = rtc::Description::Audio();
    audioDesc.addSSRC(kAudioSSRC, "audio-send");
    audioDesc.addOpusCodec(kOpusCodecPayloadType);
    mAudioTrack = mPeerConnection->addTrack(audioDesc);
}

WebrtcTransport::~WebrtcTransport()
{
    ChipLogProgress(Camera, "WebrtcTransport destroyed for sessionID: [%u]", mSessionID);
}

void WebrtcTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    // ChipLogProgress(Camera, "Sending video data of size: %u bytes", (int) size);
    auto * b           = reinterpret_cast<const std::byte *>(data);
    rtc::binary sample = {};
    sample.assign(b, b + size);
    // TODO: Get the video stream parameters, payload type from AV stream management
    // Computing the timestamp based on frame rate 30fps and timestamp unit is microseconds (us)
    int sampleDurationUs = 1000 * 1000 / 30;
    rtc::FrameInfo frameInfo(mVideoSampleTimestamp);
    frameInfo.payloadType = kH264CodecPayloadType;
    mVideoSampleTimestamp += sampleDurationUs;
    mVideoTrack->sendFrame(sample, frameInfo);
}

// Implementation of SendAudio method
void WebrtcTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    auto * b           = reinterpret_cast<const std::byte *>(data);
    rtc::binary sample = {};
    sample.assign(b, b + size);
    // TODO: Get the audio stream parameters, payload type from AV stream management
    // Default sample rate 48000 Hz, frame duration is 20ms, computing number of samples per frame
    int samplesPerFrame = (48000 * 20) / 1000;
    rtc::FrameInfo frameInfo(mAudioSampleTimestamp);
    frameInfo.payloadType = kOpusCodecPayloadType;
    mAudioSampleTimestamp += samplesPerFrame;
    mAudioTrack->sendFrame(sample, frameInfo);
}

// Implementation of SendAudioVideo method
void WebrtcTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    // Placeholder for actual WebRTC implementation to send synchronized audio/video data
}

// Implementation of CanSendVideo method
bool WebrtcTransport::CanSendVideo()
{
    return mCanSendVideo;
}

// Implementation of CanSendAudio method
bool WebrtcTransport::CanSendAudio()
{
    return mCanSendAudio;
}
