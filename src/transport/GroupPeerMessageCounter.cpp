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

#include <lib/support/CodeUtils.h>
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

/// Choose which slot to evict when a fabric's peer pool is full.
///
/// Selects the least-recently-used entry (highest index) belonging to whichever
/// keyset currently holds the most slots. Ties in slot count are broken by:
///   1. Preferring `incomingKeysetId` if tied for the maximum count.
///   2. Otherwise, choosing the highest index (globally oldest) among tied keysets.
uint32_t FindFairShareEvictionIndex(const GroupSender * list, uint32_t count, KeysetId incomingKeysetId)
{
    VerifyOrDie(count > 0);

    uint32_t evictIndex  = 0;
    uint32_t maxCount    = 0;
    bool evictIsIncoming = false;

    for (uint32_t i = 0; i < count; i++)
    {
        const KeysetId candidateKeyset = list[i].mKeysetId;

        uint32_t keysetCount = 0;
        uint32_t keysetLru   = i;
        for (uint32_t j = 0; j < count; j++)
        {
            if (list[j].mKeysetId == candidateKeyset)
            {
                keysetCount++;
                keysetLru = j;
            }
        }

        // Evaluate each distinct keyset once, at its LRU (highest) index.
        if (keysetLru != i)
        {
            continue;
        }

        const bool isIncoming         = (candidateKeyset == incomingKeysetId);
        const bool strictlyMore       = (keysetCount > maxCount);
        const bool tiedPreferIncoming = (keysetCount == maxCount && isIncoming && !evictIsIncoming);
        const bool tiedOlder          = (keysetCount == maxCount && isIncoming == evictIsIncoming && keysetLru > evictIndex);

        if (strictlyMore || tiedPreferIncoming || tiedOlder)
        {
            maxCount        = keysetCount;
            evictIndex      = keysetLru;
            evictIsIncoming = isIncoming;
        }
    }

    return evictIndex;
}

/// Find peer by (keysetId, nodeId) (move to MRU) or add it; return counter.
///
/// `list` holds `maxLimit` slots, of which the first `peerCount` entries are valid.
/// `peerType` is a label used only in eviction log messages.
/// `outCounter` is an OUTPUT value pointing to the PeerMessageCounter of the entry found or inserted.
CHIP_ERROR FindOrAddPeerInList(GroupSender * list, uint32_t maxLimit, uint8_t & peerCount, KeysetId keysetId, NodeId nodeId,
                               chip::Transport::PeerMessageCounter *& outCounter, const char * peerType)
{
    static constexpr uint32_t kInvalidIndex = std::numeric_limits<uint32_t>::max();

    GroupSender temp;
    temp.mNodeId   = nodeId;
    temp.mKeysetId = keysetId;

    uint32_t insertPos = kInvalidIndex;

    // Search for peer
    for (uint32_t i = 0; i < peerCount; i++)
    {
        if (list[i].mKeysetId == keysetId && list[i].mNodeId == nodeId)
        {
            insertPos = i;
            temp      = std::move(list[i]);
            break;
        }
    }

    if (insertPos == kInvalidIndex)
    {
        if (peerCount < maxLimit)
        {
            insertPos = peerCount;
            peerCount++;
        }
        else
        {
            insertPos = FindFairShareEvictionIndex(list, peerCount, keysetId);
            ChipLogProgress(SecureChannel,
                            "GroupPeerTable: Evicting %s peer " ChipLogFormatX64 " (keyset 0x%04x) due to table being full",
                            peerType, ChipLogValueX64(list[insertPos].mNodeId), list[insertPos].mKeysetId);
        }
    }

    ShiftAndInsert(list, insertPos, std::move(temp));
    outCounter = &(list[0].msgCounter);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FindOrAddPeerInFabric(GroupFabric & groupFabric, KeysetId keysetId, NodeId nodeId, bool isControl,
                                 chip::Transport::PeerMessageCounter *& outCounter)
{
    static_assert(CHIP_CONFIG_MAX_GROUP_DATA_PEERS >= 2 && CHIP_CONFIG_MAX_GROUP_DATA_PEERS <= UINT8_MAX,
                  "CHIP_CONFIG_MAX_GROUP_DATA_PEERS must be in [2, 255]");
    static_assert(CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS >= 2 && CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS <= UINT8_MAX,
                  "CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS must be in [2, 255]");

    if (isControl)
    {
        return FindOrAddPeerInList(groupFabric.mControlGroupSenders, CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS,
                                   groupFabric.mControlPeerCount, keysetId, nodeId, outCounter, "control");
    }
    return FindOrAddPeerInList(groupFabric.mDataGroupSenders, CHIP_CONFIG_MAX_GROUP_DATA_PEERS, groupFabric.mDataPeerCount,
                               keysetId, nodeId, outCounter, "data");
}

} // anonymous namespace

