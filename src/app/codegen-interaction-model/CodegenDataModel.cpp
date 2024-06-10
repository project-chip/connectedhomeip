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
#include <app/RequiredPrivilege.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/DataModelTypes.h>

#include <optional>
#include <variant>

namespace chip {
namespace app {
namespace {

/// Load the cluster information into the specified destination
std::variant<CHIP_ERROR, InteractionModel::ClusterInfo> LoadClusterInfo(const ConcreteClusterPath & path,
                                                                        const EmberAfCluster & cluster)
{
    DataVersion * versionPtr = emberAfDataVersionStorage(path);
    if (versionPtr == nullptr)
    {
        ChipLogError(AppServer, "Failed to get data version for %d/" ChipLogFormatMEI, static_cast<int>(path.mEndpointId),
                     ChipLogValueMEI(cluster.clusterId));
        return CHIP_ERROR_NOT_FOUND;
    }

    InteractionModel::ClusterInfo info(*versionPtr);

    // TODO: set entry flags:
    //   info->flags.Set(ClusterQualityFlags::kDiagnosticsData)

    return info;
}

/// Converts a EmberAfCluster into a ClusterEntry
std::variant<CHIP_ERROR, InteractionModel::ClusterEntry> ClusterEntryFrom(EndpointId endpointId, const EmberAfCluster & cluster)
{
    ConcreteClusterPath clusterPath(endpointId, cluster.clusterId);
    auto info = LoadClusterInfo(clusterPath, cluster);

    if (CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&info))
    {
        return *err;
    }

    if (InteractionModel::ClusterInfo * infoValue = std::get_if<InteractionModel::ClusterInfo>(&info))
    {
        return InteractionModel::ClusterEntry{
            .path = clusterPath,
            .info = *infoValue,
        };
    }
    return CHIP_ERROR_INCORRECT_STATE;
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
        if (!cluster.IsServerMask())
        {
            continue;
        }

        found_index = cluster_idx;
        auto entry  = ClusterEntryFrom(endpointId, cluster);

        if (InteractionModel::ClusterEntry * entryValue = std::get_if<InteractionModel::ClusterEntry>(&entry))
        {
            return *entryValue;
        }

#if CHIP_ERROR_LOGGING
        if (CHIP_ERROR * errValue = std::get_if<CHIP_ERROR>(&entry))
        {
            ChipLogError(AppServer, "Failed to load cluster entry: %" CHIP_ERROR_FORMAT, errValue->Format());
        }
        else
        {
            // Should NOT be possible: entryFrom has only 2 variants
            ChipLogError(AppServer, "Failed to load cluster entry, UNKNOWN entry return type");
        }
#endif
    }

    return InteractionModel::ClusterEntry::kInvalid;
}

/// Load the attribute information into the specified destination
///
/// `info` is assumed to be default-constructed/clear (i.e. this sets flags, but does not reset them).
void LoadAttributeInfo(const ConcreteAttributePath & path, const EmberAfAttributeMetadata & attribute,
                       InteractionModel::AttributeInfo * info)
{
    info->readPrivilege = RequiredPrivilege::ForReadAttribute(path);
    if (attribute.IsReadOnly())
    {
        info->writePrivilege = RequiredPrivilege::ForWriteAttribute(path);
    }

    info->flags.Set(InteractionModel::AttributeQualityFlags::kListAttribute, (attribute.attributeType == ZCL_ARRAY_ATTRIBUTE_TYPE));
    info->flags.Set(InteractionModel::AttributeQualityFlags::kTimed, attribute.MustUseTimedWrite());

    // NOTE: we do NOT provide additional info for:
    //    - IsExternal/IsSingleton/IsAutomaticallyPersisted is not used by IM handling
    //    - IsSingleton spec defines it for CLUSTERS where as we have it for ATTRIBUTES
    //    - Several specification flags are not available (reportable, quieter reporting,
    //      fixed, source attribution)

    // TODO: Set additional flags:
    // info->flags.Set(InteractionModel::AttributeQualityFlags::kFabricScoped)
    // info->flags.Set(InteractionModel::AttributeQualityFlags::kFabricSensitive)
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

InteractionModel::CommandEntry CommandEntryFrom(const ConcreteClusterPath & clusterPath, CommandId clusterCommandId)
{
    InteractionModel::CommandEntry entry;
    entry.path                 = ConcreteCommandPath(clusterPath.mEndpointId, clusterPath.mClusterId, clusterCommandId);
    entry.info.invokePrivilege = RequiredPrivilege::ForInvokeCommand(entry.path);

    entry.info.flags.Set(InteractionModel::CommandQualityFlags::kTimed,
                         CommandNeedsTimedInvoke(clusterPath.mClusterId, clusterCommandId));

    entry.info.flags.Set(InteractionModel::CommandQualityFlags::kFabricScoped,
                         CommandIsFabricScoped(clusterPath.mClusterId, clusterCommandId));

    // TODO: Set additional flags:
    // entry.info.flags.Set(InteractionModel::CommandQualityFlags::kFabricSensitive)

    return entry;
}

const ConcreteCommandPath kInvalidCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);

} // namespace

