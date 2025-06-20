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

#pragma once

#include "transport.h"
#include <rtc/rtc.hpp>

// Derived class for WebRTC transport
class WebrtcTransport : public Transport
{
public:
    WebrtcTransport(uint16_t sessionID, uint64_t nodeID, std::shared_ptr<rtc::PeerConnection> mPeerConnection);

    ~WebrtcTransport();

    // Send video data for a given stream ID
    void SendVideo(const char * data, size_t size, uint16_t videoStreamID) override;

    // Send audio data for a given stream ID
    void SendAudio(const char * data, size_t size, uint16_t audioStreamID) override;

    // Send synchronized audio/video data for given audio and video stream IDs
    void SendAudioVideo(const char * data, size_t size, uint16_t videoStreamID, uint16_t audioStreamID) override;

    // Indicates that the transport is ready to send video data
    bool CanSendVideo() override;

    // Indicates that the transport is ready to send audio data
    bool CanSendAudio() override;

    // Set video track for the transport
    void SetVideoTrack(std::shared_ptr<rtc::Track> videoTrack);

    // Set audio track for the transport
    void SetAudioTrack(std::shared_ptr<rtc::Track> audioTrack);

private:
    uint16_t mSessionID;
    uint64_t mNodeID;
    uint32_t mAudioSampleTimestamp;
    uint32_t mVideoSampleTimestamp;
    std::shared_ptr<rtc::PeerConnection> mPeerConnection;
    std::shared_ptr<rtc::Track> mVideoTrack;
    std::shared_ptr<rtc::Track> mAudioTrack;
};
