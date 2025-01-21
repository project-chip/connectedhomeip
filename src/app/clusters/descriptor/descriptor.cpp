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
#include <app/data-model-provider/MetadataTypes.h>
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
        for (auto & ep : allEndpoints)
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
    CHIP_ERROR ReadClientServerAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder, bool server);
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
    return aEncoder.EncodeList([&endpoint](const auto & encoder) -> CHIP_ERROR {
        auto tags = InteractionModelEngine::GetInstance()->GetDataModelProvider()->SemanticTags(endpoint);
        for (auto & tag : tags.GetSpanValidForLifetime())
        {
            ReturnErrorOnFailure(encoder.Encode(tag));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR DescriptorAttrAccess::ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    auto endpoints = InteractionModelEngine::GetInstance()->GetDataModelProvider()->Endpoints();
    if (endpoint == 0x00)
    {
        return aEncoder.EncodeList([&endpoints](const auto & encoder) -> CHIP_ERROR {
            for (auto & ep : endpoints.GetSpanValidForLifetime())
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
    while (idx < endpoints.Size())
    {
        if (endpoints[idx].id == endpoint)
        {
            break;
        }
        idx++;
    }
    if (idx >= endpoints.Size())
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
            for (auto & ep : endpoints.GetSpanValidForLifetime())
            {
                if (IsDescendantOf(&ep, endpoint, endpoints.GetSpanValidForLifetime()))
                {
                    ReturnErrorOnFailure(encoder.Encode(ep.id));
                }
            }
            return CHIP_NO_ERROR;
        });

    case DataModel::EndpointCompositionPattern::kTree:
        return aEncoder.EncodeList([&endpoints, endpoint](const auto & encoder) -> CHIP_ERROR {
            for (auto & ep : endpoints.GetSpanValidForLifetime())
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
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint](const auto & encoder) -> CHIP_ERROR {
        Descriptor::Structs::DeviceTypeStruct::Type deviceStruct;

        auto deviceTypes = InteractionModelEngine::GetInstance()->GetDataModelProvider()->DeviceTypes(endpoint);
        for (auto & type : deviceTypes.GetSpanValidForLifetime())
        {
            deviceStruct.deviceType = type.deviceTypeId;
            deviceStruct.revision   = type.deviceTypeRevision;
            ReturnErrorOnFailure(encoder.Encode(deviceStruct));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadClientServerAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder, bool server)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint, server](const auto & encoder) -> CHIP_ERROR {
        if (server)
        {
            auto clusters = InteractionModelEngine::GetInstance()->GetDataModelProvider()->ServerClusters(endpoint);
            for (auto & cluster : clusters.GetSpanValidForLifetime())
            {
                ReturnErrorOnFailure(encoder.Encode(cluster.clusterId));
            }
        }
        else
        {
            auto clusters = InteractionModelEngine::GetInstance()->GetDataModelProvider()->ClientClusters(endpoint);
            for (auto & id : clusters.GetSpanValidForLifetime())
            {
                ReturnErrorOnFailure(encoder.Encode(id));
            }
        }

        return CHIP_NO_ERROR;
    });

    return err;
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
        return ReadClientServerAttribute(aPath.mEndpointId, aEncoder, true);
    }
    case ClientList::Id: {
        return ReadClientServerAttribute(aPath.mEndpointId, aEncoder, false);
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
