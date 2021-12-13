/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/clusters/bindings/BindingManager.h>
#include <app/util/binding-table.h>

namespace chip {

BindingManager BindingManager::sBindingManager;

CHIP_ERROR BindingManager::CreateBinding(FabricIndex fabric, NodeId node, EndpointId localEndpoint, ClusterId cluster)
{
    ReturnErrorCodeIf(mAppServer == nullptr, CHIP_ERROR_INCORRECT_STATE);

    return EnqueueClusterAndConnect(fabric, node, localEndpoint, cluster);
}

CHIP_ERROR BindingManager::EnqueueClusterAndConnect(FabricIndex fabric, NodeId node, EndpointId endpoint, ClusterId cluster)
{
    FabricInfo * fabricInfo = mAppServer->GetFabricTable().FindFabricWithIndex(fabric);
    PeerId peer             = fabricInfo->GetPeerIdForNode(node);
    CHIP_ERROR error        = mPendingClusterMap.AddPendingCluster(peer, endpoint, cluster);
    // We shouldn't fail to create the PendingClusterEntry after Binding Table size check
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to create PendingClusterEntry");
        return error;
    }

    error = mAppServer->GetCASESessionManager()->FindOrEstablishSession(fabricInfo, node, &mOnConnectedCallback,
                                                                        &mOnConnectionFailureCallback);
    if (error == CHIP_ERROR_NO_MEMORY)
    {
        // Release the least recently used entry
        PendingClusterEntry * entry = mPendingClusterMap.FindLRUEntry();
        if (entry != nullptr)
        {
            mAppServer->GetCASESessionManager()->ReleaseSession(entry->mPeerId.GetNodeId());
            mPendingClusterMap.RemoveEntry(entry);
            // Now retry
            error = mAppServer->GetCASESessionManager()->FindOrEstablishSession(fabricInfo, node, &mOnConnectedCallback,
                                                                                &mOnConnectionFailureCallback);
        }
    }
    return error;
}

void BindingManager::HandleDeviceConnected(void * context, OperationalDeviceProxy * device)
{
    BindingManager * manager = static_cast<BindingManager *>(context);
    manager->HandleDeviceConnected(device);
}

void BindingManager::HandleDeviceConnected(OperationalDeviceProxy * device)
{
    mPendingClusterMap.ForEachActiveObject([&](PendingClusterEntry * entry) -> Loop {
        if (entry->mPeerId == device->GetPeerId())
        {
            SyncPendingClustersToPeer(device, entry);
        }

        return Loop::Continue;
    });
}

void BindingManager::SyncPendingClustersToPeer(OperationalDeviceProxy * device, PendingClusterEntry * pendingClusters)
{
    for (uint8_t i = 0; i < pendingClusters->mNumPendingClusters; i++)
    {
        ClusterId cluster   = pendingClusters->mPendingClusters[i].cluster;
        EndpointId endpoint = pendingClusters->mPendingClusters[i].endpoint;
        for (uint8_t j = 0; j < EMBER_BINDING_TABLE_SIZE; j++)
        {
            EmberBindingTableEntry entry;
            if (emberGetBinding(j, &entry) == EMBER_SUCCESS && entry.type != EMBER_UNUSED_BINDING && entry.clusterId == cluster &&
                entry.local == endpoint && mBoundDeviceChangedHandler)
            {
                mBoundDeviceChangedHandler(entry.local, entry.remote, cluster, device);
            }
        }
    }
    mPendingClusterMap.RemoveEntry(pendingClusters);
}

void BindingManager::HandleDeviceConnectionFailure(void * context, NodeId nodeId, CHIP_ERROR error)
{
    BindingManager * manager = static_cast<BindingManager *>(context);
    manager->HandleDeviceConnectionFailure(nodeId, error);
}

void BindingManager::HandleDeviceConnectionFailure(NodeId nodeId, CHIP_ERROR error)
{
    // Simply release the entry, the connection will be re-established on need.
    ChipLogError(AppServer, "Failed to establish connection to node 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
    mAppServer->GetCASESessionManager()->ReleaseSession(nodeId);
}

CHIP_ERROR BindingManager::DisconnectDevice(FabricIndex fabric, NodeId node)
{
    FabricInfo * fabricInfo     = mAppServer->GetFabricTable().FindFabricWithIndex(fabric);
    PendingClusterEntry * entry = mPendingClusterMap.FindEntry(fabricInfo->GetPeerIdForNode(node));

    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NOT_FOUND);

    mAppServer->GetCASESessionManager()->ReleaseSession(node);
    mPendingClusterMap.RemoveEntry(entry);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingManager::NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster)
{
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;
        CHIP_ERROR error = CHIP_NO_ERROR;

        if (emberGetBinding(i, &entry) == EMBER_SUCCESS && entry.type != EMBER_UNUSED_BINDING && entry.local == endpoint &&
            entry.clusterId == cluster)
        {
            OperationalDeviceProxy * peerDevice = mAppServer->GetCASESessionManager()->FindExistingSession(entry.nodeId);
            if (peerDevice != nullptr && mBoundDeviceChangedHandler)
            {
                // We already have an active connection
                mBoundDeviceChangedHandler(entry.local, entry.remote, cluster, peerDevice);
            }
            else
            {
                // Enqueue pending cluster and establish connection
                error = EnqueueClusterAndConnect(entry.fabricIndex, entry.nodeId, entry.local, entry.clusterId);
            }
        }
    }
    return CHIP_NO_ERROR;
}

BindingManager::PendingClusterEntry * BindingManager::PendingClusterMap::FindLRUEntry()
{
    PendingClusterEntry * lruEntry = nullptr;
    mPendingClusterMap.ForEachActiveObject([&](PendingClusterEntry * entry) {
        if (lruEntry == nullptr || lruEntry->mLastUpdateTime > entry->mLastUpdateTime)
        {
            lruEntry = entry;
        }
        return Loop::Continue;
    });
    return lruEntry;
}

BindingManager::PendingClusterEntry * BindingManager::PendingClusterMap::FindEntry(PeerId peerId)
{
    PendingClusterEntry * foundEntry = nullptr;
    mPendingClusterMap.ForEachActiveObject([&](PendingClusterEntry * entry) {
        if (entry->mPeerId == peerId)
        {
            foundEntry = entry;
            return Loop::Break;
        }
        return Loop::Continue;
    });
    return foundEntry;
}

CHIP_ERROR BindingManager::PendingClusterMap::AddPendingCluster(PeerId peer, EndpointId endpoint, ClusterId cluster)
{
    PendingClusterEntry * entry = FindEntry(peer);

    if (entry == nullptr)
    {
        entry = mPendingClusterMap.CreateObject();
        VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NO_MEMORY);
        entry->mPeerId = peer;
    }
    entry->AddPendingCluster(endpoint, cluster);
    entry->mLastUpdateTime = System::SystemClock().GetMonotonicTimestamp();
    return CHIP_NO_ERROR;
}

} // namespace chip
