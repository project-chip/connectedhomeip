/*
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

/**
 *    @file
 *      This file defines the Matter Group message counters of remote nodes for groups.
 *
 */

#include <lib/support/DefaultStorageKeyAllocator.h>
#include <limits>
#include <transport/GroupPeerMessageCounter.h>

#include <crypto/RandUtils.h>
#include <utility>

namespace chip {
namespace Transport {

namespace {

/// Shift elements right and insert newEntry at front (MRU position).
///
/// All elements [0..oldIndex) are shifted one slot to the right; newEntry
/// becomes list[0] as the most-recently-used entry.
void ShiftAndInsert(GroupSender * list, uint32_t oldIndex, GroupSender && newEntry)
{
    for (uint32_t j = oldIndex; j > 0; j--)
    {
        list[j] = std::move(list[j - 1]);
    }
    list[0] = std::move(newEntry);
}

/// Find peer by nodeId (move to MRU) or add it; return counter.
///
/// `list` is a list of `maxLimit` items, out of which the first `peerCount` elements are valid.
/// `nodeId` is searched for and added if not found:
///   - if found, it gets moved to `list[0]` as the most recently used element
///   - if not found, it will be inserted at index 0, shifting everything else to the right.
///     peerCount may increase if space exists, otherwise the LRU entry (at maxLimit - 1) will be evicted
/// `peerType` is a label used only in eviction log messages.
///
/// `outCounter` is an OUTPUT value, pointing to the PeerMessageCounter of the GroupSender found or inserted.
CHIP_ERROR FindOrAddPeerFabricFound(GroupSender * list, uint32_t maxLimit, uint8_t & peerCount, NodeId nodeId,
                                    chip::Transport::PeerMessageCounter *& outCounter, const char * peerType)
{
    static uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();

    GroupSender temp;
    temp.mNodeId = nodeId;

    uint32_t insertPos = kInvalidIndex;

    // Search for peer
    for (uint32_t i = 0; i < peerCount; i++)
    {
        if (list[i].mNodeId == nodeId)
        {
            insertPos = i;
            temp      = std::move(list[i]);
            break;
        }
    }

    if (insertPos == kInvalidIndex)
    {
        // GroupSender was not found, must add a new one for this node id.
        if (peerCount < maxLimit)
        {
            peerCount++;
        }
        else
        {
            // Evict LRU
            ChipLogProgress(SecureChannel, "GroupPeerTable: Evicting %s peer " ChipLogFormatX64 " due to table being full",
                            peerType, ChipLogValueX64(list[maxLimit - 1].mNodeId));
        }
        insertPos = peerCount - 1;
    }

    ShiftAndInsert(list, insertPos, std::move(temp));
    outCounter = &(list[0].msgCounter);
    return CHIP_NO_ERROR;
}

} // anonymous namespace

CHIP_ERROR GroupPeerTable::FindOrAddPeer(FabricIndex fabricIndex, NodeId nodeId, bool isControl,
                                         chip::Transport::PeerMessageCounter *& counter)
{
    if (fabricIndex == kUndefinedFabricIndex || nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (auto & groupFabric : mGroupFabrics)
    {
        if (groupFabric.mFabricIndex == kUndefinedFabricIndex)
        {
            // Already iterated through all known fabricIndex
            // Add the new peer to save some processing time
            groupFabric.mFabricIndex = fabricIndex;
            if (isControl)
            {
                groupFabric.mControlGroupSenders[0].mNodeId = nodeId;
                counter                                     = &(groupFabric.mControlGroupSenders[0].msgCounter);
                groupFabric.mControlPeerCount++;
            }
            else
            {
                groupFabric.mDataGroupSenders[0].mNodeId = nodeId;
                counter                                  = &(groupFabric.mDataGroupSenders[0].msgCounter);
                groupFabric.mDataPeerCount++;
            }
            return CHIP_NO_ERROR;
        }

        if (fabricIndex == groupFabric.mFabricIndex)
        {
            if (isControl)
            {
                return FindOrAddPeerFabricFound(groupFabric.mControlGroupSenders, CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS,
                                                groupFabric.mControlPeerCount, nodeId, counter, "control");
            }
            return FindOrAddPeerFabricFound(groupFabric.mDataGroupSenders, CHIP_CONFIG_MAX_GROUP_DATA_PEERS,
                                            groupFabric.mDataPeerCount, nodeId, counter, "data");
        }
    }

    // Exceeded the Max number of Group fabrics
    return CHIP_ERROR_NO_MEMORY;
}

// Used in case of MCSP failure
CHIP_ERROR GroupPeerTable::RemovePeer(FabricIndex fabricIndex, NodeId nodeId, bool isControl)
{
    CHIP_ERROR err    = CHIP_ERROR_NOT_FOUND;
    uint32_t fabricIt = CHIP_CONFIG_MAX_FABRICS;

    if (fabricIndex == kUndefinedFabricIndex || nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        if (fabricIndex == mGroupFabrics[it].mFabricIndex)
        {
            if (isControl)
            {
                if (RemoveSpecificPeer(mGroupFabrics[it].mControlGroupSenders, nodeId, CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS))
                {
                    fabricIt = it;
                    mGroupFabrics[it].mControlPeerCount--;
                    err = CHIP_NO_ERROR;
                }
            }
            else
            {
                if (RemoveSpecificPeer(mGroupFabrics[it].mDataGroupSenders, nodeId, CHIP_CONFIG_MAX_GROUP_DATA_PEERS))
                {
                    fabricIt = it;
                    mGroupFabrics[it].mDataPeerCount--;
                    err = CHIP_NO_ERROR;
                }
            }
            break;
        }
    }

    // Remove Fabric entry from PeerTable if empty
    if (fabricIt < CHIP_CONFIG_MAX_FABRICS)
    {
        if (mGroupFabrics[fabricIt].mDataPeerCount == 0 && mGroupFabrics[fabricIt].mControlPeerCount == 0)
        {
            RemoveAndCompactFabric(fabricIt);
        }
    }

    // Cannot find Peer to remove
    return err;
}

CHIP_ERROR GroupPeerTable::FabricRemoved(FabricIndex fabricIndex)
{
    CHIP_ERROR err = CHIP_ERROR_NOT_FOUND;

    if (fabricIndex == kUndefinedFabricIndex)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        if (fabricIndex == mGroupFabrics[it].mFabricIndex)
        {
            RemoveAndCompactFabric(it);
            return CHIP_NO_ERROR;
        }
    }

    // Cannot find Fabric to remove
    return err;
}

bool GroupPeerTable::RemoveSpecificPeer(GroupSender * list, NodeId nodeId, uint32_t size)
{
    bool removed = false;
    for (uint32_t nodeIt = 0; nodeIt < size; nodeIt++)
    {
        if (list[nodeIt].mNodeId == nodeId)
        {
            list[nodeIt].mNodeId = kUndefinedNodeId;
            list[nodeIt].msgCounter.Reset();
            removed = true;
            break;
        }
    }

    if (removed)
    {
        CompactPeers(list, size);
    }

    return removed;
}

void GroupPeerTable::CompactPeers(GroupSender * list, uint32_t size)
{
    if (list == nullptr || size == 0)
    {
        return;
    }

    uint32_t writeIndex = 0;
    for (uint32_t readIndex = 0; readIndex < size; readIndex++)
    {
        // readIndex and writeIndex will iterate together, until a point
        // is found where the node id at the read index is kUndefinedNodeId. This
        // means the GroupSender entry at this index has been removed (likely by
        // RemoveSpecificPeer()). When this removed entry index is found, the read
        // index will be 1 ahead of the write index, and then all entries
        // that follow will be shifted down the list (moving 1 to the left).
        if (list[readIndex].mNodeId != kUndefinedNodeId)
        {
            if (readIndex != writeIndex)
            {
                list[writeIndex] = std::move(list[readIndex]);
            }
            writeIndex++;
        }
    }

    // Cleanup old entries at the end of the list. These were entries that were
    // moved further up in the list, and the entries from the writeIndex onwards
    // are in an unspecified state.
    for (uint32_t i = writeIndex; i < size; i++)
    {
        list[i] = GroupSender();
    }
}

void GroupPeerTable::RemoveAndCompactFabric(uint32_t tableIndex)
{
    if (tableIndex >= CHIP_CONFIG_MAX_FABRICS)
    {
        return;
    }
    mGroupFabrics[tableIndex].mFabricIndex = kUndefinedFabricIndex;
    new (&mGroupFabrics[tableIndex]) GroupFabric();

    // To maintain logic integrity Fabric array cannot have empty slot in between data
    // Find the last non empty element
    for (uint32_t i = CHIP_CONFIG_MAX_FABRICS - 1; i > tableIndex; i--)
    {
        if (mGroupFabrics[i].mFabricIndex != kUndefinedFabricIndex)
        {
            // Logic works since all buffer are static
            // move it up front
            new (&mGroupFabrics[tableIndex]) GroupFabric(mGroupFabrics[i]);
            new (&mGroupFabrics[i]) GroupFabric();
            break;
        }
    }
}

GroupOutgoingCounters::GroupOutgoingCounters(chip::PersistentStorageDelegate * storage_delegate)
{
    TEMPORARY_RETURN_IGNORED Init(storage_delegate);
}

CHIP_ERROR GroupOutgoingCounters::Init(chip::PersistentStorageDelegate * storage_delegate)
{

    if (storage_delegate == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Spec 4.5.1.3
    mStorage      = storage_delegate;
    uint16_t size = static_cast<uint16_t>(sizeof(uint32_t));
    uint32_t temp;
    CHIP_ERROR err;
    err = mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::GroupControlCounter().KeyName(), &temp, size);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // First time retrieving the counter
        mGroupControlCounter = (chip::Crypto::GetRandU32() & kMessageCounterRandomInitMask) + 1;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    else
    {
        mGroupControlCounter = temp;
    }

    err = mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::GroupDataCounter().KeyName(), &temp, size);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // First time retrieving the counter
        mGroupDataCounter = (chip::Crypto::GetRandU32() & kMessageCounterRandomInitMask) + 1;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    else
    {
        mGroupDataCounter = temp;
    }

