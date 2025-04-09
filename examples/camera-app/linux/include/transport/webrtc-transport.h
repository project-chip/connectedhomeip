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

// Derived class for WebRTC transport
class WebrtcTransport : public Transport
{
public:
    WebrtcTransport(uint16_t sessionID, uint64_t nodeID);

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

private:
    // Dummy implementation to indicate if video can be sent
    bool mCanSendVideo = true;

    // Dummy implementation to indicate if audio can be sent
    bool mCanSendAudio = true;

    uint16_t msessionID;
    uint64_t nodeID;

    void RegisterToMediaController();
};
