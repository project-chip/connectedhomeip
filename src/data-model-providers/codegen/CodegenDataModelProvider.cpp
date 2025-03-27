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
#include <access/Privilege.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventPathParams.h>
#include <app/GlobalAttributes.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/util/DataModelHandler.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <data-model-providers/codegen/EmberMetadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/SpanSearchValue.h>

#include <cstdint>
#include <optional>

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

DataModel::ServerClusterEntry ServerClusterEntryFrom(EndpointId endpointId, const EmberAfCluster & cluster)
{
    DataModel::ServerClusterEntry entry;

    entry.clusterId = cluster.clusterId;

    DataVersion * versionPtr = emberAfDataVersionStorage(ConcreteClusterPath(endpointId, cluster.clusterId));
    if (versionPtr == nullptr)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(AppServer, "Failed to get data version for %d/" ChipLogFormatMEI, endpointId,
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

const ConcreteCommandPath kInvalidCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);

DefaultAttributePersistenceProvider gDefaultAttributePersistence;

} // namespace

CHIP_ERROR CodegenDataModelProvider::Shutdown()
{
    Reset();
    mRegistry.ClearContext();
    return CHIP_NO_ERROR;
}

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

    return mRegistry.SetContext(ServerClusterContext{
        .provider           = this,
        .storage            = mPersistentStorageDelegate,
        .interactionContext = &mContext,
    });
}

