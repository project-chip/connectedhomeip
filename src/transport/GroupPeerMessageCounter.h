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
#pragma once

#include <array>
#include <bitset>

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/NodeId.h>
#include <lib/core/PeerId.h>
#include <lib/support/Span.h>
#include <transport/PeerMessageCounter.h>

#define GROUP_MSG_COUNTER_MIN_INCREMENT 1000

namespace chip {
namespace Transport {

class GroupSender
{
public:
    NodeId mNodeId = kUndefinedNodeId;
    PeerMessageCounter msgCounter;
};

class GroupFabric
{
public:
    FabricIndex mFabricIndex  = kUndefinedFabricIndex;
    uint8_t mControlPeerCount = 0;
    uint8_t mDataPeerCount    = 0;
    GroupSender mDataGroupSenders[CHIP_CONFIG_MAX_GROUP_DATA_PEERS];
    GroupSender mControlGroupSenders[CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS];
};

class GroupPeerTable
{
public:
    CHIP_ERROR FindOrAddPeer(FabricIndex fabricIndex, NodeId nodeId, bool isControl,
                             chip::Transport::PeerMessageCounter *& counter);

    // Used in case of MCSP failure
    CHIP_ERROR RemovePeer(FabricIndex fabricIndex, NodeId nodeId, bool isControl);

    CHIP_ERROR FabricRemoved(FabricIndex fabricIndex);

    // Protected for Unit Tests inheritance
protected:
    bool RemoveSpecificPeer(GroupSender * list, NodeId nodeId, uint32_t size);
    void CompactPeers(GroupSender * list, uint32_t size);
    void RemoveAndCompactFabric(uint32_t tableIndex);

    GroupFabric mGroupFabrics[CHIP_CONFIG_MAX_FABRICS];
};

// Might want to rename this so that it is explicitly the sending side of counters
class GroupOutgoingCounters
{
public:
    static constexpr uint32_t kMessageCounterRandomInitMask = 0x0FFFFFFF; ///< 28-bit mask

    GroupOutgoingCounters(){};
    GroupOutgoingCounters(chip::PersistentStorageDelegate * storage_delegate);
    CHIP_ERROR Init(chip::PersistentStorageDelegate * storage_delegate);
    uint32_t GetCounter(bool isControl);
    CHIP_ERROR IncrementCounter(bool isControl);

    // Protected for Unit Tests inheritance
protected:
    // TODO Initialize those to random value
    uint32_t mGroupDataCounter                 = 0;
    uint32_t mGroupControlCounter              = 0;
    chip::PersistentStorageDelegate * mStorage = nullptr;
};

} // namespace Transport
} // namespace chip
