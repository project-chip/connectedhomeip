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
#include <app/codegen-data-model-provider/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

// separated out for code-reuse
#include <app/ember_coupling/EventPathValidity.mixin.h>

#include <optional>
#include <variant>

namespace chip {
namespace app {
namespace {

/// Handles going through callback-based enumeration of generated/accepted commands
/// for CommandHandlerInterface based items.
///
/// Offers the ability to focus on some operation for finding a given
/// command id:
///   - FindFirst will return the first found element
///   - FindExact finds the element with the given id
///   - FindNext finds the element following the given id
class EnumeratorCommandFinder
{
public:
    using HandlerCallbackFunction = CHIP_ERROR (CommandHandlerInterface::*)(const ConcreteClusterPath &,
                                                                            CommandHandlerInterface::CommandIdCallback, void *);

    enum class Operation
    {
        kFindFirst, // Find the first value in the list
        kFindExact, // Find the given value
        kFindNext   // Find the value AFTER this value
    };

    EnumeratorCommandFinder(HandlerCallbackFunction callback) :
        mCallback(callback), mOperation(Operation::kFindFirst), mTarget(kInvalidCommandId)
    {}

    /// Find the given command ID that matches the given operation/path.
    ///
    /// If operation is kFindFirst, then path commandID is ignored. Otherwise it is used as a key to
    /// kFindExact or kFindNext.
    ///
    /// Returns:
    ///    - std::nullopt if no command found using the command handler interface
    ///    - kInvalidCommandId if the find failed (but command handler interface does provide a list)
    ///    - valid id if command handler interface usage succeeds
    std::optional<CommandId> FindCommandId(Operation operation, const ConcreteCommandPath & path);

    /// Uses FindCommandId to find the given command and loads the command entry data
    std::optional<DataModel::CommandEntry> FindCommandEntry(Operation operation, const ConcreteCommandPath & path);

private:
    HandlerCallbackFunction mCallback;
    Operation mOperation;
    CommandId mTarget;
    std::optional<CommandId> mFound = std::nullopt;

    Loop HandlerCallback(CommandId id)
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

    static Loop HandlerCallbackFn(CommandId id, void * context)
    {
        auto self = static_cast<EnumeratorCommandFinder *>(context);
        return self->HandlerCallback(id);
    }
};

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
        // Report the error here since we lose actual error. This generally should NOT be possible as CommandHandlerInterface
        // usually returns unimplemented or should just work for our use case (our callback never fails)
        ChipLogError(DataManagement, "Enumerate error: %" CHIP_ERROR_FORMAT, err.Format());
        return kInvalidCommandId;
    }

