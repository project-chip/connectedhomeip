/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "AccessControl.h"

#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/GlobalAttributes.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/StatusIB.h>
#include <app/WriteHandler.h>
#include <app/clusters/ota-provider/ota-provider-cluster.h>
#include <app/data-model/Decode.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>
#include <app/util/endpoint-config-api.h>
#include <cstddef>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Optional.h>
#include <lib/core/TLV.h>
#include <protocols/interaction_model/Constants.h>

/**
 * This file defines the APIs needed to handle interaction model dispatch.
 * These are the APIs normally defined generated ember code,
 * however we want a different implementation of these
 * to enable more dynamic behavior, since not all framework consumers will be
 * implementing the same server clusters.
 */
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

DataVersion gMockDataVersion = 0;

OtaProviderServer gOtaProviderServer(kOtaProviderDynamicEndpointId);

} // anonymous namespace

namespace chip {
namespace app {
namespace Clusters {
namespace OTAProvider {

void SetDelegate(chip::EndpointId endpointId, OTAProviderDelegate * delegate)
{
    if (endpointId != kOtaProviderDynamicEndpointId)
    {
        ChipLogError(AppServer, "Trying to set OTA delegate on invalid endpoint %d (only %d supported)", endpointId,
                     kOtaProviderDynamicEndpointId);
        return;
    }
    gOtaProviderServer.SetDelegate(delegate);
}

} // namespace OTAProvider
} // namespace Clusters

using Access::SubjectDescriptor;
using Protocols::InteractionModel::Status;

void DispatchSingleClusterCommand(const ConcreteCommandPath & aPath, TLV::TLVReader & aReader, CommandHandler * aCommandObj)
{
    SubjectDescriptor subjectDescriptor = aCommandObj->GetSubjectDescriptor();

    DataModel::InvokeRequest invokeRequest;
    invokeRequest.path              = aPath;
    invokeRequest.subjectDescriptor = &subjectDescriptor;

    std::optional<DataModel::ActionReturnStatus> result = gOtaProviderServer.InvokeCommand(invokeRequest, aReader, aCommandObj);

    if (result.has_value())
    {
        // The cluster indicates that handler status or data was already set (or will be set asynchronously) by
        // returning std::nullopt. If any other value is returned, it is requesting that a status is set. This
        // includes CHIP_NO_ERROR: in this case CHIP_NO_ERROR would mean set a `status success on the command`
        aCommandObj->AddStatus(aPath, result->GetStatusCode());
    }
}

} // namespace app
} // namespace chip

/**
 * Returns the index of the given endpoint in the list of all endpoints that might support the given cluster server.
 */
uint16_t emberAfGetClusterServerEndpointIndex(EndpointId endpoint, ClusterId cluster, uint16_t fixedClusterServerEndpointCount)
{
    if (endpoint == kOtaProviderDynamicEndpointId && cluster == OtaSoftwareUpdateProvider::Id)
    {
        return 0;
    }
    else if (endpoint == kWebRTCRequesterDynamicEndpointId && cluster == WebRTCTransportRequestor::Id)
    {
        return 1;
    }

    return UINT16_MAX;
}

/**
 * Methods used by AttributePathExpandIterator, which need to exist
 * because it is part of libCHIP.  For AttributePathExpandIterator
 * purposes, for now, we just pretend like we have just our one
 * endpoint, the OTA Provider cluster, and no attributes (because we
 * would be erroring out from them anyway).
 */
uint16_t emberAfGetServerAttributeCount(EndpointId endpoint, ClusterId cluster)
{
    return 0;
}

uint16_t emberAfEndpointCount(void)
{
    return 2;
}

uint16_t emberAfIndexFromEndpoint(EndpointId endpoint)
{
    if (endpoint == kOtaProviderDynamicEndpointId)
    {
        return 0;
    }
    else if (endpoint == kWebRTCRequesterDynamicEndpointId)
    {
        return 1;
    }

    return UINT16_MAX;
}

EndpointId emberAfEndpointFromIndex(uint16_t index)
{
    if (index == 0)
    {
        return kOtaProviderDynamicEndpointId;
    }
    else if (index == 1)
    {
        return kWebRTCRequesterDynamicEndpointId;
    }

    return UINT16_MAX;
}

