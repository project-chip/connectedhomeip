/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "descriptor.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model/List.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor;
using namespace chip::app::Clusters::Descriptor::Attributes;

namespace {

/// Figures out if `childId` is a descendant of `parentId` given some specific endpoint entries
bool IsDescendantOf(const DataModel::EndpointEntry * __restrict__ childEndpoint, const EndpointId parentId,
                    Span<const DataModel::EndpointEntry> allEndpoints)
{
    // NOTE: this is not very efficient as we loop through all endpoints for each parent search
    //       however endpoint depth should not be as large.
    while (true)
    {

        VerifyOrReturnValue(childEndpoint != nullptr, false);
        VerifyOrReturnValue(childEndpoint->parentId != parentId, true);

        // Parent endpoint id 0 is never here: EndpointEntry::parentId uses
        // kInvalidEndpointId to reference no explicit endpoint. See `EndpointEntry`
        // comments.
        VerifyOrReturnValue(childEndpoint->parentId != kInvalidEndpointId, false);

        const auto lookupId = childEndpoint->parentId;
        childEndpoint       = nullptr; // we will look it up again

        // find the requested value in the array to get its parent
        for (const auto & ep : allEndpoints)
        {
            if (ep.id == lookupId)
            {
                childEndpoint = &ep;
                break;
            }
        }
    }
}

class DescriptorAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Descriptor cluster on all endpoints.
    DescriptorAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Descriptor::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadTagListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadClientClusters(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadServerClusters(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadFeatureMap(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR DescriptorAttrAccess::ReadFeatureMap(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    Clusters::Descriptor::Structs::SemanticTagStruct::Type tag;
    size_t index = 0;
    BitFlags<Feature> featureFlags;

    if (GetSemanticTagForEndpointAtIndex(endpoint, index, tag) == CHIP_NO_ERROR)
    {
        featureFlags.Set(Feature::kTagList);
    }
    return aEncoder.Encode(featureFlags);
}

CHIP_ERROR DescriptorAttrAccess::ReadTagListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::ListBuilder<DataModel::Provider::SemanticTag> semanticTagsList;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->SemanticTags(endpoint, semanticTagsList));

    return aEncoder.EncodeList([&semanticTagsList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & tag : semanticTagsList.TakeBuffer())
        {
            ReturnErrorOnFailure(encoder.Encode(tag));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR DescriptorAttrAccess::ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::ListBuilder<DataModel::EndpointEntry> endpointsList;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->Endpoints(endpointsList));
    auto endpoints = endpointsList.TakeBuffer();
    if (endpoint == 0x00)
    {
        return aEncoder.EncodeList([&endpoints](const auto & encoder) -> CHIP_ERROR {
            for (const auto & ep : endpoints)
            {
                if (ep.id == 0)
                {
                    continue;
                }
                ReturnErrorOnFailure(encoder.Encode(ep.id));
            }
            return CHIP_NO_ERROR;
        });
    }

    // find the given endpoint
    unsigned idx = 0;
    while (idx < endpoints.size())
    {
        if (endpoints[idx].id == endpoint)
        {
            break;
        }
        idx++;
    }
    if (idx >= endpoints.size())
    {
        // not found
        return CHIP_ERROR_NOT_FOUND;
    }

    auto & endpointInfo = endpoints[idx];

    switch (endpointInfo.compositionPattern)
    {
    case DataModel::EndpointCompositionPattern::kFullFamily:
        // encodes ALL endpoints that have the specified endpoint as a descendant
        return aEncoder.EncodeList([&endpoints, endpoint](const auto & encoder) -> CHIP_ERROR {
            for (const auto & ep : endpoints)
            {
                if (IsDescendantOf(&ep, endpoint, endpoints))
                {
                    ReturnErrorOnFailure(encoder.Encode(ep.id));
                }
            }
            return CHIP_NO_ERROR;
        });

    case DataModel::EndpointCompositionPattern::kTree:
        return aEncoder.EncodeList([&endpoints, endpoint](const auto & encoder) -> CHIP_ERROR {
            for (const auto & ep : endpoints)
            {
                if (ep.parentId != endpoint)
                {
                    continue;
                }
                ReturnErrorOnFailure(encoder.Encode(ep.id));
            }
            return CHIP_NO_ERROR;
        });
    }
    // not actually reachable and compiler will validate we
    // handle all switch cases above
    return CHIP_NO_ERROR;
}

CHIP_ERROR DescriptorAttrAccess::ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::ListBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->DeviceTypes(endpoint, deviceTypesList));

    auto deviceTypes = deviceTypesList.TakeBuffer();

    CHIP_ERROR err = aEncoder.EncodeList([&deviceTypes](const auto & encoder) -> CHIP_ERROR {
        Descriptor::Structs::DeviceTypeStruct::Type deviceStruct;
        for (const auto & type : deviceTypes)
        {
            deviceStruct.deviceType = type.deviceTypeId;
            deviceStruct.revision   = type.deviceTypeRevision;
            ReturnErrorOnFailure(encoder.Encode(deviceStruct));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadServerClusters(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::ListBuilder<DataModel::ServerClusterEntry> builder;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->ServerClusters(endpoint, builder));
    return aEncoder.EncodeList([&builder](const auto & encoder) -> CHIP_ERROR {
        for (const auto & cluster : builder.TakeBuffer())
        {
            ReturnErrorOnFailure(encoder.Encode(cluster.clusterId));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR DescriptorAttrAccess::ReadClientClusters(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::ListBuilder<ClusterId> clusterIdList;
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->GetDataModelProvider()->ClientClusters(endpoint, clusterIdList));
    return aEncoder.EncodeList([&clusterIdList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & id : clusterIdList.TakeBuffer())
        {
            ReturnErrorOnFailure(encoder.Encode(id));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR DescriptorAttrAccess::ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(kClusterRevision);
}

DescriptorAttrAccess gAttrAccess;

CHIP_ERROR DescriptorAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Descriptor::Id);

    switch (aPath.mAttributeId)
    {
    case DeviceTypeList::Id: {
        return ReadDeviceAttribute(aPath.mEndpointId, aEncoder);
    }
    case ServerList::Id: {
        return ReadServerClusters(aPath.mEndpointId, aEncoder);
    }
    case ClientList::Id: {
        return ReadClientClusters(aPath.mEndpointId, aEncoder);
    }
    case PartsList::Id: {
        return ReadPartsAttribute(aPath.mEndpointId, aEncoder);
    }
    case TagList::Id: {
        return ReadTagListAttribute(aPath.mEndpointId, aEncoder);
    }
    case ClusterRevision::Id: {
        return ReadClusterRevision(aPath.mEndpointId, aEncoder);
    }
    case FeatureMap::Id: {
        return ReadFeatureMap(aPath.mEndpointId, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

} // anonymous namespace

void MatterDescriptorPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
