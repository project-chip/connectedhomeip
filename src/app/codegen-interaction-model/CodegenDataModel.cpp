/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <app/codegen-interaction-model/CodegenDataModel.h>

#include <app-common/zap-generated/attribute-type.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>

namespace chip {
namespace app {
namespace {

/// Checks if the specified ember cluster mask corresponds to a valid
/// server cluster.
bool IsServerMask(EmberAfClusterMask mask)
{
    return (mask == 0) || ((mask & CLUSTER_MASK_SERVER) != 0);
}

/// Load the cluster information into the specified destination
void LoadClusterInfo(const ConcreteClusterPath & path, const EmberAfCluster & cluster, InteractionModel::ClusterInfo * info)
{
    chip::DataVersion * versionPtr = emberAfDataVersionStorage(path);
    if (versionPtr != nullptr)
    {
        info->dataVersion = *versionPtr;
    }
    else
    {
        ChipLogError(AppServer, "Failed to get data version for %d/" ChipLogFormatMEI, static_cast<int>(path.mEndpointId),
                     ChipLogValueMEI(cluster.clusterId));
        info->dataVersion = 0;
    }

    // TODO: set entry flags:
    //   info->flags.Set(ClusterQualityFlags::kDiagnosticsData)
}

/// Converts a EmberAfCluster into a ClusterEntry
InteractionModel::ClusterEntry ClusterEntryFrom(EndpointId endpointId, const EmberAfCluster & cluster)
{
    InteractionModel::ClusterEntry entry;

    entry.path = ConcreteClusterPath(endpointId, cluster.clusterId);
    LoadClusterInfo(entry.path, cluster, &entry.info);

    return entry;
}

/// Finds the first server cluster entry for the given endpoint data starting at [start_index]
///
/// Returns an invalid entry if no more server clusters are found
InteractionModel::ClusterEntry FirstServerClusterEntry(EndpointId endpointId, const EmberAfEndpointType * endpoint,
                                                       uint16_t start_index)
{
    for (unsigned cluster_idx = start_index; cluster_idx < endpoint->clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (!IsServerMask(cluster.mask & CLUSTER_MASK_SERVER))
        {
            continue;
        }

        return ClusterEntryFrom(endpointId, cluster);
    }

    return InteractionModel::ClusterEntry::Invalid();
}

/// Load the cluster information into the specified destination
void LoadAttributeInfo(const ConcreteAttributePath & path, const EmberAfAttributeMetadata & attribute,
                       InteractionModel::AttributeInfo * info)
{
    if (attribute.attributeType == ZCL_ARRAY_ATTRIBUTE_TYPE)
    {
        info->flags.Set(InteractionModel::AttributeQualityFlags::kListAttribute);
    }

    // TODO: Set additional flags:
    // info->flags.Set(InteractionModel::AttributeQualityFlags::kChangesOmitted)
}

InteractionModel::AttributeEntry AttributeEntryFrom(const ConcreteClusterPath & clusterPath,
                                                    const EmberAfAttributeMetadata & attribute)
{
    InteractionModel::AttributeEntry entry;

    entry.path = ConcreteAttributePath(clusterPath.mEndpointId, clusterPath.mClusterId, attribute.attributeId);
    LoadAttributeInfo(entry.path, attribute, &entry.info);

    return entry;
}

} // namespace

CHIP_ERROR CodegenDataModel::ReadAttribute(const InteractionModel::ReadAttributeRequest & request,
                                           InteractionModel::ReadState & state, AttributeValueEncoder & encoder)
{
    // TODO: this needs an implementation
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CodegenDataModel::WriteAttribute(const InteractionModel::WriteAttributeRequest & request,
                                            AttributeValueDecoder & decoder)
{
    // TODO: this needs an implementation
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR CodegenDataModel::Invoke(const InteractionModel::InvokeRequest & request, chip::TLV::TLVReader & input_arguments,
                                    InteractionModel::InvokeReply & reply)
{
    // TODO: this needs an implementation
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

EndpointId CodegenDataModel::FirstEndpoint()
{
    // find the first enabled index
    const uint16_t lastEndpointIndex = emberAfEndpointCount();
    for (uint16_t endpoint_idx = 0; endpoint_idx < lastEndpointIndex; endpoint_idx++)
    {
        if (emberAfEndpointIndexIsEnabled(endpoint_idx))
        {
            return emberAfEndpointFromIndex(endpoint_idx);
        }
    }

    // No enabled endpoint found. Give up
    return kInvalidEndpointId;
}

EndpointId CodegenDataModel::NextEndpoint(EndpointId before)
{
    // find the first enabled index
    bool beforeFound = false;

    const uint16_t lastEndpointIndex = emberAfEndpointCount();
    for (uint16_t endpoint_idx = 0; endpoint_idx < lastEndpointIndex; endpoint_idx++)
    {
        if (!beforeFound)
        {
            beforeFound = (before == emberAfEndpointFromIndex(endpoint_idx));
            continue;
        }

        if (emberAfEndpointIndexIsEnabled(endpoint_idx))
        {
            return emberAfEndpointFromIndex(endpoint_idx);
        }
    }

    // No enabled enpoint after "before" was found, give up
    return kInvalidEndpointId;
}

InteractionModel::ClusterEntry CodegenDataModel::FirstCluster(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, InteractionModel::ClusterEntry::Invalid());
    VerifyOrReturnValue(endpoint->clusterCount > 0, InteractionModel::ClusterEntry::Invalid());
    VerifyOrReturnValue(endpoint->cluster != nullptr, InteractionModel::ClusterEntry::Invalid());

    return FirstServerClusterEntry(endpointId, endpoint, 0);
}

InteractionModel::ClusterEntry CodegenDataModel::NextCluster(const ConcreteClusterPath & before)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(before.mEndpointId);
    VerifyOrReturnValue(endpoint != nullptr, InteractionModel::ClusterEntry::Invalid());
    VerifyOrReturnValue(endpoint->clusterCount > 0, InteractionModel::ClusterEntry::Invalid());
    VerifyOrReturnValue(endpoint->cluster != nullptr, InteractionModel::ClusterEntry::Invalid());

    for (uint16_t cluster_idx = 0; cluster_idx < endpoint->clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (IsServerMask(cluster.mask) && (cluster.clusterId == before.mClusterId))
        {
            return FirstServerClusterEntry(before.mEndpointId, endpoint, cluster_idx + 1);
        }
    }

    return InteractionModel::ClusterEntry::Invalid();
}

std::optional<InteractionModel::ClusterInfo> CodegenDataModel::GetClusterInfo(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnValue(cluster != nullptr, std::nullopt);

    InteractionModel::ClusterInfo info;
    LoadClusterInfo(path, *cluster, &info);

    return std::make_optional(info);
}

InteractionModel::AttributeEntry CodegenDataModel::FirstAttribute(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnValue(cluster != nullptr, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributeCount > 0, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributes != nullptr, InteractionModel::AttributeEntry::Invalid());

    return AttributeEntryFrom(path, cluster->attributes[0]);
}

InteractionModel::AttributeEntry CodegenDataModel::NextAttribute(const ConcreteAttributePath & before)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(before.mEndpointId, before.mClusterId);
    VerifyOrReturnValue(cluster != nullptr, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributeCount > 0, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributes != nullptr, InteractionModel::AttributeEntry::Invalid());

    // find the given attribute in the list and then return the next one
    bool foundPosition            = false;
    const unsigned attributeCount = cluster->attributeCount;
    unsigned startIdx             = 0;

    // Attempt to use the hint
    if ((mAttributeIterationHint < attributeCount) &&
        (cluster->attributes[mAttributeIterationHint].attributeId == before.mAttributeId))
    {
        startIdx = mAttributeIterationHint;
    }

    for (unsigned attribute_idx = startIdx; attribute_idx < attributeCount; attribute_idx++)
    {
        if (foundPosition)
        {
            mAttributeIterationHint = attribute_idx;
            return AttributeEntryFrom(before, cluster->attributes[attribute_idx]);
        }

        foundPosition = (cluster->attributes[attribute_idx].attributeId == before.mAttributeId);
    }

    return InteractionModel::AttributeEntry::Invalid();
}

std::optional<InteractionModel::AttributeInfo> CodegenDataModel::GetAttributeInfo(const ConcreteAttributePath & path)
{
    const EmberAfCluster * cluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    VerifyOrReturnValue(cluster != nullptr, std::nullopt);
    VerifyOrReturnValue(cluster->attributeCount > 0, std::nullopt);
    VerifyOrReturnValue(cluster->attributes != nullptr, std::nullopt);
    const unsigned attributeCount = cluster->attributeCount;
    for (unsigned attribute_idx = 0; attribute_idx < attributeCount; attribute_idx++)
    {
        if (cluster->attributes[attribute_idx].attributeId == path.mAttributeId)
        {
            InteractionModel::AttributeInfo info;
            LoadAttributeInfo(path, cluster->attributes[attribute_idx], &info);
            return std::make_optional(info);
        }
    }

    return std::nullopt;
}

} // namespace app
} // namespace chip
