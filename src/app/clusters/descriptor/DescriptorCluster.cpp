/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/descriptor/DescriptorCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Descriptor/Attributes.h>
#include <clusters/Descriptor/ClusterId.h>
#include <clusters/Descriptor/Metadata.h>
#include <clusters/Descriptor/Structs.h>
#include <clusters/shared/Structs.h>
#include <lib/core/DataModelTypes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor;
using namespace chip::app::Clusters::Descriptor::Attributes;
using chip::Protocols::InteractionModel::Status;

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

CHIP_ERROR ReadTagListAttribute(Span<const Clusters::Globals::Structs::SemanticTagStruct::Type> semanticTagsList,
                                EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([&semanticTagsList](const auto & encoder) -> CHIP_ERROR {
        for (const auto & tag : semanticTagsList)
        {
            ReturnErrorOnFailure(encoder.Encode(tag));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR ReadDeviceAttribute(DataModel::Provider & provider, EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> deviceTypesList;
    ReturnErrorOnFailure(provider.DeviceTypes(endpoint, deviceTypesList));

    auto deviceTypes = deviceTypesList.TakeBuffer();

    CHIP_ERROR err = aEncoder.EncodeList([&deviceTypes](const auto & encoder) -> CHIP_ERROR {
        for (const auto & type : deviceTypes)
        {
            Descriptor::Structs::DeviceTypeStruct::Type deviceStruct{ .deviceType = type.deviceTypeId,
                                                                      .revision   = type.deviceTypeRevision };
            ReturnErrorOnFailure(encoder.Encode(deviceStruct));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR ReadPartsAttribute(DataModel::Provider & provider, EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    ReadOnlyBufferBuilder<DataModel::EndpointEntry> endpointsList;
    ReturnErrorOnFailure(provider.Endpoints(endpointsList));
    auto endpoints = endpointsList.TakeBuffer();
    if (endpoint == kRootEndpointId)
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
        // encodes ALL endpoints that have the specified endpoint as a descendant.
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

} // namespace

namespace chip::app::Clusters {
CHIP_ERROR DescriptorCluster::Attributes(const ConcreteClusterPath & path,
                                         ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    AttributeListBuilder::OptionalAttributeEntry optionalAttributeEntries[] = {
        { !mSemanticTags.empty(), TagList::kMetadataEntry },
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
        { mEnabledOptionalAttributes.IsSet(EndpointUniqueID::Id), EndpointUniqueID::kMetadataEntry },
#endif
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributeEntries));
}

DataModel::ActionReturnStatus DescriptorCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                               AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id: {
        BitFlags<Descriptor::Feature> features;
        if (!mSemanticTags.empty())
        {
            features.Set(Descriptor::Feature::kTagList);
        }
        return encoder.Encode(features);
    }
    case ClusterRevision::Id:
        return encoder.Encode(Descriptor::kRevision);
    case DeviceTypeList::Id:
        return ReadDeviceAttribute(mContext->provider, request.path.mEndpointId, encoder);
    case ServerList::Id: {
        ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> builder;
        ReturnErrorOnFailure(mContext->provider.ServerClusters(request.path.mEndpointId, builder));
        ReadOnlyBuffer<DataModel::ServerClusterEntry> buffer = builder.TakeBuffer();
        return encoder.EncodeList([&buffer](const auto & itemEncoder) -> CHIP_ERROR {
            for (const auto & entry : buffer)
            {
                ReturnErrorOnFailure(itemEncoder.Encode(entry.clusterId));
            }
            return CHIP_NO_ERROR;
        });
    }
    case ClientList::Id: {
        ReadOnlyBufferBuilder<ClusterId> builder;
        ReturnErrorOnFailure(mContext->provider.ClientClusters(request.path.mEndpointId, builder));
        ReadOnlyBuffer<ClusterId> buffer = builder.TakeBuffer();
        return encoder.EncodeList([&buffer](const auto & itemEncoder) -> CHIP_ERROR {
            for (const auto & clusterId : buffer)
            {
                ReturnErrorOnFailure(itemEncoder.Encode(clusterId));
            }
            return CHIP_NO_ERROR;
        });
    }
    case PartsList::Id:
        return ReadPartsAttribute(mContext->provider, request.path.mEndpointId, encoder);
    case TagList::Id:
        return ReadTagListAttribute(mSemanticTags, request.path.mEndpointId, encoder);
#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
    case EndpointUniqueID::Id: {
        char buffer[EndpointUniqueID::TypeInfo::MaxLength()] = { 0 };
        MutableCharSpan epUniqueId(buffer);
        ReturnErrorOnFailure(mContext->provider.EndpointUniqueID(request.path.mEndpointId, epUniqueId));
        return encoder.Encode(epUniqueId);
    }
#endif
    default:
        return Status::UnsupportedAttribute;
    }
}

} // namespace chip::app::Clusters
