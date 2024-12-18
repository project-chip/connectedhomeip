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
#include "lib/support/ScopedBuffer.h"
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/Iterators.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/util/DataModelHandler.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SpanSearchValue.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <variant>

namespace chip {
namespace app {
namespace detail {

Loop EnumeratorCommandFinder::HandlerCallback(CommandId id)
{
    switch (mOperation)
    {
    case Operation::kFindFirst:
        mFound = id;
        return Loop::Break;
    case Operation::kFindExact:
        if (mTarget == id)
        {
            mFound = id; // found it
            return Loop::Break;
        }
        break;
    case Operation::kFindNext:
        if (mTarget == id)
        {
            // Once we found the ID, get the first
            mOperation = Operation::kFindFirst;
        }
        break;
    }
    return Loop::Continue; // keep searching
}

std::optional<CommandId> EnumeratorCommandFinder::FindCommandId(Operation operation, const ConcreteCommandPath & path)
{
    mOperation = operation;
    mTarget    = path.mCommandId;

    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);

    if (interface == nullptr)
    {
        return std::nullopt; // no data: no interface
    }

    CHIP_ERROR err = (interface->*mCallback)(path, HandlerCallbackFn, this);
    if (err == CHIP_ERROR_NOT_IMPLEMENTED)
    {
        return std::nullopt; // no data provided by the interface
    }

    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        // Report the error here since we lose actual error. This generally should NOT be possible as CommandHandlerInterface
        // usually returns unimplemented or should just work for our use case (our callback never fails)
        ChipLogError(DataManagement, "Enumerate error: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return kInvalidCommandId;
    }

    return mFound.value_or(kInvalidCommandId);
}

} // namespace detail

using detail::EnumeratorCommandFinder;

namespace {

/// Search by device type within a span of EmberAfDeviceType (finds the device type that matches the given
/// DataModel::DeviceTypeEntry)
struct ByDeviceType
{
    using Key  = DataModel::DeviceTypeEntry;
    using Type = const EmberAfDeviceType;
    static Span<Type> GetSpan(Span<const EmberAfDeviceType> & data) { return data; }
    static bool HasKey(const Key & id, const Type & instance)
    {
        return (instance.deviceId == id.deviceTypeId) && (instance.deviceVersion == id.deviceTypeRevision);
    }
};

const CommandId * AcceptedCommands(const EmberAfCluster & cluster)
{
    return cluster.acceptedCommandList;
}

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

class DeviceTypeEntryIterator : public DataModel::ElementIterator<DataModel::DeviceTypeEntry>
{
public:
    DeviceTypeEntryIterator(Span<const EmberAfDeviceType> values) : mDeviceTypes(values) {}

    std::optional<DataModel::DeviceTypeEntry> Next() override
    {
        if (mDeviceTypes.empty())
        {
            return std::nullopt;
        }

        auto result = DataModel::DeviceTypeEntry{
            .deviceTypeId       = mDeviceTypes.front().deviceId,
            .deviceTypeRevision = mDeviceTypes.front().deviceVersion,
        };
        mDeviceTypes = mDeviceTypes.SubSpan(1);
        return result;
    }

private:
    Span<const EmberAfDeviceType> mDeviceTypes;
};

class SemanticTagIterator : public DataModel::ElementIterator<DataModel::Provider::SemanticTag>
{
public:
    SemanticTagIterator(EndpointId endpointId) : mEndpointId(endpointId) {}

    std::optional<DataModel::Provider::SemanticTag> Next() override
    {
        DataModel::Provider::SemanticTag tag;

        if (GetSemanticTagForEndpointAtIndex(mEndpointId, mIndex, tag) == CHIP_NO_ERROR)
        {
            mIndex++;
            return tag;
        }
        return std::nullopt;
    }

private:
    const EndpointId mEndpointId;
    size_t mIndex = 0;
};

class ClientClusterIdIterator : public DataModel::ElementIterator<ClusterId>
{
public:
    ClientClusterIdIterator() = default;
    ClientClusterIdIterator(Span<const EmberAfCluster> clusters) : mClusters(clusters) {}

