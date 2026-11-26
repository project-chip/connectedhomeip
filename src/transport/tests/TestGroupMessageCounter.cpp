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
#include <memory>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/PeerMessageCounter.h>

namespace {

using namespace chip;

constexpr KeysetId kDefaultKeysetId = 1;

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
    KeysetId GetKeysetIdAt(uint8_t fabricIndex, uint8_t index, bool isControl)
    {
        if (fabricIndex < CHIP_CONFIG_MAX_FABRICS)
        {
            if (isControl && index < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS)
            {
                return mGroupFabrics[fabricIndex].mControlGroupSenders[index].mKeysetId;
            }

            if (!isControl && index < CHIP_CONFIG_MAX_GROUP_DATA_PEERS)
            {
                return mGroupFabrics[fabricIndex].mDataGroupSenders[index].mKeysetId;
            }
        }

        return kInvalidKeysetId;
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
    auto mGroupPeerMsgCounter                     = std::make_unique<TestGroupPeerTable>();

    // Add max number of group data peers
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        err = mGroupPeerMsgCounter->FindOrAddPeer(fabricIndex, kDefaultKeysetId, peerNodeId + i, false, counter);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Verify they are in the table in the correct LRU order (most recent at index 0)
    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(0, i, false), peerNodeId + (CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1 - i));
    }

    // Add (CHIP_CONFIG_MAX_GROUP_DATA_PEERS + 1)th peer (should trigger eviction of 1234,
    // which was at index CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1)
    NodeId newPeerNodeId = peerNodeId + CHIP_CONFIG_MAX_GROUP_DATA_PEERS;
    err                  = mGroupPeerMsgCounter->FindOrAddPeer(fabricIndex, kDefaultKeysetId, newPeerNodeId, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify the entire list ordering after eviction (newPeerNodeId at index 0, down to
    // 1235 at index CHIP_CONFIG_MAX_GROUP_DATA_PEERS - 1)
    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(0, i, false), newPeerNodeId - i);
    }

    // Test fabric limit (fabrics do not evict, so this should still fail)
    FabricIndex fabricCount = 1;
    do
    {
        fabricCount++;
        err = mGroupPeerMsgCounter->FindOrAddPeer(fabricCount, kDefaultKeysetId, peerNodeId, false, counter);
    } while (err != CHIP_ERROR_NO_MEMORY);
    EXPECT_EQ(fabricCount, CHIP_CONFIG_MAX_FABRICS + 1);
}

