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
    ChipLogProgress(NotSpecified, "AllocatePushTransport manager called" );
    // check valid tls endpoint
    if (transportOptions.endpointID < 0 || connectionID < 0 || connectionID >= MAX_PUSH_TRANSPORT_CONNECTION_ID) {
        return Status::ConstraintError;
    }

    // check valid ingest method
    if (transportOptions.ingestMethod != IngestMethodsEnum::kCMAFIngest){
        return Status::ConstraintError;
    }

    // check valid ContainerFormat
    if (transportOptions.containerFormat != ContainerFormatEnum::kCmaf){
        return Status::ConstraintError;
    }

    // check valid URL
    if (/* transportOptions.url */0){ // TODO: CHAYAN: how to check string is valid?
        return Status::ConstraintError;
    }

    // check valid triggertype
    if (transportOptions.triggerOptions.triggerType == TransportTriggerTypeEnum::kUnknownEnumValue){ // TODO: CHAYAN: how to check string is valid?
        return Status::ConstraintError;
    }

    // check if avpush-transport already exists. if not, create a new avpush-transport
    if (Transports[connectionID] != nullptr || ConnectionsMap[connectionID].has_value()){
        return Status::ConstraintError;
    }
    else {
        Transports[connectionID] = new PushAVTransport();
        Transports[connectionID]->TransportStatus = true;
        Transports[connectionID]->mTransportTriggerType = transportOptions.triggerOptions.triggerType;
        ConnectionsMap[connectionID] = transportOptions;
        outTransportStatus = TransportStatusEnum::kActive;
    }

    // check valid ZoneID
    // if (transportOptions.triggerOptions.motionZones.Value()){ 
    //     return Status::ConstraintError;
    // }
    // TODO: How to check motion zones?

    // TODO: Privacy conflicts check, bandwidth limits check

    // TODO: Implement allocation logic
    
    return Status::Success;
}

Protocols::InteractionModel::Status PushAVTransportManager::DeallocatePushTransport(const uint16_t connectionID)
{
    ChipLogProgress(NotSpecified, "DeallocatePushTransport manager called" );

    // check valid connectionID
    if (connectionID < 0 || connectionID >= MAX_PUSH_TRANSPORT_CONNECTION_ID) {
        return Status::ConstraintError;
    }
    
    // check if transport exists and not streaming. If so, delete transport. Otherwise, return error.
    if (Transports[connectionID] == nullptr || !ConnectionsMap[connectionID].has_value()) {
        return Status::NotFound;
    }
    else if (Transports[connectionID]->IsStreaming()){
        return Status::Busy;
    }
    else {
        Transports[connectionID]->DeleteTransport();
        Transports[connectionID] = nullptr;
        ConnectionsMap[connectionID].reset();
    }
    
    return Status::Success;
}

Protocols::InteractionModel::Status PushAVTransportManager::ModifyPushTransport(const uint16_t connectionID,
                                                                    const TransportOptionsStruct & outTransportOptions)
{
    ChipLogProgress(NotSpecified, "ModifyPushTransport manager called" );

    // check valid connectionID
    if (connectionID < 0 || connectionID >= MAX_PUSH_TRANSPORT_CONNECTION_ID) {
        return Status::ConstraintError;
    }
    
    // check if transport exists and not streaming. If so, modify transport. Otherwise, return error.
    if (Transports[connectionID] == nullptr || !ConnectionsMap[connectionID].has_value()) {
        return Status::NotFound;
    }
    else if (Transports[connectionID]->IsStreaming()){
        return Status::Busy;
    }
    else {
        Transports[connectionID]->mTransportTriggerType = outTransportOptions.triggerOptions.triggerType;
    }

    return Status::Success;
}

