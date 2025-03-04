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
#include "lib/support/logging/TextOnlyLogging.h"
#include <data-model-providers/codegen/ServerClusterInterfaceRegistry.h>

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

void ServerClusterInterfaceRegistry::ClearSingleLinkedList(RegisteredServerClusterInterface * clusters)
{
    while (clusters != nullptr)
    {
        RegisteredServerClusterInterface * next = clusters->next;
        if (mContextIsValid)
        {
            clusters->serverClusterInterface->Shutdown();
        }
        Platform::Delete(clusters);
        clusters = next;
    }
}

ServerClusterInterfaceRegistry::~ServerClusterInterfaceRegistry()
{
    while (mEndpoints != nullptr)
    {
        UnregisterAllFromEndpoint(mEndpoints->endpointId);
    }
}

CHIP_ERROR ServerClusterInterfaceRegistry::AllocateNewEndpointClusters(EndpointId endpointId, EndpointClusters *& dest)
{
    auto result = Platform::New<EndpointClusters>();
    VerifyOrReturnError(result != nullptr, CHIP_ERROR_NO_MEMORY);

    result->endpointId   = endpointId;
    result->firstCluster = nullptr;
    result->next         = mEndpoints;
    mEndpoints           = result;
    dest                 = result;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerClusterInterfaceRegistry::Register(EndpointId endpointId, ServerClusterInterface * cluster)
{
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(cluster->GetClusterId() != kInvalidClusterId, CHIP_ERROR_INVALID_ARGUMENT);

    // duplicate registrations are disallowed
    VerifyOrReturnError(Get(ConcreteClusterPath(endpointId, cluster->GetClusterId())) == nullptr, CHIP_ERROR_DUPLICATE_KEY_ID);

    EndpointClusters * endpointClusters = FindClusters(endpointId);
    if (endpointClusters == nullptr)
    {
        ReturnErrorOnFailure(AllocateNewEndpointClusters(endpointId, endpointClusters));
    }

    auto entry = Platform::New<RegisteredServerClusterInterface>(cluster, endpointClusters->firstCluster);
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NO_MEMORY);

    if (mContextIsValid)
    {
        CHIP_ERROR err = cluster->Startup(endpointId, &mContext);
        if (err != CHIP_NO_ERROR)
        {
            Platform::Delete(entry);
            return err;
        }
    }

    endpointClusters->firstCluster = entry;

    return CHIP_NO_ERROR;
}

ServerClusterInterface * ServerClusterInterfaceRegistry::Unregister(const ConcreteClusterPath & path)
{
    EndpointClusters * endpointClusters = FindClusters(path.mEndpointId);
    VerifyOrReturnValue(endpointClusters != nullptr, nullptr);
    VerifyOrReturnValue(endpointClusters->firstCluster != nullptr, nullptr);

    RegisteredServerClusterInterface * prev    = nullptr;
    RegisteredServerClusterInterface * current = endpointClusters->firstCluster;

    while (current != nullptr)
    {
        if (current->serverClusterInterface->GetClusterId() == path.mClusterId)
        {
            // take the item out of the current list and return it.
            RegisteredServerClusterInterface * next = current->next;

            if (prev == nullptr)
            {
                endpointClusters->firstCluster = next;
            }
            else
            {
                prev->next = next;
            }

            if (mCachedInterface == current->serverClusterInterface)
            {
                mCachedClusterEndpointId = kInvalidEndpointId;
                mCachedInterface         = nullptr;
            }

            ServerClusterInterface * result = current->serverClusterInterface;
            Platform::Delete(current);

            if (mContextIsValid)
            {
                result->Shutdown();
            }
            return result;
        }

        prev    = current;
        current = current->next;
    }

    // Not found.
    return nullptr;
}

ServerClusterInterfaceRegistry::ClustersList ServerClusterInterfaceRegistry::ClustersOnEndpoint(EndpointId endpointId)
{
    EndpointClusters * clusters = FindClusters(endpointId);

    if (clusters == nullptr)
    {
        return nullptr;
    }

    return clusters->firstCluster;
}

