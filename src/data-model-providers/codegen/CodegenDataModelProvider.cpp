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
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

#include <optional>
#include <variant>

namespace chip {
namespace app {

namespace {

/// Load the cluster information into the specified destination
std::variant<CHIP_ERROR, DataModel::ClusterInfo> LoadClusterInfo(const ConcreteClusterPath & path, const EmberAfCluster & cluster)
{
    DataVersion * versionPtr = emberAfDataVersionStorage(path);
    if (versionPtr == nullptr)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to get data version for %d/" ChipLogFormatMEI, static_cast<int>(path.mEndpointId),
                     ChipLogValueMEI(cluster.clusterId));
#endif
        return CHIP_ERROR_NOT_FOUND;
    }

    DataModel::ClusterInfo info(*versionPtr);
    // TODO: set entry flags:
    //   info->flags.Set(ClusterQualityFlags::kDiagnosticsData)
    return info;
}

/// Converts a EmberAfCluster into a ClusterEntry
std::variant<CHIP_ERROR, DataModel::ClusterEntry> ClusterEntryFrom(EndpointId endpointId, const EmberAfCluster & cluster)
{
    ConcreteClusterPath clusterPath(endpointId, cluster.clusterId);
    auto info = LoadClusterInfo(clusterPath, cluster);

    if (CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&info))
    {
        return *err;
    }

