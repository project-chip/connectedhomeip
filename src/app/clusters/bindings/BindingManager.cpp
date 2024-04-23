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
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace {

class BindingFabricTableDelegate : public chip::FabricTable::Delegate
{
    void OnFabricRemoved(const chip::FabricTable & fabricTable, chip::FabricIndex fabricIndex) override
    {
        chip::BindingTable & bindingTable = chip::BindingTable::GetInstance();
        auto iter                         = bindingTable.begin();
        while (iter != bindingTable.end())
        {
            if (iter->fabricIndex == fabricIndex)
            {
                bindingTable.RemoveAt(iter);
            }
            else
            {
                ++iter;
            }
        }
        chip::BindingManager::GetInstance().FabricRemoved(fabricIndex);
    }
};

BindingFabricTableDelegate gFabricTableDelegate;

} // namespace

namespace {} // namespace

namespace chip {

BindingManager BindingManager::sBindingManager;

CHIP_ERROR BindingManager::UnicastBindingCreated(uint8_t fabricIndex, NodeId nodeId)
{
    return EstablishConnection(ScopedNodeId(nodeId, fabricIndex));
}

CHIP_ERROR BindingManager::UnicastBindingRemoved(uint8_t bindingEntryId)
{
    mPendingNotificationMap.RemoveEntry(bindingEntryId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingManager::Init(const BindingManagerInitParams & params)
{
    VerifyOrReturnError(params.mCASESessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.mFabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mInitParams = params;
    params.mFabricTable->AddFabricDelegate(&gFabricTableDelegate);
    BindingTable::GetInstance().SetPersistentStorage(params.mStorage);
    CHIP_ERROR error = BindingTable::GetInstance().LoadFromStorage();
    if (error != CHIP_NO_ERROR)
    {
        // This can happen during first boot of the device.
        ChipLogProgress(AppServer, "Cannot load binding table: %" CHIP_ERROR_FORMAT, error.Format());
    }
    else
    {
        // In case the application does not want the BindingManager to establish a CASE session
        // to the available bindings, it can be disabled by setting mEstablishConnectionOnInit
        // to false.
        if (params.mEstablishConnectionOnInit)
        {
            for (const EmberBindingTableEntry & entry : BindingTable::GetInstance())
            {
                if (entry.type == MATTER_UNICAST_BINDING)
                {
                    // The CASE connection can also fail if the unicast peer is offline.
                    // There is recovery mechanism to retry connection on-demand so ignore error.
                    (void) UnicastBindingCreated(entry.fabricIndex, entry.nodeId);
                }
            }
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingManager::EstablishConnection(const ScopedNodeId & nodeId)
{
    VerifyOrReturnError(mInitParams.mCASESessionManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mLastSessionEstablishmentError = CHIP_NO_ERROR;
    auto * connectionCallback      = Platform::New<ConnectionCallback>(*this);
    VerifyOrReturnError(connectionCallback != nullptr, CHIP_ERROR_NO_MEMORY);

    mInitParams.mCASESessionManager->FindOrEstablishSession(nodeId, connectionCallback->GetOnDeviceConnected(),
                                                            connectionCallback->GetOnDeviceConnectionFailure());
    if (mLastSessionEstablishmentError == CHIP_ERROR_NO_MEMORY)
    {
        // Release the least recently used entry
        ScopedNodeId peerToRemove;
        if (mPendingNotificationMap.FindLRUConnectPeer(peerToRemove) == CHIP_NO_ERROR)
        {
            mPendingNotificationMap.RemoveAllEntriesForNode(peerToRemove);

            // Now retry
            mLastSessionEstablishmentError = CHIP_NO_ERROR;
            // At this point connectionCallback is null since it deletes itself when the callback is called.
            connectionCallback = Platform::New<ConnectionCallback>(*this);
            mInitParams.mCASESessionManager->FindOrEstablishSession(nodeId, connectionCallback->GetOnDeviceConnected(),
                                                                    connectionCallback->GetOnDeviceConnectionFailure());
        }
    }
    return mLastSessionEstablishmentError;
}

void BindingManager::HandleDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    FabricIndex fabricToRemove = kUndefinedFabricIndex;
    NodeId nodeToRemove        = kUndefinedNodeId;

    // Note: not using a const ref here, because the mPendingNotificationMap
    // iterator returns things by value anyway.
    for (PendingNotificationEntry pendingNotification : mPendingNotificationMap)
    {
        EmberBindingTableEntry entry = BindingTable::GetInstance().GetAt(pendingNotification.mBindingEntryId);

        if (sessionHandle->GetPeer() == ScopedNodeId(entry.nodeId, entry.fabricIndex))
        {
            fabricToRemove = entry.fabricIndex;
            nodeToRemove   = entry.nodeId;
            OperationalDeviceProxy device(&exchangeMgr, sessionHandle);
            mBoundDeviceChangedHandler(entry, &device, pendingNotification.mContext->GetContext());
        }
    }

    mPendingNotificationMap.RemoveAllEntriesForNode(ScopedNodeId(nodeToRemove, fabricToRemove));
}

void BindingManager::HandleDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    // Simply release the entry, the connection will be re-established as needed.
    ChipLogError(AppServer, "Failed to establish connection to node 0x" ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    mLastSessionEstablishmentError = error;
    // We don't release the entry when connection fails, because inside
    // BindingManager::EstablishConnection we may try again the connection.
    // TODO(#22173): The logic in there doesn't actually make any sense with how
    // mPendingNotificationMap and CASESessionManager are implemented today.
}

void BindingManager::FabricRemoved(FabricIndex fabricIndex)
{
    mPendingNotificationMap.RemoveAllEntriesForFabric(fabricIndex);

    // TODO(#18436): NOC cluster should handle fabric removal without needing binding manager
    //               to execute such a release. Currently not done because paths were not tested.
    mInitParams.mCASESessionManager->ReleaseSessionsForFabric(fabricIndex);
}

CHIP_ERROR BindingManager::NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster, void * context)
{
    VerifyOrReturnError(mInitParams.mFabricTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mBoundDeviceChangedHandler != nullptr, CHIP_ERROR_HANDLER_NOT_SET);

    CHIP_ERROR error      = CHIP_NO_ERROR;
    auto * bindingContext = mPendingNotificationMap.NewPendingNotificationContext(context);
    VerifyOrReturnError(bindingContext != nullptr, CHIP_ERROR_NO_MEMORY);

    bindingContext->IncrementConsumersNumber();

    for (auto iter = BindingTable::GetInstance().begin(); iter != BindingTable::GetInstance().end(); ++iter)
    {
        if (iter->local == endpoint && (iter->clusterId.value_or(cluster) == cluster))
        {
            if (iter->type == MATTER_UNICAST_BINDING)
            {
                error = mPendingNotificationMap.AddPendingNotification(iter.GetIndex(), bindingContext);
                SuccessOrExit(error);
                error = EstablishConnection(ScopedNodeId(iter->nodeId, iter->fabricIndex));
                SuccessOrExit(error);
            }
            else if (iter->type == MATTER_MULTICAST_BINDING)
            {
                mBoundDeviceChangedHandler(*iter, nullptr, bindingContext->GetContext());
            }
        }
    }

exit:
    bindingContext->DecrementConsumersNumber();

    return error;
}

} // namespace chip
