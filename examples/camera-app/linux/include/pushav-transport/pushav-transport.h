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

#include "camera-device-interface.h"
#include "pushav-clip-recorder.h"
#include "transport.h"
#include "uploader/pushav-uploader.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/push-av-stream-transport-server/constants.h>
#include <memory>
#include <protocols/interaction_model/StatusCode.h>
#include <thread>
#include <vector>

class PushAVTransport : public Transport
{
public:
    PushAVTransport(const chip::app::Clusters::PushAvStreamTransport::TransportOptionsStruct & transportOptions,
                    const uint16_t connectionID, AudioStreamStruct mAudioStreamParams, VideoStreamStruct mVideoStreamParams);
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

    bool GetBusyStatus();

    uint16_t GetPreRollLength();

    // Set Transport status
    void SetTransportStatus(chip::app::Clusters::PushAvStreamTransport::TransportStatusEnum status);

    void TriggerTransport(chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum activationReason);

    // Get Transport status
    bool GetTransportStatus()
    {
        return (mTransportStatus == chip::app::Clusters::PushAvStreamTransport::TransportStatusEnum::kInactive);
    } // 0:Active 1:Inactive

    void ConfigureRecorderSettings(const chip::app::Clusters::PushAvStreamTransport::TransportOptionsStruct & transportOptions,
                                   AudioStreamStruct mAudioStreamParams, VideoStreamStruct mVideoStreamParams);

    void ModifyPushTransport(const chip::app::Clusters::PushAvStreamTransport::TransportOptionsStorage transportOptions);

    bool HandleTriggerDetected();

    void InitializeRecorder();
    bool CanSendPacketsToRecorder();
    void readFromFile(char * filename, uint8_t ** videoBuffer, size_t * videoBufferBytes);
    void SetTLSCertPath(std::string rootCert, std::string devCert, std::string devKey);

private:
    // bool isRecorderInitialized                   = false;
    // bool isUploaderInitialized                   = false;
    bool hasAugmented                            = false;
    bool mStreaming                              = false;
    std::unique_ptr<PushAVClipRecorder> recorder = nullptr;
    std::unique_ptr<PushAVUploader> uploader     = nullptr;
    std::chrono::steady_clock::time_point blindStartTime;
    PushAVClipRecorder::ClipInfoStruct clipInfo;
    PushAVClipRecorder::AudioInfoStruct audioInfo;
    PushAVClipRecorder::VideoInfoStruct videoInfo;
    PushAVUploader::PushAVCertPath mCertPath;
    AudioStreamStruct audioStreamParams;
    VideoStreamStruct videoStreamParams;

    // Dummy implementation to indicate if video can be sent
    bool mCanSendVideo = false;

    // Dummy implementation to indicate if audio can be sent
    bool mCanSendAudio = false;

    // unsigned int mClipId = 0;

    // Enum indicating the type of trigger used to start the transport
    chip::app::Clusters::PushAvStreamTransport::TransportStatusEnum mTransportStatus;
    chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum mTransportTriggerType;
    uint16_t mConnectionID;
};