TEST(TestGroupMessageCounter, RemovePeerTest)
{
    NodeId peerNodeId                             = 1234;
    FabricIndex fabricIndex                       = 1;
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    auto mGroupPeerMsgCounter                     = std::make_unique<TestGroupPeerTable>();

    // Fill table up (max fabric and mac peer)
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; peerId++)
        {
            err = mGroupPeerMsgCounter->FindOrAddPeer(fabricIndex, kDefaultKeysetId, peerNodeId++, true, counter);
        }
        fabricIndex++;
    }
    // Verify that table is indeed full (for control Peer)
    err = mGroupPeerMsgCounter->FindOrAddPeer(99, kDefaultKeysetId, 99, true, counter);
    EXPECT_EQ(err, CHIP_ERROR_NO_MEMORY);

    // Clear all Peer
    fabricIndex = 1;
    peerNodeId  = 1234;
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; peerId++)
        {
            err = mGroupPeerMsgCounter->RemovePeer(fabricIndex, kDefaultKeysetId, peerNodeId++, true);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }
        fabricIndex++;
    }

    // Try re-adding the previous peer without any error
    err = mGroupPeerMsgCounter->FindOrAddPeer(99, kDefaultKeysetId, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter->FindOrAddPeer(104, kDefaultKeysetId, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter->FindOrAddPeer(105, kDefaultKeysetId, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter->FindOrAddPeer(106, kDefaultKeysetId, 99, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Fabric removal test
    err = mGroupPeerMsgCounter->FabricRemoved(123);
    EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

    err = mGroupPeerMsgCounter->FabricRemoved(99);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter->FabricRemoved(99);
    EXPECT_EQ(err, CHIP_ERROR_NOT_FOUND);

    // Verify that the Fabric List was compacted.
    EXPECT_EQ(106, mGroupPeerMsgCounter->GetFabricIndexAt(0));
}

TEST(TestGroupMessageCounter, PeerRetrievalTest)
{
    NodeId peerNodeId                              = 1234;
    FabricIndex fabricIndex                        = 1;
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter  = nullptr;
    chip::Transport::PeerMessageCounter * counter2 = nullptr;
    auto mGroupPeerMsgCounter                      = std::make_unique<chip::Transport::GroupPeerTable>();

    err = mGroupPeerMsgCounter->FindOrAddPeer(fabricIndex, kDefaultKeysetId, peerNodeId, true, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter, nullptr);

    err = mGroupPeerMsgCounter->FindOrAddPeer(99, kDefaultKeysetId, 99, true, counter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_NE(counter2, nullptr);
    EXPECT_NE(counter2, counter);

    err = mGroupPeerMsgCounter->FindOrAddPeer(fabricIndex, kDefaultKeysetId, peerNodeId, true, counter2);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(counter2, counter);
}

TEST(TestGroupMessageCounter, CounterCommitRolloverTest)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    auto mGroupPeerMsgCounter                     = std::make_unique<chip::Transport::GroupPeerTable>();

    err = mGroupPeerMsgCounter->FindOrAddPeer(99, kDefaultKeysetId, 99, true, counter);
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
    auto mGroupPeerMsgCounter                     = std::make_unique<chip::Transport::GroupPeerTable>();

    err = mGroupPeerMsgCounter->FindOrAddPeer(99, kDefaultKeysetId, 99, true, counter);
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
    err = mGroupPeerMsgCounter->FindOrAddPeer(1, kDefaultKeysetId, 99, true, counter);
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
    auto mGroupPeerMsgCounter                     = std::make_unique<TestGroupPeerTable>();

    err = mGroupPeerMsgCounter->FindOrAddPeer(1, kDefaultKeysetId, 1, true, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(1, kDefaultKeysetId, 2, true, counter);
    err = mGroupPeerMsgCounter->RemovePeer(1, kDefaultKeysetId, 1, true);

    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(0, 0, true), 2u);

    // with other list, add 1 to 8
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 1, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 2, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 3, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 4, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 5, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 6, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 7, false, counter);
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 8, false, counter);

    // Initial order (MRU at 0): 8, 7, 6, 5, 4, 3, 2, 1

    // Remove node id 7 (index 1). Shift left.
    // New order: 8, 6, 5, 4, 3, 2, 1
    err = mGroupPeerMsgCounter->RemovePeer(2, kDefaultKeysetId, 7, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 6, false), 1u);

    // Remove node id 4 (index 3). Shift left.
    // New order: 8, 6, 5, 3, 2, 1
    err = mGroupPeerMsgCounter->RemovePeer(2, kDefaultKeysetId, 4, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 3, false), 3u);

    // Remove node id 1 (index 5). Shift left.
    // New order: 8, 6, 5, 3, 2
    err = mGroupPeerMsgCounter->RemovePeer(2, kDefaultKeysetId, 1, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 0, false), 8u);

    // Add node id 9 after removals. Should be inserted at index 0.
    // New order: 9, 8, 6, 5, 3, 2
    err = mGroupPeerMsgCounter->FindOrAddPeer(2, kDefaultKeysetId, 9, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Verify the entire list ordering
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 0, false), 9u);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 1, false), 8u);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 2, false), 6u);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 3, false), 5u);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 4, false), 3u);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 5, false), 2u);
    EXPECT_EQ(mGroupPeerMsgCounter->GetNodeIdAt(1, 6, false), kUndefinedNodeId);
}

TEST(TestGroupMessageCounter, ReorderFabricRemovalTest)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    auto mGroupPeerMsgCounter                     = std::make_unique<TestGroupPeerTable>();

    for (uint8_t i = 0; i < CHIP_CONFIG_MAX_FABRICS; i++)
    {
        err = mGroupPeerMsgCounter->FindOrAddPeer(static_cast<chip::FabricIndex>(i + 1), kDefaultKeysetId, 1, false, counter);
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }

    // Try removing last Fabric first
    err = counter->VerifyOrTrustFirstGroup(1234);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    counter->CommitGroup(1234);

    err = mGroupPeerMsgCounter->FabricRemoved(CHIP_CONFIG_MAX_FABRICS);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter->GetFabricIndexAt(CHIP_CONFIG_MAX_FABRICS - 1), kUndefinedFabricIndex);

    err = mGroupPeerMsgCounter->FindOrAddPeer(CHIP_CONFIG_MAX_FABRICS, kDefaultKeysetId, 1, false, counter);
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
    err = mGroupPeerMsgCounter->RemovePeer(3, kDefaultKeysetId, 1, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter->GetFabricIndexAt(2), CHIP_CONFIG_MAX_FABRICS);
    err = mGroupPeerMsgCounter->RemovePeer(2, kDefaultKeysetId, 1, false);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(mGroupPeerMsgCounter->GetFabricIndexAt(1), CHIP_CONFIG_MAX_FABRICS - 1);

    // Validate that counter value were moved around correctly
    err = mGroupPeerMsgCounter->FindOrAddPeer(CHIP_CONFIG_MAX_FABRICS, kDefaultKeysetId, 1, false, counter);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = counter->VerifyOrTrustFirstGroup(4756);
    EXPECT_NE(err, CHIP_NO_ERROR);
}

