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
#include <push-av-stream-transport-delegate-impl.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PushAvStreamTransport;
using chip::Protocols::InteractionModel::Status;

// Global pointer to overall PushAV Stream Transport implementing the Cluster delegate.
std::unique_ptr<PushAvStreamTransportManager> sPushAvStramTransportInstance;
// Global pointer to PushAV Stream Transport Server SDK cluster;
std::unique_ptr<PushAvStreamTransportServer> sPushAvStramTransportClusterServerInstance;

Protocols::InteractionModel::Status
PushAvStreamTransportManager::AllocatePushTransport(const TransportOptionsDecodeableStruct & transportOptions,
                                                    TransportConfigurationStruct & outTransporConfiguration)
{
    PushAvStream stream{ outTransporConfiguration.connectionID, outTransporConfiguration };

    /*Store the allocated stream persistently*/
    pushavStreams.push_back(stream);

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
                ChipLogError(Zcl, "Set Transport Status for Push AV Stream with ID: %d", connectionID);
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
    // TODO: Validates the requested stream usage against the camera's resource management and stream priority policies.
    return Status::Success;
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

void PushAvStreamTransportManager::Init()
{
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

void emberAfPushAvStreamTransportClusterInitCallback(EndpointId endpoint)
{
    VerifyOrReturn(
        endpoint == 1, // this cluster is only enabled for endpoint 1.
        ChipLogError(Zcl, "Push AV Stream Transport cluster delegate is not implemented for endpoint with id %d.", endpoint));

    VerifyOrReturn(!sPushAvStramTransportInstance && !sPushAvStramTransportClusterServerInstance);

    sPushAvStramTransportInstance = std::make_unique<PushAvStreamTransportManager>();
    sPushAvStramTransportInstance->Init();

    sPushAvStramTransportClusterServerInstance =
        std::make_unique<PushAvStreamTransportServer>(*sPushAvStramTransportInstance.get(), endpoint);
    sPushAvStramTransportClusterServerInstance->Init();
}
