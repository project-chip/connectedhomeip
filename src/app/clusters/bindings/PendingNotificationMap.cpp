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

#include <assert.h>

#include <app/clusters/bindings/PendingNotificationMap.h>
#include <app/clusters/bindings/binding-table.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Binding {

void PendingNotificationContext::HandleTimeout(System::Layer *, void * state)
{
    auto * context = static_cast<PendingNotificationContext *>(state);
    context->mMap->RemoveContext(context, CHIP_ERROR_TIMEOUT);
}

void PendingNotificationMap::Shutdown()
{
    mIsShuttingDown = true;
    while (mNumEntries != 0)
    {
        RemoveEntry(mPendingBindingEntries[0]);
    }
}

CHIP_ERROR PendingNotificationMap::FindLRUConnectPeer(ScopedNodeId & nodeId)
{
    // When entries are added to PendingNotificationMap, they are appended to the end.
    // To find the LRU peer, we need to find the peer whose last entry in the map is closer
    // to the start of the list than the last entry of any other peer.

    // First, set up a way to easily track which entries correspond to the same peer.
    uint8_t bindingWithSamePeer[Table::kMaxBindingEntries];

    for (auto iter = mBindingTable.begin(); iter != mBindingTable.end(); ++iter)
    {
        if (iter->type != MATTER_UNICAST_BINDING)
        {
            continue;
        }
        for (auto checkIter = mBindingTable.begin(); checkIter != mBindingTable.end(); ++checkIter)
        {
            if (checkIter->type == MATTER_UNICAST_BINDING && checkIter->fabricIndex == iter->fabricIndex &&
                checkIter->nodeId == iter->nodeId)
            {
                bindingWithSamePeer[iter.GetIndex()] = checkIter.GetIndex();
                break;
            }
        }
    }

    uint16_t lastAppear[Table::kMaxBindingEntries];
    for (uint16_t & value : lastAppear)
    {
        value = UINT16_MAX;
    }
    bool peerHasCommand[Table::kMaxBindingEntries] = {};
    uint16_t appearIndex                           = 0;
    for (PendingNotificationEntry pendingNotification : *this)
    {
        const uint8_t peer = bindingWithSamePeer[pendingNotification.mBindingEntryId];
        lastAppear[peer]   = appearIndex;
        peerHasCommand[peer] =
            peerHasCommand[peer] || (pendingNotification.mContext != nullptr && pendingNotification.mContext->IsCommand());
        appearIndex++;
    }
    uint8_t lruBindingEntryIndex;
    uint16_t minLastAppearValue = UINT16_MAX;
    for (uint8_t i = 0; i < Table::kMaxBindingEntries; i++)
    {
        if (!peerHasCommand[i] && lastAppear[i] < minLastAppearValue)
        {
            lruBindingEntryIndex = i;
            minLastAppearValue   = lastAppear[i];
        }
    }
    if (minLastAppearValue < UINT16_MAX)
    {
        TableEntry entry = mBindingTable.GetAt(static_cast<uint8_t>(lruBindingEntryIndex));
        nodeId           = ScopedNodeId(entry.nodeId, entry.fabricIndex);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR PendingNotificationMap::AddPendingNotification(uint8_t bindingEntryId, PendingNotificationContext * context)
{
    VerifyOrReturnError(!mIsShuttingDown, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!HasPendingNotification(bindingEntryId, true) &&
                            !(context != nullptr && context->IsCommand() && HasPendingNotification(bindingEntryId)),
                        CHIP_ERROR_BUSY);
    RemoveEntry(bindingEntryId);
    if (mNumEntries == Table::kMaxBindingEntries)
    {
        // We know that the RemoveEntry above did not do anything so we don't need to try restoring it.
        return CHIP_ERROR_NO_MEMORY;
    }
    mPendingBindingEntries[mNumEntries] = bindingEntryId;
    mPendingContexts[mNumEntries]       = context;
    if (context)
    {
        context->IncrementConsumersNumber();
    }
    mNumEntries++;
    return CHIP_NO_ERROR;
}

bool PendingNotificationMap::HasPendingNotification(uint8_t bindingEntryId, bool commandsOnly) const
{
    for (uint8_t i = 0; i < mNumEntries; ++i)
    {
        if (mPendingBindingEntries[i] == bindingEntryId &&
            (!commandsOnly || (mPendingContexts[i] != nullptr && mPendingContexts[i]->IsCommand())))
        {
            return true;
        }
    }
    return false;
}

void PendingNotificationMap::RemoveEntry(uint8_t bindingEntryId, CHIP_ERROR error)
{
    for (uint8_t i = 0; i < mNumEntries; ++i)
    {
        if (mPendingBindingEntries[i] != bindingEntryId)
        {
            continue;
        }
        auto * context           = mPendingContexts[i];
        const TableEntry binding = mBindingTable.GetAt(bindingEntryId);
        --mNumEntries;
        for (uint8_t j = i; j < mNumEntries; ++j)
        {
            mPendingBindingEntries[j] = mPendingBindingEntries[j + 1];
            mPendingContexts[j]       = mPendingContexts[j + 1];
        }
        if (context != nullptr)
        {
            context->Failed(binding, error);
            context->DecrementConsumersNumber();
        }
        return;
    }
}

void PendingNotificationMap::RemoveContext(PendingNotificationContext * context, CHIP_ERROR error)
{
    context->IncrementConsumersNumber();
    for (uint8_t i = 0; i < mNumEntries;)
    {
        if (mPendingContexts[i] == context)
        {
            RemoveEntry(mPendingBindingEntries[i], error);
        }
        else
        {
            ++i;
        }
    }
    context->DecrementConsumersNumber();
}

void PendingNotificationMap::RemoveAllEntriesForNode(const ScopedNodeId & nodeId)
{
    for (uint8_t i = 0; i < mNumEntries;)
    {
        const TableEntry entry = mBindingTable.GetAt(mPendingBindingEntries[i]);
        if (entry.fabricIndex == nodeId.GetFabricIndex() && entry.nodeId == nodeId.GetNodeId())
        {
            RemoveEntry(mPendingBindingEntries[i]);
        }
        else
        {
            ++i;
        }
    }
}

void PendingNotificationMap::RemoveAllEntriesForFabric(FabricIndex fabric)
{
    for (uint8_t i = 0; i < mNumEntries;)
    {
        if (mBindingTable.GetAt(mPendingBindingEntries[i]).fabricIndex == fabric)
        {
            RemoveEntry(mPendingBindingEntries[i]);
        }
        else
        {
            ++i;
        }
    }
}

} // namespace Binding
} // namespace Clusters
} // namespace app
} // namespace chip