    if (DataModel::ClusterInfo * infoValue = std::get_if<DataModel::ClusterInfo>(&info))
    {
        return DataModel::ClusterEntry{
            .path = clusterPath,
            .info = *infoValue,
        };
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

/// Finds the first server cluster entry for the given endpoint data starting at [start_index]
///
/// Returns an invalid entry if no more server clusters are found
DataModel::ClusterEntry FirstServerClusterEntry(EndpointId endpointId, const EmberAfEndpointType * endpoint, unsigned start_index,
                                                unsigned & found_index)
{
    for (unsigned cluster_idx = start_index; cluster_idx < endpoint->clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (!cluster.IsServer())
        {
            continue;
        }

        found_index = cluster_idx;
        auto entry  = ClusterEntryFrom(endpointId, cluster);

        if (DataModel::ClusterEntry * entryValue = std::get_if<DataModel::ClusterEntry>(&entry))
        {
            return *entryValue;
        }

#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
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

    return DataModel::ClusterEntry::kInvalid;
}

ClusterId FirstClientClusterId(const EmberAfEndpointType * endpoint, unsigned start_index, unsigned & found_index)
{
    for (unsigned cluster_idx = start_index; cluster_idx < endpoint->clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (!cluster.IsClient())
        {
            continue;
        }

        found_index = cluster_idx;
        return cluster.clusterId;
    }

    return kInvalidClusterId;
}

/// Load the attribute information into the specified destination
///
/// `info` is assumed to be default-constructed/clear (i.e. this sets flags, but does not reset them).
void LoadAttributeInfo(const ConcreteAttributePath & path, const EmberAfAttributeMetadata & attribute,
                       DataModel::AttributeInfo * info)
{
    info->readPrivilege = RequiredPrivilege::ForReadAttribute(path);
    if (!attribute.IsReadOnly())
    {
        info->writePrivilege = RequiredPrivilege::ForWriteAttribute(path);
    }

    info->flags.Set(DataModel::AttributeQualityFlags::kListAttribute, (attribute.attributeType == ZCL_ARRAY_ATTRIBUTE_TYPE));
    info->flags.Set(DataModel::AttributeQualityFlags::kTimed, attribute.MustUseTimedWrite());

    // NOTE: we do NOT provide additional info for:
    //    - IsExternal/IsSingleton/IsAutomaticallyPersisted is not used by IM handling
    //    - IsSingleton spec defines it for CLUSTERS where as we have it for ATTRIBUTES
    //    - Several specification flags are not available (reportable, quieter reporting,
    //      fixed, source attribution)

    // TODO: Set additional flags:
    // info->flags.Set(DataModel::AttributeQualityFlags::kFabricScoped)
    // info->flags.Set(DataModel::AttributeQualityFlags::kFabricSensitive)
    // info->flags.Set(DataModel::AttributeQualityFlags::kChangesOmitted)
}

DataModel::AttributeEntry AttributeEntryFrom(const ConcreteClusterPath & clusterPath, const EmberAfAttributeMetadata & attribute)
{
    DataModel::AttributeEntry entry;

    entry.path = ConcreteAttributePath(clusterPath.mEndpointId, clusterPath.mClusterId, attribute.attributeId);
    LoadAttributeInfo(entry.path, attribute, &entry.info);

    return entry;
}

DataModel::CommandEntry CommandEntryFrom(const ConcreteClusterPath & clusterPath, CommandId clusterCommandId)
{
    DataModel::CommandEntry entry;
    entry.path                 = ConcreteCommandPath(clusterPath.mEndpointId, clusterPath.mClusterId, clusterCommandId);
    entry.info.invokePrivilege = RequiredPrivilege::ForInvokeCommand(entry.path);

    entry.info.flags.Set(DataModel::CommandQualityFlags::kTimed, CommandNeedsTimedInvoke(clusterPath.mClusterId, clusterCommandId));

    entry.info.flags.Set(DataModel::CommandQualityFlags::kFabricScoped,
                         CommandIsFabricScoped(clusterPath.mClusterId, clusterCommandId));

    entry.info.flags.Set(DataModel::CommandQualityFlags::kLargeMessage,
                         CommandHasLargePayload(clusterPath.mClusterId, clusterCommandId));
    return entry;
}

// TODO: DeviceTypeEntry content is IDENTICAL to EmberAfDeviceType, so centralizing
//       to a common type is probably better. Need to figure out dependencies since
//       this would make ember return datamodel-provider types.
//       See: https://github.com/project-chip/connectedhomeip/issues/35889
DataModel::DeviceTypeEntry DeviceTypeEntryFromEmber(const EmberAfDeviceType & other)
{
    DataModel::DeviceTypeEntry entry;

    entry.deviceTypeId       = other.deviceId;
    entry.deviceTypeRevision = other.deviceVersion;

    return entry;
}

// Explicitly compare for identical entries. note that types are different,
// so you must do `a == b` and the `b == a` will not work.
bool operator==(const DataModel::DeviceTypeEntry & a, const EmberAfDeviceType & b)
{
    return (a.deviceTypeId == b.deviceId) && (a.deviceTypeRevision == b.deviceVersion);
}

/// Find the `index` where one of the following holds:
///    - types[index - 1] == previous OR
///    - index == types.size()  // i.e. not found or there is no next
///
/// hintWherePreviousMayBe represents a search hint where previous may exist.
unsigned FindNextDeviceTypeIndex(Span<const EmberAfDeviceType> types, const DataModel::DeviceTypeEntry & previous,
                                 unsigned hintWherePreviousMayBe)
{
    if (hintWherePreviousMayBe < types.size())
    {
        // this is a valid hint ... see if we are lucky
        if (previous == types[hintWherePreviousMayBe])
        {
            return hintWherePreviousMayBe + 1; // return the next index
        }
    }

    // hint was not useful. We have to do a full search
    for (unsigned idx = 0; idx < types.size(); idx++)
    {
        if (previous == types[idx])
        {
            return idx + 1;
        }
    }

    // cast should be safe as we know we do not have that many types
    return static_cast<unsigned>(types.size());
}

const ConcreteCommandPath kInvalidCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);

std::optional<DataModel::EndpointInfo> GetEndpointInfoAtIndex(uint16_t endpointIndex)
{
    VerifyOrReturnValue(emberAfEndpointIndexIsEnabled(endpointIndex), std::nullopt);
    EndpointId parent = emberAfParentEndpointFromIndex(endpointIndex);
    if (GetCompositionForEndpointIndex(endpointIndex) == EndpointComposition::kFullFamily)
    {
        return DataModel::EndpointInfo(parent, DataModel::EndpointCompositionPattern::kFullFamily);
    }
    if (GetCompositionForEndpointIndex(endpointIndex) == EndpointComposition::kTree)
    {
        return DataModel::EndpointInfo(parent, DataModel::EndpointCompositionPattern::kTree);
    }
    return std::nullopt;
}

DataModel::EndpointEntry FirstEndpointEntry(unsigned start_index, uint16_t & found_index)
{
    // find the first enabled index after the start index
    const uint16_t lastEndpointIndex = emberAfEndpointCount();
    for (uint16_t endpoint_idx = static_cast<uint16_t>(start_index); endpoint_idx < lastEndpointIndex; endpoint_idx++)
    {
        if (emberAfEndpointIndexIsEnabled(endpoint_idx))
        {
            found_index                            = endpoint_idx;
            DataModel::EndpointEntry endpointEntry = DataModel::EndpointEntry::kInvalid;
            endpointEntry.id                       = emberAfEndpointFromIndex(endpoint_idx);
            auto endpointInfo                      = GetEndpointInfoAtIndex(endpoint_idx);
            // The endpoint info should have value as this endpoint should be valid at this time
            VerifyOrDie(endpointInfo.has_value());
            endpointEntry.info = endpointInfo.value();
            return endpointEntry;
        }
    }

    // No enabled endpoint found. Give up
    return DataModel::EndpointEntry::kInvalid;
}

bool operator==(const DataModel::Provider::SemanticTag & tagA, const DataModel::Provider::SemanticTag & tagB)
{
    // Label is an optional and nullable value of CharSpan. Optional and Nullable have overload for ==,
    // But `==` is deleted for CharSpan. Here we only check whether the string is the same.
    if (tagA.label.HasValue() != tagB.label.HasValue())
    {
        return false;
    }
    if (tagA.label.HasValue())
    {
        if (tagA.label.Value().IsNull() != tagB.label.Value().IsNull())
        {
            return false;
        }
        if (!tagA.label.Value().IsNull())
        {
            if (!tagA.label.Value().Value().data_equal(tagB.label.Value().Value()))
            {
                return false;
            }
        }
    }
    return (tagA.tag == tagB.tag) && (tagA.mfgCode == tagB.mfgCode) && (tagA.namespaceID == tagB.namespaceID);
}

std::optional<unsigned> FindNextSemanticTagIndex(EndpointId endpoint, const DataModel::Provider::SemanticTag & previous,
                                                 unsigned hintWherePreviousMayBe)
{
    DataModel::Provider::SemanticTag hintTag;
    // Check whether the hint is the previous tag
    if (GetSemanticTagForEndpointAtIndex(endpoint, hintWherePreviousMayBe, hintTag) == CHIP_NO_ERROR)
    {
        if (previous == hintTag)
        {
            return std::make_optional(hintWherePreviousMayBe + 1);
        }
    }
    // If the hint is not the previous tag, iterate over all the tags to find the index for the previous tag
    unsigned index = 0;
    // Ensure that the next index is in the range
    while (GetSemanticTagForEndpointAtIndex(endpoint, index + 1, hintTag) == CHIP_NO_ERROR &&
           GetSemanticTagForEndpointAtIndex(endpoint, index, hintTag) == CHIP_NO_ERROR)
    {
        if (previous == hintTag)
        {
            return std::make_optional(index + 1);
        }
        index++;
    }
    return std::nullopt;
}

DefaultAttributePersistenceProvider gDefaultAttributePersistence;

} // namespace

CHIP_ERROR CodegenDataModelProvider::Startup(DataModel::InteractionModelContext context)
{
    ReturnErrorOnFailure(DataModel::Provider::Startup(context));

    // Ember NVM requires have a data model provider. attempt to create one if one is not available
    //
    // It is not a critical failure to not have one, however if one is not set up, ember NVM operations
    // will error out with a `persistence not available`.
    if (GetAttributePersistenceProvider() == nullptr)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogProgress(DataManagement, "Ember attribute persistence requires setting up");
#endif
        if (mPersistentStorageDelegate != nullptr)
        {
            ReturnErrorOnFailure(gDefaultAttributePersistence.Init(mPersistentStorageDelegate));
            SetAttributePersistenceProvider(&gDefaultAttributePersistence);
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        }
        else
        {
            ChipLogError(DataManagement, "No storage delegate available, will not set up attribute persistence.");
#endif
        }
    }

