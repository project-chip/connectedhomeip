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
#include "app/ConcreteAttributePath.h"
#include "app/util/attribute-metadata.h"
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

DataModel::ServerClusterEntry ServerClusterEntryFrom(EndpointId endpointId, const EmberAfCluster & cluster)
{
    DataModel::ServerClusterEntry entry;

    entry.clusterId = cluster.clusterId;

    DataVersion * versionPtr = emberAfDataVersionStorage(ConcreteClusterPath(endpointId, cluster.clusterId));
    if (versionPtr == nullptr)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to get data version for %d/" ChipLogFormatMEI, static_cast<int>(endpointId),
                     ChipLogValueMEI(cluster.clusterId));
#endif
        entry.dataVersion = 0;
    }
    else
    {
        entry.dataVersion = *versionPtr;
    }

    // TODO: set entry flags:
    //   entry.flags.Set(ClusterQualityFlags::kDiagnosticsData)

    return entry;
}

DataModel::AttributeEntry AttributeEntryFrom(const ConcreteClusterPath & clusterPath, const EmberAfAttributeMetadata & attribute)
{
    DataModel::AttributeEntry entry;

    const ConcreteAttributePath attributePath(clusterPath.mEndpointId, clusterPath.mClusterId, attribute.attributeId);

    entry.attributeId   = attribute.attributeId;
    entry.readPrivilege = RequiredPrivilege::ForReadAttribute(attributePath);
    if (!attribute.IsReadOnly())
    {
        entry.writePrivilege = RequiredPrivilege::ForWriteAttribute(attributePath);
    }

    entry.flags.Set(DataModel::AttributeQualityFlags::kListAttribute, (attribute.attributeType == ZCL_ARRAY_ATTRIBUTE_TYPE));
    entry.flags.Set(DataModel::AttributeQualityFlags::kTimed, attribute.MustUseTimedWrite());

    // NOTE: we do NOT provide additional info for:
    //    - IsExternal/IsSingleton/IsAutomaticallyPersisted is not used by IM handling
    //    - IsSingleton spec defines it for CLUSTERS where as we have it for ATTRIBUTES
    //    - Several specification flags are not available (reportable, quieter reporting,
    //      fixed, source attribution)

    // TODO: Set additional flags:
    // entry.flags.Set(DataModel::AttributeQualityFlags::kFabricScoped)
    // entry.flags.Set(DataModel::AttributeQualityFlags::kFabricSensitive)
    // entry.flags.Set(DataModel::AttributeQualityFlags::kChangesOmitted)
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

DataModel::MetadataList<DataModel::ServerClusterEntry> CodegenDataModelProvider::ServerClusters(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);

    DataModel::MetadataList<DataModel::ServerClusterEntry> result;

    VerifyOrReturnValue(endpoint != nullptr, result);
    VerifyOrReturnValue(endpoint->clusterCount > 0, result);
    VerifyOrReturnValue(endpoint->cluster != nullptr, result);

    const EmberAfCluster * begin = endpoint->cluster;
    const EmberAfCluster * end   = endpoint->cluster + endpoint->clusterCount;

    size_t serverClusterCount = 0;
    for (const EmberAfCluster * cluster = begin; cluster != end; cluster++)
    {
        if (cluster->IsServer())
        {
            serverClusterCount++;
        }
    }

    CHIP_ERROR err = result.reserve(serverClusterCount);
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to reserve space for client clusters: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return {};
    }

    for (const EmberAfCluster * cluster = begin; cluster != end; cluster++)
    {
        if (!cluster->IsServer())
        {
            continue;
        }

        err = result.Append(ServerClusterEntryFrom(endpointId, *cluster));
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to append client cluster id: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
    }

    return result;
}

DataModel::MetadataList<DataModel::AttributeEntry> CodegenDataModelProvider::Attributes(const ConcreteClusterPath & path)
{
    const EmberAfCluster * cluster = FindServerCluster(path);

    DataModel::MetadataList<DataModel::AttributeEntry> result;

    VerifyOrReturnValue(cluster != nullptr, result);
    VerifyOrReturnValue(cluster->attributeCount > 0, result);
    VerifyOrReturnValue(cluster->attributes != nullptr, result);

    CHIP_ERROR err = result.reserve(cluster->attributeCount);
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to reserve space for attributes: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return {};
    }

    Span<const EmberAfAttributeMetadata> attributeSpan(cluster->attributes, cluster->attributeCount);

    for (auto & attribute : attributeSpan)
    {
        err = result.Append(AttributeEntryFrom(path, attribute));
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to append attribute entry: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
    }

    return result;
}

DataModel::MetadataList<ClusterId> CodegenDataModelProvider::ClientClusters(EndpointId endpointId)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);

    DataModel::MetadataList<ClusterId> result;

    VerifyOrReturnValue(endpoint != nullptr, result);
    VerifyOrReturnValue(endpoint->clusterCount > 0, result);
    VerifyOrReturnValue(endpoint->cluster != nullptr, result);

    const EmberAfCluster * begin = endpoint->cluster;
    const EmberAfCluster * end   = endpoint->cluster + endpoint->clusterCount;

    size_t clientClusterCount = 0;

    for (const EmberAfCluster * cluster = begin; cluster != end; cluster++)
    {
        if (cluster->IsClient())
        {
            clientClusterCount++;
        }
    }

    CHIP_ERROR err = result.reserve(clientClusterCount);
    if (err != CHIP_NO_ERROR)
    {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to reserve space for client clusters: %" CHIP_ERROR_FORMAT, err.Format());
#endif
        return {};
    }

    for (const EmberAfCluster * cluster = begin; cluster != end; cluster++)
    {
        if (!cluster->IsClient())
        {
            continue;
        }

        err = result.Append(cluster->clusterId);
        if (err != CHIP_NO_ERROR)
        {
#if CHIP_ERROR_LOGGING && CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
            ChipLogError(AppServer, "Failed to append client cluster id: %" CHIP_ERROR_FORMAT, err.Format());
#endif
            break;
        }
    }

    return result;
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
