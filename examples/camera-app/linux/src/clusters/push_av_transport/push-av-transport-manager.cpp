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

#include "push-av-transport-manager.h"

#include <app-common/zap-generated/cluster-enums.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/support/logging/CHIPLogging.h>

#include <iostream>

#define MAX_PUSH_TRANSPORT_CONNECTION_ID 65535

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;

using Status = Protocols::InteractionModel::Status;

using namespace Camera;

Protocols::InteractionModel::Status PushAVTransportManager::AllocatePushTransport(uint16_t connectionID,
                                                                                  const TransportOptionsStruct & transportOptions,
                                                                                  TransportStatusEnum & outTransportStatus)
{
    ChipLogProgress(NotSpecified, "AllocatePushTransport manager called");

    // check if avpush-transport already exists. if not, create a new avpush-transport
    if (Transports[connectionID] != nullptr || ConnectionsMap[connectionID].has_value())
    {
        return Status::ConstraintError;
    }
    else
    {
        Transports[connectionID]                        = new PushAVTransport(connectionID, 1);
        Transports[connectionID]->TransportStatus       = true;
        Transports[connectionID]->mTransportTriggerType = transportOptions.triggerOptions.triggerType;
        ConnectionsMap[connectionID]                    = transportOptions;
        outTransportStatus                              = TransportStatusEnum::kActive;
    }

    return Status::Success;
}

Protocols::InteractionModel::Status PushAVTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    ChipLogProgress(NotSpecified, "DeallocatePushTransport manager called");

    // check if transport exists and not streaming. If so, delete transport. Otherwise, return error.
    if (Transports[connectionID] == nullptr || !ConnectionsMap[connectionID].has_value())
    {
        return Status::NotFound;
    }
    else
    {
        Transports[connectionID]->~PushAVTransport();
        Transports[connectionID] = nullptr;
        ConnectionsMap[connectionID].reset();
    }

    return Status::Success;
}

Protocols::InteractionModel::Status PushAVTransportManager::ModifyPushTransport(const uint16_t connectionID,
                                                                                const TransportOptionsStruct & outTransportOptions)
{
    ChipLogProgress(NotSpecified, "ModifyPushTransport manager called");

    // check if transport exists and not streaming. If so, modify transport. Otherwise, return error.
    if (Transports[connectionID] == nullptr || !ConnectionsMap[connectionID].has_value())
    {
        return Status::NotFound;
    }
    else
    {
        Transports[connectionID]->mTransportTriggerType = outTransportOptions.triggerOptions.triggerType;
    }

    return Status::Success;
}

Protocols::InteractionModel::Status PushAVTransportManager::SetTransportStatus(const uint16_t connectionID,
                                                                               TransportStatusEnum transportStatus)
{
    // TODO: Implement allocation logic
    ChipLogProgress(NotSpecified, "SetTransportStatus manager called");

    // check if transport exists. If so, set the transport status as specified to the transport object attribute. Otherwise, return
    // error.
    if (Transports[connectionID] == nullptr || !ConnectionsMap[connectionID].has_value())
    {
        return Status::NotFound;
    }
    else
    {
        if (transportStatus == TransportStatusEnum::kActive)
        {
            Transports[connectionID]->TransportStatus = true;
        }
        else if (transportStatus == TransportStatusEnum::kInactive)
        {
            Transports[connectionID]->TransportStatus = false;
        }
        else
        {
            return Status::ConstraintError; // invalid status value passed in. Return error.
        }
    }
    return Status::Success;
}

Protocols::InteractionModel::Status
PushAVTransportManager::ManuallyTriggerTransport(const uint16_t connectionID, TriggerActivationReasonEnum activationReason,
                                                 const TransportMotionTriggerTimeControlStruct & timeControl)
{
    // TODO: Implement triggering logic
    ChipLogProgress(NotSpecified, "ManuallyTriggerTransport manager called");
    return Status::UnsupportedCluster;
}

Protocols::InteractionModel::Status
PushAVTransportManager::FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                                      DataModel::List<TransportConfigurationStruct> & outtransportConfigurations)
{
    // TODO: Implement allocation logic
    ChipLogProgress(NotSpecified, "FindTransport manager called");
    return Status::UnsupportedCluster;
}

void PushAVTransportManager::OnAttributeChanged(AttributeId attributeId)
{
    // TODO: Implement attribute change logic
}

CHIP_ERROR PushAVTransportManager::LoadCurrentConnections(std::vector<uint16_t> & currentConnections)
{
    ChipLogError(Zcl, "LoadCurrentConnections called");
    mCurrentConnections = currentConnections;
    for (auto it : mCurrentConnections)
    {
        Transports[it] = new PushAVTransport(it, 1);
        // TODO: complete implementation required
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAVTransportManager::PersistentAttributesLoadedCallback()
{
    ChipLogError(Zcl, "PersistentAttributesLoadedCallback called");
    return CHIP_NO_ERROR;
}