    return CHIP_NO_ERROR;
}

std::optional<CommandId> CodegenDataModelProvider::EmberCommandListIterator::First(const CommandId * list)
{
    VerifyOrReturnValue(list != nullptr, std::nullopt);
    mCurrentList = mCurrentHint = list;

    VerifyOrReturnValue(*mCurrentList != kInvalidCommandId, std::nullopt);
    return *mCurrentList;
}

std::optional<CommandId> CodegenDataModelProvider::EmberCommandListIterator::Next(const CommandId * list, CommandId previousId)
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

bool CodegenDataModelProvider::EmberCommandListIterator::Exists(const CommandId * list, CommandId toCheck)
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

std::optional<DataModel::ActionReturnStatus> CodegenDataModelProvider::Invoke(const DataModel::InvokeRequest & request,
                                                                              TLV::TLVReader & input_arguments,
                                                                              CommandHandler * handler)
{
    CommandHandlerInterface * handler_interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(request.path.mEndpointId, request.path.mClusterId);

    if (handler_interface)
    {
        CommandHandlerInterface::HandlerContext context(*handler, request.path, input_arguments);
        handler_interface->InvokeCommand(context);

        // If the command was handled, don't proceed any further and return successfully.
        if (context.mCommandHandled)
        {
            return std::nullopt;
        }
    }

    // Ember always sets the return in the handler
    DispatchSingleClusterCommand(request.path, input_arguments, handler);
    return std::nullopt;
}

