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
#include <app/clusters/push-av-stream-transport-server/CodegenIntegration.h>
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

Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsStruct & transportOptions, const uint16_t connectionID,
                                                    FabricIndex accessingFabricIndex)
{
    PushAvStream stream{ connectionID, transportOptions, TransportStatusEnum::kInactive, PushAvStreamTransportStatusEnum::kIdle };

    /*Store the allocated stream persistently*/
    pushavStreams.push_back(stream);

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    pushavStreams.erase(std::remove_if(pushavStreams.begin(), pushavStreams.end(),
                                       [connectionID](const PushAvStream & stream) { return stream.id == connectionID; }),
                        pushavStreams.end());
    ChipLogProgress(Zcl, "Deallocated Push AV Stream with ID: %d", connectionID);
    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ModifyPushTransport(const uint16_t connectionID, const TransportOptionsStorage transportOptions)
{
    for (PushAvStream & stream : pushavStreams)
    {
        if (stream.id == connectionID)
        {
            ChipLogProgress(Zcl, "Modified Push AV Stream with ID: %d", connectionID);
            return Status::Success;
        }
    }
    ChipLogError(Zcl, "Allocated Push AV Stream with ID: %d not found", connectionID);
    return Status::NotFound;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetTransportStatus(const std::vector<uint16_t> connectionIDList,
                                                                                     TransportStatusEnum transportStatus)
{
    for (PushAvStream & stream : pushavStreams)
    {
        for (uint16_t connectionID : connectionIDList)
        {
            if (stream.id == connectionID)
            {
                stream.transportStatus = transportStatus;
                ChipLogProgress(Zcl, "Set Transport Status for Push AV Stream with ID: %d", connectionID);
            }
        }
    }
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ManuallyTriggerTransport(
    const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
    const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl)
{
    // TODO: Validates the requested stream usage against the camera's resource management and stream priority policies.
    for (PushAvStream & stream : pushavStreams)
    {
        if (stream.id == connectionID)
        {
            stream.connectionStatus = PushAvStreamTransportStatusEnum::kBusy;
            ChipLogProgress(Zcl, "Transport triggered for Push AV Stream with ID: %d", connectionID);
        }
    }
    return Status::Success;
}

bool PushAvStreamTransportManager::ValidateStreamUsage(StreamUsageEnum streamUsage)
{
    return true;
}

bool PushAvStreamTransportManager::ValidateSegmentDuration(uint16_t segmentDuration,
                                                           const Optional<DataModel::Nullable<uint16_t>> & videoStreamId)
{
    return true;
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

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetVideoStream(uint16_t videoStreamId)
{
    // TODO: Validate videoStreamID from the allocated videoStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::SetAudioStream(uint16_t audioStreamId)
{
    // TODO: Validate audioStreamID from the allocated audioStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ValidateZoneId(uint16_t zoneId)
{
    // TODO: Validate zoneId from the allocated zones
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

bool PushAvStreamTransportManager::ValidateMotionZoneListSize(size_t zoneListSize)
{
    // TODO: Validate motion zone size
    // Returning true to pass through checks in the Server Implementation.
    return true;
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

void PushAvStreamTransportManager::Init()
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Initialized");
}
CHIP_ERROR
PushAvStreamTransportManager::LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections)
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

CHIP_ERROR PushAvStreamTransportManager::IsHardPrivacyModeActive(bool & isActive)
{
    isActive = false;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportManager::IsSoftRecordingPrivacyModeActive(bool & isActive)
{
    isActive = false;
    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAvStreamTransportManager::IsSoftLivestreamPrivacyModeActive(bool & isActive)
{
    isActive = false;
    return CHIP_NO_ERROR;
}
