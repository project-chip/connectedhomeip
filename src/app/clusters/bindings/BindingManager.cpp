/*
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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
#include <app/clusters/bindings/binding-table.h>
#include <credentials/FabricTable.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Scoped.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {

Manager Manager::sBindingManager;

Manager::~Manager()
{
    mIsShuttingDown = true;
    while (!mConnectionCallbacks.Empty())
    {
        auto * callback = &*mConnectionCallbacks.begin();
        mConnectionCallbacks.Remove(callback);
        Platform::Delete(callback);
    }
    mPendingNotificationMap.Shutdown();
}

CHIP_ERROR Manager::UnicastBindingCreated(uint8_t fabricIndex, NodeId nodeId)
{
    return EstablishConnection(ScopedNodeId(nodeId, fabricIndex));
}

CHIP_ERROR Manager::UnicastBindingRemoved(uint8_t bindingEntryId)
{
    ScopedChange removing(mRemovingBindings, true);
    mPendingNotificationMap.RemoveEntry(bindingEntryId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Manager::Init(const ManagerInitParams & params)
{
    VerifyOrReturnError(!mIsShuttingDown, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(params.mCASESessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.mFabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(params.mStorage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mInitParams = params;
    mBindingTable.SetPersistentStorage(params.mStorage);
    TEMPORARY_RETURN_IGNORED params.mFabricTable->AddFabricDelegate(this);

    CHIP_ERROR error = mBindingTable.LoadFromStorage();
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
            for (const TableEntry & entry : mBindingTable)
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

CHIP_ERROR Manager::EstablishConnection(const ScopedNodeId & nodeId)
{
    VerifyOrReturnError(!mIsShuttingDown, CHIP_ERROR_INCORRECT_STATE);
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
            VerifyOrReturnError(connectionCallback != nullptr, CHIP_ERROR_NO_MEMORY);
            mInitParams.mCASESessionManager->FindOrEstablishSession(nodeId, connectionCallback->GetOnDeviceConnected(),
                                                                    connectionCallback->GetOnDeviceConnectionFailure());
        }
    }
    return mLastSessionEstablishmentError;
}

void Manager::HandleDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    for (auto it = mBindingTable.begin(); it != mBindingTable.end(); ++it)
    {
        if (it->type != MATTER_UNICAST_BINDING || sessionHandle->GetPeer() != ScopedNodeId(it->nodeId, it->fabricIndex))
        {
            continue;
        }
        for (PendingNotificationEntry pending : mPendingNotificationMap)
        {
            if (pending.mBindingEntryId == it.GetIndex())
            {
                if (pending.mContext->HasExpired())
                {
                    mPendingNotificationMap.RemoveEntry(pending.mBindingEntryId, CHIP_ERROR_TIMEOUT);
                    break;
                }
                const TableEntry entry = *it;
                auto * context         = pending.mContext;
                context->IncrementConsumersNumber();
                mPendingNotificationMap.RemoveEntry(pending.mBindingEntryId, CHIP_NO_ERROR);
                OperationalDeviceProxy device(&exchangeMgr, sessionHandle);
                mBoundDeviceChangedHandler(entry, &device, context->GetContext());
                context->DecrementConsumersNumber();
                break;
            }
        }
    }
}

void Manager::HandleDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    // Simply release the entry, the connection will be re-established as needed.
    ChipLogError(AppServer, "Failed to establish connection to node 0x" ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    mLastSessionEstablishmentError = error;
    // We don't release the entry when connection fails, because inside
    // BindingManager::EstablishConnection we may try again the connection.
    // TODO(#22173): The logic in there doesn't actually make any sense with how
    // mPendingNotificationMap and CASESessionManager are implemented today.
}

void Manager::FabricRemoved(FabricIndex fabricIndex)
{
    VerifyOrDie(mInitParams.mCASESessionManager != nullptr);

    ScopedChange removing(mRemovingBindings, true);
    mPendingNotificationMap.RemoveAllEntriesForFabric(fabricIndex);

    // TODO(#18436): NOC cluster should handle fabric removal without needing binding manager
    //               to execute such a release. Currently not done because paths were not tested.
    mInitParams.mCASESessionManager->ReleaseSessionsForFabric(fabricIndex);
}

CHIP_ERROR Manager::NotifyBoundClusterChanged(EndpointId endpoint, ClusterId cluster, void * context)
{
    VerifyOrReturnError(mInitParams.mFabricTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mIsShuttingDown && !mRemovingBindings && !mPendingNotificationMap.IsShuttingDown(),
                        CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mBoundDeviceChangedHandler != nullptr, CHIP_ERROR_HANDLER_NOT_SET);

    for (auto it = mBindingTable.begin(); it != mBindingTable.end(); ++it)
    {
        if (it->local == endpoint && it->clusterId.value_or(cluster) == cluster && it->type == MATTER_UNICAST_BINDING)
        {
            VerifyOrReturnError(!mPendingNotificationMap.HasPendingNotification(it.GetIndex(), true), CHIP_ERROR_BUSY);
        }
    }

    auto * bindingContext = mPendingNotificationMap.NewPendingNotificationContext(context);
    VerifyOrReturnError(bindingContext != nullptr, CHIP_ERROR_NO_MEMORY);

    bindingContext->IncrementConsumersNumber();

    for (auto iter = mBindingTable.begin(); iter != mBindingTable.end(); ++iter)
    {
        if (iter->local == endpoint && (iter->clusterId.value_or(cluster) == cluster))
        {
            if (iter->type == MATTER_UNICAST_BINDING)
            {
                CHIP_ERROR error = mPendingNotificationMap.AddPendingNotification(iter.GetIndex(), bindingContext);
                if (error == CHIP_NO_ERROR)
                {
                    error = EstablishConnection(ScopedNodeId(iter->nodeId, iter->fabricIndex));
                }
                // bindingContext owns the context at this point. Returning an error would make the caller
                // release the context a second time, so the failure is only logged.
                if (error != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                 "Failed to notify bound cluster change on endpoint %u, cluster " ChipLogFormatMEI
                                 " to node " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                                 endpoint, ChipLogValueMEI(cluster), ChipLogValueX64(iter->nodeId), error.Format());
                }
            }
            else if (iter->type == MATTER_MULTICAST_BINDING)
            {
                mBoundDeviceChangedHandler(*iter, nullptr, bindingContext->GetContext());
            }
        }
    }

    bindingContext->DecrementConsumersNumber();

    return CHIP_NO_ERROR;
}

CHIP_ERROR Manager::DispatchCommand(EndpointId endpoint, ClusterId cluster, void * context,
                                    PendingCommandFailureHandler failureHandler, System::Layer & layer,
                                    System::Clock::Timeout timeout)
{
    VerifyOrReturnError(mInitParams.mFabricTable != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mIsShuttingDown && !mRemovingBindings && !mPendingNotificationMap.IsShuttingDown(),
                        CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mBoundDeviceChangedHandler != nullptr, CHIP_ERROR_HANDLER_NOT_SET);
    VerifyOrReturnError(failureHandler != nullptr && timeout.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);
    bool hasUnicast = false;
    for (auto it = mBindingTable.begin(); it != mBindingTable.end(); ++it)
    {
        if (it->local == endpoint && it->clusterId.value_or(cluster) == cluster && it->type == MATTER_UNICAST_BINDING)
        {
            hasUnicast = true;
            VerifyOrReturnError(!mPendingNotificationMap.HasPendingNotification(it.GetIndex()), CHIP_ERROR_BUSY);
        }
    }
    auto * pending = mPendingNotificationMap.NewPendingNotificationContext(context, failureHandler);
    VerifyOrReturnError(pending != nullptr, CHIP_ERROR_NO_MEMORY);
    if (hasUnicast)
    {
        CHIP_ERROR error = pending->StartTimer(mPendingNotificationMap, layer, timeout);
        if (error != CHIP_NO_ERROR)
        {
            Platform::Delete(pending);
            return error;
        }
    }
    pending->IncrementConsumersNumber();
    for (auto it = mBindingTable.begin(); it != mBindingTable.end(); ++it)
    {
        if (it->local == endpoint && it->clusterId.value_or(cluster) == cluster && it->type == MATTER_UNICAST_BINDING)
        {
            VerifyOrDie(mPendingNotificationMap.AddPendingNotification(it.GetIndex(), pending) == CHIP_NO_ERROR);
        }
    }
    for (auto it = mBindingTable.begin(); it != mBindingTable.end(); ++it)
    {
        if (it->local != endpoint || it->clusterId.value_or(cluster) != cluster)
        {
            continue;
        }
        if (it->type == MATTER_MULTICAST_BINDING)
        {
            mBoundDeviceChangedHandler(*it, nullptr, context);
        }
        else if (it->type == MATTER_UNICAST_BINDING)
        {
            // Session setup failure is retriable until the CASE deadline, without replaying sent commands.
            (void) EstablishConnection(ScopedNodeId(it->nodeId, it->fabricIndex));
        }
    }
    pending->DecrementConsumersNumber();
    return CHIP_NO_ERROR;
}

CHIP_ERROR Manager::AddBindingEntry(const Binding::TableEntry & entry)
{
    CHIP_ERROR err = mBindingTable.Add(entry);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return CHIP_IM_GLOBAL_STATUS(ResourceExhausted);
    }

    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (entry.type == Binding::MATTER_UNICAST_BINDING)
    {
        err = UnicastBindingCreated(entry.fabricIndex, entry.nodeId);
        if (err != CHIP_NO_ERROR)
        {
            // Unicast connection failure can happen if peer is offline. We'll retry connection on-demand.
            ChipLogError(
                Zcl, "Binding: Failed to create session for unicast binding to device " ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                ChipLogValueX64(entry.nodeId), err.Format());
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace Binding

CHIP_ERROR AddBindingEntry(const Binding::TableEntry & entry)
{
    return Binding::Manager::GetInstance().AddBindingEntry(entry);
}

} // namespace Clusters
} // namespace app
} // namespace chip
