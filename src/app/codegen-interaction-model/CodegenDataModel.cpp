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
#include <lib/core/DataModelTypes.h>
#include <optional>

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
    DataVersion * versionPtr = emberAfDataVersionStorage(path);
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
                                                       unsigned start_index, unsigned & found_index)
{
    for (unsigned cluster_idx = start_index; cluster_idx < endpoint->clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (!IsServerMask(cluster.mask & CLUSTER_MASK_SERVER))
        {
            continue;
        }

        found_index = cluster_idx;
        return ClusterEntryFrom(endpointId, cluster);
    }

    return InteractionModel::ClusterEntry::Invalid();
}

/// Load the attribute information into the specified destination
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

InteractionModel::AttributeEntry AttributeEntryForGlobalListAttribute(const ConcreteClusterPath & clusterPath,
                                                                      AttributeId globalAttributeId)
{
    InteractionModel::AttributeEntry entry;

    entry.path = ConcreteAttributePath(clusterPath.mEndpointId, clusterPath.mClusterId, globalAttributeId);
    entry.info.flags.Set(InteractionModel::AttributeQualityFlags::kListAttribute);

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

CHIP_ERROR CodegenDataModel::Invoke(const InteractionModel::InvokeRequest & request, TLV::TLVReader & input_arguments,
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
            mEndpointIterationHint = endpoint_idx;
            return emberAfEndpointFromIndex(endpoint_idx);
        }
    }

    // No enabled endpoint found. Give up
    return kInvalidEndpointId;
}

std::optional<unsigned> CodegenDataModel::TryFindEndpointIndex(EndpointId id) const
{
    const uint16_t lastEndpointIndex = emberAfEndpointCount();

    if ((mEndpointIterationHint < lastEndpointIndex) && emberAfEndpointIndexIsEnabled(mEndpointIterationHint) &&
        (id == emberAfEndpointFromIndex(mEndpointIterationHint)))
    {
        return std::make_optional(mEndpointIterationHint);
    }

    // Linear search, this may be slow
    uint16_t idx = emberAfIndexFromEndpoint(id);
    if (idx == kEmberInvalidEndpointIndex)
    {
        return std::nullopt;
    }

    return std::make_optional<unsigned>(idx);
}

EndpointId CodegenDataModel::NextEndpoint(EndpointId before)
{
    const unsigned lastEndpointIndex = emberAfEndpointCount();

    std::optional<unsigned> before_idx = TryFindEndpointIndex(before);
    if (!before_idx.has_value())
    {
        return kInvalidEndpointId;
    }

    // find the first enabled index
    for (uint16_t endpoint_idx = static_cast<uint16_t>(*before_idx + 1); endpoint_idx < lastEndpointIndex; endpoint_idx++)
    {
        if (emberAfEndpointIndexIsEnabled(endpoint_idx))
        {
            mEndpointIterationHint = endpoint_idx;
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

    return FirstServerClusterEntry(endpointId, endpoint, 0, mClusterIterationHint);
}

std::optional<unsigned> CodegenDataModel::TryFindServerClusterIndex(const EmberAfEndpointType * endpoint, ClusterId id) const
{
    const unsigned clusterCount = endpoint->clusterCount;

    if (mClusterIterationHint < clusterCount)
    {
        const EmberAfCluster & cluster = endpoint->cluster[mClusterIterationHint];
        if (IsServerMask(cluster.mask) && (cluster.clusterId == id))
        {
            return std::make_optional(mClusterIterationHint);
        }
    }

    // linear search, this may be slow
    // does NOT use emberAfClusterIndex to not iteratoe over endpoints as we have
    // already found the correct endpoint
    for (unsigned cluster_idx = 0; cluster_idx < clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (IsServerMask(cluster.mask) && (cluster.clusterId == id))
        {
            return std::make_optional(cluster_idx);
        }
    }

    return std::nullopt;
}

InteractionModel::ClusterEntry CodegenDataModel::NextCluster(const ConcreteClusterPath & before)
{
    // TODO: This search still seems slow (ember will loop). Should use index hints as long
    //       as ember API supports it
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(before.mEndpointId);

    VerifyOrReturnValue(endpoint != nullptr, InteractionModel::ClusterEntry::Invalid());
    VerifyOrReturnValue(endpoint->clusterCount > 0, InteractionModel::ClusterEntry::Invalid());
    VerifyOrReturnValue(endpoint->cluster != nullptr, InteractionModel::ClusterEntry::Invalid());

    std::optional<unsigned> cluster_idx = TryFindServerClusterIndex(endpoint, before.mClusterId);
    if (!cluster_idx.has_value())
    {
        return InteractionModel::ClusterEntry::Invalid();
    }

    return FirstServerClusterEntry(before.mEndpointId, endpoint, *cluster_idx + 1, mClusterIterationHint);
}

std::optional<InteractionModel::ClusterInfo> CodegenDataModel::GetClusterInfo(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);

    InteractionModel::ClusterInfo info;
    LoadClusterInfo(path, *cluster, &info);

    return std::make_optional(info);
}

InteractionModel::AttributeEntry CodegenDataModel::FirstAttribute(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributeCount > 0, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributes != nullptr, InteractionModel::AttributeEntry::Invalid());

    mAttributeIterationHint = 0;
    return AttributeEntryFrom(path, cluster->attributes[0]);
}

