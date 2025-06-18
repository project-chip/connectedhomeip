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

WebrtcTransport::WebrtcTransport(uint16_t sessionID, uint64_t nodeID, std::shared_ptr<rtc::PeerConnection> peerConnection)
{
    ChipLogProgress(Camera, "WebrtcTransport created for sessionID: %u", sessionID);
    mSessionID            = sessionID;
    mNodeID               = nodeID;
    mPeerConnection       = peerConnection;
    mVideoSampleTimestamp = 0;
    mAudioSampleTimestamp = 0;
}

WebrtcTransport::~WebrtcTransport()
{
    ChipLogProgress(Camera, "WebrtcTransport destroyed for sessionID: [%u]", mSessionID);
}

void WebrtcTransport::SendVideo(const char * data, size_t size, uint16_t videoStreamID)
{
    mVideoTrack->send(reinterpret_cast<const std::byte *>(data), size);
}

// Implementation of SendAudio method
void WebrtcTransport::SendAudio(const char * data, size_t size, uint16_t audioStreamID)
{
    mAudioTrack->send(reinterpret_cast<const std::byte *>(data), size);
}

// Implementation of SendAudioVideo method
void WebrtcTransport::SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID)
{
    // Placeholder for actual WebRTC implementation to send synchronized audio/video data
}

// Implementation of CanSendVideo method
bool WebrtcTransport::CanSendVideo()
{
    return mVideoTrack != nullptr && mPeerConnection != nullptr;
}

// Implementation of CanSendAudio method
bool WebrtcTransport::CanSendAudio()
{
    return mAudioTrack != nullptr && mPeerConnection != nullptr;
}

// Implementation of SetVideoTrack method
void WebrtcTransport::SetVideoTrack(std::shared_ptr<rtc::Track> videoTrack)
{
    ChipLogProgress(Camera, "Setting video track for sessionID: %u", mSessionID);
    mVideoTrack = videoTrack;
}

// Implementation of SetAudioTrack method
void WebrtcTransport::SetAudioTrack(std::shared_ptr<rtc::Track> audioTrack)
{
    ChipLogProgress(Camera, "Setting audio track for sessionID: %u", mSessionID);
    mAudioTrack = audioTrack;
}