TEST(TestGroupMessageCounter, MultipleEvictionsTest)
{
    chip::Transport::PeerMessageCounter * counter = nullptr;
    auto table                                    = std::make_unique<TestGroupPeerTable>();
    FabricIndex fabric                            = 1;

    // --- Data peer path: fill table then cycle 100 more unique NodeIDs ---
    // Fill the table to capacity
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; i++)
    {
        NodeId nodeId = 0x1000 + i;
        EXPECT_EQ(table->FindOrAddPeer(fabric, kDefaultKeysetId, nodeId, false, counter), CHIP_NO_ERROR);
    }

    // Cycle 100 more unique NodeIDs through the table; each should become MRU
    for (uint32_t i = 0; i < 100; i++)
    {
        NodeId nodeId = 0x2000 + i;
        EXPECT_EQ(table->FindOrAddPeer(fabric, kDefaultKeysetId, nodeId, false, counter), CHIP_NO_ERROR);

        // Verify the most recently added peer is always at index 0
        EXPECT_EQ(table->GetNodeIdAt(0, 0, false), nodeId) << "iteration " << i;
    }

    // --- Control peer path: fill table then cycle a few more unique NodeIDs ---
    // The control peer path is currently not tested for eviction.
    FabricIndex fabric2 = 2;
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; i++)
    {
        NodeId nodeId = 0x3000 + i;
        EXPECT_EQ(table->FindOrAddPeer(fabric2, kDefaultKeysetId, nodeId, true, counter), CHIP_NO_ERROR);
    }

    // Cycle more unique NodeIDs through the control peer table
    for (uint32_t i = 0; i < 10; i++)
    {
        NodeId nodeId = 0x4000 + i;
        EXPECT_EQ(table->FindOrAddPeer(fabric2, kDefaultKeysetId, nodeId, true, counter), CHIP_NO_ERROR);

        // Verify the most recently added peer is always at index 0
        EXPECT_EQ(table->GetNodeIdAt(1, 0, true), nodeId) << "control iteration " << i;
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

TEST(TestGroupMessageCounter, KeySetIsolationAndRemovalTest)
{
    auto table                                     = std::make_unique<TestGroupPeerTable>();
    chip::Transport::PeerMessageCounter * counterA = nullptr;
    chip::Transport::PeerMessageCounter * counterB = nullptr;

    constexpr FabricIndex kFabric  = 1;
    constexpr KeysetId kKeySetA    = 535;
    constexpr KeysetId kKeySetB    = 540;
    constexpr KeysetId kKeySetC    = 545;
    constexpr NodeId kSenderNodeId = 0x00A00001;

    // Invalid keysetId is rejected across all entry points.
    EXPECT_EQ(table->FindOrAddPeer(kFabric, kInvalidKeysetId, kSenderNodeId, false, counterA), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(table->RemovePeer(kFabric, kInvalidKeysetId, kSenderNodeId, false), CHIP_ERROR_INVALID_ARGUMENT);
    EXPECT_EQ(table->KeySetRemoved(kFabric, kInvalidKeysetId), CHIP_ERROR_INVALID_ARGUMENT);

    // Sender establishes counter 0x0201B0BF under KeySet A.
    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSenderNodeId, false, counterA));
    ASSERT_NE(counterA, nullptr);
    ASSERT_SUCCESS(counterA->VerifyOrTrustFirstGroup(0x0201B0BFu));
    counterA->CommitGroup(0x0201B0BFu);

    // Same sender NodeId under KeySet B gets an independent entry; a large forward jump
    // under KeySet B must not advance KeySet A's counter window.
    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetB, kSenderNodeId, false, counterB));
    ASSERT_NE(counterB, nullptr);
    EXPECT_EQ(table->GetKeysetIdAt(0, 0, false), kKeySetB);
    EXPECT_EQ(table->GetKeysetIdAt(0, 1, false), kKeySetA);
    ASSERT_SUCCESS(counterB->VerifyOrTrustFirstGroup(0x4201BC77u));
    counterB->CommitGroup(0x4201BC77u);

    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSenderNodeId, false, counterA));
    EXPECT_SUCCESS(counterA->VerifyOrTrustFirstGroup(0x0201C82Fu));
    counterA->CommitGroup(0x0201C82Fu);

    // Fair-share eviction: add peers up to floor(CHIP_CONFIG_MAX_GROUP_DATA_PEERS / 2) under KeySet A,
    // then flood KeySet B past capacity. All of KeySet A's entries must remain protected.
    constexpr uint32_t kTwoKeySetFairShare = CHIP_CONFIG_MAX_GROUP_DATA_PEERS / 2;
    for (uint32_t i = 1; i < kTwoKeySetFairShare; i++)
    {
        chip::Transport::PeerMessageCounter * peerCounter = nullptr;
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSenderNodeId + i, false, peerCounter));
        ASSERT_SUCCESS(peerCounter->VerifyOrTrustFirstGroup(1000u + i));
        peerCounter->CommitGroup(1000u + i);
    }

    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS + 5; i++)
    {
        chip::Transport::PeerMessageCounter * churnCounter = nullptr;
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetB, 0x2000 + i, false, churnCounter)) << "churn " << i;
    }

    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSenderNodeId, false, counterA));
    EXPECT_EQ(counterA->VerifyOrTrustFirstGroup(0x0201C82Fu), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
    for (uint32_t i = 1; i < kTwoKeySetFairShare; i++)
    {
        chip::Transport::PeerMessageCounter * peerCounter = nullptr;
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSenderNodeId + i, false, peerCounter));
        EXPECT_EQ(peerCounter->VerifyOrTrustFirstGroup(1000u + i), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED) << "peer " << i;
    }

    // KeySetRemoved removes all KeySet B entries while preserving KeySet A's replay state.
    EXPECT_SUCCESS(table->KeySetRemoved(kFabric, kKeySetB));
    EXPECT_EQ(table->KeySetRemoved(kFabric, kKeySetB), CHIP_ERROR_NOT_FOUND);
    for (uint8_t idx = 0; idx < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; idx++)
    {
        EXPECT_NE(table->GetKeysetIdAt(0, idx, false), kKeySetB);
    }

    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSenderNodeId, false, counterA));
    EXPECT_EQ(counterA->VerifyOrTrustFirstGroup(0x0201C82Fu), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

    // Three-keyset fair share: a third keyset acquires slots from the largest holder without starving KeySet A.
    constexpr uint32_t kThreeKeySetFairShare = CHIP_CONFIG_MAX_GROUP_DATA_PEERS / 3;
    for (uint32_t i = 0; i < kThreeKeySetFairShare; i++)
    {
        chip::Transport::PeerMessageCounter * peerCounter = nullptr;
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetC, 0x3000 + i, false, peerCounter));
        ASSERT_SUCCESS(peerCounter->VerifyOrTrustFirstGroup(2000u + i));
        peerCounter->CommitGroup(2000u + i);
    }
    for (uint32_t i = 0; i < CHIP_CONFIG_MAX_GROUP_DATA_PEERS + 5; i++)
    {
        chip::Transport::PeerMessageCounter * churnCounter = nullptr;
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetB, 0x4000 + i, false, churnCounter));
    }
    uint32_t keySetACount = 0;
    for (uint8_t idx = 0; idx < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; idx++)
    {
        if (table->GetKeysetIdAt(0, idx, false) == kKeySetA)
        {
            keySetACount++;
        }
    }
    EXPECT_GE(keySetACount, kThreeKeySetFairShare);

    for (uint32_t i = 0; i < kThreeKeySetFairShare; i++)
    {
        chip::Transport::PeerMessageCounter * peerCounter = nullptr;
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetC, 0x3000 + i, false, peerCounter));
        EXPECT_EQ(peerCounter->VerifyOrTrustFirstGroup(2000u + i), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED) << "keyset C peer " << i;
    }

    // KeySetRemoved clears both data and control peer lists for the removed keyset.
    chip::Transport::PeerMessageCounter * controlCounter = nullptr;
    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetC, 0x3000, true, controlCounter));
    EXPECT_SUCCESS(table->KeySetRemoved(kFabric, kKeySetC));
    EXPECT_EQ(table->GetKeysetIdAt(0, 0, true), kInvalidKeysetId);
    for (uint8_t idx = 0; idx < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; idx++)
    {
        EXPECT_NE(table->GetKeysetIdAt(0, idx, false), kKeySetC);
    }

    // Removing the remaining keysets compacts the emptied fabric entry.
    EXPECT_SUCCESS(table->KeySetRemoved(kFabric, kKeySetB));
    EXPECT_SUCCESS(table->KeySetRemoved(kFabric, kKeySetA));
    EXPECT_EQ(table->GetFabricIndexAt(0), kUndefinedFabricIndex);
}

