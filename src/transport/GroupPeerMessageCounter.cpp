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
#include <transport/GroupPeerMessageCounter.h>

namespace chip {
namespace Transport {

CHIP_ERROR GroupPeerTable::FindOrAddPeer(FabricId fabricId, NodeId nodeId, bool isControl,
                                         chip::Transport::PeerMessageCounter *& counter)
{
    if (fabricId == kUndefinedFabricId || nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        if (mGroupFabrics[it].mFabricId == kUndefinedFabricId)
        {
            // Already iterate through all known fabricId
            // Add the new peer to save some processing time
            mGroupFabrics[it].mFabricId = fabricId;
            if (isControl)
            {
                mGroupFabrics[it].mControlGroupSenders[0].mNodeId = nodeId;
                counter                                           = &(mGroupFabrics[it].mControlGroupSenders[0].msgCounter);
                mGroupFabrics[it].mControlPeerCount++;
            }
            else
            {
                mGroupFabrics[it].mDataGroupSenders[0].mNodeId = nodeId;
                counter                                        = &(mGroupFabrics[it].mDataGroupSenders[0].msgCounter);
                mGroupFabrics[it].mDataPeerCount++;
            }
            return CHIP_NO_ERROR;
        }

        if (fabricId == mGroupFabrics[it].mFabricId)
        {
            if (isControl)
            {
                for (uint32_t nodeIt = 0; nodeIt < GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_CONTROL_PEER; nodeIt++)
                {
                    if (mGroupFabrics[it].mControlGroupSenders[nodeIt].mNodeId == kUndefinedNodeId)
                    {
                        // Already iterate through all known NodeId
                        // Add the new peer to save some processing time
                        mGroupFabrics[it].mControlGroupSenders[nodeIt].mNodeId = nodeId;
                        counter = &(mGroupFabrics[it].mControlGroupSenders[nodeIt].msgCounter);
                        mGroupFabrics[it].mControlPeerCount++;
                        return CHIP_NO_ERROR;
                    }

                    if (mGroupFabrics[it].mControlGroupSenders[nodeIt].mNodeId == nodeId)
                    {
                        counter = &(mGroupFabrics[it].mControlGroupSenders[nodeIt].msgCounter);
                        return CHIP_NO_ERROR;
                    }
                }
            }
            else
            {
                for (uint32_t nodeIt = 0; nodeIt < GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_DATA_PEER; nodeIt++)
                {
                    if (mGroupFabrics[it].mDataGroupSenders[nodeIt].mNodeId == kUndefinedNodeId)
                    {
                        // Already iterate through all known NodeId
                        // Add the new peer to save some processing time
                        mGroupFabrics[it].mDataGroupSenders[nodeIt].mNodeId = nodeId;
                        counter = &(mGroupFabrics[it].mDataGroupSenders[nodeIt].msgCounter);
                        mGroupFabrics[it].mDataPeerCount++;
                        return CHIP_NO_ERROR;
                    }

                    if (mGroupFabrics[it].mDataGroupSenders[nodeIt].mNodeId == nodeId)
                    {
                        counter = &(mGroupFabrics[it].mDataGroupSenders[nodeIt].msgCounter);
                        return CHIP_NO_ERROR;
                    }
                }
            }
            // Exceed the Max number of Group peer
            return CHIP_ERROR_TOO_MANY_PEER_NODES;
        }
    }

    // Exceed the Max number of Group peer
    return CHIP_ERROR_TOO_MANY_PEER_NODES;
}

// Used in case of MCSP failure
CHIP_ERROR GroupPeerTable::RemovePeer(FabricId fabricId, NodeId nodeId, bool isControl)
{
    CHIP_ERROR err       = CHIP_ERROR_NOT_FOUND;
    uint32_t fabricIndex = CHIP_CONFIG_MAX_FABRICS;

    if (fabricId == kUndefinedFabricId || nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        if (fabricId == mGroupFabrics[it].mFabricId)
        {
            if (isControl)
            {
                for (uint32_t nodeIt = 0; nodeIt < GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_CONTROL_PEER; nodeIt++)
                {
                    if (mGroupFabrics[it].mControlGroupSenders[nodeIt].mNodeId == nodeId)
                    {
                        fabricIndex                                            = it;
                        mGroupFabrics[it].mControlGroupSenders[nodeIt].mNodeId = kUndefinedNodeId;
                        mGroupFabrics[it].mControlGroupSenders[nodeIt].msgCounter.Reset();
                        mGroupFabrics[it].mControlPeerCount--;
                        err = CHIP_NO_ERROR;
                    }
                }
            }
            else
            {
                for (uint32_t nodeIt = 0; nodeIt < GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_DATA_PEER; nodeIt++)
                {
                    if (mGroupFabrics[it].mDataGroupSenders[nodeIt].mNodeId == nodeId)
                    {
                        fabricIndex                                         = it;
                        mGroupFabrics[it].mDataGroupSenders[nodeIt].mNodeId = kUndefinedNodeId;
                        mGroupFabrics[it].mDataGroupSenders[nodeIt].msgCounter.Reset();
                        mGroupFabrics[it].mDataPeerCount--;
                        err = CHIP_NO_ERROR;
                    }
                }
            }
            break;
        }
    }

    // Remove Fabric entry from PeerTable if empty
    if (fabricIndex < CHIP_CONFIG_MAX_FABRICS)
    {
        if (mGroupFabrics[fabricIndex].mDataPeerCount == 0 && mGroupFabrics[fabricIndex].mControlPeerCount == 0)
        {
            mGroupFabrics[fabricIndex].mFabricId = kUndefinedFabricId;
            // To maintain logic integrety Fabric array cannot have empty slot in between data
            // Move Fabric around
            for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS;)
            {
                GroupFabric buf = mGroupFabrics[it];
                if (buf.mFabricId != kUndefinedFabricId)
                {
                    it++;
                    continue;
                }
                // Find the last non empty element
                for (uint32_t i = CHIP_CONFIG_MAX_FABRICS - 1; i > it; i--)
                {
                    if (mGroupFabrics[i].mFabricId != kUndefinedFabricId)
                    {
                        // Logic works since all buffer are static
                        // move it up front
                        memcpy(static_cast<void *>(mGroupFabrics + it), static_cast<void *>(mGroupFabrics + i),
                               sizeof(GroupFabric));
                        // replace with empty object (easiest way to make sure everything is cleared)
                        memcpy(static_cast<void *>(mGroupFabrics + i), static_cast<void *>(&buf), sizeof(GroupFabric));
                        it++;
                        break;
                    }
                }
                // Nothing to move around
                break;
            }
        }
    }

