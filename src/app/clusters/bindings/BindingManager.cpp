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
        auto iter = chip::BindingTable::GetInstance().begin();
        while (iter != chip::BindingTable::GetInstance().end())
        {
            if (iter->fabricIndex == fabricIndex)
            {
                iter = chip::BindingTable::GetInstance().RemoveAt(iter);
            }
            else
            {
                ++iter;
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

namespace {

chip::PeerId PeerIdForNode(chip::FabricTable & fabricTable, chip::FabricIndex fabric, chip::NodeId node)
{
    chip::FabricInfo * fabricInfo = fabricTable.FindFabricWithIndex(fabric);
    if (fabricInfo == nullptr)
    {
        return chip::PeerId();
    }
    return fabricInfo->GetPeerIdForNode(node);
}

} // namespace

namespace chip {

BindingManager BindingManager::sBindingManager;

CHIP_ERROR BindingManager::UnicastBindingCreated(uint8_t fabricIndex, NodeId nodeId)
{
    return EstablishConnection(fabricIndex, nodeId);
}

CHIP_ERROR BindingManager::UnicastBindingRemoved(uint8_t bindingEntryId)
{
    mPendingNotificationMap.RemoveEntry(bindingEntryId);
    return CHIP_NO_ERROR;
}

void BindingManager::SetAppServer(Server * appServer)
{
    mAppServer = appServer;
    mAppServer->GetFabricTable().AddFabricDelegate(&gFabricTableDelegate);
}

CHIP_ERROR BindingManager::EstablishConnection(FabricIndex fabric, NodeId node)
{
    VerifyOrReturnError(mAppServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    PeerId peer = PeerIdForNode(mAppServer->GetFabricTable(), fabric, node);
    VerifyOrReturnError(peer.GetNodeId() != kUndefinedNodeId, CHIP_ERROR_NOT_FOUND);
    CHIP_ERROR error =
        mAppServer->GetCASESessionManager()->FindOrEstablishSession(peer, &mOnConnectedCallback, &mOnConnectionFailureCallback);
    if (error == CHIP_ERROR_NO_MEMORY)
    {
        // Release the least recently used entry
        // TODO: Some reference counting mechanism shall be added the CASESessionManager
        // so that other session clients don't get accidentally closed.
        FabricIndex fabricToRemove;
        NodeId nodeToRemove;
        if (mPendingNotificationMap.FindLRUConnectPeer(&fabricToRemove, &nodeToRemove) == CHIP_NO_ERROR)
        {
            mPendingNotificationMap.RemoveAllEntriesForNode(fabricToRemove, nodeToRemove);
            PeerId lruPeer = PeerIdForNode(mAppServer->GetFabricTable(), fabricToRemove, nodeToRemove);
            mAppServer->GetCASESessionManager()->ReleaseSession(lruPeer);
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
    FabricIndex fabricToRemove = kUndefinedFabricIndex;
    NodeId nodeToRemove        = kUndefinedNodeId;

    // Note: not using a const ref here, because the mPendingNotificationMap
    // iterator returns things by value anyway.
    for (PendingNotificationEntry pendingNotification : mPendingNotificationMap)
    {
        EmberBindingTableEntry entry = BindingTable::GetInstance().GetAt(pendingNotification.mBindingEntryId);

        PeerId peer = PeerIdForNode(mAppServer->GetFabricTable(), entry.fabricIndex, entry.nodeId);
        if (device->GetPeerId() == peer)
        {
            fabricToRemove = entry.fabricIndex;
            nodeToRemove   = entry.nodeId;
            mBoundDeviceChangedHandler(entry, device, pendingNotification.mContext);
        }
    }
    mPendingNotificationMap.RemoveAllEntriesForNode(fabricToRemove, nodeToRemove);
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
    mPendingNotificationMap.RemoveAllEntriesForFabric(fabricIndex);
    mAppServer->GetCASESessionManager()->ReleaseSessionsForFabric(compressedFabricId);
}

CHIP_ERROR BindingManager::NotifyBindingAdded(const EmberBindingTableEntry & binding)
{
    if (mBindingAddedHandler)
    {
        mBindingAddedHandler(binding);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingManager::NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster, void * context)
{
    VerifyOrReturnError(mAppServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

    for (auto iter = BindingTable::GetInstance().begin(); iter != BindingTable::GetInstance().end(); ++iter)
    {
        if (iter->local == endpoint && (!iter->clusterId.HasValue() || iter->clusterId.Value() == cluster))
        {
            if (iter->type == EMBER_UNICAST_BINDING)
            {
                FabricInfo * fabricInfo = mAppServer->GetFabricTable().FindFabricWithIndex(iter->fabricIndex);
                VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_NOT_FOUND);
                PeerId peer                         = fabricInfo->GetPeerIdForNode(iter->nodeId);
                OperationalDeviceProxy * peerDevice = mAppServer->GetCASESessionManager()->FindExistingSession(peer);
                if (peerDevice != nullptr && peerDevice->IsConnected() && mBoundDeviceChangedHandler)
                {
                    // We already have an active connection
                    mBoundDeviceChangedHandler(*iter, peerDevice, context);
                }
                else
                {
                    mPendingNotificationMap.AddPendingNotification(iter.GetIndex(), context);
                    if (peerDevice == nullptr || !peerDevice->IsConnecting())
                    {
                        ReturnErrorOnFailure(EstablishConnection(iter->fabricIndex, iter->nodeId));
                    }
                }
            }
            else if (iter->type == EMBER_MULTICAST_BINDING)
            {
                mBoundDeviceChangedHandler(*iter, nullptr, context);
            }
        }
    }
    return CHIP_NO_ERROR;
}

} // namespace chip