TEST(TestGroupMessageCounter, FairShareEvictionTieBreakTest)
{
    static_assert(CHIP_CONFIG_MAX_GROUP_DATA_PEERS >= 4, "Test requires at least 4 group data peer slots");

    auto table                                    = std::make_unique<TestGroupPeerTable>();
    chip::Transport::PeerMessageCounter * counter = nullptr;

    constexpr FabricIndex kFabric     = 1;
    constexpr KeysetId kKeySetFiller  = 100;
    constexpr KeysetId kKeySetA       = 200;
    constexpr KeysetId kKeySetB       = 300;
    constexpr KeysetId kKeySetC       = 400;
    constexpr uint32_t kFillerCount   = CHIP_CONFIG_MAX_GROUP_DATA_PEERS % 2;
    constexpr uint32_t kTiedSlotCount = CHIP_CONFIG_MAX_GROUP_DATA_PEERS / 2;

    if (kFillerCount == 1)
    {
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetFiller, 0x9000, false, counter));
    }

    // Fill KeySet A then KeySet B with equal counts so KeySet A holds the older LRU index.
    for (uint32_t i = 0; i < kTiedSlotCount; i++)
    {
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, 0x1000 + i, false, counter));
    }
    for (uint32_t i = 0; i < kTiedSlotCount; i++)
    {
        ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetB, 0x2000 + i, false, counter));
    }

    // Incoming KeySet C is not among the tied max-holders {A, B}; eviction picks the
    // globally oldest entry among the tied max-holders (KeySet A's 0x1000), leaving
    // KeySet B and the non-tied filler entry intact.
    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetC, 0x3000, false, counter));

    EXPECT_EQ(table->GetKeysetIdAt(0, 0, false), kKeySetC);
    EXPECT_EQ(table->GetNodeIdAt(0, 0, false), 0x3000u);

    uint32_t countA      = 0;
    uint32_t countB      = 0;
    uint32_t countFiller = 0;
    for (uint8_t idx = 0; idx < CHIP_CONFIG_MAX_GROUP_DATA_PEERS; idx++)
    {
        const KeysetId ks = table->GetKeysetIdAt(0, idx, false);
        const NodeId node = table->GetNodeIdAt(0, idx, false);
        if (ks == kKeySetA)
        {
            EXPECT_NE(node, 0x1000u);
            countA++;
        }
        else if (ks == kKeySetB)
        {
            countB++;
        }
        else if (ks == kKeySetFiller)
        {
            countFiller++;
        }
    }
    EXPECT_EQ(countA, kTiedSlotCount - 1);
    EXPECT_EQ(countB, kTiedSlotCount);
    EXPECT_EQ(countFiller, kFillerCount);
}