bool CodegenDataModelProvider::EndpointExists(EndpointId endpoint)
{
    return (emberAfIndexFromEndpoint(endpoint) != kEmberInvalidEndpointIndex);
}

std::optional<DataModel::EndpointInfo> CodegenDataModelProvider::GetEndpointInfo(EndpointId endpoint)
{
    std::optional<unsigned> endpoint_idx = TryFindEndpointIndex(endpoint);
    if (endpoint_idx.has_value())
    {
        return GetEndpointInfoAtIndex(static_cast<uint16_t>(*endpoint_idx));
    }
    return std::nullopt;
}

DataModel::EndpointEntry CodegenDataModelProvider::FirstEndpoint()
{
    return FirstEndpointEntry(0, mEndpointIterationHint);
}

std::optional<unsigned> CodegenDataModelProvider::TryFindEndpointIndex(EndpointId id) const
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

DataModel::EndpointEntry CodegenDataModelProvider::NextEndpoint(EndpointId before)
{
    std::optional<unsigned> before_idx = TryFindEndpointIndex(before);
    if (!before_idx.has_value())
    {
        return DataModel::EndpointEntry::kInvalid;
    }
    return FirstEndpointEntry(*before_idx + 1, mEndpointIterationHint);
}

DataModel::ClusterEntry CodegenDataModelProvider::FirstServerCluster(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->clusterCount > 0, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->cluster != nullptr, DataModel::ClusterEntry::kInvalid);

    return FirstServerClusterEntry(endpointId, endpoint, 0, mServerClusterIterationHint);
}

std::optional<unsigned> CodegenDataModelProvider::TryFindClusterIndex(const EmberAfEndpointType * endpoint, ClusterId id,
                                                                      ClusterSide side) const
{
    const unsigned clusterCount = endpoint->clusterCount;
    unsigned hint               = side == ClusterSide::kServer ? mServerClusterIterationHint : mClientClusterIterationHint;

    if (hint < clusterCount)
    {
        const EmberAfCluster & cluster = endpoint->cluster[hint];
        if (((side == ClusterSide::kServer) && cluster.IsServer()) || ((side == ClusterSide::kClient) && cluster.IsClient()))
        {
            if (cluster.clusterId == id)
            {
                return std::make_optional(hint);
            }
        }
    }

    // linear search, this may be slow
    // does NOT use emberAfClusterIndex to not iterate over endpoints as we have
    // already found the correct endpoint
    for (unsigned cluster_idx = 0; cluster_idx < clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (((side == ClusterSide::kServer) && !cluster.IsServer()) || ((side == ClusterSide::kClient) && !cluster.IsClient()))
        {
            continue;
        }
        if (cluster.clusterId == id)
        {
            return std::make_optional(cluster_idx);
        }
    }

    return std::nullopt;
}

DataModel::ClusterEntry CodegenDataModelProvider::NextServerCluster(const ConcreteClusterPath & before)
{
    // TODO: This search still seems slow (ember will loop). Should use index hints as long
    //       as ember API supports it
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(before.mEndpointId);

    VerifyOrReturnValue(endpoint != nullptr, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->clusterCount > 0, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->cluster != nullptr, DataModel::ClusterEntry::kInvalid);

    std::optional<unsigned> cluster_idx = TryFindClusterIndex(endpoint, before.mClusterId, ClusterSide::kServer);
    if (!cluster_idx.has_value())
    {
        return DataModel::ClusterEntry::kInvalid;
    }

    return FirstServerClusterEntry(before.mEndpointId, endpoint, *cluster_idx + 1, mServerClusterIterationHint);
}

