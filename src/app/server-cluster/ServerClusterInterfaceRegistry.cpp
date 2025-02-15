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
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>

#include <app/ConcreteClusterPath.h>
#include <app/server-cluster/ServerClusterInterface.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace app {

ServerClusterInterfaceRegistry & ServerClusterInterfaceRegistry::Instance()
{
    static ServerClusterInterfaceRegistry sRegistry;
    return sRegistry;
}

ServerClusterInterfaceRegistry::~ServerClusterInterfaceRegistry()
{
    for (auto & ep : mPreallocateEndpoints)
    {
        if (ep.endpointId != kInvalidEndpointId)
        {
            UnregisterAllFromEndpoint(ep.endpointId);
        }
    }

    while (mDynamicEndpoints != nullptr)
    {
        UnregisterAllFromEndpoint(mDynamicEndpoints->endpointId);
    }
}

CHIP_ERROR ServerClusterInterfaceRegistry::AllocateNewEndpointClusters(EndpointId endpointId, EndpointClusters *& dest)
{
    for (auto & ep : mPreallocateEndpoints)
    {
        if (ep.endpointId == kInvalidEndpointId)
        {
            ep.endpointId   = endpointId;
            ep.firstCluster = nullptr;
            dest            = &ep;
            return CHIP_NO_ERROR;
        }
    }

    // need to allocate dynamically
    auto result = Platform::New<DynamicEndpointClusters>();
    VerifyOrReturnError(result != nullptr, CHIP_ERROR_NO_MEMORY);

    result->endpointId   = endpointId;
    result->firstCluster = nullptr;
    result->next         = mDynamicEndpoints;
    mDynamicEndpoints    = result;
    dest                 = result;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServerClusterInterfaceRegistry::Register(EndpointId endpointId, ServerClusterInterface * cluster)
{
    VerifyOrReturnError(cluster != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!cluster->IsInList(), CHIP_ERROR_IN_USE);
    VerifyOrReturnError(endpointId != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(cluster->GetClusterId() != kInvalidClusterId, CHIP_ERROR_INVALID_ARGUMENT);

    // duplicate registrations are disallowed
    VerifyOrReturnError(Get(ConcreteClusterPath(endpointId, cluster->GetClusterId())) == nullptr, CHIP_ERROR_DUPLICATE_KEY_ID);

    EndpointClusters * endpointClusters = FindClusters(endpointId);
    if (endpointClusters == nullptr)
    {
        ReturnErrorOnFailure(AllocateNewEndpointClusters(endpointId, endpointClusters));
    }

    cluster->SetNextListItem(endpointClusters->firstCluster);
    endpointClusters->firstCluster = cluster;

    return CHIP_NO_ERROR;
}

ServerClusterInterface * ServerClusterInterfaceRegistry::Unregister(const ConcreteClusterPath & path)
{
    EndpointClusters * endpointClusters = FindClusters(path.mEndpointId);
    VerifyOrReturnValue(endpointClusters != nullptr, nullptr);
    VerifyOrReturnValue(endpointClusters->firstCluster != nullptr, nullptr);

    ServerClusterInterface * prev    = nullptr;
    ServerClusterInterface * current = endpointClusters->firstCluster;

    while (current != nullptr)
    {
        if (current->GetClusterId() == path.mClusterId)
        {
            // takes the item out of the current list and return it.
            ServerClusterInterface * next = current->GetNextListItem();

            if (prev == nullptr)
            {
                endpointClusters->firstCluster = next;
            }
            else
            {
                prev->SetNextListItem(next);
            }

            if (mCachedInterface == current)
            {
                mCachedClusterEndpointId = kInvalidEndpointId;
                mCachedInterface         = nullptr;
            }
            current->SetNotInList();

            return current;
        }

        prev    = current;
        current = current->GetNextListItem();
    }

    // Not found.
    return nullptr;
}

void ServerClusterInterfaceRegistry::DestroySingleLinkedList(ServerClusterInterface * clusters)
{
    while (clusters != nullptr)
    {
        ServerClusterInterface * next = clusters->GetNextListItem();
        clusters->SetNotInList();
        clusters = next;
    }
}

void ServerClusterInterfaceRegistry::UnregisterAllFromEndpoint(EndpointId endpointId)
{
    if ((mEndpointClustersCache != nullptr) && (mEndpointClustersCache->endpointId == endpointId))
    {
        mEndpointClustersCache = nullptr;
    }

    if (mCachedClusterEndpointId == endpointId)
    {
        // all clusters on the given endpoints will be destroyed.
        mCachedInterface         = nullptr;
        mCachedClusterEndpointId = kInvalidEndpointId;
    }

    // if it is static, just clear it
    for (auto & ep : mPreallocateEndpoints)
    {
        if (ep.endpointId == endpointId)
        {
            ep.endpointId = kInvalidEndpointId;
            DestroySingleLinkedList(ep.firstCluster);
            ep.firstCluster = nullptr;
            return;
        }
    }

    // not found, search dynamic. Special handling for first;
    VerifyOrReturn(mDynamicEndpoints != nullptr);
    if (mDynamicEndpoints->endpointId == endpointId)
    {
        DynamicEndpointClusters * value = mDynamicEndpoints;
        mDynamicEndpoints               = mDynamicEndpoints->next;
        DestroySingleLinkedList(value->firstCluster);
        Platform::Delete(value);
        return;
    }
    DynamicEndpointClusters * prev    = mDynamicEndpoints;
    DynamicEndpointClusters * current = prev->next;

    while (current != nullptr)
    {
        if (current->endpointId == endpointId)
        {
            prev->next = current->next;
            DestroySingleLinkedList(current->firstCluster);
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
    ServerClusterInterface * current = endpointClusters->firstCluster;

    while (current != nullptr)
    {
        if (current->GetClusterId() == path.mClusterId)
        {
            mCachedClusterEndpointId = path.mEndpointId;
            mCachedInterface         = current;
            return current;
        }

        current = current->GetNextListItem();
    }

    // not found
    return nullptr;
}

ServerClusterInterfaceRegistry::EndpointClusters * ServerClusterInterfaceRegistry::FindClusters(EndpointId endpointId)
{
    // invalid cluster id is NOT acceptable (since static allocation uses the
    // invalid cluster id as a marker of "not used")
    VerifyOrReturnValue(endpointId != kInvalidEndpointId, nullptr);

    if ((mEndpointClustersCache != nullptr) && (mEndpointClustersCache->endpointId == endpointId))
    {
        return mEndpointClustersCache;
    }

    // search statically first
    for (auto & ep : mPreallocateEndpoints)
    {
        if (ep.endpointId == endpointId)
        {
            mEndpointClustersCache = &ep;
            return mEndpointClustersCache;
        }
    }

    // not found, search dynamic
    for (DynamicEndpointClusters * p = mDynamicEndpoints; p != nullptr; p = p->next)
    {
        if (p->endpointId == endpointId)
        {
            mEndpointClustersCache = p;
            return mEndpointClustersCache;
        }
    }

    return nullptr;
}

} // namespace app
} // namespace chip
