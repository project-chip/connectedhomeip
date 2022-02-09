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
#include <lib/core/NodeId.h>
#include <lib/core/PeerId.h>
#include <lib/support/Span.h>
#include <transport/PeerMessageCounter.h>

#define GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_DATA_PEER 15
#define GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_CONTROL_PEER 15

#define GROUP_MSG_COUNTER_MIN_INCREMENT 1000

namespace chip {
namespace Transport {

class GroupSender
{
public:
    NodeId mNodeId                = kUndefinedNodeId;
    PeerMessageCounter msgCounter = { true };
};

class GroupFabric
{
public:
    FabricId mFabricId        = kUndefinedFabricId;
    uint8_t mControlPeerCount = 0;
    uint8_t mDataPeerCount    = 0;
    GroupSender mDataGroupSenders[GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_DATA_PEER];
    GroupSender mControlGroupSenders[GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_CONTROL_PEER];
};

class GroupPeerTable
{
public:
    CHIP_ERROR FindOrAddPeer(FabricId fabricId, NodeId nodeId, bool isControl, chip::Transport::PeerMessageCounter *& counter);

    // Used in case of MCSP failure
    CHIP_ERROR RemovePeer(FabricId fabricId, NodeId nodeId, bool isControl);

    // For Unit Test
    FabricId GetFabricIdAt(uint8_t index);

private:
    GroupFabric mGroupFabrics[CHIP_CONFIG_MAX_FABRICS];
};

// Might want to rename this so that it is explicitly the sending side of counters
class GroupClientCounters
{
public:
    GroupClientCounters(){};
    GroupClientCounters(chip::PersistentStorageDelegate * storage_delegate);
    CHIP_ERROR Init(chip::PersistentStorageDelegate * storage_delegate);
    uint32_t GetCounter(bool isControl);
    void SetCounter(bool isControl, uint32_t value);

private:
    // TODO Initialize those to random value
    uint32_t mGroupDataCounter                 = 0;
    uint32_t mGroupControlCounter              = 0;
    chip::PersistentStorageDelegate * mStorage = nullptr;
};

} // namespace Transport
} // namespace chip