std::optional<DataModel::ClusterInfo> CodegenDataModelProvider::GetServerClusterInfo(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);

    auto info = LoadClusterInfo(path, *cluster);

    if (CHIP_ERROR * err = std::get_if<CHIP_ERROR>(&info))
    {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to load cluster info: %" CHIP_ERROR_FORMAT, err->Format());
#else
        (void) err->Format();
#endif
        return std::nullopt;
    }

    return std::make_optional(std::get<DataModel::ClusterInfo>(info));
}

ConcreteClusterPath CodegenDataModelProvider::FirstClientCluster(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, ConcreteClusterPath(endpointId, kInvalidClusterId));
    VerifyOrReturnValue(endpoint->clusterCount > 0, ConcreteClusterPath(endpointId, kInvalidClusterId));
    VerifyOrReturnValue(endpoint->cluster != nullptr, ConcreteClusterPath(endpointId, kInvalidClusterId));

    return ConcreteClusterPath(endpointId, FirstClientClusterId(endpoint, 0, mClientClusterIterationHint));
}

ConcreteClusterPath CodegenDataModelProvider::NextClientCluster(const ConcreteClusterPath & before)
{
    // TODO: This search still seems slow (ember will loop). Should use index hints as long
    //       as ember API supports it
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(before.mEndpointId);

    VerifyOrReturnValue(endpoint != nullptr, ConcreteClusterPath(before.mEndpointId, kInvalidClusterId));
    VerifyOrReturnValue(endpoint->clusterCount > 0, ConcreteClusterPath(before.mEndpointId, kInvalidClusterId));
    VerifyOrReturnValue(endpoint->cluster != nullptr, ConcreteClusterPath(before.mEndpointId, kInvalidClusterId));

    std::optional<unsigned> cluster_idx = TryFindClusterIndex(endpoint, before.mClusterId, ClusterSide::kClient);
    if (!cluster_idx.has_value())
    {
        return ConcreteClusterPath(before.mEndpointId, kInvalidClusterId);
    }

    return ConcreteClusterPath(before.mEndpointId, FirstClientClusterId(endpoint, *cluster_idx + 1, mClientClusterIterationHint));
}

DataModel::AttributeEntry CodegenDataModelProvider::FirstAttribute(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, DataModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributeCount > 0, DataModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributes != nullptr, DataModel::AttributeEntry::kInvalid);

    mAttributeIterationHint = 0;
    return AttributeEntryFrom(path, cluster->attributes[0]);
}

std::optional<unsigned> CodegenDataModelProvider::TryFindAttributeIndex(const EmberAfCluster * cluster, AttributeId id) const
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

const EmberAfCluster * CodegenDataModelProvider::FindServerCluster(const ConcreteClusterPath & path)
{
    if (mPreviouslyFoundCluster.has_value() && (mPreviouslyFoundCluster->path == path) &&
        (mEmberMetadataStructureGeneration == emberAfMetadataStructureGeneration()))

    {
        return mPreviouslyFoundCluster->cluster;
    }

    const EmberAfCluster * cluster = emberAfFindServerCluster(path.mEndpointId, path.mClusterId);
    if (cluster != nullptr)
    {
        mPreviouslyFoundCluster           = std::make_optional<ClusterReference>(path, cluster);
        mEmberMetadataStructureGeneration = emberAfMetadataStructureGeneration();
    }
    return cluster;
}

DataModel::AttributeEntry CodegenDataModelProvider::NextAttribute(const ConcreteAttributePath & before)
{
    const EmberAfCluster * cluster = FindServerCluster(before);
    VerifyOrReturnValue(cluster != nullptr, DataModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributeCount > 0, DataModel::AttributeEntry::kInvalid);
    VerifyOrReturnValue(cluster->attributes != nullptr, DataModel::AttributeEntry::kInvalid);

    // find the given attribute in the list and then return the next one
    std::optional<unsigned> attribute_idx = TryFindAttributeIndex(cluster, before.mAttributeId);
    if (!attribute_idx.has_value())
    {
        return DataModel::AttributeEntry::kInvalid;
    }

    unsigned next_idx = *attribute_idx + 1;
    if (next_idx < cluster->attributeCount)
    {
        mAttributeIterationHint = next_idx;
        return AttributeEntryFrom(before, cluster->attributes[next_idx]);
    }

    // iteration complete
    return DataModel::AttributeEntry::kInvalid;
}

