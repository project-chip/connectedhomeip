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
    uint16_t connectionID = outTransporConfiguration.connectionID;

    mTransportOptionsMap[connectionID] = transportOptions;
    mTransportConfigMap[connectionID]  = outTransporConfiguration;

    ChipLogProgress(Camera, "PushAvStreamTransportManager, Create PushAV Transport for Connection: [%u]", connectionID);
    mTransportMap[connectionID] =
        std::move(std::make_unique<PushAVTransport>(connectionID, transportOptions.triggerOptions.triggerType));

    mMediaController->RegisterTransport(mTransportMap[connectionID].get(), 1, 1 /*videoStreamID, audioStreamID*/);

    return Status::Success;
}

PushAvStreamTransportManager::~PushAvStreamTransportManager()
{
    // Unregister all transports from Media Controller before deleting them. This will ensure that any ongoing streams are stopped.
    for (auto & transport : mTransportMap)
    {
        mMediaController->UnregisterTransport(transport.second.get());
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
PushAvStreamTransportManager::FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                                            DataModel::List<const TransportConfigurationStruct> & outtransportConfigurations)
{
    if (connectionID.Value().IsNull())
    {
        ChipLogError(Camera, "PushAvStreamTransportManager, connectionID is null");
        return Status::Failure;
    }

    std::vector<TransportConfigurationStruct> configList;

    for (auto & it : mTransportConfigMap)
    {
        if (connectionID.Value().Value() == it.first)
        {
            configList.push_back(it.second);
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
