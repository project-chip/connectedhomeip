/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
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
 *      This file implements unit tests for the SessionManager implementation.
 */

#include <errno.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/PeerMessageCounter.h>

namespace {

using namespace chip;

class TestGroupOutgoingCounters : public chip::Transport::GroupOutgoingCounters
{
public:
    TestGroupOutgoingCounters(){};
    TestGroupOutgoingCounters(chip::PersistentStorageDelegate * storage_delegate) : GroupOutgoingCounters(storage_delegate){};
    void SetCounter(bool isControl, uint32_t value)
    {
        uint32_t temp = 0;

        StorageKeyName key = StorageKeyName::Uninitialized();

        if (isControl)
        {
            mGroupControlCounter = value;
            key                  = DefaultStorageKeyAllocator::GroupControlCounter();
        }
        else
        {
            mGroupDataCounter = value;
            key               = DefaultStorageKeyAllocator::GroupDataCounter();
        }

        if (mStorage == nullptr)
        {
            return;
        }

        // Always Update storage for Test purposes
        temp = value + GROUP_MSG_COUNTER_MIN_INCREMENT;
        EXPECT_SUCCESS(mStorage->SyncSetKeyValue(key.KeyName(), &temp, sizeof(uint32_t)));
    }
};

class TestGroupPeerTable : public chip::Transport::GroupPeerTable
{
public:
    TestGroupPeerTable(){};
    FabricIndex GetFabricIndexAt(uint8_t index)
    {
        if (index < CHIP_CONFIG_MAX_FABRICS)
        {
            return mGroupFabrics[index].mFabricIndex;
        }

        return kUndefinedFabricIndex;
    }
    NodeId GetNodeIdAt(uint8_t fabricIndex, uint8_t index, bool isControl)
    {
        if (fabricIndex < CHIP_CONFIG_MAX_FABRICS)
        {
            if (isControl && index < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS)
            {
                return mGroupFabrics[fabricIndex].mControlGroupSenders[index].mNodeId;
            }

            if (!isControl && index < CHIP_CONFIG_MAX_GROUP_DATA_PEERS)
            {
                return mGroupFabrics[fabricIndex].mDataGroupSenders[index].mNodeId;
            }
        }

        return kUndefinedNodeId;
    }
};

TEST(TestGroupMessageCounter, AddPeerTest)
{
    NodeId peerNodeId                             = 1234;
    FabricIndex fabricIndex                       = 1;
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable mGroupPeerMsgCounter;

    // Add max number of group data peers
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId + i, false, counter);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Verify they are in the table in the correct LRU order (most recent at index 0)
    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(0, i, false), peerNodeId + (CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1 - i));
    }

    // Add (CHIP_CONFIG_MAX_GROUP_DATA_PEERS + 1)th peer (should trigger eviction of 1234,
    // which was at index CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1)
    NodeId newPeerNodeId = peerNodeId + CHIP_CONFIG_MAX_GROUP_DATA_PEERS;
    err                  = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, newPeerNodeId, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify the entire list ordering after eviction (newPeerNodeId at index 0, down to
    // 1235 at index CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1)
    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(0, i, false), newPeerNodeId - i);
    }

    // Test fabric limit (fabrics do not evict, so this should still fail)
    FabricIndex fabricCount = 1;
    do
    {
        fabricCount++;
        err = mGroupPeerMsgCounter.FindOrAddPeer(fabricCount, peerNodeId, false, counter);
    } while (err != CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(fabricCount, CHIP_CONFIG_MAX_FABRICS + 1);
}

TEST(TestGroupMessageCounter, RemovePeerTest)
{
    NodeId peerNodeId                             = 1234;
    FabricIndex fabricIndex                       = 1;
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable mGroupPeerMsgCounter;

    // Fill table up (max fabric and mac peer)
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; peerId++)
        {
            err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId++, true, counter);
        }
        fabricIndex++;
    }
    // Verify that table is indeed full (for control Peer)
    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    EXPECT_EQ(err, CHIP_ERROR_NO_MEMORY);

    // Clear all Peer
    fabricIndex = 1;
    peerNodeId  = 1234;
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; peerId++)
        {
            err = mGroupPeerMsgCounter.RemovePeer(fabricIndex, peerNodeId++, true);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }
        fabricIndex++;
    }

    // Try re-adding the previous peer without any error
    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter.FindOrAddPeer(104, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter.FindOrAddPeer(105, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter.FindOrAddPeer(106, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Fabric removal test
    err = mGroupPeerMsgCounter.FabricRemoved(123);
    EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

    err = mGroupPeerMsgCounter.FabricRemoved(99);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter.FabricRemoved(99);
    EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

    // Verify that the Fabric List was compacted.
    EXPECT_EQ(106, mGroupPeerMsgCounter.GetFabricIndexAt(0));
}

TEST(TestGroupMessageCounter, PeerRetrievalTest)
{
    NodeId peerNodeId                              = 1234;
    FabricIndex fabricIndex                        = 1;
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter  = nullptr;
    chip::Transport::PeerMessageCounter * counter2 = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter, nullptr);

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter2, nullptr);
    EXPECT_NE(counter2, counter);

    err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId, true, counter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(counter2, counter);
}