std::optional<DataModel::ActionReturnStatus> CodegenDataModelProvider::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                     TLV::TLVReader & input_arguments,
                                                                                     CommandHandler * handler)
{
    if (auto * cluster = mRegistry.Get(request.path); cluster != nullptr)
    {
        return cluster->InvokeCommand(request, input_arguments, handler);
    }

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

CHIP_ERROR CodegenDataModelProvider::Endpoints(DataModel::ListBuilder<DataModel::EndpointEntry> & builder)
{
    const uint16_t endpointCount = emberAfEndpointCount();

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(endpointCount));

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
        case EndpointComposition::kInvalid: // should NOT happen, but force compiler to check we validate all versions
            entry.compositionPattern = DataModel::EndpointCompositionPattern::kTree;
            break;
        }
        ReturnErrorOnFailure(builder.Append(entry));
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR CodegenDataModelProvider::ServerClusters(EndpointId endpointId,
                                                    DataModel::ListBuilder<DataModel::ServerClusterEntry> & builder)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);

    VerifyOrReturnValue(endpoint != nullptr, CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnValue(endpoint->clusterCount > 0, CHIP_NO_ERROR);
    VerifyOrReturnValue(endpoint->cluster != nullptr, CHIP_NO_ERROR);

    // We build the cluster list by merging two lists:
    //   - mRegistry items from ServerClusterInterfaces
    //   - ember metadata clusters
    //
    // This is done because `ServerClusterInterface` allows full control for all its metadata,
    // in particular `data version` and `flags`.
    //
    // To allow cluster implementations to be incrementally converted to storing their own data versions,
    // instead of relying on the out-of-band emberAfDataVersionStorage, first check for clusters that are
    // using the new data version storage and are registered via ServerClusterInterfaceRegistry, then fill
    // in the data versions for the rest via the out-of-band mechanism.

    // assume the clusters on endpoint does not change in between these two loops
    auto clusters               = mRegistry.ClustersOnEndpoint(endpointId);
    size_t registryClusterCount = 0;
    for ([[maybe_unused]] auto _ : clusters)
    {
        registryClusterCount++;
    }

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(registryClusterCount));

    DataModel::ListBuilder<ClusterId> knownClustersBuilder;
    ReturnErrorOnFailure(knownClustersBuilder.EnsureAppendCapacity(registryClusterCount));
    for (auto * cluster : mRegistry.ClustersOnEndpoint(endpointId))
    {
        const ConcreteClusterPath path = cluster->GetPath();
        ReturnErrorOnFailure(builder.Append({
            .clusterId   = path.mClusterId,
            .dataVersion = cluster->GetDataVersion(),
            .flags       = cluster->GetClusterFlags(),
        }));
        ReturnErrorOnFailure(knownClustersBuilder.Append(path.mClusterId));
    }

    DataModel::ReadOnlyBuffer<ClusterId> knownClusters = knownClustersBuilder.TakeBuffer();

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(emberAfClusterCountForEndpointType(endpoint, /* server = */ true)));

    const EmberAfCluster * begin = endpoint->cluster;
    const EmberAfCluster * end   = endpoint->cluster + endpoint->clusterCount;
    for (const EmberAfCluster * cluster = begin; cluster != end; cluster++)
    {
        if (!cluster->IsServer())
        {
            continue;
        }

        // linear search as this is a somewhat compact number list, so performance is probably not too bad
        // This results in smaller code than some memory allocation + std::sort + std::binary_search
        bool found = false;
        for (ClusterId clusterId : knownClusters)
        {
            if (clusterId == cluster->clusterId)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            // value already filled from the ServerClusterRegistry. That one has the correct/overriden
            // flags and data version
            continue;
        }

        ReturnErrorOnFailure(builder.Append(ServerClusterEntryFrom(endpointId, *cluster)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CodegenDataModelProvider::Attributes(const ConcreteClusterPath & path,
                                                DataModel::ListBuilder<DataModel::AttributeEntry> & builder)
{
    if (auto * cluster = mRegistry.Get(path); cluster != nullptr)
    {
        return cluster->Attributes(path, builder);
    }

    const EmberAfCluster * cluster = FindServerCluster(path);

    VerifyOrReturnValue(cluster != nullptr, CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnValue(cluster->attributeCount > 0, CHIP_NO_ERROR);
    VerifyOrReturnValue(cluster->attributes != nullptr, CHIP_NO_ERROR);

    // TODO: if ember would encode data in AttributeEntry form, we could reference things directly (shorter code,
    //       although still allocation overhead due to global attributes not in metadata)
    //
    // We have Attributes from ember + global attributes that are NOT in ember metadata.
    // We have to report them all
    constexpr size_t kGlobalAttributeNotInMetadataCount = MATTER_ARRAY_SIZE(GlobalAttributesNotInMetadata);

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(cluster->attributeCount + kGlobalAttributeNotInMetadataCount));

    Span<const EmberAfAttributeMetadata> attributeSpan(cluster->attributes, cluster->attributeCount);

    for (auto & attribute : attributeSpan)
    {
        ReturnErrorOnFailure(builder.Append(AttributeEntryFrom(path, attribute)));
    }

    // This "GlobalListEntry" is specific for metadata that ember does not include
    // in its attribute list metadata.
    //
    // By spec these Attribute/AcceptedCommands/GeneratedCommants lists are:
    //   - lists of elements
    //   - read-only, with read privilege view
    //   - fixed value (no such flag exists, so this is not a quality flag we set/track)
    DataModel::AttributeEntry globalListEntry;

    globalListEntry.readPrivilege = Access::Privilege::kView;
    globalListEntry.flags.Set(DataModel::AttributeQualityFlags::kListAttribute);

    for (auto & attribute : GlobalAttributesNotInMetadata)
    {
        globalListEntry.attributeId = attribute;
        ReturnErrorOnFailure(builder.Append(globalListEntry));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CodegenDataModelProvider::ClientClusters(EndpointId endpointId, DataModel::ListBuilder<ClusterId> & builder)
{
    const EmberAfEndpointType * endpoint = emberAfFindEndpointType(endpointId);

    VerifyOrReturnValue(endpoint != nullptr, CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnValue(endpoint->clusterCount > 0, CHIP_NO_ERROR);
    VerifyOrReturnValue(endpoint->cluster != nullptr, CHIP_NO_ERROR);

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(emberAfClusterCountForEndpointType(endpoint, /* server = */ false)));

    const EmberAfCluster * begin = endpoint->cluster;
    const EmberAfCluster * end   = endpoint->cluster + endpoint->clusterCount;
    for (const EmberAfCluster * cluster = begin; cluster != end; cluster++)
    {
        if (!cluster->IsClient())
        {
            continue;
        }
        ReturnErrorOnFailure(builder.Append(cluster->clusterId));
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR CodegenDataModelProvider::AcceptedCommands(const ConcreteClusterPath & path,
                                                      DataModel::ListBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (auto * cluster = mRegistry.Get(path); cluster != nullptr)
    {
        return cluster->AcceptedCommands(path, builder);
    }

    // Some CommandHandlerInterface instances are registered of ALL endpoints, so make sure first that
    // the cluster actually exists on this endpoint before asking the CommandHandlerInterface what commands
    // it claims to support.
    const EmberAfCluster * serverCluster = FindServerCluster(path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);

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
                DataModel::ListBuilder<DataModel::AcceptedCommandEntry> * acceptedCommandList;
                CHIP_ERROR processingError;
            };

            EnumerationData enumerationData;
            enumerationData.commandPath         = ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId);
            enumerationData.processingError     = CHIP_NO_ERROR;
            enumerationData.acceptedCommandList = &builder;

            ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

            ReturnErrorOnFailure(interface->EnumerateAcceptedCommands(
                path,
                [](CommandId commandId, void * context) -> Loop {
                    auto input                    = reinterpret_cast<EnumerationData *>(context);
                    input->commandPath.mCommandId = commandId;
                    CHIP_ERROR appendError        = input->acceptedCommandList->Append(AcceptedCommandEntryFor(input->commandPath));
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
            VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
            return CHIP_NO_ERROR;
        }
        VerifyOrReturnError(err == CHIP_ERROR_NOT_IMPLEMENTED, err);
    }

    VerifyOrReturnError(serverCluster->acceptedCommandList != nullptr, CHIP_NO_ERROR);

    const chip::CommandId * endOfList = serverCluster->acceptedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }
    const auto commandCount = static_cast<size_t>(endOfList - serverCluster->acceptedCommandList);

    // TODO: if ember would store command entries, we could simplify this code to use static data
    ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

    ConcreteCommandPath commandPath = ConcreteCommandPath(path.mEndpointId, path.mClusterId, kInvalidCommandId);
    for (const chip::CommandId * p = serverCluster->acceptedCommandList; p != endOfList; p++)
    {
        commandPath.mCommandId = *p;
        ReturnErrorOnFailure(builder.Append(AcceptedCommandEntryFor(commandPath)));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CodegenDataModelProvider::GeneratedCommands(const ConcreteClusterPath & path,
                                                       DataModel::ListBuilder<CommandId> & builder)
{
    if (auto * cluster = mRegistry.Get(path); cluster != nullptr)
    {
        return cluster->GeneratedCommands(path, builder);
    }

    // Some CommandHandlerInterface instances are registered of ALL endpoints, so make sure first that
    // the cluster actually exists on this endpoint before asking the CommandHandlerInterface what commands
    // it claims to support.
    const EmberAfCluster * serverCluster = FindServerCluster(path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);

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
            ReturnErrorOnFailure(builder.EnsureAppendCapacity(commandCount));

            using EnumerationData = struct
            {
                DataModel::ListBuilder<CommandId> * generatedCommandList;
                CHIP_ERROR processingError;
            };
            EnumerationData enumerationData;
            enumerationData.processingError      = CHIP_NO_ERROR;
            enumerationData.generatedCommandList = &builder;

            ReturnErrorOnFailure(interface->EnumerateGeneratedCommands(
                path,
                [](CommandId id, void * context) -> Loop {
                    auto input = reinterpret_cast<EnumerationData *>(context);

                    CHIP_ERROR appendError = input->generatedCommandList->Append(id);
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
            VerifyOrReturnError(builder.Size() == commandCount, CHIP_ERROR_INTERNAL);
            return CHIP_NO_ERROR;
        }
        VerifyOrReturnError(err == CHIP_ERROR_NOT_IMPLEMENTED, err);
    }

    VerifyOrReturnError(serverCluster->generatedCommandList != nullptr, CHIP_NO_ERROR);

    const chip::CommandId * endOfList = serverCluster->generatedCommandList;
    while (*endOfList != kInvalidCommandId)
    {
        endOfList++;
    }
    const auto commandCount = static_cast<size_t>(endOfList - serverCluster->generatedCommandList);
    return builder.ReferenceExisting({ serverCluster->generatedCommandList, commandCount });
}

void CodegenDataModelProvider::InitDataModelForTesting()
{
    // Call the Ember-specific InitDataModelHandler
    InitDataModelHandler();
}

CHIP_ERROR CodegenDataModelProvider::DeviceTypes(EndpointId endpointId,
                                                 DataModel::ListBuilder<DataModel::DeviceTypeEntry> & builder)
{
    std::optional<unsigned> endpoint_index = TryFindEndpointIndex(endpointId);
    if (!endpoint_index.has_value())
    {
        return {};
    }

    CHIP_ERROR err = CHIP_NO_ERROR;

    builder.ReferenceExisting(emberAfDeviceTypeListFromEndpointIndex(*endpoint_index, err));
    return CHIP_NO_ERROR;
}

CHIP_ERROR CodegenDataModelProvider::SemanticTags(EndpointId endpointId, DataModel::ListBuilder<SemanticTag> & builder)
{
    DataModel::Provider::SemanticTag semanticTag;
    size_t count = 0;

    while (GetSemanticTagForEndpointAtIndex(endpointId, count, semanticTag) == CHIP_NO_ERROR)
    {
        count++;
    }

    ReturnErrorOnFailure(builder.EnsureAppendCapacity(count));

    for (size_t idx = 0; idx < count; idx++)
    {
        ReturnErrorOnFailure(GetSemanticTagForEndpointAtIndex(endpointId, idx, semanticTag));
        ReturnErrorOnFailure(builder.Append(semanticTag));
    }

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
