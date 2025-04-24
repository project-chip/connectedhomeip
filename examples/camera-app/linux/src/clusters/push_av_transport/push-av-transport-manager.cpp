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

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using chip::Protocols::InteractionModel::Status;
using namespace Camera;

Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsDecodeableStruct & transportOptions,
                                                    TransportConfigurationStruct & outTransporConfiguration)
{
    PushAvStream stream{ outTransporConfiguration.connectionID, outTransporConfiguration };

    /*Store the allocated stream persistently*/
    pushavStreams.push_back(stream);
    ChipLogError(Zcl, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]",
                 outTransporConfiguration.connectionID);
    Transports[outTransporConfiguration.connectionID] =
        new PushAVTransport(outTransporConfiguration.connectionID, transportOptions.triggerOptions.triggerType);
    mMediaController->RegisterTransport(Transports[outTransporConfiguration.connectionID], 1, 1 /*videoStreamID, audioStreamID*/);

    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    pushavStreams.erase(std::remove_if(pushavStreams.begin(), pushavStreams.end(),
                                       [connectionID](const PushAvStream & stream) { return stream.id == connectionID; }),
                        pushavStreams.end());
    return Status::Success;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::ModifyPushTransport(const uint16_t connectionID,
                                                  const TransportOptionsDecodeableStruct & transportOptions)
{
    for (PushAvStream & stream : pushavStreams)
    {
        if (stream.id == connectionID)
        {
            ChipLogError(Zcl, "Modified Push AV Stream with ID: %d", connectionID);
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
                stream.transportConfig.transportStatus = transportStatus;
                if (Transports[connectionID])
                {
                    ChipLogError(Zcl, "Set Transport Status for Push AV Stream with ID: %d", connectionID);
                    Transports[connectionID]->setTransportStatus(transportStatus);
                }
            }
            else
            {
                return Status::NotFound;
            }
        }
    }
    return Status::Success;
}

Protocols::InteractionModel::Status PushAvStreamTransportManager::ManuallyTriggerTransport(
    const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
    const Optional<Structs::TransportMotionTriggerTimeControlStruct::DecodableType> & timeControl)
{
    if (activationReason == TriggerActivationReasonEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "PushAvStreamTransportManager, Manual Trigger failed for connection [%u], reason: [%u]", connectionID,
                     (uint16_t) activationReason);
        return Status::Failure;
    }

    for (PushAvStream & stream : pushavStreams)
    {
        if (stream.id == connectionID)
        {
            if (Transports[connectionID])
            {
                ChipLogProgress(Zcl, "Manual Trigger PushAV Transport, id: %d", connectionID);
                Transports[connectionID]->TriggerTransport(activationReason);
                return Status::Success;
            }
        }
    }

    ChipLogError(Zcl, "PushAvStreamTransportManager, Manual Trigger, connectionID [%u] not found ", connectionID);
    return Status::NotFound;
}

Protocols::InteractionModel::Status
PushAvStreamTransportManager::FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                                            DataModel::List<const TransportConfigurationStruct> & outtransportConfigurations)
{
    configList.clear();
    for (PushAvStream & stream : pushavStreams)
    {
        if (connectionID.Value().IsNull())
        {
            configList.push_back(stream.transportConfig);
        }
        else if (connectionID.Value().Value() == stream.id)
        {
            configList.push_back(stream.transportConfig);
        }
    }
    outtransportConfigurations = DataModel::List<const TransportConfigurationStruct>(configList.data(), configList.size());
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