    return mFound.value_or(kInvalidCommandId);
}

/// Load the cluster information into the specified destination
std::variant<CHIP_ERROR, DataModel::ClusterInfo> LoadClusterInfo(const ConcreteClusterPath & path, const EmberAfCluster & cluster)
{
    DataVersion * versionPtr = emberAfDataVersionStorage(path);
    if (versionPtr == nullptr)
    {
        ChipLogError(AppServer, "Failed to get data version for %d/" ChipLogFormatMEI, static_cast<int>(path.mEndpointId),
                     ChipLogValueMEI(cluster.clusterId));
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

    return entry;
}

std::optional<DataModel::CommandEntry> EnumeratorCommandFinder::FindCommandEntry(Operation operation,
                                                                                 const ConcreteCommandPath & path)
{

    std::optional<CommandId> id = FindCommandId(operation, path);

    if (!id.has_value())
    {
        return std::nullopt;
    }

    return (*id == kInvalidCommandId) ? DataModel::CommandEntry::kInvalid : CommandEntryFrom(path, *id);
}

// TODO: DeviceTypeEntry content is IDENTICAL to EmberAfDeviceType, so centralizing
//       to a common type is probably better. Need to figure out dependencies since
//       this would make ember return datamodel-provider types.
//       See: https://github.com/project-chip/connectedhomeip/issues/35889
DataModel::DeviceTypeEntry DeviceTypeEntryFromEmber(const EmberAfDeviceType & other)
{
    DataModel::DeviceTypeEntry entry;

    entry.deviceTypeId      = other.deviceId;
    entry.deviceTypeVersion = other.deviceVersion;

    return entry;
}

// Explicitly compare for identical entries. note that types are different,
// so you must do `a == b` and the `b == a` will not work.
bool operator==(const DataModel::DeviceTypeEntry & a, const EmberAfDeviceType & b)
{
    return (a.deviceTypeId == b.deviceId) && (a.deviceTypeVersion == b.deviceVersion);
}

/// Find the `index` where one of the following holds:
///    - types[index - 1] == previous OR
///    - index == types.size()  // i.e. not found or there is no next
///
/// hintWherePreviousMayBe represents a search hint where previous may exist.
unsigned FindNextDeviceTypeIndex(Span<const EmberAfDeviceType> types, const DataModel::DeviceTypeEntry previous,
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

} // namespace

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

EndpointId CodegenDataModelProvider::FirstEndpoint()
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

EndpointId CodegenDataModelProvider::NextEndpoint(EndpointId before)
{
    const uint16_t lastEndpointIndex = emberAfEndpointCount();

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

DataModel::ClusterEntry CodegenDataModelProvider::FirstCluster(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);
    VerifyOrReturnValue(endpoint != nullptr, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->clusterCount > 0, DataModel::ClusterEntry::kInvalid);
    VerifyOrReturnValue(endpoint->cluster != nullptr, DataModel::ClusterEntry::kInvalid);

    return FirstServerClusterEntry(endpointId, endpoint, 0, mClusterIterationHint);
}

std::optional<unsigned> CodegenDataModelProvider::TryFindServerClusterIndex(const EmberAfEndpointType * endpoint,
                                                                            ClusterId id) const
{
    const unsigned clusterCount = endpoint->clusterCount;

    if (mClusterIterationHint < clusterCount)
    {
        const EmberAfCluster & cluster = endpoint->cluster[mClusterIterationHint];
        if (cluster.IsServer() && (cluster.clusterId == id))
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
        if (cluster.IsServer() && (cluster.clusterId == id))
        {
            return std::make_optional(cluster_idx);
        }
    }

    return std::nullopt;
}

DataModel::ClusterEntry CodegenDataModelProvider::NextCluster(const ConcreteClusterPath & before)
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

    return FirstServerClusterEntry(before.mEndpointId, endpoint, *cluster_idx + 1, mClusterIterationHint);
}

std::optional<DataModel::ClusterInfo> CodegenDataModelProvider::GetClusterInfo(const ConcreteClusterPath & path)
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

    return std::make_optional(std::get<DataModel::ClusterInfo>(info));
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
    auto handlerInterfaceValue = EnumeratorCommandFinder(&CommandHandlerInterface::EnumerateAcceptedCommands)
                                     .FindCommandEntry(EnumeratorCommandFinder::Operation::kFindFirst,
                                                       ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId));

    if (handlerInterfaceValue.has_value())
    {
        return *handlerInterfaceValue;
    }

    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, DataModel::CommandEntry::kInvalid);

    std::optional<CommandId> commandId = mAcceptedCommandsIterator.First(cluster->acceptedCommandList);
    VerifyOrReturnValue(commandId.has_value(), DataModel::CommandEntry::kInvalid);

    return CommandEntryFrom(path, *commandId);
}

DataModel::CommandEntry CodegenDataModelProvider::NextAcceptedCommand(const ConcreteCommandPath & before)
{
    // TODO: `Next` redirecting to a callback is slow O(n^2).
    //       see https://github.com/project-chip/connectedhomeip/issues/35790
    auto handlerInterfaceValue = EnumeratorCommandFinder(&CommandHandlerInterface::EnumerateAcceptedCommands)
                                     .FindCommandEntry(EnumeratorCommandFinder::Operation::kFindNext, before);

    if (handlerInterfaceValue.has_value())
    {
        return *handlerInterfaceValue;
    }

    const EmberAfCluster * cluster = FindServerCluster(before);

    VerifyOrReturnValue(cluster != nullptr, DataModel::CommandEntry::kInvalid);

    std::optional<CommandId> commandId = mAcceptedCommandsIterator.Next(cluster->acceptedCommandList, before.mCommandId);
    VerifyOrReturnValue(commandId.has_value(), DataModel::CommandEntry::kInvalid);

    return CommandEntryFrom(before, *commandId);
}

