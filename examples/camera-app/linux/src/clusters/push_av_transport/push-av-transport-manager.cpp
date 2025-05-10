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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/push-av-stream-transport-server/push-av-stream-transport-server.h>
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <push-av-transport-manager.h>
#include <pushav-transport.h>
#include <pushav-clip-recorder.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using chip::Protocols::InteractionModel::Status;
using namespace Camera;

// TODO: ConfigureRecorderSettings improvements needed
// 1. Replace hardcoded audio/video settings with values from camera-av-stream-manager structs
// 2. Implement proper storage path resolution instead of fixed "/workspace/"
// 3. Calculate frame durations dynamically from stream properties
// 4. Add safety checks before accessing Optional values
// 5. Remove direct struct assignments - initialize fields individually
void ConfigureRecorderSettings(PushAVTransport * transport,
                                const TransportOptionsDecodeableStruct & transportOptions,
                                TransportConfigurationStruct & outTransporConfiguration){

    PushAVClipRecorder::ClipInfoStruct clipInfo;
    PushAVClipRecorder::AudioInfoStruct audioInfo;
    PushAVClipRecorder::VideoInfoStruct videoInfo;

    clipInfo.mHasAudio = transport->CanSendAudio();
    clipInfo.mHasVideo = transport->CanSendVideo();
    clipInfo.mMaxClipDuration = outTransporConfiguration.transportOptions.triggerOptions.motionTimeControl.Value().initialDuration;
    clipInfo.mChunkDuration = outTransporConfiguration.transportOptions.containerOptions.CMAFContainerOptions.Value().chunkDuration;
    clipInfo.mAudioStreamID = outTransporConfiguration.transportOptions.audioStreamID.Value();
    clipInfo.mVideoStreamID = outTransporConfiguration.transportOptions.videoStreamID.Value();
    clipInfo.mClipID = 0;
    clipInfo.mRecorderID = std::to_string(clipInfo.mVideoStreamID) + "-" + std::to_string(clipInfo.mAudioStreamID);
    clipInfo.mOutputPath = "/workspace/";
    clipInfo.mSupportedCodec = {
            AV_CODEC_ID_OPUS,
            AV_CODEC_ID_H264
        };
    clipInfo.mInputTb = { 1, 1000000 };

    audioInfo.mChannelLayout = AV_CH_LAYOUT_STEREO;
    audioInfo.mChannels = 2;
    audioInfo.mAudioCodecId = AV_CODEC_ID_OPUS;
    audioInfo.mSampleRate = 48000;
    audioInfo.mBitRate = 20000;
    audioInfo.mAPts = 0;
    audioInfo.mADts = 0;
    audioInfo.aStreamIndex = -1;
    audioInfo.mAudioFrameDuration = 960;
    audioInfo.mAudioTb = { 1, audioInfo.mSampleRate }; 

    videoInfo.mVideoCodecId = AV_CODEC_ID_H264;
    videoInfo.mVPts = 0;
    videoInfo.mVDts = 0;
    videoInfo.mWidth = 320;
    videoInfo.mHeight = 240;
    videoInfo.mFrameRate = 15;
    videoInfo.mVideoFrameDuration = 66667;
    videoInfo.mVideoTb = { 1, 90000 };
    videoInfo.vStreamIndex = -1;
    videoInfo.mBitRate = 0;

    transport->clipInfo = clipInfo;
    transport->audioInfo = audioInfo;
    transport->videoInfo = videoInfo;
}


Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsDecodeableStruct & transportOptions,
                                                    TransportConfigurationStruct & outTransporConfiguration)
{
    uint16_t connectionID = outTransporConfiguration.connectionID;

    mTransportOptionsMap[connectionID] = transportOptions;
    mTransportConfigMap[connectionID]  = outTransporConfiguration;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]", connectionID);
    mTransportMap[connectionID] =
        std::move(std::make_unique<PushAVTransport>(connectionID, transportOptions.url.data(), transportOptions.triggerOptions.triggerType));

    mMediaController->RegisterTransport(mTransportMap[connectionID].get(), transportOptions.videoStreamID.Value(), transportOptions.audioStreamID.Value());
    ConfigureRecorderSettings(mTransportMap[connectionID].get(), transportOptions, outTransporConfiguration);

    return Status::Success;
}

