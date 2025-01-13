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
#include "lib/core/ErrorStr.h"
#include <cstdint>
#include <data-model-providers/codegen/CodegenDataModelProvider.h>

#include <access/AccessControl.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/MetadataList.h>
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

#include <optional>
#include <variant>

namespace chip {
namespace app {
namespace {

DataModel::AcceptedCommandEntry AcceptedCommandEntryFor(const ConcreteCommandPath & path)
{
    const CommandId commandId = path.mCommandId;

    DataModel::AcceptedCommandEntry entry;

    entry.commandId       = path.mCommandId;
    entry.invokePrivilege = RequiredPrivilege::ForInvokeCommand(path);
    entry.flags.Set(DataModel::CommandQualityFlags::kTimed, CommandNeedsTimedInvoke(path.mClusterId, commandId));
    entry.flags.Set(DataModel::CommandQualityFlags::kFabricScoped, CommandIsFabricScoped(path.mClusterId, commandId));
    entry.flags.Set(DataModel::CommandQualityFlags::kLargeMessage, CommandHasLargePayload(path.mClusterId, commandId));

    return entry;
}

/// Fills `result` with accepted command data. In case of failures,
/// returns the first failure (and stops filling the result, which may
/// be partial)
CHIP_ERROR FetchAcceptedCommands(const ConcreteClusterPath & path, const EmberAfCluster * serverCluster,
                                 DataModel::MetadataList<DataModel::AcceptedCommandEntry> & result)
{

    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);
    if (interface != nullptr)
    {
        size_t commandCount = 0;

        CHIP_ERROR err = interface->EnumerateAcceptedCommands(
            path,
            [](CommandId id, void * context) -> Loop {
                *reinterpret_cast<size_t *>(context) += 1;
                return Loop::Continue;
            },
            reinterpret_cast<void *>(&commandCount));

        if (err == CHIP_NO_ERROR)
        {
            using EnumerationData = struct
            {
                ConcreteCommandPath commandPath;
                DataModel::MetadataList<DataModel::AcceptedCommandEntry> acceptedCommandList;
                CHIP_ERROR processingError;
            };

            EnumerationData enumerationData;
            enumerationData.commandPath     = ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId);
            enumerationData.processingError = CHIP_NO_ERROR;

            ReturnErrorOnFailure(enumerationData.acceptedCommandList.reserve(commandCount));

            ReturnErrorOnFailure(interface->EnumerateAcceptedCommands(
                path,
                [](CommandId commandId, void * context) -> Loop {
                    auto input                    = reinterpret_cast<EnumerationData *>(context);
                    input->commandPath.mCommandId = commandId;
                    CHIP_ERROR appendError        = input->acceptedCommandList.Append(AcceptedCommandEntryFor(input->commandPath));
                    if (appendError != CHIP_NO_ERROR)
                    {
                        input->processingError = appendError;
                        return Loop::Break;
                    }
                    return Loop::Continue;
                },
                reinterpret_cast<void *>(&enumerationData)));
            ReturnErrorOnFailure(enumerationData.processingError);

            // the two invocations MUST return the same sizes.
            VerifyOrReturnError(enumerationData.acceptedCommandList.size() == commandCount, CHIP_ERROR_INTERNAL);

            result = std::move(enumerationData.acceptedCommandList);
            return CHIP_NO_ERROR;
        }
        VerifyOrReturnError(err == CHIP_ERROR_NOT_IMPLEMENTED, err);
    }

    if ((serverCluster == nullptr) || (serverCluster->acceptedCommandList == nullptr))
    {
        // No data if cluster is not valid.
        return CHIP_NO_ERROR;
    }
    const chip::CommandId * endOfList = serverCluster->acceptedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }
    const size_t commandCount = static_cast<size_t>(endOfList - serverCluster->acceptedCommandList);

    ReturnErrorOnFailure(result.reserve(commandCount));

    ConcreteCommandPath commandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId);
    for (const chip::CommandId * p = serverCluster->acceptedCommandList; p != endOfList; p++)
    {
        commandPath.mCommandId = *p;
        ReturnErrorOnFailure(result.Append(AcceptedCommandEntryFor(commandPath)));
    }

    return CHIP_NO_ERROR;
}

/// Fills `result` with generated command data. In case of failures,
/// returns the first failure (and stops filling the result, which may
/// be partial)
CHIP_ERROR FetchGeneratedCommands(const ConcreteClusterPath & path, const EmberAfCluster * serverCluster,
                                  DataModel::MetadataList<CommandId> & result)
{
    CommandHandlerInterface * interface =
        CommandHandlerInterfaceRegistry::Instance().GetCommandHandler(path.mEndpointId, path.mClusterId);
    if (interface != nullptr)
    {
        size_t commandCount = 0;

        CHIP_ERROR err = interface->EnumerateGeneratedCommands(
            path,
            [](CommandId id, void * context) -> Loop {
                *reinterpret_cast<size_t *>(context) += 1;
                return Loop::Continue;
            },
            reinterpret_cast<void *>(&commandCount));

        if (err == CHIP_NO_ERROR)
        {
            ReturnErrorOnFailure(result.reserve(commandCount));

            using EnumerationData = struct
            {
                DataModel::MetadataList<CommandId> generatedCommandList;
                CHIP_ERROR processingError;
            };
            EnumerationData enumerationData;
            enumerationData.processingError = CHIP_NO_ERROR;

            ReturnErrorOnFailure(interface->EnumerateGeneratedCommands(
                path,
                [](CommandId id, void * context) -> Loop {
                    auto input = reinterpret_cast<EnumerationData *>(context);

                    CHIP_ERROR appendError = input->generatedCommandList.Append(id);
                    if (appendError != CHIP_NO_ERROR)
                    {
                        input->processingError = appendError;
                        return Loop::Break;
                    }
                    return Loop::Continue;
                },
                reinterpret_cast<void *>(&result)));
            ReturnErrorOnFailure(enumerationData.processingError);

            // the two invocations MUST return the same sizes.
            VerifyOrReturnError(enumerationData.generatedCommandList.size() == commandCount, CHIP_ERROR_INTERNAL);

            result = std::move(enumerationData.generatedCommandList);
            return CHIP_NO_ERROR;
        }
        VerifyOrReturnError(err == CHIP_ERROR_NOT_IMPLEMENTED, err);
    }

    if ((serverCluster == nullptr) || (serverCluster->generatedCommandList == nullptr))
    {
        return {};
    }
    const chip::CommandId * endOfList = serverCluster->generatedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }
    const size_t commandCount = static_cast<size_t>(endOfList - serverCluster->generatedCommandList);
    result = DataModel::MetadataList<CommandId>::FromConstSpan({ serverCluster->generatedCommandList, commandCount });

    return CHIP_NO_ERROR;
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