std::optional<DataModel::CommandInfo> CodegenDataModelProvider::GetAcceptedCommandInfo(const ConcreteCommandPath & path)
{
    auto handlerInterfaceValue = EnumeratorCommandFinder(&CommandHandlerInterface::EnumerateAcceptedCommands)
                                     .FindCommandEntry(EnumeratorCommandFinder::Operation::kFindExact, path);

    if (handlerInterfaceValue.has_value())
    {
        return handlerInterfaceValue->IsValid() ? std::make_optional(handlerInterfaceValue->info) : std::nullopt;
    }

    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, std::nullopt);
    VerifyOrReturnValue(mAcceptedCommandsIterator.Exists(cluster->acceptedCommandList, path.mCommandId), std::nullopt);

    return CommandEntryFrom(path, path.mCommandId).info;
}

ConcreteCommandPath CodegenDataModelProvider::FirstGeneratedCommand(const ConcreteClusterPath & path)
{
    std::optional<CommandId> commandId =
        EnumeratorCommandFinder(&CommandHandlerInterface::EnumerateGeneratedCommands)
            .FindCommandId(EnumeratorCommandFinder::Operation::kFindFirst,
                           ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId));
    if (commandId.has_value())
    {
        return *commandId == kInvalidCommandId ? kInvalidCommandPath
                                               : ConcreteCommandPath(path.mEndpointId, path.mClusterId, *commandId);
    }

    const EmberAfCluster * cluster = FindServerCluster(path);
    VerifyOrReturnValue(cluster != nullptr, kInvalidCommandPath);

    commandId = mGeneratedCommandsIterator.First(cluster->generatedCommandList);
    VerifyOrReturnValue(commandId.has_value(), kInvalidCommandPath);
    return ConcreteCommandPath(path.mEndpointId, path.mClusterId, *commandId);
}

ConcreteCommandPath CodegenDataModelProvider::NextGeneratedCommand(const ConcreteCommandPath & before)
{
    // TODO: `Next` redirecting to a callback is slow O(n^2).
    //       see https://github.com/project-chip/connectedhomeip/issues/35790
    auto nextId = EnumeratorCommandFinder(&CommandHandlerInterface::EnumerateGeneratedCommands)
                      .FindCommandId(EnumeratorCommandFinder::Operation::kFindNext, before);

    if (nextId.has_value())
    {
        return (*nextId == kInvalidCommandId) ? kInvalidCommandPath
                                              : ConcreteCommandPath(before.mEndpointId, before.mClusterId, *nextId);
    }

    const EmberAfCluster * cluster = FindServerCluster(before);

    VerifyOrReturnValue(cluster != nullptr, kInvalidCommandPath);

    std::optional<CommandId> commandId = mGeneratedCommandsIterator.Next(cluster->generatedCommandList, before.mCommandId);
    VerifyOrReturnValue(commandId.has_value(), kInvalidCommandPath);

    return ConcreteCommandPath(before.mEndpointId, before.mClusterId, *commandId);
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

bool CodegenDataModelProvider::EventPathIncludesAccessibleConcretePath(const EventPathParams & path,
                                                                       const Access::SubjectDescriptor & descriptor)
{

    if (!path.HasWildcardEndpointId())
    {
        // No need to check whether the endpoint is enabled, because
        // emberAfFindEndpointType returns null for disabled endpoints.
        return HasValidEventPathForEndpoint(path.mEndpointId, path, descriptor);
    }

    for (uint16_t endpointIndex = 0; endpointIndex < emberAfEndpointCount(); ++endpointIndex)
    {
        if (!emberAfEndpointIndexIsEnabled(endpointIndex))
        {
            continue;
        }
        if (HasValidEventPathForEndpoint(emberAfEndpointFromIndex(endpointIndex), path, descriptor))
        {
            return true;
        }
    }
    return false;
}

} // namespace app
} // namespace chip
