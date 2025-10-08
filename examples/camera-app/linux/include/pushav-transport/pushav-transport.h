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
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-cluster.h>
#include <functional>
#include <memory>
#include <protocols/interaction_model/StatusCode.h>
#include <thread>
#include <vector>

class PushAVTransport : public Transport
{
public:
    PushAVTransport(const chip::app::Clusters::PushAvStreamTransport::TransportOptionsStruct & transportOptions,
                    const uint16_t connectionID, AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams);
    ~PushAVTransport() override;

    // Send video data for a given stream ID
    void SendVideo(const char * data, size_t size, int64_t timestamp, uint16_t videoStreamID) override;

    // Send audio data for a given stream ID
    void SendAudio(const char * data, size_t size, int64_t timestamp, uint16_t audioStreamID) override;

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

    void TriggerTransport(chip::app::Clusters::PushAvStreamTransport::TriggerActivationReasonEnum activationReason, int zoneId = -1,
                          int sensitivity = 5);
    // Get Transport status
    bool GetTransportStatus()
    {
        return (mTransportStatus == chip::app::Clusters::PushAvStreamTransport::TransportStatusEnum::kInactive);
    } // 0:Active 1:Inactive

    void ConfigureRecorderSettings(const chip::app::Clusters::PushAvStreamTransport::TransportOptionsStruct & transportOptions,
                                   AudioStreamStruct & audioStreamParams, VideoStreamStruct & videoStreamParams);

    void ModifyPushTransport(const chip::app::Clusters::PushAvStreamTransport::TransportOptionsStorage & transportOptions);

    bool HandleTriggerDetected();

    void InitializeRecorder();

    bool CanSendPacketsToRecorder();

    void readFromFile(char * filename, uint8_t ** videoBuffer, size_t * videoBufferBytes);

    void SetTLSCertPath(std::string rootCert, std::string devCert, std::string devKey);

    void SetTLSCert(std::vector<uint8_t> bufferRootCert, std::vector<uint8_t> bufferClientCert,
                    std::vector<uint8_t> bufferClientCertKey, std::vector<std::vector<uint8_t>> bufferIntermediateCerts);

    void SetZoneSensitivityList(std::vector<std::pair<chip::app::DataModel::Nullable<uint16_t>, uint8_t>> zoneSensitivityList)
    {
        mZoneSensitivityList = zoneSensitivityList;
    }

    void SetCurrentlyUsedBandwidthMbps(double currentlyUsedBandwidthMbps)
    {
        mCurrentlyUsedBandwidthMbps = currentlyUsedBandwidthMbps;
    }

    double GetCurrentlyUsedBandwidthMbps() { return mCurrentlyUsedBandwidthMbps; }

    // Set the cluster server reference for direct API calls
    void SetPushAvStreamTransportServer(chip::app::Clusters::PushAvStreamTransportServer * server)
    {
        mPushAvStreamTransportServer = server;
    }

    void ConfigureRecorderTimeSetting(
        const chip::app::Clusters::PushAvStreamTransport::Structs::TransportMotionTriggerTimeControlStruct::DecodableType &
            timeControl);

private:
    bool mHasAugmented                                                              = false;
    bool mStreaming                                                                 = false;
    std::unique_ptr<PushAVClipRecorder> mRecorder                                   = nullptr;
    std::unique_ptr<PushAVUploader> mUploader                                       = nullptr;
    chip::app::Clusters::PushAvStreamTransportServer * mPushAvStreamTransportServer = nullptr;

    std::chrono::steady_clock::time_point mBlindStartTime;
    PushAVClipRecorder::ClipInfoStruct mClipInfo;
    PushAVClipRecorder::AudioInfoStruct mAudioInfo;
    PushAVClipRecorder::VideoInfoStruct mVideoInfo;
    PushAVUploader::PushAVCertPath mCertPath;
    PushAVUploader::CertificatesInfo mCertBuffer;
    AudioStreamStruct mAudioStreamParams;
    VideoStreamStruct mVideoStreamParams;

    // Note, a ZoneID within a Zone List can be Null, meaning the entry applies to all zones.
    std::vector<std::pair<chip::app::DataModel::Nullable<uint16_t>, uint8_t>> mZoneSensitivityList;

    // Dummy implementation to indicate if video can be sent
    bool mCanSendVideo = false;

    // Dummy implementation to indicate if audio can be sent
    bool mCanSendAudio = false;

    chip::app::Clusters::PushAvStreamTransport::TransportStatusEnum mTransportStatus;
    chip::app::Clusters::PushAvStreamTransport::TransportTriggerTypeEnum mTransportTriggerType;
    uint16_t mConnectionID;
    double mCurrentlyUsedBandwidthMbps = 0.0;
};
