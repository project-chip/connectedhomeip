/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/****************************************************************************
 * @file
 * @brief Implementation for the Descriptor Server Cluster
 ***************************************************************************/

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
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
    static constexpr uint16_t ClusterRevision = 2;

    CHIP_ERROR ReadTagListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadClientServerAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder, bool server);
    CHIP_ERROR ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadFeatureMap(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

constexpr uint16_t DescriptorAttrAccess::ClusterRevision;

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
        Clusters::Descriptor::Structs::SemanticTagStruct::Type tag;
        size_t index   = 0;
        CHIP_ERROR err = CHIP_NO_ERROR;
        while ((err = GetSemanticTagForEndpointAtIndex(endpoint, index, tag)) == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(encoder.Encode(tag));
            index++;
        }
        if (err == CHIP_ERROR_NOT_FOUND)
        {
            return CHIP_NO_ERROR;
        }
        return err;
    });
}

CHIP_ERROR DescriptorAttrAccess::ReadPartsAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (endpoint == 0x00)
    {
        err = aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
            for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
            {
                if (emberAfEndpointIndexIsEnabled(index))
                {
                    EndpointId endpointId = emberAfEndpointFromIndex(index);
                    if (endpointId == 0)
                        continue;

                    ReturnErrorOnFailure(encoder.Encode(endpointId));
                }
            }

            return CHIP_NO_ERROR;
        });
    }
    else if (IsFlatCompositionForEndpoint(endpoint))
    {
        err = aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
            for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
            {
                if (!emberAfEndpointIndexIsEnabled(index))
                    continue;

                uint16_t childIndex = index;
                while (childIndex != chip::kInvalidListIndex)
                {
                    EndpointId parentEndpointId = emberAfParentEndpointFromIndex(childIndex);
                    if (parentEndpointId == chip::kInvalidEndpointId)
                        break;

                    if (parentEndpointId == endpoint)
                    {
                        ReturnErrorOnFailure(encoder.Encode(emberAfEndpointFromIndex(index)));
                        break;
                    }

                    childIndex = emberAfIndexFromEndpoint(parentEndpointId);
                }
            }

            return CHIP_NO_ERROR;
        });
    }
    else if (IsTreeCompositionForEndpoint(endpoint))
    {
        err = aEncoder.EncodeList([endpoint](const auto & encoder) -> CHIP_ERROR {
            for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
            {
                if (!emberAfEndpointIndexIsEnabled(index))
                    continue;

                EndpointId parentEndpointId = emberAfParentEndpointFromIndex(index);
                if (parentEndpointId == endpoint)
                {
                    ReturnErrorOnFailure(encoder.Encode(emberAfEndpointFromIndex(index)));
                }
            }

            return CHIP_NO_ERROR;
        });
    }

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadDeviceAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint](const auto & encoder) -> CHIP_ERROR {
        Descriptor::Structs::DeviceTypeStruct::Type deviceStruct;
        CHIP_ERROR err2;

        auto deviceTypeList = emberAfDeviceTypeListFromEndpoint(endpoint, err2);
        ReturnErrorOnFailure(err2);

        for (auto & deviceType : deviceTypeList)
        {
            deviceStruct.deviceType = deviceType.deviceId;
            deviceStruct.revision   = deviceType.deviceVersion;
            ReturnErrorOnFailure(encoder.Encode(deviceStruct));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadClientServerAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder, bool server)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint, server](const auto & encoder) -> CHIP_ERROR {
        uint8_t clusterCount = emberAfClusterCount(endpoint, server);

        for (uint8_t clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
        {
            const EmberAfCluster * cluster = emberAfGetNthCluster(endpoint, clusterIndex, server);
            ReturnErrorOnFailure(encoder.Encode(cluster->clusterId));
        }

        return CHIP_NO_ERROR;
    });

    return err;
}

CHIP_ERROR DescriptorAttrAccess::ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(ClusterRevision);
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
    registerAttributeAccessOverride(&gAttrAccess);
}
