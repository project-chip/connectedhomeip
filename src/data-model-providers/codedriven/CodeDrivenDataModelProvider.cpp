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
#include <app/server-cluster/ServerClusterContext.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <data-model-providers/codedriven/endpoint/EndpointInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

namespace {

using namespace chip;
using namespace chip::app;

CHIP_ERROR StartupServerClusters(EndpointInterface * endpointProvider, ServerClusterContext & serverClusterContext)
{
    VerifyOrReturnError(endpointProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    bool had_failure = false;

    ReadOnlyBufferBuilder<ServerClusterInterface *> serverClusterBuilder;
    ReturnErrorOnFailure(endpointProvider->ServerClusters(serverClusterBuilder));
    auto serverClusters = serverClusterBuilder.TakeBuffer();

    for (auto * serverCluster : serverClusters)
    {
        VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        CHIP_ERROR err = serverCluster->Startup(serverClusterContext);
        if (err != CHIP_NO_ERROR)
        {
            had_failure = true;
            VerifyOrDie(!serverCluster->GetPaths().empty()); // API says it must have at least one path

            const ConcreteClusterPath path = serverCluster->GetPaths().front();
            ChipLogError(DataManagement, "Cluster " ChipLogFormatMEI " on Endpoint %u startup failed: %" CHIP_ERROR_FORMAT,
                         ChipLogValueMEI(path.mClusterId), path.mEndpointId, err.Format());
        }
    }
    if (had_failure)
    {
        // If we had any failures during the startup of clusters, return an error.
        return CHIP_ERROR_HAD_FAILURES;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ShutdownServerClusters(EndpointInterface * endpointProvider)
{
    VerifyOrReturnError(endpointProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReadOnlyBufferBuilder<ServerClusterInterface *> serverClusterBuilder;
    ReturnErrorOnFailure(endpointProvider->ServerClusters(serverClusterBuilder));
    auto serverClusters = serverClusterBuilder.TakeBuffer();

    for (auto * serverCluster : serverClusters)
    {
        if (serverCluster != nullptr)
        {
            // TODO: The API for ServerClusterInterface->Shutdown swallows errors here. We should update it to return CHIP_ERROR.
            serverCluster->Shutdown();
        }
    }
    return CHIP_NO_ERROR;
}

} // namespace

namespace chip {
namespace app {

CHIP_ERROR CodeDrivenDataModelProvider::Startup(DataModel::InteractionModelContext context)
{
    ReturnErrorOnFailure(DataModel::Provider::Startup(context));

    mServerClusterContext.emplace(ServerClusterContext({
        .provider           = this,
        .storage            = GetPersistentStorageDelegate(),
        .interactionContext = &mContext,
    }));

    // Startup all server clusters across all endpoints
    bool had_failure = false;
    for (auto * endpointProvider : mEndpointInterfaceRegistry)
    {
        if (StartupServerClusters(endpointProvider, *mServerClusterContext) != CHIP_NO_ERROR)
        {
            had_failure = true;
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
    // Shutdown all server clusters across all endpoints
    for (auto * endpointProvider : mEndpointInterfaceRegistry)
    {
        ShutdownServerClusters(endpointProvider);
    }
    mServerClusterContext.reset();
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus CodeDrivenDataModelProvider::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                         AttributeValueEncoder & encoder)
{
    DataModel::ActionReturnStatus status{ Status::Success };
    ServerClusterInterface * serverCluster = GetServerCluster(request.path.mEndpointId, request.path.mClusterId, &status);
    VerifyOrReturnError(serverCluster != nullptr, status);
    return serverCluster->ReadAttribute(request, encoder);
}

DataModel::ActionReturnStatus CodeDrivenDataModelProvider::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                          AttributeValueDecoder & decoder)
{
    DataModel::ActionReturnStatus status{ Status::Success };
    ServerClusterInterface * serverCluster = GetServerCluster(request.path.mEndpointId, request.path.mClusterId, &status);
    VerifyOrReturnError(serverCluster != nullptr, status);
    return serverCluster->WriteAttribute(request, decoder);
}

void CodeDrivenDataModelProvider::ListAttributeWriteNotification(const ConcreteAttributePath & path,
                                                                 DataModel::ListWriteOperation opType)
{
    ServerClusterInterface * serverCluster = GetServerCluster(path.mEndpointId, path.mClusterId, nullptr);
    if (serverCluster != nullptr)
    {
        serverCluster->ListAttributeWriteNotification(path, opType);
    }
}

std::optional<DataModel::ActionReturnStatus> CodeDrivenDataModelProvider::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                        TLV::TLVReader & input_arguments,
                                                                                        CommandHandler * handler)
{
    DataModel::ActionReturnStatus status{ Status::Success };
    ServerClusterInterface * serverCluster = GetServerCluster(request.path.mEndpointId, request.path.mClusterId, &status);
    VerifyOrReturnError(serverCluster != nullptr, status);
    return serverCluster->InvokeCommand(request, input_arguments, handler);
}

CHIP_ERROR CodeDrivenDataModelProvider::Endpoints(ReadOnlyBufferBuilder<DataModel::EndpointEntry> & out)
{
    //TODO: Add a size() method to EndpointInterfaceRegistry to avoid iterating twice.
    size_t count = 0;
    for (const auto * endpointProvider : mEndpointInterfaceRegistry)
    {
        (void) endpointProvider; // Silence unused variable warning
        count++;
    }

    ReturnErrorOnFailure(out.EnsureAppendCapacity(count));
    for (const auto * endpointProvider : mEndpointInterfaceRegistry)
    {
        ReturnErrorOnFailure(out.Append(endpointProvider->GetEndpointEntry()));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR
CodeDrivenDataModelProvider::SemanticTags(EndpointId endpointId,
                                          ReadOnlyBufferBuilder<Clusters::Descriptor::Structs::SemanticTagStruct::Type> & out)
{
    EndpointInterface * epProvider = GetEndpointInterface(endpointId);
    if (epProvider == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return epProvider->SemanticTags(out);
}

CHIP_ERROR CodeDrivenDataModelProvider::DeviceTypes(EndpointId endpointId, ReadOnlyBufferBuilder<DataModel::DeviceTypeEntry> & out)
{
    EndpointInterface * epProvider = GetEndpointInterface(endpointId);
    if (epProvider == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return epProvider->DeviceTypes(out);
}

CHIP_ERROR CodeDrivenDataModelProvider::ClientClusters(EndpointId endpointId, ReadOnlyBufferBuilder<ClusterId> & out)
{
    EndpointInterface * epProvider = GetEndpointInterface(endpointId);
    if (epProvider == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return epProvider->ClientClusters(out);
}

CHIP_ERROR CodeDrivenDataModelProvider::ServerClusters(EndpointId endpointId,
                                                       ReadOnlyBufferBuilder<DataModel::ServerClusterEntry> & out)
{
    EndpointInterface * epProvider = GetEndpointInterface(endpointId);
    if (epProvider == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    ReadOnlyBufferBuilder<ServerClusterInterface *> serverClusterBuilder;
    ReturnErrorOnFailure(epProvider->ServerClusters(serverClusterBuilder));
    auto serverClusters = serverClusterBuilder.TakeBuffer();

    // Calculate total number of paths for capacity.
    size_t totalPaths = 0;
    for (const auto * serverCluster : serverClusters)
    {
        VerifyOrDie(!serverCluster->GetPaths().empty()); // API says it must have at least one path
        totalPaths += serverCluster->GetPaths().size();
    }
    ReturnErrorOnFailure(out.EnsureAppendCapacity(totalPaths));

    for (const auto * serverCluster : serverClusters)
    {
        for (const auto & path : serverCluster->GetPaths())
        {
            ReturnErrorOnFailure(
                out.Append({ path.mClusterId, serverCluster->GetDataVersion(path), serverCluster->GetClusterFlags(path) }));
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CodeDrivenDataModelProvider::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & out)
{
    ServerClusterInterface * serverCluster = GetServerCluster(path.mEndpointId, path.mClusterId, nullptr);
    if (serverCluster == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return serverCluster->GeneratedCommands(path, out);
}
CHIP_ERROR CodeDrivenDataModelProvider::AcceptedCommands(const ConcreteClusterPath & path,
                                                         ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & out)
{
    ServerClusterInterface * serverCluster = GetServerCluster(path.mEndpointId, path.mClusterId, nullptr);
    if (serverCluster == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return serverCluster->AcceptedCommands(path, out);
}

CHIP_ERROR CodeDrivenDataModelProvider::Attributes(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AttributeEntry> & out)
{
    ServerClusterInterface * serverCluster = GetServerCluster(path.mEndpointId, path.mClusterId, nullptr);
    if (serverCluster == nullptr)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return serverCluster->Attributes(path, out);
}

CHIP_ERROR CodeDrivenDataModelProvider::EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo)
{
    ServerClusterInterface * serverCluster = GetServerCluster(path.mEndpointId, path.mClusterId, nullptr);
    VerifyOrReturnError(serverCluster != nullptr, CHIP_ERROR_NOT_FOUND);
    return serverCluster->EventInfo(path, eventInfo);
}

void CodeDrivenDataModelProvider::Temporary_ReportAttributeChanged(const AttributePathParams & path)
{
    if (mContext.dataModelChangeListener == nullptr)
    {
        ChipLogError(DataManagement, "Temporary_ReportAttributeChanged called before provider has been started.");
        return;
    }
    mContext.dataModelChangeListener->MarkDirty(path);
}

CHIP_ERROR CodeDrivenDataModelProvider::AddEndpoint(EndpointInterfaceRegistration & registration)
{
    VerifyOrReturnError(registration.endpointInterface != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(mEndpointInterfaceRegistry.Register(registration));

    // If the provider has not been started, return early, no need to startup clusters.
    VerifyOrReturnError(mContext.dataModelChangeListener != nullptr, CHIP_NO_ERROR);

    // If the provider has already been started (its mContext is initialized) AND its server
    // cluster context is also set up, then the clusters for this new endpoint should be started.
    VerifyOrReturnError(mServerClusterContext.has_value(), CHIP_ERROR_INCORRECT_STATE);
    return StartupServerClusters(registration.endpointInterface, *mServerClusterContext);
}

CHIP_ERROR CodeDrivenDataModelProvider::RemoveEndpoint(EndpointId endpointId)
{
    EndpointInterface * provider = mEndpointInterfaceRegistry.Get(endpointId);
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_NOT_FOUND);

    // If the provider has been started (its mContext is initialized),
    // then the clusters for this endpoint might have been started and should be shut down.
    // We also need mServerClusterContext to have been set up.
    if (mContext.dataModelChangeListener != nullptr) // Provider has been Startup()-ed
    {
        ShutdownServerClusters(provider);
    }

    return mEndpointInterfaceRegistry.Unregister(endpointId);
}

EndpointInterface * CodeDrivenDataModelProvider::GetEndpointInterface(EndpointId endpointId)
{
    return mEndpointInterfaceRegistry.Get(endpointId);
}

ServerClusterInterface * CodeDrivenDataModelProvider::GetServerCluster(EndpointId endpointId, ClusterId clusterId,
                                                                       DataModel::ActionReturnStatus * outStatus)
{
    EndpointInterface * epProvider = GetEndpointInterface(endpointId);
    if (epProvider == nullptr)
    {
        if (outStatus != nullptr)
        {
            *outStatus = Status::UnsupportedEndpoint;
        }
        return nullptr;
    }

    ServerClusterInterface * serverCluster = epProvider->GetServerCluster(clusterId);
    if (serverCluster == nullptr)
    {
        if (outStatus != nullptr)
        {
            *outStatus = Status::UnsupportedCluster;
        }
        return nullptr;
    }
    return serverCluster;
}

void CodeDrivenDataModelProvider::SetPersistentStorageDelegate(PersistentStorageDelegate * delegate)
{
    // This function should be called before Startup. If called after, it will
    // replace the server cluster context, which may have unintended side effects
    // on clusters that are already started.
    if (mContext.dataModelChangeListener != nullptr)
    {
        ChipLogError(DataManagement,
                     "SetPersistentStorageDelegate called after provider has been started. This is not recommended.");
    }
    mServerClusterContext.emplace(ServerClusterContext({
        .provider           = this,
        .storage            = delegate,
        .interactionContext = &mContext,
    }));
}

PersistentStorageDelegate * CodeDrivenDataModelProvider::GetPersistentStorageDelegate() const
{
    if (!mServerClusterContext.has_value())
    {
        return nullptr;
    }
    return mServerClusterContext->storage;
}

} // namespace app
} // namespace chip
