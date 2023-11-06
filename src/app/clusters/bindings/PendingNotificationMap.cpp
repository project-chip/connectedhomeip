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

#include <assert.h>

#include <app/clusters/bindings/PendingNotificationMap.h>
#include <app/util/binding-table.h>
#include <app/util/config.h>

namespace chip {

CHIP_ERROR PendingNotificationMap::FindLRUConnectPeer(ScopedNodeId & nodeId)
{
    // When entries are added to PendingNotificationMap, they are appended to the end.
    // To find the LRU peer, we need to find the peer whose last entry in the map is closer
    // to the start of the list than the last entry of any other peer.

    // First, set up a way to easily track which entries correspond to the same peer.
    uint8_t bindingWithSamePeer[EMBER_BINDING_TABLE_SIZE];

    for (auto iter = BindingTable::GetInstance().begin(); iter != BindingTable::GetInstance().end(); ++iter)
    {
        if (iter->type != EMBER_UNICAST_BINDING)
        {
            continue;
        }
        for (auto checkIter = BindingTable::GetInstance().begin(); checkIter != BindingTable::GetInstance().end(); ++checkIter)
        {
            if (checkIter->type == EMBER_UNICAST_BINDING && checkIter->fabricIndex == iter->fabricIndex &&
                checkIter->nodeId == iter->nodeId)
            {
                bindingWithSamePeer[iter.GetIndex()] = checkIter.GetIndex();
                break;
            }
        }
    }

    uint16_t lastAppear[EMBER_BINDING_TABLE_SIZE];
    for (uint16_t & value : lastAppear)
    {
        value = UINT16_MAX;
    }
    uint16_t appearIndex = 0;
    for (PendingNotificationEntry pendingNotification : *this)
    {
        lastAppear[bindingWithSamePeer[pendingNotification.mBindingEntryId]] = appearIndex;
        appearIndex++;
    }
    uint8_t lruBindingEntryIndex;
    uint16_t minLastAppearValue = UINT16_MAX;
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        if (lastAppear[i] < minLastAppearValue)
        {
            lruBindingEntryIndex = i;
            minLastAppearValue   = lastAppear[i];
        }
    }
    if (minLastAppearValue < UINT16_MAX)
    {
        EmberBindingTableEntry entry = BindingTable::GetInstance().GetAt(static_cast<uint8_t>(lruBindingEntryIndex));
        nodeId                       = ScopedNodeId(entry.nodeId, entry.fabricIndex);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR PendingNotificationMap::AddPendingNotification(uint8_t bindingEntryId, PendingNotificationContext * context)
{
    RemoveEntry(bindingEntryId);
    if (mNumEntries == EMBER_BINDING_TABLE_SIZE)
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

void PendingNotificationMap::RemoveEntry(uint8_t bindingEntryId)
{
    uint8_t newEntryCount = 0;
    for (int i = 0; i < mNumEntries; i++)
    {
        if (mPendingBindingEntries[i] != bindingEntryId)
        {
            mPendingBindingEntries[newEntryCount] = mPendingBindingEntries[i];
            mPendingContexts[newEntryCount]       = mPendingContexts[i];
            newEntryCount++;
        }
        else if (mPendingContexts[i] != nullptr)
        {
            mPendingContexts[i]->DecrementConsumersNumber();
        }
    }
    mNumEntries = newEntryCount;
}

void PendingNotificationMap::RemoveAllEntriesForNode(const ScopedNodeId & nodeId)
{
    uint8_t newEntryCount = 0;
    for (int i = 0; i < mNumEntries; i++)
    {
        EmberBindingTableEntry entry = BindingTable::GetInstance().GetAt(mPendingBindingEntries[i]);
        if (entry.fabricIndex != nodeId.GetFabricIndex() || entry.nodeId != nodeId.GetNodeId())
        {
            mPendingBindingEntries[newEntryCount] = mPendingBindingEntries[i];
            mPendingContexts[newEntryCount]       = mPendingContexts[i];
            newEntryCount++;
        }
        else if (mPendingContexts[i] != nullptr)
        {
            mPendingContexts[i]->DecrementConsumersNumber();
        }
    }
    mNumEntries = newEntryCount;
}

void PendingNotificationMap::RemoveAllEntriesForFabric(FabricIndex fabric)
{
    uint8_t newEntryCount = 0;
    for (int i = 0; i < mNumEntries; i++)
    {
        EmberBindingTableEntry entry = BindingTable::GetInstance().GetAt(mPendingBindingEntries[i]);
        if (entry.fabricIndex != fabric)
        {
            mPendingBindingEntries[newEntryCount] = mPendingBindingEntries[i];
            mPendingContexts[newEntryCount]       = mPendingContexts[i];
            newEntryCount++;
        }
        else if (mPendingContexts[i] != nullptr)
        {
            mPendingContexts[i]->DecrementConsumersNumber();
        }
    }
    mNumEntries = newEntryCount;
}

} // namespace chip
