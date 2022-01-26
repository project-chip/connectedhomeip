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

int16_t PendingNotificationMap::FindLRUBindingEntryIndex()
{
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
    int16_t lruBindingEntryIndex = -1;
    uint16_t minApperValue       = UINT16_MAX;
    for (uint8_t i = 0; i < EMBER_BINDING_TABLE_SIZE; i++)
    {
        if (lastAppear[i] < minApperValue)
        {
            lruBindingEntryIndex = i;
            minApperValue        = lastAppear[i];
        }
    }
    return lruBindingEntryIndex;
}

void PendingNotificationMap::AddPendingNotification(uint8_t bindingEntryId, void * context)
{
    RemoveEntry(bindingEntryId);
    uint8_t addIndex                 = mNumEntries < kMaxPendingNotifications ? mNumEntries : mNextToOverride;
    mPendingBindingEntries[addIndex] = bindingEntryId;
    mPendingContexts[addIndex]       = context;
    if (mNumEntries < kMaxPendingNotifications)
    {
        mNumEntries++;
    }
    else
    {
        mNextToOverride++;
    }
}

void PendingNotificationMap::RemoveEntry(uint8_t bindingEntryId)
{
    uint8_t newBindingEntries[kMaxPendingNotifications];
    void * newContexts[kMaxPendingNotifications];
    uint8_t newEntryCount = 0;
    for (const PendingNotificationEntry & pendingNotification : *this)
    {
        if (pendingNotification.mBindingEntryId != bindingEntryId)
        {
            newBindingEntries[newEntryCount] = pendingNotification.mBindingEntryId;
            newContexts[newEntryCount]       = pendingNotification.mContext;
            newEntryCount++;
        }
    }
    memcpy(mPendingBindingEntries, newBindingEntries, sizeof(newBindingEntries));
    memcpy(mPendingContexts, newContexts, sizeof(newContexts));
    mNextToOverride = 0;
    mNumEntries     = newEntryCount;
}

void PendingNotificationMap::RemoveAllEntriesForNode(FabricIndex fabric, NodeId node)
{
    uint8_t newBindingEntries[kMaxPendingNotifications];
    void * newContexts[kMaxPendingNotifications];
    uint8_t newEntryCount = 0;
    for (const PendingNotificationEntry & pendingNotification : *this)
    {
        EmberBindingTableEntry entry;
        emberGetBinding(pendingNotification.mBindingEntryId, &entry);
        if (entry.fabricIndex != fabric || entry.nodeId != node)
        {
            newBindingEntries[newEntryCount] = pendingNotification.mBindingEntryId;
            newContexts[newEntryCount]       = pendingNotification.mContext;
            newEntryCount++;
        }
    }
    memcpy(mPendingBindingEntries, newBindingEntries, sizeof(newBindingEntries));
    memcpy(mPendingContexts, newContexts, sizeof(newContexts));
    mNextToOverride = 0;
    mNumEntries     = newEntryCount;
}

} // namespace chip