    std::optional<ClusterId> Next() override
    {
        while (true)
        {
            if (mClusters.empty())
            {
                return std::nullopt;
            }

            if (mClusters.front().IsClient())
            {
                ClusterId id = mClusters.front().clusterId;
                mClusters    = mClusters.SubSpan(1);
                return id;
            }
            mClusters = mClusters.SubSpan(1);
        }
    }

private:
    Span<const EmberAfCluster> mClusters;
};

class EndpointIterator : public DataModel::MetaDataIterator<EndpointId, DataModel::EndpointInfo>
{
public:
    EndpointIterator(uint16_t & hint) : mIterationHint(hint) {}

    std::optional<EndpointId> Next() override
    {
        const uint16_t lastEndpointIndex = emberAfEndpointCount();

        for (; mIndex < lastEndpointIndex; mIndex++)
        {
            if (!emberAfEndpointIndexIsEnabled(mIndex))
            {
                continue;
            }

            mMetadataIndex = mIndex;
            mIterationHint = mIndex;
            mIndex++;
            return emberAfEndpointFromIndex(mMetadataIndex);
        }
        mMetadataIndex = kInvalidIndex;
        return std::nullopt;
    }

    bool SeekTo(const EndpointId & id) override
    {
        // the only speedup if is hint matches somehow
        const uint16_t lastEndpointIndex = emberAfEndpointCount();
        const uint16_t hint              = mIterationHint;

        if ((hint < lastEndpointIndex) && (emberAfEndpointFromIndex(hint) == id))
        {
            mMetadataIndex = hint;
            mIndex         = static_cast<uint16_t>(hint + 1);
            return true;
        }
        // not found. Just seek until found
        mIndex = 0;
        for (auto search_id = Next(); search_id.has_value(); search_id = Next())
        {
            if (*search_id == id)
            {
                return true;
            }
        }
        return false;
    }
    std::optional<DataModel::EndpointInfo> GetMetadata() override
    {
        VerifyOrReturnValue(mMetadataIndex != kInvalidIndex, std::nullopt);
        return GetEndpointInfoAtIndex(mMetadataIndex);
    }

private:
    uint16_t & mIterationHint;
    uint16_t mIndex         = 0;
    uint16_t mMetadataIndex = kInvalidIndex;

    static constexpr uint16_t kInvalidIndex = std::numeric_limits<uint16_t>::max();
};

class GeneratedCommandsIterator : public DataModel::ElementIterator<CommandId>
{
public:
    GeneratedCommandsIterator(const CommandId * commands) : mCommands(commands) {}

    std::optional<CommandId> Next() override
    {
        VerifyOrReturnValue(mCommands != nullptr, std::nullopt);
        VerifyOrReturnValue(*mCommands != kInvalidCommandId, std::nullopt);

        auto id = *mCommands;
        ++mCommands;
        return id;
    }

private:
    const CommandId * mCommands;
};

class AllocatedGeneratedCommandsIterator : public DataModel::ElementIterator<CommandId>
{
public:
    AllocatedGeneratedCommandsIterator(CommandId * commands, size_t size) : mCommands(commands), mSize(size) {}
    ~AllocatedGeneratedCommandsIterator() override { Platform::Impl::PlatformMemoryManagement::MemoryFree(mCommands); }

    AllocatedGeneratedCommandsIterator(const AllocatedGeneratedCommandsIterator &)             = delete;
    AllocatedGeneratedCommandsIterator & operator=(const AllocatedGeneratedCommandsIterator &) = delete;

    std::optional<CommandId> Next() override
    {
        VerifyOrReturnValue(mCommands != nullptr, std::nullopt);
        VerifyOrReturnValue(mIndex < mSize, std::nullopt);

        auto id = mCommands[mIndex];
        ++mIndex;
        return id;
    }

private:
    CommandId * mCommands;
    const size_t mSize;
    size_t mIndex = 0;
};

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

    InitDataModelForTesting();

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

std::unique_ptr<DataModel::MetaDataIterator<EndpointId, DataModel::EndpointInfo>> CodegenDataModelProvider::GetEndpoints()
{
    return std::make_unique<EndpointIterator>(mEndpointIterationHint);
}

DataModel::ClusterEntry CodegenDataModelProvider::FirstServerCluster(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->clusterCount > 0, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->cluster != nullptr, DataModel::ClusterEntry::kInvalid);

