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

#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/Provider.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <app/util/AttributesChangedListener.h>
#include <app/util/DataModelHandler.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <app/util/persistence/AttributePersistenceProvider.h>
#include <app/util/persistence/DefaultAttributePersistenceProvider.h>
#include <data-model-providers/codegen/CodegenServerCluster.h>
#include <data-model-providers/codegen/EmberMetadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>
#include <optional>

namespace chip {
namespace app {
namespace {

CodegenServerCluster gCodegenServerCluster;

class ContextAttributesChangeListener : public AttributesChangedListener
{
public:
    ContextAttributesChangeListener(const DataModel::InteractionModelContext & context) : mListener(context.dataModelChangeListener)
    {}
    void MarkDirty(const AttributePathParams & path) override { mListener->MarkDirty(path); }

private:
    DataModel::ProviderChangeListener * mListener;
};

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

const ConcreteCommandPath kInvalidCommandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);

DefaultAttributePersistenceProvider gDefaultAttributePersistence;

} // namespace

CHIP_ERROR CodegenDataModelProvider::Shutdown()
{
    Reset();
    mRegistry.ClearContext();
    gCodegenServerCluster.Shutdown();
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

    mServerClusterContext.reset();
    mServerClusterContext.emplace(ServerClusterContext({
        .provider           = this,
        .storage            = mPersistentStorageDelegate,
        .interactionContext = &mContext,
    }));
    gCodegenServerCluster.Startup(*mServerClusterContext);

    return mRegistry.SetContext({
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

    return gCodegenServerCluster.InvokeCommand(request, input_arguments, handler);
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
    for (const auto clusterId : mRegistry.ClustersOnEndpoint(endpointId))
    {
        ConcreteClusterPath path(endpointId, clusterId);
        ServerClusterInterface * cluster = mRegistry.Get(path);

        // path MUST be valid: we just got it from iterating our registrations...
        VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INTERNAL);

        ReturnErrorOnFailure(builder.Append({
            .clusterId   = path.mClusterId,
            .dataVersion = cluster->GetDataVersion(path),
            .flags       = cluster->GetClusterFlags(path),
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

    return gCodegenServerCluster.Attributes(path, builder);
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
    return gCodegenServerCluster.AcceptedCommands(path, builder);
}

CHIP_ERROR CodegenDataModelProvider::GeneratedCommands(const ConcreteClusterPath & path,
                                                       DataModel::ListBuilder<CommandId> & builder)
{
    if (auto * cluster = mRegistry.Get(path); cluster != nullptr)
    {
        return cluster->GeneratedCommands(path, builder);
    }
    return gCodegenServerCluster.GeneratedCommands(path, builder);
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

DataModel::ActionReturnStatus CodegenDataModelProvider::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                       AttributeValueDecoder & decoder)
{
    if (auto * cluster = mRegistry.Get(request.path); cluster != nullptr)
    {
        return cluster->WriteAttribute(request, decoder);
    }
    return gCodegenServerCluster.WriteAttribute(request, decoder);
}

DataModel::ActionReturnStatus CodegenDataModelProvider::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                      AttributeValueEncoder & encoder)
{
    ChipLogDetail(DataManagement,
                  "Reading attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI " (expanded=%d)",
                  ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId, ChipLogValueMEI(request.path.mAttributeId),
                  request.path.mExpanded);

    if (auto * cluster = mRegistry.Get(request.path); cluster != nullptr)
    {
        return cluster->ReadAttribute(request, encoder);
    }
    return gCodegenServerCluster.ReadAttribute(request, encoder);
}

void CodegenDataModelProvider::ListAttributeWriteNotification(const ConcreteAttributePath & aPath,
                                                              DataModel::ListWriteOperation opType)
{
    if (auto * cluster = mRegistry.Get(aPath); cluster != nullptr)
    {
        cluster->ListAttributeWriteNotification(aPath, opType);
        return;
    }
    gCodegenServerCluster.ListAttributeWriteNotification(aPath, opType);
}

void CodegenDataModelProvider::Temporary_ReportAttributeChanged(const AttributePathParams & path)
{
    ContextAttributesChangeListener change_listener(CurrentContext());
    if (path.mClusterId != kInvalidClusterId)
    {
        emberAfAttributeChanged(path.mEndpointId, path.mClusterId, path.mAttributeId, &change_listener);
    }
    else
    {
        // When the path has wildcard cluster Id, call the emberAfEndpointChanged to mark attributes on the given endpoint
        // as having changing, but do NOT increase/alter any cluster data versions, as this happens when a bridged endpoint is
        // added or removed from a bridge and the cluster data is not changed during the process.
        emberAfEndpointChanged(path.mEndpointId, &change_listener);
    }
}

} // namespace app
} // namespace chip