Optional<ClusterId> emberAfGetNthClusterId(EndpointId endpoint, uint8_t n, bool server)
{
    if (endpoint == kOtaProviderDynamicEndpointId && n == 0 && server)
    {
        return MakeOptional(OtaSoftwareUpdateProvider::Id);
    }
    else if (endpoint == kWebRTCRequesterDynamicEndpointId && n == 0 && server)
    {
        return MakeOptional(WebRTCTransportRequestor::Id);
    }

    return NullOptional;
}

uint16_t emberAfGetServerAttributeIndexByAttributeId(EndpointId endpoint, ClusterId cluster, AttributeId attributeId)
{
    return UINT16_MAX;
}

bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId)
{
    return false;
}

uint8_t emberAfClusterCount(EndpointId endpoint, bool server)
{
    if (endpoint == kOtaProviderDynamicEndpointId && server)
    {
        return 1;
    }

    if (endpoint == kWebRTCRequesterDynamicEndpointId && server)
    {
        return 1;
    }

    return 0;
}

uint8_t emberAfClusterCountForEndpointType(const EmberAfEndpointType * type, bool server)
{
    const EmberAfClusterMask cluster_mask = server ? MATTER_CLUSTER_FLAG_SERVER : MATTER_CLUSTER_FLAG_CLIENT;

    return static_cast<uint8_t>(std::count_if(type->cluster, type->cluster + type->clusterCount,
                                              [=](const EmberAfCluster & cluster) { return (cluster.mask & cluster_mask) != 0; }));
}

Optional<AttributeId> emberAfGetServerAttributeIdByIndex(EndpointId endpoint, ClusterId cluster, uint16_t attributeIndex)
{
    return NullOptional;
}

uint8_t emberAfClusterIndex(EndpointId endpoint, ClusterId clusterId, EmberAfClusterMask mask)
{
    if (endpoint == kOtaProviderDynamicEndpointId && clusterId == OtaSoftwareUpdateProvider::Id &&
        (mask & MATTER_CLUSTER_FLAG_SERVER))
    {
        return 0;
    }

    if (endpoint == kWebRTCRequesterDynamicEndpointId && clusterId == WebRTCTransportRequestor::Id &&
        (mask & MATTER_CLUSTER_FLAG_SERVER))
    {
        return 0;
    }

    return UINT8_MAX;
}

bool emberAfEndpointIndexIsEnabled(uint16_t index)
{
    return index == 0;
}

namespace {

const CommandId acceptedOtaProviderCommands[] = { Clusters::OtaSoftwareUpdateProvider::Commands::QueryImage::Id,
                                                  Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateRequest::Id,
                                                  Clusters::OtaSoftwareUpdateProvider::Commands::NotifyUpdateApplied::Id,
                                                  kInvalidCommandId };

const CommandId generatedOtaProviderCommands[] = { Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::Id,
                                                   Clusters::OtaSoftwareUpdateProvider::Commands::ApplyUpdateResponse::Id,
                                                   kInvalidCommandId };

const EmberAfCluster otaProviderCluster{
    .clusterId            = Clusters::OtaSoftwareUpdateProvider::Id,
    .attributes           = nullptr,
    .attributeCount       = 0,
    .clusterSize          = 0,
    .mask                 = MATTER_CLUSTER_FLAG_SERVER,
    .functions            = nullptr,
    .acceptedCommandList  = acceptedOtaProviderCommands,
    .generatedCommandList = generatedOtaProviderCommands,
    .eventList            = nullptr,
    .eventCount           = 0,
};

const EmberAfEndpointType otaProviderEndpoint{ .cluster = &otaProviderCluster, .clusterCount = 1, .endpointSize = 0 };

const CommandId acceptedWebRTCRequestorCommands[] = { Clusters::WebRTCTransportRequestor::Commands::Offer::Id,
                                                      Clusters::WebRTCTransportRequestor::Commands::Answer::Id,
                                                      Clusters::WebRTCTransportRequestor::Commands::ICECandidates::Id,
                                                      Clusters::WebRTCTransportRequestor::Commands::End::Id, kInvalidCommandId };

const CommandId generatedWebRTCRequestorCommands[] = { kInvalidCommandId };

const EmberAfCluster webRTCReqeustorCluster{
    .clusterId            = Clusters::WebRTCTransportRequestor::Id,
    .attributes           = nullptr,
    .attributeCount       = 0,
    .clusterSize          = 0,
    .mask                 = MATTER_CLUSTER_FLAG_SERVER,
    .functions            = nullptr,
    .acceptedCommandList  = acceptedWebRTCRequestorCommands,
    .generatedCommandList = generatedWebRTCRequestorCommands,
    .eventList            = nullptr,
    .eventCount           = 0,
};

const EmberAfEndpointType webRTCRequestorEndpoint{ .cluster = &webRTCReqeustorCluster, .clusterCount = 1, .endpointSize = 0 };

} // namespace