std::optional<unsigned> CodegenDataModel::TryFindAttributeIndex(const EmberAfCluster * cluster, AttributeId id) const
{
    const unsigned attributeCount = cluster->attributeCount;

    // attempt to find this based on the embedded hint
    if ((mAttributeIterationHint < attributeCount) && (cluster->attributes[mAttributeIterationHint].attributeId == id))
    {
        return std::make_optional(mAttributeIterationHint);
    }

    // linear search is required. This may be slow
    for (unsigned attribute_idx = 0; attribute_idx < attributeCount; attribute_idx++)
    {

        if (cluster->attributes[attribute_idx].attributeId == id)
        {
            return std::make_optional(attribute_idx);
        }
    }

    return std::nullopt;
}

const EmberAfCluster * CodegenDataModel::FindServerCluster(const ConcreteClusterPath & path)
{
    // cache things
    if (mPreviouslyFoundCluster.has_value() && (mPreviouslyFoundCluster->path == path))
    {
        return mPreviouslyFoundCluster->cluster;
    }

    const EmberAfCluster * cluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    if (cluster != nullptr)
    {
        mPreviouslyFoundCluster = std::make_optional<ClusterReference>(path, cluster);
    }
    return cluster;
}

InteractionModel::AttributeEntry CodegenDataModel::NextAttribute(const ConcreteAttributePath & before)
{
    const EmberAfCluster * cluster = FindServerCluster(before);
    VerifyOrReturnValue(cluster != nullptr, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributeCount > 0, InteractionModel::AttributeEntry::Invalid());
    VerifyOrReturnValue(cluster->attributes != nullptr, InteractionModel::AttributeEntry::Invalid());

    // Handles global attribute iteration: if we got a global attribute, move to the next
    switch (before.mAttributeId)
    {
    case Clusters::Globals::Attributes::GeneratedCommandList::Id:
        return AttributeEntryForGlobalListAttribute(before, Clusters::Globals::Attributes::AcceptedCommandList::Id);
    case Clusters::Globals::Attributes::AcceptedCommandList::Id:
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        return AttributeEntryForGlobalListAttribute(before, Clusters::Globals::Attributes::EventList::Id);
    case Clusters::Globals::Attributes::EventList::Id:
#endif // CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
        return AttributeEntryForGlobalListAttribute(before, Clusters::Globals::Attributes::AttributeList::Id);
    case Clusters::Globals::Attributes::AttributeList::Id:
        return InteractionModel::AttributeEntry::Invalid();
    default:
        // pass-through: not a global attribute, try to find a "regular" attribute
        break;
    }

    // find the given attribute in the list and then return the next one
    std::optional<unsigned> attribute_idx = TryFindAttributeIndex(cluster, before.mAttributeId);
    if (!attribute_idx.has_value())
    {
        return InteractionModel::AttributeEntry::Invalid();
    }

    unsigned next_idx = *attribute_idx + 1;
    if (next_idx < cluster->attributeCount)
    {
        mAttributeIterationHint = next_idx;
        return AttributeEntryFrom(before, cluster->attributes[next_idx]);
    }

    // We reach here if next_idx is just past the last attribute metadata. Return the first global
    // attribute
    return AttributeEntryForGlobalListAttribute(before, Clusters::Globals::Attributes::GeneratedCommandList::Id);
}

std::optional<InteractionModel::AttributeInfo> CodegenDataModel::GetAttributeInfo(const ConcreteAttributePath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);
    VerifyOrReturnValue(cluster->attributeCount > 0, std::nullopt);
    VerifyOrReturnValue(cluster->attributes != nullptr, std::nullopt);

    switch (path.mAttributeId)
    {
    case Clusters::Globals::Attributes::GeneratedCommandList::Id:
    case Clusters::Globals::Attributes::AcceptedCommandList::Id:
#if CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
    case Clusters::Globals::Attributes::EventList::Id:
#endif // CHIP_CONFIG_ENABLE_EVENTLIST_ATTRIBUTE
    case Clusters::Globals::Attributes::AttributeList::Id: {
        InteractionModel::AttributeInfo info;
        info.flags.Set(InteractionModel::AttributeQualityFlags::kListAttribute);
        return info;
    }
    default:
        // pass-through: not a global attribute, try to find a "regular" attribute
        break;
    }

    std::optional<unsigned> attribute_idx = TryFindAttributeIndex(cluster, path.mAttributeId);

    if (!attribute_idx.has_value())
    {
        return std::nullopt;
    }

    InteractionModel::AttributeInfo info;
    LoadAttributeInfo(path, cluster->attributes[*attribute_idx], &info);
    return std::make_optional(info);
}

InteractionModel::CommandEntry CodegenDataModel::FirstAcceptedCommand(const ConcreteClusterPath & cluster)
{
    // FIXME: implement
    return InteractionModel::CommandEntry::Invalid();
}

InteractionModel::CommandEntry CodegenDataModel::NextAcceptedCommand(const ConcreteCommandPath & before)
{
    // FIXME: implement
    return InteractionModel::CommandEntry::Invalid();
}

std::optional<InteractionModel::CommandInfo> CodegenDataModel::GetAcceptedCommandInfo(const ConcreteCommandPath & path)
{
    // FIXME: implement
    return std::nullopt;
}

ConcreteCommandPath CodegenDataModel::FirstGeneratedCommand(const ConcreteClusterPath & cluster)
{
    // FIXME: implement
    return ConcreteCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
}

ConcreteCommandPath CodegenDataModel::NextGeneratedCommand(const ConcreteCommandPath & before)
{
    // FIXME: implement
    return ConcreteCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
}

} // namespace app
} // namespace chip