std::optional<DataModel::AttributeInfo> CodegenDataModelProvider::GetAttributeInfo(const ConcreteAttributePath & path)
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

    DataModel::AttributeInfo info;
    LoadAttributeInfo(path, cluster->attributes[*attribute_idx], &info);
    return std::make_optional(info);
}

DataModel::CommandEntry CodegenDataModelProvider::FirstAcceptedCommand(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, DataModel::CommandEntry::kInvalid);
    VerifyOrReturnValue(cluster->acceptedCommandList != nullptr, DataModel::CommandEntry::kInvalid);

    auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);

    std::optional<CommandId> mCommand = mAcceptedCommandsIterator.First(cluster->acceptedCommandList);
    while (mCommand.has_value())
    {
        const ConcreteCommandPath commandPath(path.mEndpointId, path.mClusterId, *mCommand);
        if ((commandHandler == nullptr) || commandHandler->AcceptsCommandId(commandPath))
        {
            return CommandEntryFrom(path, commandPath.mCommandId);
        }
        mCommand = mAcceptedCommandsIterator.Next(cluster->acceptedCommandList, *mCommand);
    }
    return DataModel::CommandEntry::kInvalid;
}

DataModel::CommandEntry CodegenDataModelProvider::NextAcceptedCommand(const ConcreteCommandPath & before)
{
    const EmberAfCluster * cluster = FindServerCluster(before);

    VerifyOrReturnValue(cluster != nullptr, DataModel::CommandEntry::kInvalid);
    VerifyOrReturnValue(cluster->acceptedCommandList != nullptr, DataModel::CommandEntry::kInvalid);

    auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(before.mEndpointId, before.mClusterId);

    std::optional<CommandId> mCommand = mAcceptedCommandsIterator.Next(cluster->acceptedCommandList, before.mCommandId);
    while (mCommand.has_value())
    {
        const ConcreteCommandPath commandPath(before.mEndpointId, before.mClusterId, *mCommand);
        if ((commandHandler == nullptr) || commandHandler->AcceptsCommandId(commandPath))
        {
            return CommandEntryFrom(before, commandPath.mCommandId);
        }
        mCommand = mAcceptedCommandsIterator.Next(cluster->acceptedCommandList, *mCommand);
    }
    return DataModel::CommandEntry::kInvalid;
}

std::optional<DataModel::CommandInfo> CodegenDataModelProvider::GetAcceptedCommandInfo(const ConcreteCommandPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);
    VerifyOrReturnValue(cluster->acceptedCommandList != nullptr, std::nullopt);

    auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);

    VerifyOrReturnValue(mAcceptedCommandsIterator.Exists(cluster->acceptedCommandList, path.mCommandId), std::nullopt);

    const ConcreteCommandPath commandPath(path.mEndpointId, path.mClusterId, path.mCommandId);
    VerifyOrReturnValue((commandHandler == nullptr) || commandHandler->AcceptsCommandId(commandPath), std::nullopt);

    return CommandEntryFrom(path, commandPath.mCommandId).info;
}

ConcreteCommandPath CodegenDataModelProvider::FirstGeneratedCommand(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, ConcreteCommandPath());
    VerifyOrReturnValue(cluster->generatedCommandList != nullptr, ConcreteCommandPath());

    auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);

    std::optional<CommandId> mCommand = mGeneratedCommandsIterator.First(cluster->generatedCommandList);
    while (mCommand.has_value())
    {
        const ConcreteCommandPath commandPath(path.mEndpointId, path.mClusterId, *mCommand);
        if ((commandHandler == nullptr) || commandHandler->GeneratesCommandId(commandPath))
        {
            return commandPath;
        }
        mCommand = mAcceptedCommandsIterator.Next(cluster->acceptedCommandList, *mCommand);
    }
    return {};
}