// TODO: DeviceTypeEntry content is IDENTICAL to EmberAfDeviceType, so centralizing
//       to a common type is probably better. Need to figure out dependencies since
//       this would make ember return datamodel-provider types.
//       See: https://github.com/project-chip/connectedhomeip/issues/35889
DataModel::DeviceTypeEntry DeviceTypeEntryFromEmber(const EmberAfDeviceType & other)
{
    return DataModel::DeviceTypeEntry{
        .deviceTypeId       = other.deviceId,
        .deviceTypeRevision = other.deviceVersion,
    };
}

const ConcreteCommandPath kInvalidCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);

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

DataModel::MetadataList<DataModel::EndpointEntry> CodegenDataModelProvider::Endpoints()
{
    DataModel::MetadataList<DataModel::EndpointEntry> result;

    const uint16_t endpointCount = emberAfEndpointCount();

    // allocate the max as some endpoints may be disabled
    CHIP_ERROR err = result.reserve(endpointCount);
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to allocate space for endpoints: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return {};
    }

    for (uint16_t endpointIndex = 0; endpointIndex < endpointCount; endpointIndex++)
    {
        if (!emberAfEndpointIndexIsEnabled(endpointIndex))
        {
            continue;
        }

        DataModel::EndpointEntry entry;
        entry.id       = emberAfEndpointFromIndex(endpointIndex);
        entry.parentId = emberAfParentEndpointFromIndex(endpointIndex);

        switch (GetCompositionForEndpointIndex(endpointIndex))
        {
        case EndpointComposition::kFullFamily:
            entry.compositionPattern = DataModel::EndpointCompositionPattern::kFullFamily;
            break;
        case EndpointComposition::kTree:
        default:
            entry.compositionPattern = DataModel::EndpointCompositionPattern::kTree;
            break;
        }

        err = result.Append(entry);
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to append endpoint data: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
    }

    return result;
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

DataModel::MetadataList<DataModel::AcceptedCommandEntry>
CodegenDataModelProvider::AcceptedCommands(const ConcreteClusterPath & path)
{
    DataModel::MetadataList<DataModel::AcceptedCommandEntry> result;

    [[maybe_unused]] CHIP_ERROR err = FetchAcceptedCommands(path, FindServerCluster(path), result);

#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to fetch accepted commands: %" CHIP_ERROR_FORMAT, err.Format());
    }
#endif

    return result;
}

DataModel::MetadataList<CommandId> CodegenDataModelProvider::GeneratedCommands(const ConcreteClusterPath & path)
{
    DataModel::MetadataList<CommandId> result;

    [[maybe_unused]] CHIP_ERROR err = FetchGeneratedCommands(path, FindServerCluster(path), result);

#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to fetch generated commands: %" CHIP_ERROR_FORMAT, err.Format());
    }
#endif

    return result;
}

void CodegenDataModelProvider::InitDataModelForTesting()
{
    // Call the Ember-specific InitDataModelHandler
    InitDataModelHandler();
}

DataModel::MetadataList<DataModel::DeviceTypeEntry> CodegenDataModelProvider::DeviceTypes(EndpointId endpointId)
{
    std::optional<unsigned> endpoint_index = TryFindEndpointIndex(endpointId);
    if (!endpoint_index.has_value())
    {
        return {};
    }

    CHIP_ERROR err                            = CHIP_NO_ERROR;
    Span<const EmberAfDeviceType> deviceTypes = emberAfDeviceTypeListFromEndpointIndex(*endpoint_index, err);

    DataModel::MetadataList<DataModel::DeviceTypeEntry> result;
    err = result.reserve(deviceTypes.size());
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to reserve device type buffer space: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return {};
    }
    for (auto & entry : deviceTypes)
    {
        err = result.Append(DeviceTypeEntryFromEmber(entry));
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to append device type entry: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
    }

    return result;
}

DataModel::MetadataList<DataModel::Provider::SemanticTag> CodegenDataModelProvider::SemanticTags(EndpointId endpointId)
{
    DataModel::Provider::SemanticTag semanticTag;
    size_t count = 0;

    while (GetSemanticTagForEndpointAtIndex(endpointId, count, semanticTag) == CHIP_NO_ERROR)
    {
        count++;
    }
    DataModel::MetadataList<DataModel::Provider::SemanticTag> result;

    CHIP_ERROR err = result.reserve(count);
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to reserve semantic tag buffer space: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return {};
    }

    for (size_t idx = 0; idx < count; idx++)
    {
        err = GetSemanticTagForEndpointAtIndex(endpointId, idx, semanticTag);
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to get semantic tag data: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
        err = result.Append(semanticTag);
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to append semantic tag: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
    }

    return result;
}

} // namespace app
} // namespace chip