std::optional<CommandId> CodegenDataModel::EmberCommandListIterator::First(const CommandId * list)
{
    VerifyOrReturnValue(list != nullptr, std::nullopt);
    mCurrentList = mCurrentHint = list;

    VerifyOrReturnValue(*mCurrentList != kInvalidCommandId, std::nullopt);
    return *mCurrentList;
}

std::optional<CommandId> CodegenDataModel::EmberCommandListIterator::Next(const CommandId * list, CommandId previousId)
{
    VerifyOrReturnValue(list != nullptr, std::nullopt);
    VerifyOrReturnValue(previousId != kInvalidCommandId, std::nullopt);

    if (mCurrentList != list)
    {
        // invalidate the hint if switching lists...
        mCurrentHint = nullptr;
        mCurrentList = list;
    }

    if ((mCurrentHint == nullptr) || (*mCurrentHint != previousId))
    {
        // we did not find a usable hint. Search from the to set the hint
        mCurrentHint = mCurrentList;
        while ((*mCurrentHint != kInvalidCommandId) && (*mCurrentHint != previousId))
        {
            mCurrentHint++;
        }
    }

    VerifyOrReturnValue(*mCurrentHint == previousId, std::nullopt);

    // hint is valid and can be used immediately
    mCurrentHint++; // this is the next value
    return (*mCurrentHint == kInvalidCommandId) ? std::nullopt : std::make_optional(*mCurrentHint);
}

bool CodegenDataModel::EmberCommandListIterator::Exists(const CommandId * list, CommandId toCheck)
{
    VerifyOrReturnValue(list != nullptr, false);
    VerifyOrReturnValue(toCheck != kInvalidCommandId, false);

    if (mCurrentList != list)
    {
        // invalidate the hint if switching lists...
        mCurrentHint = nullptr;
        mCurrentList = list;
    }

    // maybe already positioned correctly
    if ((mCurrentHint != nullptr) && (*mCurrentHint == toCheck))
    {
        return true;
    }

    // move and try to find it
    mCurrentHint = mCurrentList;
    while ((*mCurrentHint != kInvalidCommandId) && (*mCurrentHint != toCheck))
    {
        mCurrentHint++;
    }

    return (*mCurrentHint == toCheck);
}

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
    VerifyOrReturnValue(endpoint != nullptr, InteractionModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->clusterCount > 0, InteractionModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->cluster != nullptr, InteractionModel::ClusterEntry::kInvalid);

    return FirstServerClusterEntry(endpointId, endpoint, 0, mClusterIterationHint);
}

std::optional<unsigned> CodegenDataModel::TryFindServerClusterIndex(const EmberAfEndpointType * endpoint, ClusterId id) const
{
    const unsigned clusterCount = endpoint->clusterCount;

    if (mClusterIterationHint < clusterCount)
    {
        const EmberAfCluster & cluster = endpoint->cluster[mClusterIterationHint];
        if (cluster.IsServerMask() && (cluster.clusterId == id))
        {
            return std::make_optional(mClusterIterationHint);
        }
    }

    // linear search, this may be slow
    // does NOT use emberAfClusterIndex to not iterate over endpoints as we have
    // already found the correct endpoint
    for (unsigned cluster_idx = 0; cluster_idx < clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (cluster.IsServerMask() && (cluster.clusterId == id))
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

    VerifyOrReturnValue(endpoint != nullptr, InteractionModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->clusterCount > 0, InteractionModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->cluster != nullptr, InteractionModel::ClusterEntry::kInvalid);

    std::optional<unsigned> cluster_idx = TryFindServerClusterIndex(endpoint, before.mClusterId);
    if (!cluster_idx.has_value())
    {
        return InteractionModel::ClusterEntry::kInvalid;
    }

    return FirstServerClusterEntry(before.mEndpointId, endpoint, *cluster_idx + 1, mClusterIterationHint);
}

std::optional<InteractionModel::ClusterInfo> CodegenDataModel::GetClusterInfo(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);

    auto info = LoadClusterInfo(path, *cluster);

    if (CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&info))
    {
#if CHIP_ERROR_LOGGING
        ChipLogError(AppServer, "Failed to load cluster info: %" CHIP_ERROR_FORMAT, err->Format());
#else
        (void) err->Format();
#endif
        return std::nullopt;
    }

    return std::make_optional(std::get<InteractionModel::ClusterInfo>(info));
}

