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

#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/PeerMessageCounter.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

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

        DefaultStorageKeyAllocator key;

        if (isControl)
        {
            mGroupControlCounter = value;
            key.GroupControlCounter();
        }
        else
        {
            mGroupDataCounter = value;
            key.GroupDataCounter();
        }

        if (mStorage == nullptr)
        {
            return;
        }

        // Always Update storage for Test purposes
        temp = value + GROUP_MSG_COUNTER_MIN_INCREMENT;
        mStorage->SyncSetKeyValue(key.KeyName(), &temp, sizeof(uint32_t));
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

void AddPeerTest(nlTestSuite * inSuite, void * inContext)
{
    NodeId peerNodeId                             = 1234;
    FabricIndex fabricIndex                       = 1;
    uint32_t i                                    = 0;
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    do
    {
        err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId++, false, counter);
        i++;

    } while (err != CHIP_ERROR_TOO_MANY_PEER_NODES);

    NL_TEST_ASSERT(inSuite, i == CHIP_CONFIG_MAX_GROUP_DATA_PEERS + 1);

    i = 1;
    do
    {
        err = mGroupPeerMsgCounter.FindOrAddPeer(++fabricIndex, peerNodeId, false, counter);
        i++;
    } while (err != CHIP_ERROR_TOO_MANY_PEER_NODES);
    NL_TEST_ASSERT(inSuite, i == CHIP_CONFIG_MAX_FABRICS + 1);
}

void RemovePeerTest(nlTestSuite * inSuite, void * inContext)
{
    NodeId peerNodeId                             = 1234;
    FabricIndex fabricIndex                       = 1;
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

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
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_TOO_MANY_PEER_NODES);

    // Clear all Peer
    fabricIndex = 1;
    peerNodeId  = 1234;
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < CHIP_CONFIG_MAX_GROUP_CONTROL_PEERS; peerId++)
        {
            err = mGroupPeerMsgCounter.RemovePeer(fabricIndex, peerNodeId++, true);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }
        fabricIndex++;
    }

    // Try re-adding the previous peer without any error
    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void PeerRetrievalTest(nlTestSuite * inSuite, void * inContext)
{
    NodeId peerNodeId                              = 1234;
    FabricIndex fabricIndex                        = 1;
    CHIP_ERROR err                                 = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter  = nullptr;
    chip::Transport::PeerMessageCounter * counter2 = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter2 != nullptr);
    NL_TEST_ASSERT(inSuite, counter2 != counter);

    err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId, true, counter2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter2 == counter);
}

void CounterCommitRolloverTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = counter->VerifyOrTrustFirst(UINT32_MAX, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    counter->CommitWithRollOver(UINT32_MAX);

    err = counter->VerifyOrTrustFirst(0, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    counter->CommitWithRollOver(0);

    err = counter->VerifyOrTrustFirst(1, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    counter->CommitWithRollOver(1);
}

void CounterTrustFirstTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = counter->VerifyOrTrustFirst(5656, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    counter->CommitWithRollOver(5656);

    err = counter->VerifyOrTrustFirst(5756, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->CommitWithRollOver(5756);
    err = counter->VerifyOrTrustFirst(4756, true);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // test sequential reception
    err = counter->VerifyOrTrustFirst(5757, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->CommitWithRollOver(5757);

    err = counter->VerifyOrTrustFirst(5758, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->CommitWithRollOver(5758);

    err = counter->VerifyOrTrustFirst(5756, true);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // Test Roll over
    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = counter->VerifyOrTrustFirst(UINT32_MAX - 6, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->CommitWithRollOver(UINT32_MAX - 6);

    err = counter->VerifyOrTrustFirst(UINT32_MAX - 1, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->CommitWithRollOver(UINT32_MAX - 1);

    err = counter->VerifyOrTrustFirst(UINT32_MAX, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(0, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(1, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(2, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(3, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(4, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(5, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(6, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = counter->VerifyOrTrustFirst(7, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void ReorderPeerRemovalTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 2, true, counter);
    err = mGroupPeerMsgCounter.RemovePeer(1, 1, true);

    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetNodeIdAt(0, 0, true) == 2);

    // with other list
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 2, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 3, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 4, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 5, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 6, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 7, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 8, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 9, false, counter);

    err = mGroupPeerMsgCounter.RemovePeer(2, 7, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetNodeIdAt(1, 6, false) == 9);

    err = mGroupPeerMsgCounter.RemovePeer(2, 4, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetNodeIdAt(1, 3, false) == 8);

    err = mGroupPeerMsgCounter.RemovePeer(2, 1, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetNodeIdAt(1, 0, false) == 9);
}

#if !__ZEPHYR__

void ReorderFabricRemovalTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    TestGroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(3, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(4, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(5, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(6, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(7, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(8, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(9, 1, false, counter);

    err = counter->VerifyOrTrustFirst(5656, true);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->CommitWithRollOver(5656);

    err = counter->VerifyOrTrustFirst(4756, true);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter.FindOrAddPeer(10, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(11, 1, false, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(12, 1, false, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = mGroupPeerMsgCounter.RemovePeer(3, 1, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(2) == 12);
    err = mGroupPeerMsgCounter.RemovePeer(8, 1, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(7) == 11);
    err = mGroupPeerMsgCounter.RemovePeer(11, 1, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(7) == 10);
    err = mGroupPeerMsgCounter.RemovePeer(1, 1, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(0) == 9);
    err = mGroupPeerMsgCounter.RemovePeer(10, 1, false);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(7) == 0);

    // Validate that counter value were moved around correctly
    err = mGroupPeerMsgCounter.FindOrAddPeer(9, 1, false, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = counter->VerifyOrTrustFirst(4756, true);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}
#endif // !__ZEPHYR__
void GroupMessageCounterTest(nlTestSuite * inSuite, void * inContext)
{

    chip::TestPersistentStorageDelegate delegate;
    TestGroupOutgoingCounters groupCientCounter;
    CHIP_ERROR err = groupCientCounter.Init(&delegate);

    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Start Test with Control counter
    NL_TEST_ASSERT(inSuite, groupCientCounter.GetCounter(true) == 0);
    groupCientCounter.IncrementCounter(true);

    NL_TEST_ASSERT(inSuite, groupCientCounter.GetCounter(true) == 1);

    groupCientCounter.SetCounter(true, UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);
    NL_TEST_ASSERT(inSuite, groupCientCounter.GetCounter(true) == UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Persistence
    TestGroupOutgoingCounters groupCientCounter2(&delegate);

    NL_TEST_ASSERT(inSuite, groupCientCounter2.GetCounter(true) == UINT32_MAX);
    NL_TEST_ASSERT(inSuite, groupCientCounter2.GetCounter(false) == GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Roll over
    groupCientCounter2.IncrementCounter(true);
    NL_TEST_ASSERT(inSuite, groupCientCounter2.GetCounter(true) == 0);

    TestGroupOutgoingCounters groupCientCounter3(&delegate);
    NL_TEST_ASSERT(inSuite, groupCientCounter3.GetCounter(true) == (UINT32_MAX + GROUP_MSG_COUNTER_MIN_INCREMENT));

    // Redo the test with the second counter

    // Start Test with Control counter
    NL_TEST_ASSERT(inSuite, groupCientCounter.GetCounter(false) == 0);
    groupCientCounter.IncrementCounter(false);
    NL_TEST_ASSERT(inSuite, groupCientCounter.GetCounter(false) == 1);

    groupCientCounter.SetCounter(false, UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);
    NL_TEST_ASSERT(inSuite, groupCientCounter.GetCounter(false) == UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Persistence
    TestGroupOutgoingCounters groupCientCounter4(&delegate);

    NL_TEST_ASSERT(inSuite, groupCientCounter4.GetCounter(false) == UINT32_MAX);

    // Test Roll over
    groupCientCounter4.IncrementCounter(false);
    NL_TEST_ASSERT(inSuite, groupCientCounter4.GetCounter(false) == 0);

    TestGroupOutgoingCounters groupCientCounter5(&delegate);
    NL_TEST_ASSERT(inSuite, groupCientCounter5.GetCounter(false) == (UINT32_MAX + GROUP_MSG_COUNTER_MIN_INCREMENT));
}

} // namespace

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Add Peer",               AddPeerTest),
    NL_TEST_DEF("Remove Peer",            RemovePeerTest),
    NL_TEST_DEF("Peer retrieval",         PeerRetrievalTest),
    NL_TEST_DEF("Counter Rollover",       CounterCommitRolloverTest),
    NL_TEST_DEF("Counter Trust first",    CounterTrustFirstTest),
    NL_TEST_DEF("Reorder Peer removal",   ReorderPeerRemovalTest),
    #if !__ZEPHYR__
    NL_TEST_DEF("Reorder Fabric Removal", ReorderFabricRemovalTest),
    #endif
    NL_TEST_DEF("Group Message Counter",  GroupMessageCounterTest),
    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Main
 */
int TestGroupMessageCounter()
{
    // Run test suit against one context

    nlTestSuite theSuite = { "Transport-TestGroupMessageCounter", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestGroupMessageCounter);
