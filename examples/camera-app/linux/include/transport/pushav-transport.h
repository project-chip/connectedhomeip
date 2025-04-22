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

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include "transport.h"
#include "pushav-clip-recorder.h"
#include <app-common/zap-generated/cluster-enums.h>

// Derived class for PushAV transport
class PushAVTransport : public camera::Transport
{
public:
    PushAVTransport(uint16_t sessionID, uint64_t nodeID);
    ~PushAVTransport() override;
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

    // Dummy implementation to indicate whether the transport is streaming or not
    bool IsStreaming();

    // Dummy implementation to indicate the status of the transport stream
    bool TransportStatus;

    // Enum indicating the type of trigger used to start the transport
    chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum mTransportTriggerType;
    void initializeRecorder();
    AVPacket* createPacket(const uint8_t* data, int size,  uint16_t videoStreamID, uint16_t audioStreamID);
    void readFromFile(char* filename, uint8_t** videoBuffer, size_t *videoBufferBytes);
    bool isH264Iframe(const uint8_t *data_ptr, unsigned int data_len);
    std::mutex mtx;
    bool isRecorderInitialized = false;
    int64_t v_pts=3000;
    int64_t v_dts=3000;

    int64_t a_pts=960;
    int64_t a_dts=960;

    int vid = 1;
    std::shared_ptr<PushAVClipRecorder> recorder = nullptr;

private:
    // Dummy implementation to indicate if video can be sent
    bool mCanSendVideo = false;

    // Dummy implementation to indicate if audio can be sent
    bool mCanSendAudio = false;

};