    temp = mGroupControlCounter + GROUP_MSG_COUNTER_MIN_INCREMENT;
    size = static_cast<uint16_t>(sizeof(temp));
    ReturnErrorOnFailure(mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::GroupControlCounter().KeyName(), &temp, size));

    temp = mGroupDataCounter + GROUP_MSG_COUNTER_MIN_INCREMENT;

    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::GroupDataCounter().KeyName(), &temp, size);
}

uint32_t GroupOutgoingCounters::GetCounter(bool isControl)
{
    return (isControl) ? mGroupControlCounter : mGroupDataCounter;
}

CHIP_ERROR GroupOutgoingCounters::IncrementCounter(bool isControl)
{
    uint32_t temp  = 0;
    uint16_t size  = static_cast<uint16_t>(sizeof(uint32_t));
    uint32_t value = 0;

    StorageKeyName key = StorageKeyName::Uninitialized();

    if (isControl)
    {
        mGroupControlCounter++;
        key   = DefaultStorageKeyAllocator::GroupControlCounter();
        value = mGroupControlCounter;
    }
    else
    {
        mGroupDataCounter++;
        key   = DefaultStorageKeyAllocator::GroupDataCounter();
        value = mGroupDataCounter;
    }

    if (mStorage == nullptr)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(key.KeyName(), &temp, size));
    if (temp == value)
    {
        temp = value + GROUP_MSG_COUNTER_MIN_INCREMENT;
        return mStorage->SyncSetKeyValue(key.KeyName(), &temp, sizeof(uint32_t));
    }
    return CHIP_NO_ERROR;
}

} // namespace Transport
} // namespace chip
