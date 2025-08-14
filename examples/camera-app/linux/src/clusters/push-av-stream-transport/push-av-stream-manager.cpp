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

#include "push-av-stream-manager.h"

#include <Options.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/logging/CHIPLogging.h>

#include <regex>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using chip::Protocols::InteractionModel::Status;

void PushAvStreamManager::Init()
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Initialized");
}

Protocols::InteractionModel::Status PushAvStreamManager::AllocatePushTransport(const TransportOptionsStruct & transportOptions,
                                                                               const uint16_t connectionID)
{
    PushAvStream stream{ connectionID, transportOptions, TransportStatusEnum::kInactive, PushAvStreamTransportStatusEnum::kIdle };

    /*Store the allocated stream persistently*/
    pushavStreams.push_back(stream);

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamManager::DeallocatePushTransport(const uint16_t connectionID)
{
    pushavStreams.erase(std::remove_if(pushavStreams.begin(), pushavStreams.end(),
                                       [connectionID](const PushAvStream & stream) { return stream.id == connectionID; }),
                        pushavStreams.end());
    ChipLogProgress(Zcl, "Deallocated Push AV Stream with ID: %d", connectionID);
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamManager::ModifyPushTransport(const uint16_t connectionID,
                                                                             const TransportOptionsStorage transportOptions)
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

Protocols::InteractionModel::Status PushAvStreamManager::SetTransportStatus(const std::vector<uint16_t> connectionIDList,
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

Protocols::InteractionModel::Status PushAvStreamManager::ManuallyTriggerTransport(
    const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
    const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl)
{
    // TODO: Validates the requested stream usage against the camera's resource management and stream priority policies.
    auto it = std::find_if(pushavStreams.begin(), pushavStreams.end(),
                           [connectionID](const PushAvStream & stream) { return stream.id == connectionID; });

    if (it == pushavStreams.end())
    {
        ChipLogError(Zcl, "Push AV Stream with ID: %d not found for triggering", connectionID);
        return Status::NotFound;
    }

    it->connectionStatus = PushAvStreamTransportStatusEnum::kBusy;
    ChipLogProgress(Zcl, "Transport triggered for Push AV Stream with ID: %d", connectionID);
    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamManager::ValidateBandwidthLimit(StreamUsageEnum streamUsage,
                                            const Optional<DataModel::Nullable<uint16_t>> & videoStreamId,
                                            const Optional<DataModel::Nullable<uint16_t>> & audioStreamId)
{
    // TODO: Validates the requested stream usage against the camera's resource management.
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

bool PushAvStreamManager::ValidateUrl(std::string url)
{
    // Basic URL validation: checks for scheme and host
    // Accepts http, https, rtsp, rtmp, etc.
    static const std::regex url_regex(R"(^(https?|rtsp|rtmp)://([a-zA-Z0-9\-\.]+)(:[0-9]+)?(/.*)?$)", std::regex::icase);
    return std::regex_match(url, url_regex);
}

Protocols::InteractionModel::Status PushAvStreamManager::SelectVideoStream(StreamUsageEnum streamUsage, uint16_t & videoStreamId)
{
    // TODO: Select and Assign videoStreamID from the allocated videoStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamManager::SelectAudioStream(StreamUsageEnum streamUsage, uint16_t & audioStreamId)
{
    // TODO: Select and Assign audioStreamID from the allocated audioStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamManager::ValidateVideoStream(uint16_t videoStreamId)
{
    // TODO: Validate videoStreamID from the allocated videoStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamManager::ValidateAudioStream(uint16_t audioStreamId)
{
    // TODO: Validate audioStreamID from the allocated audioStreams
    // Returning Status::Success to pass through checks in the Server Implementation.
    return Status::Success;
}

PushAvStreamTransportStatusEnum PushAvStreamManager::GetTransportBusyStatus(const uint16_t connectionID)
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

void PushAvStreamManager::OnAttributeChanged(AttributeId attributeId)
{
    ChipLogProgress(Zcl, "Attribute changed for AttributeId = " ChipLogFormatMEI, ChipLogValueMEI(attributeId));
}

CHIP_ERROR
PushAvStreamManager::LoadCurrentConnections(std::vector<TransportConfigurationStorage> & currentConnections)
{
    ChipLogProgress(Zcl, "Push AV Current Connections loaded");

    return CHIP_NO_ERROR;
}

CHIP_ERROR
PushAvStreamManager::PersistentAttributesLoadedCallback()
{
    ChipLogProgress(Zcl, "Push AV Stream Transport Persistent attributes loaded");

    return CHIP_NO_ERROR;
}
