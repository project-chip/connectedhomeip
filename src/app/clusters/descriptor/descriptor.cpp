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

/****************************************************************************
 * @file
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

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
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Descriptor;
using namespace chip::app::Clusters::Descriptor::Attributes;

namespace {

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
        auto it = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetSemanticTags(endpoint);
        for (auto tag = it->Next(); tag.has_value(); tag = it->Next())
        {
            ReturnErrorOnFailure(encoder.Encode(tag.value()));
        }
        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR DescriptorAttrAccess::ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    if (endpoint == 0x00)
    {
        return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
            auto endpoints = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetEndpoints();
            for (auto id = endpoints->Next(); id.has_value(); id = endpoints->Next())
            {
                if (*id == 0)
                {
                    continue;
                }
                ReturnErrorOnFailure(encoder.Encode(*id));
            }
            return CHIP_NO_ERROR;
        });
    }

    auto endpointsIterator = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetEndpoints();

    VerifyOrReturnError(endpointsIterator->SeekTo(endpoint), CHIP_ERROR_KEY_NOT_FOUND);

    auto endpointInfo = endpointsIterator->GetMetadata();
    // if SEEK returns true, metadata MUST be valid
    VerifyOrReturnError(endpointInfo.has_value(), CHIP_ERROR_INTERNAL);

    if (endpointInfo->compositionPattern == DataModel::EndpointCompositionPattern::kFullFamily)
    {
        return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
            auto endpoints = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetEndpoints();

            for (auto id = endpoints->Next(); id.has_value(); id = endpoints->Next())
            {
                std::optional<DataModel::EndpointInfo> info = endpoints->GetMetadata();

                if (!info.has_value())
                {
                    // this is a bug probably
                    ChipLogError(InteractionModel, "Failed to fetch info for endpoint %d", *id);
                    break;
                }

                EndpointId parentEndpointId = info->parentId;
                while (parentEndpointId != chip::kInvalidEndpointId)
                {
                    if (parentEndpointId == endpoint)
                    {
                        ReturnErrorOnFailure(encoder.Encode(*id));
                        break;
                    }
                    //
                    auto endpoints2 = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetEndpoints();
                    if (!endpoints2->SeekTo(parentEndpointId))
                    {
                        // this is a logic error ...
                        ChipLogError(InteractionModel, "Failed to seek to parent endpoint %d", parentEndpointId);
                        break;
                    }
                    info = endpoints2->GetMetadata();
                    if (!info.has_value())
                    {
                        ChipLogError(InteractionModel, "Failed to fetch info for parent endpoint %d", parentEndpointId);
                        break;
                    }
                    parentEndpointId = info->parentId;
                }
            }

            return CHIP_NO_ERROR;
        });
    }

    if (endpointInfo->compositionPattern == DataModel::EndpointCompositionPattern::kTree)
    {
        return aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
            auto endpoints = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetEndpoints();
            for (auto id = endpoints->Next(); id.has_value(); id = endpoints->Next())
            {
                std::optional<DataModel::EndpointInfo> info = endpoints->GetMetadata();
                if (info.has_value() && info->parentId == endpoint)
                {
                    ReturnErrorOnFailure(encoder.Encode(*id));
                }
            }
            return CHIP_NO_ERROR;
        });
    }

    ChipLogError(InteractionModel, "Unknown composition pattern: %d", endpoint);
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR DescriptorAttrAccess::ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint](const auto & encoder) -> CHIP_ERROR {
        auto it = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetDeviceTypes(endpoint);
        for (auto deviceType = it->Next(); deviceType.has_value(); deviceType = it->Next())
        {
            Descriptor::Structs::DeviceTypeStruct::Type deviceStruct;
            deviceStruct.deviceType = deviceType->deviceTypeId;
            deviceStruct.revision   = deviceType->deviceTypeRevision;
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
            auto clusterEntry = InteractionModelEngine::GetInstance()->GetDataModelProvider()->FirstServerCluster(endpoint);
            while (clusterEntry.IsValid())
            {
                ReturnErrorOnFailure(encoder.Encode(clusterEntry.path.mClusterId));
                clusterEntry = InteractionModelEngine::GetInstance()->GetDataModelProvider()->NextServerCluster(clusterEntry.path);
            }
        }
        else
        {
            auto it = InteractionModelEngine::GetInstance()->GetDataModelProvider()->GetClientClusters(endpoint);

            for (auto clusterID = it->Next(); clusterID.has_value(); clusterID = it->Next())
            {
                ReturnErrorOnFailure(encoder.Encode(*clusterID));
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
