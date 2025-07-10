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
#include <fstream>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>
#include <push-av-stream-transport-delegate-impl.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using chip::Protocols::InteractionModel::Status;

void PushAvStreamTransportManager::SetMediaController(MediaController * mediaController)
{
    mMediaController = mediaController;
}

PushAvStreamTransportManager::~PushAvStreamTransportManager()
{
    // Unregister all transports from Media Controller before deleting them. This will ensure that any ongoing streams are
    // stopped.
    if (mMediaController != nullptr)
    {
        for (auto & kv : mTransportMap)
        {
            mMediaController->UnregisterTransport(kv.second.get());
        }
    }
    mTransportMap.clear();
    mTransportOptionsMap.clear();
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID)
{

    mTransportOptionsMap[connectionID] = transportOptions;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]", connectionID);
    mTransportMap[connectionID] =
        std::move(std::make_unique<PushAVTransport>(transportOptions, connectionID, mAudioStreamParams, mVideoStreamParams));

    if (mMediaController == nullptr)
    {
        ChipLogError(Camera, "PushAvStreamTransportManager: MediaController is not set");
        mTransportMap.erase(connectionID);
        return Status::NotFound;
    }

    mMediaController->RegisterTransport(mTransportMap[connectionID].get(), transportOptions.videoStreamID.Value().Value(),
                                        transportOptions.audioStreamID.Value().Value());

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }
    mTransportMap[connectionID].reset();
    mMediaController->UnregisterTransport(mTransportMap[connectionID].get());
    mTransportMap.erase(connectionID);
    mTransportOptionsMap.erase(connectionID);

    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ModifyPushTransport(const uint16_t connectionID, const TransportOptionsStorage transportOptions)
{
    if (mTransportMap.find(connectionID) == mTransportMap.end())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, failed to find Connection :[%u]", connectionID);
        return Status::NotFound;
    }

    mTransportOptionsMap[connectionID] = transportOptions;
    mTransportMap[connectionID].get()->ModifyPushTransport(transportOptions);
    ChipLogProgress(Camera, "PushAvStreamTransportManager, success to modify Connection :[%u]", connectionID);

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetTransportStatus(const std::vector<uint16_t> connectionIDList,
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
PushAvStreamTransportManager::ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                                     const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                                     const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    // TODO: Validates the requested stream usage against the camera's resource management.
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

bool PushAvStreamTransportManager::ValidateUrl(std::string url)
{
    const std::string https = "https://";

    // Check minimum length and https prefix
    if (url.size() <= https.size() || url.substr(0, https.size()) != https)
    {
        return false;
    }

    // Check for non-empty host
    size_t hostStart = https.size();
    size_t hostEnd   = url.find('/', hostStart);
    std::string host = (hostEnd == std::string::npos) ? url.substr(hostStart) : url.substr(hostStart, hostEnd - hostStart);

    return !host.empty();
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SelectVideoStream(StreamUsageEnum streamUsage,
                                                                                    uint16_t & videoStreamId)
{
    // TODO: Select and Assign videoStreamID from the allocated videoStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SelectAudioStream(StreamUsageEnum streamUsage,
                                                                                    uint16_t & audioStreamId)
{
    // TODO: Select and Assign audioStreamID from the allocated audioStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ValidateVideoStream(uint16_t videoStreamId)
{
    // TODO: Validate videoStreamID from the allocated videoStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ValidateAudioStream(uint16_t audioStreamId)
{
    // TODO: Validate audioStreamID from the allocated audioStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

PushAvStreamTransportStatusEnum PushAvStreamTransportManager::GetTransportBusyStatus(const uint16_t connectionID)
{
    for (PushAvStream & stream : pushavStreams)
    {
        if (stream.id == connectionID)
        {
            return stream.connectionStatus;
        }
    }
    return PushAvStreamTransportStatusEnum::kUnknown;
}

void PushAvStreamTransportManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
}

void PushAvStreamTransportManager::Init(AudioStreamStruct aAudioStreamParams, VideoStreamStruct aVideoStreamParams)
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Initialized");
    // mMediaController   = mediaController;
    mVideoStreamParams = aVideoStreamParams;
    mAudioStreamParams = aAudioStreamParams;
    return;
}

CHIP_ERROR PushAvStreamTransportManager::LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections)
{
    ChipLogProgress(Zcl, "Push AV Current Connections loaded");

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PushAvStreamTransportManager::PersistentAttributesLoadedCallback()
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Persistent attributes loaded");

    return CHIP_NO_ERROR;
}
