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

namespace chip {

CHIP_ERROR PendingNotificationMap::FindLRUConnectPeer(FabricIndex * fabric, NodeId * node)
{
    // When entries are added to PendingNotificationMap, they are appended to the end.
    // To find the LRU peer, we need to find the peer whose last entry in the map is closer
    // to the start of the list than the last entry of any other peer.

    // First, set up a way to easily track which entries correspond to the same peer.
    uint8_t bindingWithSamePeer[EMBER_BINDING_TABLE_SIZE];

    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(i, &entry);
        if (entry.type != EMBER_UNICAST_BINDING)
        {
            continue;
        }
        bool foundSamePeer = false;
        for (uint8_t j = 0; j < i; j++)
        {
            EmberBindingTableEntry checkEntry;
            emberGetBinding(j, &checkEntry);
            if (checkEntry.type == EMBER_UNICAST_BINDING && checkEntry.fabricIndex == entry.fabricIndex &&
                checkEntry.nodeId == entry.nodeId)
            {
                foundSamePeer          = true;
                bindingWithSamePeer[i] = j;
                break;
            }
        }
        if (!foundSamePeer)
        {
            bindingWithSamePeer[i] = i;
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
        EmberBindingTableEntry entry;
        emberGetBinding(lruBindingEntryIndex, &entry);
        *fabric = entry.fabricIndex;
        *node   = entry.nodeId;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

void PendingNotificationMap::AddPendingNotification(uint8_t bindingEntryId, void * context)
{
    RemoveEntry(bindingEntryId);
    mPendingBindingEntries[mNumEntries] = bindingEntryId;
    mPendingContexts[mNumEntries]       = context;
    mNumEntries++;
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
    }
    mNumEntries = newEntryCount;
}

void PendingNotificationMap::RemoveAllEntriesForNode(FabricIndex fabric, NodeId node)
{
    uint8_t newEntryCount = 0;
    for (int i = 0; i < mNumEntries; i++)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(mPendingBindingEntries[i], &entry);

        if (entry.fabricIndex != fabric || entry.nodeId != node)
        {
            mPendingBindingEntries[newEntryCount] = mPendingBindingEntries[i];
            mPendingContexts[newEntryCount]       = mPendingContexts[i];
            newEntryCount++;
        }
    }
    mNumEntries = newEntryCount;
}

void PendingNotificationMap::RemoveAllEntriesForFabric(FabricIndex fabric)
{
    uint8_t newEntryCount = 0;
    for (int i = 0; i < mNumEntries; i++)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(mPendingBindingEntries[i], &entry);

        if (entry.fabricIndex != fabric)
        {
            mPendingBindingEntries[newEntryCount] = mPendingBindingEntries[i];
            mPendingContexts[newEntryCount]       = mPendingContexts[i];
            newEntryCount++;
        }
    }
    mNumEntries = newEntryCount;
}

} // namespace chip