Protocols::InteractionModel::Status PushAVTransportManager::SetTransportStatus(const uint16_t connectionID,
                                                                   TransportStatusEnum transportStatus)
{
    // TODO: Implement allocation logic
    ChipLogProgress(NotSpecified, "SetTransportStatus manager called" );

    // check valid connectionID
    if (connectionID < 0 || connectionID >= MAX_PUSH_TRANSPORT_CONNECTION_ID) {
        return Status::ConstraintError;
    }
    
    // check if transport exists. If so, set the transport status as specified to the transport object attribute. Otherwise, return error.
    if (Transports[connectionID] == nullptr || !ConnectionsMap[connectionID].has_value()) {
        return Status::NotFound;
    }
    else {
        if (transportStatus == TransportStatusEnum::kActive){
            Transports[connectionID]->TransportStatus = true;
        }
        else if (transportStatus == TransportStatusEnum::kInactive){
            Transports[connectionID]->TransportStatus = false;
        }
        else {
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
    ChipLogProgress(NotSpecified, "ManuallyTriggerTransport manager called" );
    return Status::UnsupportedCluster;
}

void convertTrigger (Structs::TransportTriggerOptionsStruct::DecodableType & triggerOptionsD, Structs::TransportTriggerOptionsStruct::Type & triggerOptions) 
{
    triggerOptions.triggerType = triggerOptionsD.triggerType;
}

// void convertcontainerOptions (Structs::ContainerOptionsStruct::DecodableType & containerOptionsD, Structs::ContainerOptionsStruct::Type & containerOptions) 
// {
//     containerOptions.containerType = containerOptionsD.containerType;
// }

// void convertTransport (Structs::TransportOptionsStruct::DecodableType & transportOptionsD, Structs::TransportOptionsStruct::Type & transportOptions) 
// {
//     Structs::TransportTriggerOptionsStruct::Type mtriggerOptions;
//     Structs::ContainerOptionsStruct::Type mcontainerOptions;

//     convertTrigger(transportOptionsD.triggerOptions, mtriggerOptions);
//     convertcontainerOptions(transportOptionsD.containerOptions, mcontainerOptions);
    
//     transportOptions.streamUsage = transportOptionsD.streamUsage;
//     transportOptions.videoStreamID = transportOptionsD.videoStreamID;
//     transportOptions.audioStreamID = transportOptionsD.audioStreamID;
//     transportOptions.endpointID = transportOptionsD.endpointID;
//     transportOptions.url = transportOptionsD.url;
//     transportOptions.triggerOptions = mtriggerOptions;
//     transportOptions.ingestMethod = transportOptionsD.ingestMethod;
//     transportOptions.containerFormat = transportOptionsD.containerFormat;
//     transportOptions.containerOptions = mcontainerOptions;
//     transportOptions.metadataOptions = transportOptionsD.metadataOptions;
//     transportOptions.expiryTime = transportOptionsD.expiryTime;
// }

Protocols::InteractionModel::Status
    PushAVTransportManager::FindTransport(const Optional<DataModel::Nullable<uint16_t>> & connectionID,
                  DataModel::List<TransportConfigurationStruct> & outtransportConfigurations)
{
    // TODO: Implement allocation logic
    ChipLogProgress(NotSpecified, "FindTransport manager called" );

    if (connectionID.HasValue()) {
        if (connectionID.Value().IsNull()) {
            // TODO: Handle for null connectionId
        }
        else {
            // uint16_t connID = connectionID.Value().Value();
            // TransportConfigurationStruct configArray[1];
            // configArray[0].connectionID = connID;
            // configArray[0].transportStatus = Transports[connID]->TransportStatus ? TransportStatusEnum::kActive : TransportStatusEnum::kInactive;
            // // configArray[0].transportOptions = ConnectionsMap[connID].value();

            // chip::app::DataModel::List<TransportConfigurationStruct> configList;
            // configList = chip::app::DataModel::List<TransportConfigurationStruct>(configArray, 1);
            // outtransportConfigurations = configList;

            return Status::Success;
        }
    }
    return Status::UnsupportedCluster;
}   

void PushAVTransportManager::OnAttributeChanged(AttributeId attributeId) {
    // TODO: Implement attribute change logic
}

CHIP_ERROR PushAVTransportManager::LoadCurrentConnections(std::vector<uint16_t> & currentConnections){
    
    ChipLogError(Zcl, "LoadCurrentConnections called" );
    mCurrentConnections = currentConnections;
    for (auto it : mCurrentConnections) {
        if (it > MAX_PUSH_TRANSPORT_CONNECTION_ID){
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        Transports[it] = new PushAVTransport();
        // TODO: complete implementation required
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR PushAVTransportManager::PersistentAttributesLoadedCallback() {

    ChipLogError(Zcl, "PersistentAttributesLoadedCallback called" );
    return CHIP_NO_ERROR;
}