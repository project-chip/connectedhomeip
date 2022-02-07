/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <credentials/FabricTable.h>

namespace {

class BindingFabricTableDelegate : public chip::FabricTableDelegate
{
    void OnFabricDeletedFromStorage(chip::CompressedFabricId compressedFabricId, chip::FabricIndex fabricIndex)
    {
        for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
        {
            EmberBindingTableEntry entry;
            emberGetBinding(i, &entry);
            if (entry.fabricIndex == fabricIndex)
            {
                ChipLogProgress(Zcl, "Remove binding for fabric %d\n", entry.fabricIndex);
                entry.type = EMBER_UNUSED_BINDING;
            }
        }
        chip::BindingManager::GetInstance().FabricRemoved(compressedFabricId, fabricIndex);
    }

    // Intentionally left blank
    void OnFabricRetrievedFromStorage(chip::FabricInfo * fabricInfo) {}

    // Intentionally left blank
    void OnFabricPersistedToStorage(chip::FabricInfo * fabricInfo) {}
};

BindingFabricTableDelegate gFabricTableDelegate;

} // namespace

namespace chip {

BindingManager BindingManager::sBindingManager;

void BindingManager::SetAppServer(Server * appServer)
{
    mAppServer = appServer;
    mAppServer->GetFabricTable().AddFabricDelegate(&gFabricTableDelegate);
}

CHIP_ERROR BindingManager::EstablishConnection(FabricIndex fabric, NodeId node)
{
    VerifyOrReturnError(mAppServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    FabricInfo * fabricInfo = mAppServer->GetFabricTable().FindFabricWithIndex(fabric);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_NOT_FOUND);
    PeerId peer = fabricInfo->GetPeerIdForNode(node);
    CHIP_ERROR error =
        mAppServer->GetCASESessionManager()->FindOrEstablishSession(peer, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    if (error == CHIP_ERROR_NO_MEMORY)
    {
        // Release the least recently used entry
        // TODO: Some reference counting mechanism shall be added the CASESessionManager
        // so that other session clients don't get accidentally closed.
        PendingNotificationEntry * entry = mPendingNotificationMap.FindLRUEntry();
        if (entry != nullptr)
        {
            mAppServer->GetCASESessionManager()->ReleaseSession(entry->GetPeerId());
            mPendingNotificationMap.RemoveEntry(entry);
            // Now retry
            error = mAppServer->GetCASESessionManager()->FindOrEstablishSession(peer, &mOnConnectedCallback,
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
    mPendingNotificationMap.ForEachActiveObject([&](PendingNotificationEntry * entry) -> Loop {
        if (entry->GetPeerId() == device->GetPeerId())
        {
            SyncPendingNotificationsToPeer(device, entry);
        }

        return Loop::Continue;
    });
}

void BindingManager::SyncPendingNotificationsToPeer(OperationalDeviceProxy * device, PendingNotificationEntry * pendingClusters)
{
    for (const ClusterPath & path : *pendingClusters)
    {
        ClusterId cluster   = path.cluster;
        EndpointId endpoint = path.endpoint;
        for (uint8_t j = 0; j < EMBER_BINDING_TABLE_SIZE; j++)
        {
            EmberBindingTableEntry entry;
            if (emberGetBinding(j, &entry) == EMBER_SUCCESS && entry.type == EMBER_UNICAST_BINDING && entry.clusterId == cluster &&
                entry.local == endpoint && mBoundDeviceChangedHandler)
            {
                mBoundDeviceChangedHandler(&entry, device, path.context);
            }
        }
    }
    mPendingNotificationMap.RemoveEntry(pendingClusters);
}

void BindingManager::HandleDeviceConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error)
{
    BindingManager * manager = static_cast<BindingManager *>(context);
    manager->HandleDeviceConnectionFailure(peerId, error);
}

void BindingManager::HandleDeviceConnectionFailure(PeerId peerId, CHIP_ERROR error)
{
    // Simply release the entry, the connection will be re-established as needed.
    ChipLogError(AppServer, "Failed to establish connection to node 0x" ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    mAppServer->GetCASESessionManager()->ReleaseSession(peerId);
}

void BindingManager::FabricRemoved(CompressedFabricId compressedFabricId, FabricIndex fabricIndex)
{
    mPendingNotificationMap.ForEachActiveObject([&](PendingNotificationEntry * entry) {
        if (entry->GetFabricIndex() == fabricIndex)
        {
            mPendingNotificationMap.RemoveEntry(entry);
            return Loop::Break;
        }
        return Loop::Continue;
    });
    mAppServer->GetCASESessionManager()->ReleaseSessionForFabric(compressedFabricId);
}

CHIP_ERROR BindingManager::LastUnicastBindingRemoved(FabricIndex fabricIndex, NodeId node)
{
    VerifyOrReturnError(mAppServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    FabricInfo * fabricInfo = mAppServer->GetFabricTable().FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_NOT_FOUND);
    PeerId peer                      = fabricInfo->GetPeerIdForNode(node);
    PendingNotificationEntry * entry = mPendingNotificationMap.FindEntry(fabricIndex, node);
    if (entry)
    {
        mPendingNotificationMap.RemoveEntry(entry);
    }

    mAppServer->GetCASESessionManager()->ReleaseSession(peer);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingManager::NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster, void * context)
{
    VerifyOrReturnError(mAppServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;

        if (emberGetBinding(i, &entry) == EMBER_SUCCESS && entry.type != EMBER_UNUSED_BINDING && entry.local == endpoint &&
            entry.clusterId == cluster)
        {
            if (entry.type == EMBER_UNICAST_BINDING)
            {
                FabricInfo * fabricInfo = mAppServer->GetFabricTable().FindFabricWithIndex(entry.fabricIndex);
                VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_NOT_FOUND);
                PeerId peer                         = fabricInfo->GetPeerIdForNode(entry.nodeId);
                OperationalDeviceProxy * peerDevice = mAppServer->GetCASESessionManager()->FindExistingSession(peer);
                if (peerDevice != nullptr && mBoundDeviceChangedHandler)
                {
                    // We already have an active connection
                    mBoundDeviceChangedHandler(&entry, peerDevice, context);
                }
                else
                {
                    // Enqueue pending cluster and establish connection
                    ReturnErrorOnFailure(mPendingNotificationMap.AddPendingNotification(entry.fabricIndex, entry.nodeId, endpoint,
                                                                                        cluster, context));
                    ReturnErrorOnFailure(EstablishConnection(entry.fabricIndex, entry.nodeId));
                }
            }
            else if (entry.type == EMBER_MULTICAST_BINDING)
            {
                mBoundDeviceChangedHandler(&entry, nullptr, context);
            }
        }
    }
    return CHIP_NO_ERROR;
}

BindingManager::PendingNotificationEntry * BindingManager::PendingNotificationMap::FindLRUEntry()
{
    PendingNotificationEntry * lruEntry = nullptr;
    mPendingNotificationMap.ForEachActiveObject([&](PendingNotificationEntry * entry) {
        if (lruEntry == nullptr || lruEntry->GetLastUpdateTime() > entry->GetLastUpdateTime())
        {
            lruEntry = entry;
        }
        return Loop::Continue;
    });
    return lruEntry;
}

BindingManager::PendingNotificationEntry * BindingManager::PendingNotificationMap::FindEntry(FabricIndex fabricIndex, NodeId node)
{
    PendingNotificationEntry * foundEntry = nullptr;
    mPendingNotificationMap.ForEachActiveObject([&](PendingNotificationEntry * entry) {
        if (entry->GetFabricIndex() == fabricIndex && entry->GetNodeId() == node)
        {
            foundEntry = entry;
            return Loop::Break;
        }
        return Loop::Continue;
    });
    return foundEntry;
}

CHIP_ERROR BindingManager::PendingNotificationMap::AddPendingNotification(FabricIndex fabric, NodeId node, EndpointId endpoint,
                                                                          ClusterId cluster, void * context)
{
    PendingNotificationEntry * entry = FindEntry(fabric, node);

    if (entry == nullptr)
    {
        entry = mPendingNotificationMap.CreateObject(fabric, node);
        VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NO_MEMORY);
    }
    entry->AddPendingNotification(endpoint, cluster, context);
    entry->Touch();
    return CHIP_NO_ERROR;
}

} // namespace chip