TEST(TestGroupMessageCounter, RemovePeerIsolatesKeySetTest)
{
    auto table                                    = std::make_unique<TestGroupPeerTable>();
    chip::Transport::PeerMessageCounter * counter = nullptr;

    constexpr FabricIndex kFabric = 1;
    constexpr KeysetId kKeySetA   = 10;
    constexpr KeysetId kKeySetB   = 20;
    constexpr NodeId kSharedPeer  = 0x1001;

    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetA, kSharedPeer, false, counter));
    ASSERT_SUCCESS(counter->VerifyOrTrustFirstGroup(400u));
    counter->CommitGroup(400u);

    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetB, kSharedPeer, false, counter));
    ASSERT_SUCCESS(counter->VerifyOrTrustFirstGroup(500u));
    counter->CommitGroup(500u);

    // Removing (KeySet A, kSharedPeer) leaves (KeySet B, kSharedPeer) and its replay state intact.
    EXPECT_SUCCESS(table->RemovePeer(kFabric, kKeySetA, kSharedPeer, false));
    EXPECT_EQ(table->GetFabricIndexAt(0), kFabric);
    EXPECT_EQ(table->GetKeysetIdAt(0, 0, false), kKeySetB);
    EXPECT_EQ(table->GetNodeIdAt(0, 0, false), kSharedPeer);
    EXPECT_EQ(table->GetNodeIdAt(0, 1, false), kUndefinedNodeId);

    ASSERT_SUCCESS(table->FindOrAddPeer(kFabric, kKeySetB, kSharedPeer, false, counter));
    EXPECT_EQ(counter->VerifyOrTrustFirstGroup(500u), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

    // Removing the last peer compacts the fabric entry too.
    EXPECT_SUCCESS(table->RemovePeer(kFabric, kKeySetB, kSharedPeer, false));
    EXPECT_EQ(table->GetFabricIndexAt(0), kUndefinedFabricIndex);
}

} // namespace