TEST(TestGroupMessageCounter, CounterCommitRolloverTest)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter, nullptr);

    err = counter->VerifyOrTrustFirstGroup(UINT32_MAX);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    counter->CommitGroup(UINT32_MAX);

    err = counter->VerifyOrTrustFirstGroup(0);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    counter->CommitGroup(0);

    err = counter->VerifyOrTrustFirstGroup(1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    counter->CommitGroup(1);
}

TEST(TestGroupMessageCounter, CounterTrustFirstTest)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter, nullptr);

    err = counter->VerifyOrTrustFirstGroup(5656);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    counter->CommitGroup(5656);

    err = counter->VerifyOrTrustFirstGroup(5756);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(5756);
    err = counter->VerifyOrTrustFirstGroup(4756);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // test sequential reception
    err = counter->VerifyOrTrustFirstGroup(5757);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(5757);

    err = counter->VerifyOrTrustFirstGroup(5758);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(5758);

    err = counter->VerifyOrTrustFirstGroup(5756);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Test Roll over
    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter, nullptr);

    err = counter->VerifyOrTrustFirstGroup(UINT32_MAX - 6);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(UINT32_MAX - 6);

    err = counter->VerifyOrTrustFirstGroup(UINT32_MAX - 1);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(UINT32_MAX - 1);

    err = counter->VerifyOrTrustFirstGroup(UINT32_MAX);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(0);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(1);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(3);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(4);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(5);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(6);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirstGroup(7);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST(TestGroupMessageCounter, ReorderPeerRemovalTest)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 2, true, counter);
    err = mGroupPeerMsgCounter.RemovePeer(1, 1, true);

    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(0, 0, true), 2u);

    // with other list, add 1 to 8
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 2, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 3, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 4, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 5, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 6, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 7, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 8, false, counter);

    // Initial order (MRU at 0): 8, 7, 6, 5, 4, 3, 2, 1

    // Remove node id 7 (index 1). Shift left.
    // New order: 8, 6, 5, 4, 3, 2, 1
    err = mGroupPeerMsgCounter.RemovePeer(2, 7, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 6, false), 1u);

    // Remove node id 4 (index 3). Shift left.
    // New order: 8, 6, 5, 3, 2, 1
    err = mGroupPeerMsgCounter.RemovePeer(2, 4, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 3, false), 3u);

    // Remove node id 1 (index 5). Shift left.
    // New order: 8, 6, 5, 3, 2
    err = mGroupPeerMsgCounter.RemovePeer(2, 1, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 0, false), 8u);

    // Add node id 9 after removals. Should be inserted at index 0.
    // New order: 9, 8, 6, 5, 3, 2
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 9, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify the entire list ordering
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 0, false), 9u);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 1, false), 8u);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 2, false), 6u);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 3, false), 5u);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 4, false), 3u);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 5, false), 2u);
    EXPECT_EQ(mGroupPeerMsgCounter.GetNodeIdAt(1, 6, false), kUndefinedNodeId);
}

