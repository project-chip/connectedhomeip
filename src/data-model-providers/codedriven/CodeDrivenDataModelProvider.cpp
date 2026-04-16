/*
 *    Copyright (c) 2025 Project CHIP Authors
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
#include "CodeDrivenDataModelProvider.h"
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

CHIP_ERROR CodeDrivenDataModelProvider::Startup(DataModel::InteractionModelContext context)
{
    ReturnErrorOnFailure(DataModel::Provider::Startup(context));

    mInteractionModelContext.emplace(context);

    mServerClusterContext.emplace(ServerClusterContext{
        .provider           = *this,
        .storage            = mPersistentStorageDelegate,
        .attributeStorage   = mAttributePersistenceProvider,
        .interactionContext = *mInteractionModelContext,
    });

    // Start up registered server clusters if one of their associated endpoints is registered.
    bool had_failure = false;
    for (auto * cluster : mServerClusterRegistry.AllServerClusterInstances())
    {
        bool endpointRegistered = false;
        for (const auto & path : cluster->GetPaths())
        {
            if (mEndpointInterfaceRegistry.Get(path.mEndpointId) != nullptr)
            {
                endpointRegistered = true;
                break;
            }
        }

        if (endpointRegistered)
        {
            if (cluster->Startup(*mServerClusterContext) != CHIP_NO_ERROR)
            {
                had_failure = true;
            }
        }
    }

    if (had_failure)
    {
        return CHIP_ERROR_HAD_FAILURES;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CodeDrivenDataModelProvider::Shutdown()
{
    bool had_failure = false;

    // Remove all endpoints. This will trigger Shutdown() on associated clusters.
    while (mEndpointInterfaceRegistry.begin() != mEndpointInterfaceRegistry.end())
    {
        if (RemoveEndpoint(mEndpointInterfaceRegistry.begin()->GetEndpointEntry().id) != CHIP_NO_ERROR)
        {
            had_failure = true;
        }
    }

    // Now we're safe to clean up the cluster registry.
    while (mServerClusterRegistry.AllServerClusterInstances().begin() != mServerClusterRegistry.AllServerClusterInstances().end())
    {
        ServerClusterInterface * clusterToRemove = *mServerClusterRegistry.AllServerClusterInstances().begin();
        if (mServerClusterRegistry.Unregister(clusterToRemove) != CHIP_NO_ERROR)
        {
            had_failure = true;
        }
    }

    mServerClusterContext.reset();
    mInteractionModelContext.reset();

    if (had_failure)
    {
        return CHIP_ERROR_HAD_FAILURES;
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus CodeDrivenDataModelProvider::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(request.path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->ReadAttribute(request, encoder);
}

DataModel::ActionReturnStatus CodeDrivenDataModelProvider::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                          AttributeValueDecoder & decoder)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(request.path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->WriteAttribute(request, decoder);
}

void CodeDrivenDataModelProvider::ListAttributeWriteNotification(const ConcreteAttributePath & path,
                                                                 DataModel::ListWriteOperation opType, FabricIndex accessingFabric)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(path);
    VerifyOrReturn(serverCluster != nullptr);
    serverCluster->ListAttributeWriteNotification(path, opType, accessingFabric);
}

std::optional<DataModel::ActionReturnStatus> CodeDrivenDataModelProvider::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(request.path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->InvokeCommand(request, input_arguments, handler);
}

CHIP_ERROR CodeDrivenDataModelProvider::Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & out)
{
    // TODO: Add a size() method to EndpointInterfaceRegistry to avoid iterating twice.
    size_t count = 0;
    for (const auto & registration : mEndpointInterfaceRegistry)
    {
        (void) registration; // Silence unused variable warning
        count++;
    }

    ReturnErrorOnFailure(out.EnsureAppendCapacity(count));
    for (const auto & registration : mEndpointInterfaceRegistry)
    {
        ReturnErrorOnFailure(out.Append(registration.GetEndpointEntry()));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CodeDrivenDataModelProvider::DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out)
{
    EndpointInterface * endpoint = GetEndpointInterface(endpointId);
    VerifyOrReturnError(endpoint != nullptr, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
    return endpoint->DeviceTypes(out);
}

CHIP_ERROR CodeDrivenDataModelProvider::ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & out)
{
    EndpointInterface * endpoint = GetEndpointInterface(endpointId);
    VerifyOrReturnError(endpoint != nullptr, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
    return endpoint->ClientClusters(out);
}

CHIP_ERROR CodeDrivenDataModelProvider::ServerClusters(EndpointId endpointId,
                                                       ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> & out)
{
    EndpointInterface * endpoint = GetEndpointInterface(endpointId);
    VerifyOrReturnError(endpoint != nullptr, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));

    size_t count = 0;
    for (auto * cluster : mServerClusterRegistry.AllServerClusterInstances())
    {
        for (const auto & path : cluster->GetPaths())
        {
            if (path.mEndpointId == endpointId)
            {
                count++;
            }
        }
    }

    ReturnErrorOnFailure(out.EnsureAppendCapacity(count));

    for (auto * cluster : mServerClusterRegistry.AllServerClusterInstances())
    {
        for (const auto & path : cluster->GetPaths())
        {
            if (path.mEndpointId == endpointId)
            {
                ReturnErrorOnFailure(
                    out.Append({ path.mClusterId, cluster->GetDataVersion(path), cluster->GetClusterFlags(path) }));
            }
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CodeDrivenDataModelProvider::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & out)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->GeneratedCommands(path, out);
}
CHIP_ERROR CodeDrivenDataModelProvider::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & out)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->AcceptedCommands(path, out);
}

CHIP_ERROR CodeDrivenDataModelProvider::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & out)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->Attributes(path, out);
}

CHIP_ERROR CodeDrivenDataModelProvider::EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo)
{
    ServerClusterInterface * serverCluster = GetServerClusterInterface(path);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_KEY_NOT_FOUND);
    return serverCluster->EventInfo(path, eventInfo);
}

#if CHIP_CONFIG_USE_ENDPOINT_UNIQUE_ID
CHIP_ERROR CodeDrivenDataModelProvider::EndpointUniqueID(EndpointId endpointId, MutableCharSpan & EndpointUniqueId)
{
    EndpointInterface * endpoint = GetEndpointInterface(endpointId);
    VerifyOrReturnError(endpoint != nullptr, CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint));
    CharSpan uniqueId = endpoint->EndpointUniqueID();
    return CopyCharSpanToMutableCharSpan(uniqueId, EndpointUniqueId);
}
#endif

void CodeDrivenDataModelProvider::Temporary_ReportAttributeChanged(const AttributePathParams & path)
{
    if (!mInteractionModelContext)
    {
        ChipLogError(DataManagement, "Temporary_ReportAttributeChanged called before provider has been started.");
        return;
    }
    mInteractionModelContext->dataModelChangeListener.MarkDirty(path);
}

CHIP_ERROR CodeDrivenDataModelProvider::AddEndpoint(EndpointInterfaceRegistration & registration)
{
    VerifyOrReturnError(registration.endpointEntry.id != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);

    // If the endpoint ID is already in use, return an error.
    if (mEndpointInterfaceRegistry.Get(registration.endpointEntry.id) != nullptr)
    {
        return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    ReturnErrorOnFailure(mEndpointInterfaceRegistry.Register(registration));

    if (mServerClusterContext.has_value())
    {
        // If the provider has been started, we need to check if any clusters on this new endpoint
        // should be started up.
        for (auto * cluster : mServerClusterRegistry.AllServerClusterInstances())
        {
            bool clusterIsOnNewEndpoint = false;
            int registeredEndpointCount = 0;

            for (const auto & path : cluster->GetPaths())
            {
                if (mEndpointInterfaceRegistry.Get(path.mEndpointId) != nullptr)
                {
                    registeredEndpointCount++;
                }
                if (path.mEndpointId == registration.endpointEntry.id)
                {
                    clusterIsOnNewEndpoint = true;
                }
            }

            // If the cluster is on the endpoint we just added, and this is the *only*
            // registered endpoint for this cluster, it's time to start it.
            if (clusterIsOnNewEndpoint && registeredEndpointCount == 1)
            {
                ReturnErrorOnFailure(cluster->Startup(*mServerClusterContext));
            }
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CodeDrivenDataModelProvider::RemoveEndpoint(EndpointId endpointId, ClusterShutdownType shutdownType)
{
    if (mServerClusterContext.has_value())
    {
        // If the provider has been started, we need to check if any clusters on this endpoint
        // need to be shut down because it's their last registered endpoint.
        for (auto * cluster : mServerClusterRegistry.AllServerClusterInstances())
        {
            bool clusterIsOnEndpoint    = false;
            int registeredEndpointCount = 0;

            for (const auto & path : cluster->GetPaths())
            {
                if (mEndpointInterfaceRegistry.Get(path.mEndpointId) != nullptr)
                {
                    registeredEndpointCount++;
                }
                if (path.mEndpointId == endpointId)
                {
                    clusterIsOnEndpoint = true;
                }
            }

            if (clusterIsOnEndpoint && registeredEndpointCount == 1)
            {
                // This is the last registered endpoint for this cluster. Shut it down.
                cluster->Shutdown(shutdownType);
            }
        }
    }

    return mEndpointInterfaceRegistry.Unregister(endpointId);
}

CHIP_ERROR CodeDrivenDataModelProvider::AddCluster(ServerClusterRegistration & entry)
{
    VerifyOrReturnError(entry.serverClusterInterface != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (mServerClusterContext.has_value())
    {
        // If the provider has been started, prevent non-atomic changes to an endpoint.
        // Check if any of the cluster's paths are associated with an already registered endpoint.
        for (const auto & path : entry.serverClusterInterface->GetPaths())
        {
            if (mEndpointInterfaceRegistry.Get(path.mEndpointId) != nullptr)
            {
                return CHIP_ERROR_INCORRECT_STATE;
            }
        }
    }

    return mServerClusterRegistry.Register(entry);
}

CHIP_ERROR CodeDrivenDataModelProvider::RemoveCluster(ServerClusterInterface * cluster, ClusterShutdownType shutdownType)
{
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (mServerClusterContext.has_value())
    {
        for (const auto & path : cluster->GetPaths())
        {
            if (mEndpointInterfaceRegistry.Get(path.mEndpointId) != nullptr)
            {
                return CHIP_ERROR_INCORRECT_STATE;
            }
        }
    }

    return mServerClusterRegistry.Unregister(cluster, shutdownType);
}

EndpointInterface * CodeDrivenDataModelProvider::GetEndpointInterface(EndpointId endpointId)
{
    return mEndpointInterfaceRegistry.Get(endpointId);
}

ServerClusterInterface * CodeDrivenDataModelProvider::GetServerClusterInterface(const ConcreteClusterPath & clusterPath)
{
    return mServerClusterRegistry.Get(clusterPath);
}

} // namespace app
} // namespace chip