    return FirstServerClusterEntry(endpointId, endpoint, 0, mServerClusterIterationHint);
}

std::optional<unsigned> CodegenDataModelProvider::TryFindServerClusterIndex(const EmberAfEndpointType * endpoint,
                                                                            ClusterId id) const
{
    const unsigned clusterCount = endpoint->clusterCount;
    unsigned hint               = mServerClusterIterationHint;

    if (mServerClusterIterationHint < clusterCount)
    {
        const EmberAfCluster & cluster = endpoint->cluster[hint];
        if (cluster.IsServer() && (cluster.clusterId == id))
        {
            return std::make_optional(hint);
        }
    }

    // linear search, this may be slow
    // does NOT use emberAfClusterIndex to not iterate over endpoints as we have
    // already found the correct endpoint
    for (unsigned cluster_idx = 0; cluster_idx < clusterCount; cluster_idx++)
    {
        const EmberAfCluster & cluster = endpoint->cluster[cluster_idx];
        if (!cluster.IsServer())
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

    std::optional<unsigned> cluster_idx = TryFindServerClusterIndex(endpoint, before.mClusterId);
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

std::unique_ptr<DataModel::ElementIterator<ClusterId>> CodegenDataModelProvider::GetClientClusters(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);

    VerifyOrReturnValue(endpoint != nullptr, std::make_unique<ClientClusterIdIterator>());
    VerifyOrReturnValue(endpoint->clusterCount > 0, std::make_unique<ClientClusterIdIterator>());
    VerifyOrReturnValue(endpoint->cluster != nullptr, std::make_unique<ClientClusterIdIterator>());

    return std::make_unique<ClientClusterIdIterator>(Span(endpoint->cluster, endpoint->clusterCount));
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

CommandId CodegenDataModelProvider::FindCommand(const ConcreteCommandPath & path, detail::EnumeratorCommandFinder & handlerFinder,
                                                detail::EnumeratorCommandFinder::Operation operation,
                                                CodegenDataModelProvider::EmberCommandListIterator & emberIterator,
                                                CommandListGetter commandListGetter)
{

    std::optional<CommandId> handlerCommandId = handlerFinder.FindCommandId(operation, path);
    if (handlerCommandId.has_value())
    {
        return *handlerCommandId;
    }

    const EmberAfCluster * cluster = FindServerCluster(path);
    VerifyOrReturnValue(cluster != nullptr, kInvalidCommandId);

    const CommandId * commandList = commandListGetter(*cluster);

    switch (operation)
    {
    case EnumeratorCommandFinder::Operation::kFindFirst:
        return emberIterator.First(commandList).value_or(kInvalidCommandId);
    case EnumeratorCommandFinder::Operation::kFindNext:
        return emberIterator.Next(commandList, path.mCommandId).value_or(kInvalidCommandId);
    case EnumeratorCommandFinder::Operation::kFindExact:
    default:
        return emberIterator.Exists(commandList, path.mCommandId) ? path.mCommandId : kInvalidCommandId;
    }
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
    EnumeratorCommandFinder handlerFinder(&CommandHandlerInterface::EnumerateAcceptedCommands);

    CommandId commandId =
        FindCommand(ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId), handlerFinder,
                    detail::EnumeratorCommandFinder::Operation::kFindFirst, mAcceptedCommandsIterator, AcceptedCommands);

    VerifyOrReturnValue(commandId != kInvalidCommandId, DataModel::CommandEntry::kInvalid);
    return CommandEntryFrom(path, commandId);
}

DataModel::CommandEntry CodegenDataModelProvider::NextAcceptedCommand(const ConcreteCommandPath & before)
{

    EnumeratorCommandFinder handlerFinder(&CommandHandlerInterface::EnumerateAcceptedCommands);
    CommandId commandId = FindCommand(before, handlerFinder, detail::EnumeratorCommandFinder::Operation::kFindNext,
                                      mAcceptedCommandsIterator, AcceptedCommands);

    VerifyOrReturnValue(commandId != kInvalidCommandId, DataModel::CommandEntry::kInvalid);
    return CommandEntryFrom(before, commandId);
}

std::optional<DataModel::CommandInfo> CodegenDataModelProvider::GetAcceptedCommandInfo(const ConcreteCommandPath & path)
{

    EnumeratorCommandFinder handlerFinder(&CommandHandlerInterface::EnumerateAcceptedCommands);
    CommandId commandId = FindCommand(path, handlerFinder, detail::EnumeratorCommandFinder::Operation::kFindExact,
                                      mAcceptedCommandsIterator, AcceptedCommands);

    VerifyOrReturnValue(commandId != kInvalidCommandId, std::nullopt);
    return CommandEntryFrom(path, commandId).info;
}

std::unique_ptr<DataModel::ElementIterator<CommandId>>
CodegenDataModelProvider::GetGeneratedCommands(ConcreteClusterPath clusterPath)
{
    // we should FIND the handler and allocate things (if needed)
    // OR return metadata items
    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(clusterPath.mEndpointId, clusterPath.mClusterId);

    if (interface != nullptr)
    {
        // maybe we have actual generated commands here ...
        size_t commandCount = 0;
        CHIP_ERROR err      = interface->EnumerateGeneratedCommands(
            clusterPath,
            [](CommandId commandId, void * context) -> Loop {
                (*reinterpret_cast<size_t *>(context))++;
                return Loop::Continue;
            },
            reinterpret_cast<void *>(&commandCount));

        if (err == CHIP_NO_ERROR)
        {
            typedef struct
            {
                Platform::ScopedMemoryBuffer<CommandId> commands;
                size_t index;
                size_t maxSize;
            } FetchData;

            FetchData data;
            data.index   = 0;
            data.maxSize = commandCount;

            if (!data.commands.Alloc(commandCount))
            {
                ChipLogError(DataManagement, "Out of memory reading generated commands");
                return std::make_unique<GeneratedCommandsIterator>(nullptr);
            }

            err = interface->EnumerateGeneratedCommands(
                clusterPath,
                [](CommandId commandId, void * context) -> Loop {
                    auto data = reinterpret_cast<FetchData *>(context);

                    if (data->index < data->maxSize)
                    {
                        data->commands[data->index] = commandId;
                    }
                    data->index++;
                    return Loop::Continue;
                },
                reinterpret_cast<void *>(&data));

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement, "Error fetching commands from CHI");
                return std::make_unique<GeneratedCommandsIterator>(nullptr);
            }

            if (data.index != data.maxSize)
            {
                ChipLogError(DataManagement, "Command size mismatch: command list is not deterministic");
                return std::make_unique<GeneratedCommandsIterator>(nullptr);
            }

            return std::make_unique<AllocatedGeneratedCommandsIterator>(data.commands.Release(), data.maxSize);
        }

        if (err != CHIP_ERROR_NOT_IMPLEMENTED)
        {
            // hard failure, we cannot return anything
            return std::make_unique<GeneratedCommandsIterator>(nullptr);
        }
    }

    const EmberAfCluster * cluster = FindServerCluster(clusterPath);
    VerifyOrReturnValue(cluster != nullptr, std::make_unique<GeneratedCommandsIterator>(nullptr));

    return std::make_unique<GeneratedCommandsIterator>(cluster->generatedCommandList);
}

void CodegenDataModelProvider::InitDataModelForTesting()
{
    // Call the Ember-specific InitDataModelHandler
    InitDataModelHandler();
}

std::unique_ptr<DataModel::ElementIterator<DataModel::DeviceTypeEntry>>
CodegenDataModelProvider::GetDeviceTypes(EndpointId endpointId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return std::make_unique<DeviceTypeEntryIterator>(emberAfDeviceTypeListFromEndpoint(endpointId, err));
}

std::unique_ptr<DataModel::ElementIterator<DataModel::Provider::SemanticTag>>
CodegenDataModelProvider::GetSemanticTags(EndpointId endpointId)
{
    return std::make_unique<SemanticTagIterator>(endpointId);
}

} // namespace app
} // namespace chip