const EmberAfEndpointType * emberAfFindEndpointType(EndpointId endpoint)
{
    if (endpoint == kOtaProviderDynamicEndpointId)
    {
        return &otaProviderEndpoint;
    }
    else if (endpoint == kWebRTCRequesterDynamicEndpointId)
    {
        return &webRTCRequestorEndpoint;
    }

    return nullptr;
}

const EmberAfCluster * emberAfFindServerCluster(EndpointId endpoint, ClusterId cluster)
{
    if (endpoint == kOtaProviderDynamicEndpointId && cluster == Clusters::OtaSoftwareUpdateProvider::Id)
    {
        return &otaProviderCluster;
    }

    if (endpoint == kWebRTCRequesterDynamicEndpointId && cluster == Clusters::WebRTCTransportRequestor::Id)
    {
        return &webRTCReqeustorCluster;
    }

    return nullptr;
}

unsigned emberAfMetadataStructureGeneration()
{
    // DynamicDispatcher at this point hardcodes a single OTA provider cluster.
    // The structure does not change over time, so the current version stays at 0.
    return 0;
}

Protocols::InteractionModel::Status emberAfWriteAttribute(const ConcreteAttributePath & path, const EmberAfWriteDataInput & input)
{
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

Protocols::InteractionModel::Status emAfReadOrWriteAttribute(const EmberAfAttributeSearchRecord * attRecord,
                                                             const EmberAfAttributeMetadata ** metadata, uint8_t * buffer,
                                                             uint16_t readLength, bool write)
{
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

namespace chip {
namespace app {

EndpointComposition GetCompositionForEndpointIndex(uint16_t endpointIndex)
{
    return EndpointComposition::kFullFamily;
}

} // namespace app
} // namespace chip

EndpointId emberAfParentEndpointFromIndex(uint16_t index)
{
    return kInvalidEndpointId;
}

CHIP_ERROR GetSemanticTagForEndpointAtIndex(EndpointId endpoint, size_t index,
                                            Clusters::Descriptor::Structs::SemanticTagStruct::Type & tag)
{
    return CHIP_ERROR_NOT_FOUND;
}

void emberAfAttributeChanged(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId,
                             AttributesChangedListener * listener)
{
    gMockDataVersion++;
    listener->MarkDirty(AttributePathParams(endpoint, clusterId, attributeId));
}

void emberAfEndpointChanged(EndpointId endpoint, AttributesChangedListener * listener)
{
    listener->MarkDirty(AttributePathParams(endpoint));
}

DataVersion * emberAfDataVersionStorage(const ConcreteClusterPath & aConcreteClusterPath)
{
    return &gMockDataVersion;
}

Protocols::InteractionModel::Status emAfWriteAttributeExternal(const ConcreteAttributePath & path,
                                                               const EmberAfWriteDataInput & input)
{
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

Span<const EmberAfDeviceType> emberAfDeviceTypeListFromEndpointIndex(unsigned endpointIndex, CHIP_ERROR & err)
{
    err = CHIP_ERROR_NOT_IMPLEMENTED;
    return Span<const EmberAfDeviceType>();
}

const EmberAfCluster * emberAfFindClusterInType(const EmberAfEndpointType * endpointType, ClusterId clusterId,
                                                EmberAfClusterMask mask, uint8_t * index)
{
    if ((endpointType == &otaProviderEndpoint) && (clusterId == Clusters::OtaSoftwareUpdateProvider::Id))
    {
        return &otaProviderCluster;
    }

    if ((endpointType == &webRTCRequestorEndpoint) && (clusterId == Clusters::WebRTCTransportRequestor::Id))
    {
        return &webRTCReqeustorCluster;
    }

    return nullptr;
}

const EmberAfAttributeMetadata * emberAfLocateAttributeMetadata(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    // no known attributes even for OTA
    return nullptr;
}