CHIP_ERROR GroupPeerTable::FindOrAddPeer(FabricIndex fabricIndex, KeysetId keysetId, NodeId nodeId, bool isControl,
                                         chip::Transport::PeerMessageCounter *& counter)
{
    if (fabricIndex == kUndefinedFabricIndex || keysetId == kInvalidKeysetId || nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (auto & groupFabric : mGroupFabrics)
    {
        if (groupFabric.mFabricIndex == kUndefinedFabricIndex)
        {
            groupFabric.mFabricIndex = fabricIndex;
            return FindOrAddPeerInFabric(groupFabric, keysetId, nodeId, isControl, counter);
        }

        if (fabricIndex == groupFabric.mFabricIndex)
        {
            return FindOrAddPeerInFabric(groupFabric, keysetId, nodeId, isControl, counter);
        }
    }

    // Exceeded the Max number of Group fabrics
    return CHIP_ERROR_NO_MEMORY;
}

// Used in case of MCSP failure
CHIP_ERROR GroupPeerTable::RemovePeer(FabricIndex fabricIndex, KeysetId keysetId, NodeId nodeId, bool isControl)
{
    if (fabricIndex == kUndefinedFabricIndex || keysetId == kInvalidKeysetId || nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::optional<uint32_t> fabricPos = FindFabricIndex(fabricIndex);
    VerifyOrReturnError(fabricPos.has_value(), CHIP_ERROR_NOT_FOUND);

    GroupFabric & groupFabric = mGroupFabrics[*fabricPos];
    GroupSender * list        = isControl ? groupFabric.mControlGroupSenders : groupFabric.mDataGroupSenders;
    uint32_t limit            = isControl ? CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS : CHIP_CONFIG_MAX_GROUP_DATA_PEERS;
    uint8_t & peerCount       = isControl ? groupFabric.mControlPeerCount : groupFabric.mDataPeerCount;

    VerifyOrReturnError(RemoveSpecificPeer(list, keysetId, nodeId, limit), CHIP_ERROR_NOT_FOUND);
    peerCount--;

    if (groupFabric.mDataPeerCount == 0 && groupFabric.mControlPeerCount == 0)
    {
        RemoveAndCompactFabric(*fabricPos);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupPeerTable::KeySetRemoved(FabricIndex fabricIndex, KeysetId keysetId)
{
    if (fabricIndex == kUndefinedFabricIndex || keysetId == kInvalidKeysetId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::optional<uint32_t> fabricPos = FindFabricIndex(fabricIndex);
    VerifyOrReturnError(fabricPos.has_value(), CHIP_ERROR_NOT_FOUND);

    GroupFabric & groupFabric = mGroupFabrics[*fabricPos];
    uint8_t removedData =
        RemovePeersForKeySet(groupFabric.mDataGroupSenders, groupFabric.mDataPeerCount, keysetId, CHIP_CONFIG_MAX_GROUP_DATA_PEERS);
    uint8_t removedControl = RemovePeersForKeySet(groupFabric.mControlGroupSenders, groupFabric.mControlPeerCount, keysetId,
                                                  CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS);

    if (removedData == 0 && removedControl == 0)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    if (groupFabric.mDataPeerCount == 0 && groupFabric.mControlPeerCount == 0)
    {
        RemoveAndCompactFabric(*fabricPos);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupPeerTable::FabricRemoved(FabricIndex fabricIndex)
{
    if (fabricIndex == kUndefinedFabricIndex)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    std::optional<uint32_t> fabricPos = FindFabricIndex(fabricIndex);
    VerifyOrReturnError(fabricPos.has_value(), CHIP_ERROR_NOT_FOUND);

    RemoveAndCompactFabric(*fabricPos);
    return CHIP_NO_ERROR;
}

std::optional<uint32_t> GroupPeerTable::FindFabricIndex(FabricIndex fabricIndex) const
{
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_FABRICS; i++)
    {
        if (mGroupFabrics[i].mFabricIndex == kUndefinedFabricIndex)
        {
            break;
        }
        if (mGroupFabrics[i].mFabricIndex == fabricIndex)
        {
            return i;
        }
    }
    return std::nullopt;
}

uint8_t GroupPeerTable::RemovePeersForKeySet(GroupSender * list, uint8_t & peerCount, KeysetId keysetId, uint32_t size)
{
    uint8_t removed = 0;
    for (uint32_t i = 0; i < peerCount; i++)
    {
        if (list[i].mKeysetId == keysetId)
        {
            list[i].Reset();
            removed++;
        }
    }

    if (removed > 0)
    {
        CompactPeers(list, size);
        peerCount = static_cast<uint8_t>(peerCount - removed);
    }

    return removed;
}

bool GroupPeerTable::RemoveSpecificPeer(GroupSender * list, KeysetId keysetId, NodeId nodeId, uint32_t size)
{
    bool removed = false;
    for (uint32_t nodeIt = 0; nodeIt < size; nodeIt++)
    {
        if (list[nodeIt].mKeysetId == keysetId && list[nodeIt].mNodeId == nodeId)
        {
            list[nodeIt].Reset();
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
        list[i].Reset();
    }
}

void GroupPeerTable::RemoveAndCompactFabric(uint32_t tableIndex)
{
    if (tableIndex >= CHIP_CONFIG_MAX_FABRICS)
    {
        return;
    }
    mGroupFabrics[tableIndex].Reset();

    // Maintain a dense prefix of active GroupFabric entries.
    for (uint32_t i = CHIP_CONFIG_MAX_FABRICS - 1; i > tableIndex; i--)
    {
        if (mGroupFabrics[i].mFabricIndex != kUndefinedFabricIndex)
        {
            mGroupFabrics[tableIndex] = std::move(mGroupFabrics[i]);
            mGroupFabrics[i].Reset();
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