TEST(TestGroupMessageCounter, ReorderFabricRemovalTest)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable mGroupPeerMsgCounter;

    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_FABRICS; i++)
    {
        err = mGroupPeerMsgCounter.FindOrAddPeer(static_cast<chip::FabricIndex>(i + 1), 1, false, counter);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Try removing last Fabric first
    err = counter->VerifyOrTrustFirstGroup(1234);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(1234);

    err = mGroupPeerMsgCounter.FabricRemoved(CHIP_CONFIG_MAX_FABRICS);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter.GetFabricIndexAt(CHIP_CONFIG_MAX_FABRICS - 1), kUndefinedFabricIndex);

    err = mGroupPeerMsgCounter.FindOrAddPeer(CHIP_CONFIG_MAX_FABRICS, 1, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify that the counter was indeed cleared
    err = counter->VerifyOrTrustFirstGroup(1234);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Set a counter that will be moved around
    err = counter->VerifyOrTrustFirstGroup(5656);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(5656);

    err = counter->VerifyOrTrustFirstGroup(4756);
    EXPECT_NE(err, CHIP_NO_ERROR);

    // Per Spec CHIP_CONFIG_MAX_FABRICS can only be as low as 4
    err = mGroupPeerMsgCounter.RemovePeer(3, 1, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter.GetFabricIndexAt(2), CHIP_CONFIG_MAX_FABRICS);
    err = mGroupPeerMsgCounter.RemovePeer(2, 1, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter.GetFabricIndexAt(1), CHIP_CONFIG_MAX_FABRICS - 1);

    // Validate that counter value were moved around correctly
    err = mGroupPeerMsgCounter.FindOrAddPeer(CHIP_CONFIG_MAX_FABRICS, 1, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = counter->VerifyOrTrustFirstGroup(4756);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(TestGroupMessageCounter, MultipleEvictionsTest)
{
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable table;
    FabricIndex fabric = 1;

    // --- Data peer path: fill table then cycle 100 more unique NodeIDs ---
    // Fill the table to capacity
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        NodeId nodeId = 0x1000 + i;
        EXPECT_EQ(table.FindOrAddPeer(fabric, nodeId, false, counter), CHIP_NO_ERROR);
    }

    // Cycle 100 more unique NodeIDs through the table; each should become MRU
    for (uint32_t i = 0; i < 100; i++)
    {
        NodeId nodeId = 0x2000 + i;
        EXPECT_EQ(table.FindOrAddPeer(fabric, nodeId, false, counter), CHIP_NO_ERROR);

        // Verify the most recently added peer is always at index 0
        EXPECT_EQ(table.GetNodeIdAt(0, 0, false), nodeId) << "iteration " << i;
    }

    // --- Control peer path: fill table then cycle a few more unique NodeIDs ---
    // The control peer path is currently not tested for eviction.
    FabricIndex fabric2 = 2;
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; i++)
    {
        NodeId nodeId = 0x3000 + i;
        EXPECT_EQ(table.FindOrAddPeer(fabric2, nodeId, true, counter), CHIP_NO_ERROR);
    }

    // Cycle more unique NodeIDs through the control peer table
    for (uint32_t i = 0; i < 10; i++)
    {
        NodeId nodeId = 0x4000 + i;
        EXPECT_EQ(table.FindOrAddPeer(fabric2, nodeId, true, counter), CHIP_NO_ERROR);

        // Verify the most recently added peer is always at index 0
        EXPECT_EQ(table.GetNodeIdAt(1, 0, true), nodeId) << "control iteration " << i;
    }
}

TEST(TestGroupMessageCounter, GroupMessageCounterTest)
{

    chip::TestPersistentStorageDelegate delegate;
    TestGroupOutgoingCounters groupCientCounter;
    uint32_t controlCounter = 0, dataCounter = 0;
    CHIP_ERROR err = groupCientCounter.Init(&delegate);

    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Start Test with Control counter
    // Counter should be random
    controlCounter = groupCientCounter.GetCounter(true);
    dataCounter    = groupCientCounter.GetCounter(false);
    EXPECT_SUCCESS(groupCientCounter.IncrementCounter(true));

    EXPECT_EQ((groupCientCounter.GetCounter(true) - controlCounter), 1u);

    groupCientCounter.SetCounter(true, UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);
    EXPECT_EQ(groupCientCounter.GetCounter(true), UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Persistence
    TestGroupOutgoingCounters groupCientCounter2(&delegate);

    EXPECT_EQ(groupCientCounter2.GetCounter(true), UINT32_MAX);
    EXPECT_EQ((groupCientCounter2.GetCounter(false) - dataCounter), (uint32_t) GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Roll over
    EXPECT_SUCCESS(groupCientCounter2.IncrementCounter(true));
    EXPECT_EQ(groupCientCounter2.GetCounter(true), 0u);

    TestGroupOutgoingCounters groupCientCounter3(&delegate);
    EXPECT_EQ(groupCientCounter3.GetCounter(true), (UINT32_MAX + GROUP_MSG_COUNTER_MIN_INCREMENT));

    // Redo the test with the second counter

    // Start Test with Control counter
    dataCounter = groupCientCounter.GetCounter(false);
    EXPECT_SUCCESS(groupCientCounter.IncrementCounter(false));
    EXPECT_EQ((groupCientCounter.GetCounter(false) - dataCounter), 1u);

    groupCientCounter.SetCounter(false, UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);
    EXPECT_EQ(groupCientCounter.GetCounter(false), UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Persistence
    TestGroupOutgoingCounters groupCientCounter4(&delegate);

    EXPECT_EQ(groupCientCounter4.GetCounter(false), UINT32_MAX);

    // Test Roll over
    EXPECT_SUCCESS(groupCientCounter4.IncrementCounter(false));
    EXPECT_EQ(groupCientCounter4.GetCounter(false), 0u);

    TestGroupOutgoingCounters groupCientCounter5(&delegate);
    EXPECT_EQ(groupCientCounter5.GetCounter(false), (UINT32_MAX + GROUP_MSG_COUNTER_MIN_INCREMENT));
}

} // namespace