void ServerClusterInterfaceRegistry::UnregisterAllFromEndpoint(EndpointId endpointId)
{
    if (mCachedClusterEndpointId == endpointId)
    {
        // all clusters on the given endpoint will be unregistered.
        mCachedInterface         = nullptr;
        mCachedClusterEndpointId = kInvalidEndpointId;
    }

    EndpointClusters * prev    = nullptr;
    EndpointClusters * current = mEndpoints;
    while (current != nullptr)
    {
        if (current->endpointId == endpointId)
        {
            if (prev == nullptr)
            {
                mEndpoints = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            ClearSingleLinkedList(current->firstCluster);
            Platform::Delete(current);
            return;
        }

        prev    = current;
        current = current->next;
    }
}

ServerClusterInterface * ServerClusterInterfaceRegistry::Get(const ConcreteClusterPath & path)
{
    EndpointClusters * endpointClusters = FindClusters(path.mEndpointId);
    VerifyOrReturnValue(endpointClusters != nullptr, nullptr);
    VerifyOrReturnValue(endpointClusters->firstCluster != nullptr, nullptr);

    // Check the cache to speed things up
    if ((mCachedClusterEndpointId == path.mEndpointId) && (mCachedInterface != nullptr) &&
        (mCachedInterface->GetClusterId() == path.mClusterId))
    {
        return mCachedInterface;
    }

    // The cluster searched for is not cached, do a linear search for it
    RegisteredServerClusterInterface * current = endpointClusters->firstCluster;

    while (current != nullptr)
    {
        if (current->serverClusterInterface->GetClusterId() == path.mClusterId)
        {
            mCachedClusterEndpointId = path.mEndpointId;
            mCachedInterface         = current->serverClusterInterface;
            return mCachedInterface;
        }

        current = current->next;
    }

    // not found
    return nullptr;
}

ServerClusterInterfaceRegistry::EndpointClusters * ServerClusterInterfaceRegistry::FindClusters(EndpointId endpointId)
{
    // invalid cluster id is NOT acceptable (since static allocation uses the
    // invalid cluster id as a marker of "not used")
    VerifyOrReturnValue(endpointId != kInvalidEndpointId, nullptr);

    for (EndpointClusters * p = mEndpoints; p != nullptr; p = p->next)
    {
        if (p->endpointId == endpointId)
        {
            return p;
        }
    }

    return nullptr;
}

CHIP_ERROR ServerClusterInterfaceRegistry::SetContext(const ServerClusterContext & context)
{
    if (mContextIsValid)
    {
        // if there is no difference, do not re-initialize.
        VerifyOrReturnError(mContext != context, CHIP_NO_ERROR);
        ClearContext();
    }

    mContext         = context;
    mContextIsValid  = true;
    bool had_failure = false;

    for (EndpointClusters * endpoint = mEndpoints; endpoint != nullptr; endpoint = endpoint->next)
    {
        for (RegisteredServerClusterInterface * cluster = endpoint->firstCluster; cluster != nullptr; cluster = cluster->next)
        {
            CHIP_ERROR err = cluster->serverClusterInterface->Startup(endpoint->endpointId, &mContext);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement, "Cluster %u/" ChipLogFormatMEI " startup failed: %" CHIP_ERROR_FORMAT,
                             endpoint->endpointId, ChipLogValueMEI(cluster->serverClusterInterface->GetClusterId()), err.Format());
                had_failure = true;
                // NOTE: this makes the object be in an awkward state:
                //       - cluster is not initialized
                //       - mContext is valid
                //       As a result, ::Shutdown on this cluster WILL be called even if startup failed.
            }
        }
    }

    if (had_failure)
    {
        return CHIP_ERROR_HAD_FAILURES;
    }

    return CHIP_NO_ERROR;
}

void ServerClusterInterfaceRegistry::ClearContext()
{
    if (!mContextIsValid)
    {
        return;
    }
    for (EndpointClusters * endpoint = mEndpoints; endpoint != nullptr; endpoint = endpoint->next)
    {
        for (RegisteredServerClusterInterface * cluster = endpoint->firstCluster; cluster != nullptr; cluster = cluster->next)
        {
            cluster->serverClusterInterface->Shutdown();
        }
    }

    mContext        = ServerClusterContext{};
    mContextIsValid = false;
}

} // namespace app
} // namespace chip