ConcreteCommandPath CodegenDataModelProvider::NextGeneratedCommand(const ConcreteCommandPath & before)
{
    const EmberAfCluster * cluster = FindServerCluster(before);

    VerifyOrReturnValue(cluster != nullptr, ConcreteCommandPath());
    VerifyOrReturnValue(cluster->generatedCommandList != nullptr, ConcreteCommandPath());

    // TODO: this does NOT make use of the hint because the command list is NOT sized (it is value-terminated)
    //       and we have no way to check `is hint in bounds`
    auto * commandHandler = CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(before.mEndpointId, before.mClusterId);

    std::optional<CommandId> mCommand = mGeneratedCommandsIterator.Next(cluster->generatedCommandList, before.mCommandId);
    while (mCommand.has_value())
    {
        const ConcreteCommandPath commandPath(before.mEndpointId, before.mClusterId, *mCommand);
        if ((commandHandler == nullptr) || commandHandler->GeneratesCommandId(commandPath))
        {
            return commandPath;
        }
        mCommand = mGeneratedCommandsIterator.Next(cluster->generatedCommandList, *mCommand);
    }

    return {};
}

std::optional<DataModel::DeviceTypeEntry> CodegenDataModelProvider::FirstDeviceType(EndpointId endpoint)
{
    // Use the `Index` version even though `emberAfDeviceTypeListFromEndpoint` would work because
    // index finding is cached in TryFindEndpointIndex and this avoids an extra `emberAfIndexFromEndpoint`
    // during `Next` loops. This avoids O(n^2) on number of indexes when iterating over all device types.
    //
    // Not actually needed for `First`, however this makes First and Next consistent.
    std::optional<unsigned> endpoint_index = TryFindEndpointIndex(endpoint);
    if (!endpoint_index.has_value())
    {
        return std::nullopt;
    }

    CHIP_ERROR err                            = CHIP_NO_ERROR;
    Span<const EmberAfDeviceType> deviceTypes = emberAfDeviceTypeListFromEndpointIndex(*endpoint_index, err);

    if (deviceTypes.empty())
    {
        return std::nullopt;
    }

    // we start at the beginning
    mDeviceTypeIterationHint = 0;
    return DeviceTypeEntryFromEmber(deviceTypes[0]);
}

std::optional<DataModel::DeviceTypeEntry> CodegenDataModelProvider::NextDeviceType(EndpointId endpoint,
                                                                                   const DataModel::DeviceTypeEntry & previous)
{
    // Use the `Index` version even though `emberAfDeviceTypeListFromEndpoint` would work because
    // index finding is cached in TryFindEndpointIndex and this avoids an extra `emberAfIndexFromEndpoint`
    // during `Next` loops. This avoids O(n^2) on number of indexes when iterating over all device types.
    std::optional<unsigned> endpoint_index = TryFindEndpointIndex(endpoint);
    if (!endpoint_index.has_value())
    {
        return std::nullopt;
    }

    CHIP_ERROR err                            = CHIP_NO_ERROR;
    Span<const EmberAfDeviceType> deviceTypes = emberAfDeviceTypeListFromEndpointIndex(*endpoint_index, err);

    unsigned idx = FindNextDeviceTypeIndex(deviceTypes, previous, mDeviceTypeIterationHint);

    if (idx >= deviceTypes.size())
    {
        return std::nullopt;
    }

    mDeviceTypeIterationHint = idx;
    return DeviceTypeEntryFromEmber(deviceTypes[idx]);
}

std::optional<DataModel::Provider::SemanticTag> CodegenDataModelProvider::GetFirstSemanticTag(EndpointId endpoint)
{
    Clusters::Descriptor::Structs::SemanticTagStruct::Type tag;
    // we start at the beginning
    mSemanticTagIterationHint = 0;
    if (GetSemanticTagForEndpointAtIndex(endpoint, 0, tag) == CHIP_NO_ERROR)
    {
        return std::make_optional(tag);
    }
    return std::nullopt;
}

std::optional<DataModel::Provider::SemanticTag> CodegenDataModelProvider::GetNextSemanticTag(EndpointId endpoint,
                                                                                             const SemanticTag & previous)
{
    Clusters::Descriptor::Structs::SemanticTagStruct::Type tag;
    std::optional<unsigned> idx = FindNextSemanticTagIndex(endpoint, previous, mSemanticTagIterationHint);
    if (idx.has_value() && GetSemanticTagForEndpointAtIndex(endpoint, *idx, tag) == CHIP_NO_ERROR)
    {
        return std::make_optional(tag);
    }
    return std::nullopt;
}

} // namespace app
} // namespace chip