InteractionModel::AttributeEntry CodegenDataModel::FirstAttribute(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, InteractionModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributeCount > 0, InteractionModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributes != nullptr, InteractionModel::AttributeEntry::kInvalid);

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
    VerifyOrReturnValue(cluster != nullptr, InteractionModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributeCount > 0, InteractionModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributes != nullptr, InteractionModel::AttributeEntry::kInvalid);

    // find the given attribute in the list and then return the next one
    std::optional<unsigned> attribute_idx = TryFindAttributeIndex(cluster, before.mAttributeId);
    if (!attribute_idx.has_value())
    {
        return InteractionModel::AttributeEntry::kInvalid;
    }

    unsigned next_idx = *attribute_idx + 1;
    if (next_idx < cluster->attributeCount)
    {
        mAttributeIterationHint = next_idx;
        return AttributeEntryFrom(before, cluster->attributes[next_idx]);
    }

    // iteration complete
    return InteractionModel::AttributeEntry::kInvalid;
}

std::optional<InteractionModel::AttributeInfo> CodegenDataModel::GetAttributeInfo(const ConcreteAttributePath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);
    VerifyOrReturnValue(cluster->attributeCount > 0, std::nullopt);
    VerifyOrReturnValue(cluster->attributes != nullptr, std::nullopt);

    std::optional<unsigned> attribute_idx = TryFindAttributeIndex(cluster, path.mAttributeId);

    if (!attribute_idx.has_value())
    {
        return std::nullopt;
    }

    InteractionModel::AttributeInfo info;
    LoadAttributeInfo(path, cluster->attributes[*attribute_idx], &info);
    return std::make_optional(info);
}

InteractionModel::CommandEntry CodegenDataModel::FirstAcceptedCommand(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, InteractionModel::CommandEntry::kInvalid);

    std::optional<CommandId> commandId = mAcceptedCommandsIterator.First(cluster->acceptedCommandList);
    VerifyOrReturnValue(commandId.has_value(), InteractionModel::CommandEntry::kInvalid);

    return CommandEntryFrom(path, *commandId);
}

InteractionModel::CommandEntry CodegenDataModel::NextAcceptedCommand(const ConcreteCommandPath & before)
{
    const EmberAfCluster * cluster = FindServerCluster(before);

    VerifyOrReturnValue(cluster != nullptr, InteractionModel::CommandEntry::kInvalid);

    std::optional<CommandId> commandId = mAcceptedCommandsIterator.Next(cluster->acceptedCommandList, before.mCommandId);
    VerifyOrReturnValue(commandId.has_value(), InteractionModel::CommandEntry::kInvalid);

    return CommandEntryFrom(before, *commandId);
}

std::optional<InteractionModel::CommandInfo> CodegenDataModel::GetAcceptedCommandInfo(const ConcreteCommandPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);
    VerifyOrReturnValue(mAcceptedCommandsIterator.Exists(cluster->acceptedCommandList, path.mCommandId), std::nullopt);

    return CommandEntryFrom(path, path.mCommandId).info;
}

ConcreteCommandPath CodegenDataModel::FirstGeneratedCommand(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, kInvalidCommandPath);

    std::optional<CommandId> commandId = mGeneratedCommandsIterator.First(cluster->generatedCommandList);
    VerifyOrReturnValue(commandId.has_value(), kInvalidCommandPath);
    return ConcreteCommandPath(path.mEndpointId, path.mClusterId, *commandId);
}

ConcreteCommandPath CodegenDataModel::NextGeneratedCommand(const ConcreteCommandPath & before)
{
    const EmberAfCluster * cluster = FindServerCluster(before);

    VerifyOrReturnValue(cluster != nullptr, kInvalidCommandPath);

    std::optional<CommandId> commandId = mGeneratedCommandsIterator.Next(cluster->generatedCommandList, before.mCommandId);
    VerifyOrReturnValue(commandId.has_value(), kInvalidCommandPath);

    return ConcreteCommandPath(before.mEndpointId, before.mClusterId, *commandId);
}

} // namespace app
} // namespace chip