PushAvStreamTransportManager::~PushAvStreamTransportManager()
{
    // Unregister all transports from Media Controller before deleting them. This will ensure that any ongoing streams are stopped.
    if (mMediaController != nullptr)
    {
        for (auto & kv : mTransportMap)
        {
            mMediaController->UnregisterTransport(kv.second.get());
        }
    }
    mTransportMap.clear();
    mTransportOptionsMap.clear();
    mTransportConfigMap.clear();
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }

    mMediaController->UnregisterTransport(mTransportMap[connectionID].get());
    mTransportMap.erase(connectionID);
    mTransportOptionsMap.erase(connectionID);
    mTransportConfigMap.erase(connectionID);

    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ModifyPushTransport(const uint16_t connectionID,
                                                  const TransportOptionsDecodeableStruct & transportOptions)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }

    mTransportOptionsMap[connectionID] = transportOptions;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetTransportStatus(const std::vector<uint16_t> & connectionIDList,
                                                                                     TransportStatusEnum transportStatus)
{
    if (connectionIDList.empty())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, connectionIDList is empty");
        return Status::Failure;
    }

    if (transportStatus == TransportStatusEnum::kUnknownEnumValue)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, Invalid TransportStatus, transportStatus: [%u]",
                     (uint16_t) transportStatus);
        return Status::Failure;
    }

    for (uint16_t connectionID : connectionIDList)
    {
        if (mTransportMap.find(connectionID) == mTransportMap.end())
        {
            ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
            continue;
        }
        mTransportMap[connectionID]->setTransportStatus(transportStatus);
    }

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ManuallyTriggerTransport(
    const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
    const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl)
{
    if (activationReason == TriggerActivationReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, Manual Trigger failed for connection [%u], reason: [%u]", connectionID,
                     (uint16_t) activationReason);
        return Status::Failure;
    }

    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Trigger PushAV Transport for Connection: [%u]", connectionID);
    mTransportMap[connectionID]->TriggerTransport(activationReason);

    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                                            DataModel::List<const TransportConfigurationStruct> & outTransportConfigurations)
{
    if (!connectionID.HasValue())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, connectionID not available");
        return Status::Failure;
    }

    std::vector<TransportConfigurationStruct> configList;

    if (connectionID.Value().IsNull()) {
        for (auto & it : mTransportConfigMap)
        {
            configList.push_back(it.second);
        }
    }
    else {
        for (auto & it : mTransportConfigMap)
        {
            if (connectionID.Value().Value() == it.first)
            {
                configList.push_back(it.second);
            }
        }
    }

    outTransportConfigurations = DataModel::List<const TransportConfigurationStruct>(configList.data(), configList.size());
    
    return Status::Success;
}

CHIP_ERROR
PushAvStreamTransportManager::ValidateStreamUsage(StreamUsageEnum streamUsage,
                                                  const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                  const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    // TODO: Validates the requested stream usage against the camera's resource management and stream priority policies.
    return CHIP_NO_ERROR;
}

void PushAvStreamTransportManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
}

void PushAvStreamTransportManager::Init(MediaController * mediaController)
{
    mMediaController = mediaController;
    return;
}

CHIP_ERROR PushAvStreamTransportManager::LoadCurrentConnections(std::vector<TransportConfigurationStruct> & currentConnections)
{
    ChipLogError(Zcl, "Push AV Current Connections loaded");

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PushAvStreamTransportManager::PersistentAttributesLoadedCallback()
{
    ChipLogError(Zcl, "Persistent attributes loaded");

    return CHIP_NO_ERROR;
}