    // Cannot find Peer to remove
    return err;
}

// For Unit Test
FabricId GroupPeerTable::GetFabricIdAt(uint8_t index)
{
    if (index < CHIP_CONFIG_MAX_FABRICS)
    {
        return mGroupFabrics[index].mFabricId;
    }

    return kUndefinedFabricId;
}

GroupClientCounters::GroupClientCounters(chip::PersistentStorageDelegate * storage_delegate)
{
    Init(storage_delegate);
}

CHIP_ERROR GroupClientCounters::Init(chip::PersistentStorageDelegate * storage_delegate)
{

    if (storage_delegate == nullptr)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // TODO Implement Logic for first time use / factory reset to be random
    // Spec 4.5.1.3
    mStorage      = storage_delegate;
    uint16_t size = static_cast<uint16_t>(sizeof(uint32_t));
    DefaultStorageKeyAllocator key;
    mStorage->SyncGetKeyValue(key.GroupControlCounter(), &mGroupControlCounter, size);
    mStorage->SyncGetKeyValue(key.GroupDataCounter(), &mGroupDataCounter, size);

    uint32_t temp = mGroupControlCounter + GROUP_MSG_COUNTER_MIN_INCREMENT;
    mStorage->SyncSetKeyValue(key.GroupControlCounter(), &temp, size);

    temp = mGroupDataCounter + GROUP_MSG_COUNTER_MIN_INCREMENT;
    mStorage->SyncSetKeyValue(key.GroupDataCounter(), &temp, size);

    return CHIP_NO_ERROR;
}

uint32_t GroupClientCounters::GetCounter(bool isControl)
{
    return (isControl) ? mGroupControlCounter : mGroupDataCounter;
}

void GroupClientCounters::SetCounter(bool isControl, uint32_t value)
{
    uint32_t temp = 0;
    uint16_t size = static_cast<uint16_t>(sizeof(uint32_t));
    DefaultStorageKeyAllocator key;

    if (mStorage == nullptr)
    {
        return;
    }

    if (isControl)
    {
        mStorage->SyncGetKeyValue(key.GroupControlCounter(), &temp, size);
        if (temp <= value || ((temp > (UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT)) && (value < GROUP_MSG_COUNTER_MIN_INCREMENT)))
        {
            temp = value + GROUP_MSG_COUNTER_MIN_INCREMENT;
            mStorage->SyncSetKeyValue(key.GroupControlCounter(), &temp, sizeof(uint32_t));
        }
        mGroupControlCounter = value;
    }
    else
    {
        mStorage->SyncGetKeyValue(key.GroupDataCounter(), &temp, size);
        if (temp <= value || ((temp > (UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT)) && (value < GROUP_MSG_COUNTER_MIN_INCREMENT)))
        {
            temp = value + GROUP_MSG_COUNTER_MIN_INCREMENT;
            mStorage->SyncSetKeyValue(key.GroupDataCounter(), &temp, sizeof(uint32_t));
        }
        mGroupDataCounter = value;
    }
}

} // namespace Transport
} // namespace chip
