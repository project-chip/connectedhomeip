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

#include <lib/support/TestPersistentStorageDelegate.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/PeerMessageCounter.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

namespace {

using namespace chip;

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

    NL_TEST_ASSERT(inSuite, i == GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_DATA_PEER + 1);

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
    FabricIndex fabricIndex                             = 1;
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    // Fill table up (max fabric and mac peer)
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_CONTROL_PEER; peerId++)
        {
            err = mGroupPeerMsgCounter.FindOrAddPeer(fabricIndex, peerNodeId++, true, counter);
        }
        fabricIndex++;
    }
    // Verify that table is indeed full (for control Peer)
    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_ERROR_TOO_MANY_PEER_NODES);

    // Clear all Peer
    fabricIndex   = 1;
    peerNodeId = 1234;
    for (uint32_t it = 0; it < CHIP_CONFIG_MAX_FABRICS; it++)
    {
        for (uint32_t peerId = 0; peerId < GROUP_MSG_COUNTER_MAX_NUMBER_OF_GROUP_CONTROL_PEER; peerId++)
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
    FabricIndex fabricIndex                              = 1;
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

void CounterTrustFirstTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(99, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = counter->VerifyOrTrustFirst(5656);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->Commit(5656);

    err = counter->VerifyOrTrustFirst(5756);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->Commit(5756);

    err = counter->VerifyOrTrustFirst(4756);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // test sequential reception
    err = counter->VerifyOrTrustFirst(5757);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->Commit(5757);

    err = counter->VerifyOrTrustFirst(5758);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->Commit(5758);

    err = counter->VerifyOrTrustFirst(5756);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // Test Roll over
    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 99, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = counter->VerifyOrTrustFirst(UINT32_MAX - 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->Commit(UINT32_MAX - 1);

    err = counter->VerifyOrTrustFirst(5);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void ReorderFabricRemovalTest(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err                                = CHIP_NO_ERROR;
    chip::Transport::PeerMessageCounter * counter = nullptr;
    chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

    err = mGroupPeerMsgCounter.FindOrAddPeer(1, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(2, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(3, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(4, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(5, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(6, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(7, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(8, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(9, 1, true, counter);

    err = counter->VerifyOrTrustFirst(5656);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    counter->Commit(5656);

    err = counter->VerifyOrTrustFirst(4756);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    err = mGroupPeerMsgCounter.FindOrAddPeer(10, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(11, 1, true, counter);
    err = mGroupPeerMsgCounter.FindOrAddPeer(12, 1, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, counter != nullptr);

    err = mGroupPeerMsgCounter.RemovePeer(3, 1, true);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(2) == 12);
    err = mGroupPeerMsgCounter.RemovePeer(8, 1, true);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(7) == 11);
    err = mGroupPeerMsgCounter.RemovePeer(11, 1, true);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(7) == 10);
    err = mGroupPeerMsgCounter.RemovePeer(1, 1, true);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(0) == 9);
    err = mGroupPeerMsgCounter.RemovePeer(10, 1, true);
    NL_TEST_ASSERT(inSuite, mGroupPeerMsgCounter.GetFabricIndexAt(7) == 0);

    // Validate that counter value were moved around correctly
    err = mGroupPeerMsgCounter.FindOrAddPeer(9, 1, true, counter);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = counter->VerifyOrTrustFirst(4756);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void GroupMessageCounterTest(nlTestSuite * inSuite, void * inContext)
{
    chip::TestPersistentStorageDelegate delegate;
    chip::Transport::GroupClientCounters groupCientCounter(&delegate);
    uint32_t counter = 99;

    // Start Test with Control counter
    counter = groupCientCounter.GetCounter(true);
    NL_TEST_ASSERT(inSuite, counter == 0);

    groupCientCounter.SetCounter(true, 563);
    counter = groupCientCounter.GetCounter(true);
    NL_TEST_ASSERT(inSuite, counter == 563);
    counter = groupCientCounter.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == 0);

    // Test Persistence
    chip::Transport::GroupClientCounters groupCientCounter2(&delegate);
    counter = groupCientCounter2.GetCounter(true);
    // Expect GROUP_MSG_COUNTER_MIN_INCREMENT since new object is initialized
    // with persistent data
    NL_TEST_ASSERT(inSuite, counter == GROUP_MSG_COUNTER_MIN_INCREMENT);
    counter = groupCientCounter2.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == GROUP_MSG_COUNTER_MIN_INCREMENT);

    // Test Roll over
    counter = UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT - 1;
    groupCientCounter2.SetCounter(true, counter);
    counter = groupCientCounter2.GetCounter(true);
    NL_TEST_ASSERT(inSuite, counter == (UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT - 1));

    // As of now value in persistent storage should be of
    // UINT32_MAX - 1
    chip::Transport::GroupClientCounters groupCientCounter3(&delegate);
    counter = groupCientCounter3.GetCounter(true);
    NL_TEST_ASSERT(inSuite, counter == (UINT32_MAX - 1));

    counter = 256; // To simulate roll over
    groupCientCounter3.SetCounter(true, counter);
    counter = groupCientCounter3.GetCounter(true);
    NL_TEST_ASSERT(inSuite, counter == 256);

    // Verify Persistence value again
    chip::Transport::GroupClientCounters groupCientCounter4(&delegate);
    counter = groupCientCounter4.GetCounter(true);
    NL_TEST_ASSERT(inSuite, counter == (UINT32_MAX - 1 + GROUP_MSG_COUNTER_MIN_INCREMENT));

    // Redo some of the test but with the Data counter

    counter = groupCientCounter.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == 0);

    groupCientCounter.SetCounter(false, 563);
    counter = groupCientCounter.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == 563);

    // Test Roll over
    counter = UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT - 4;
    groupCientCounter2.SetCounter(false, counter);
    counter = groupCientCounter2.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == (UINT32_MAX - GROUP_MSG_COUNTER_MIN_INCREMENT - 4));

    // As of now value in persistent storage should be of
    // UINT32_MAX - 1
    chip::Transport::GroupClientCounters groupCientCounter5(&delegate);
    counter = groupCientCounter5.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == (UINT32_MAX - 4));

    counter = 512; // To simulate roll over
    groupCientCounter5.SetCounter(false, counter);
    counter = groupCientCounter5.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == 512);

    // Verify Persistence value again
    chip::Transport::GroupClientCounters groupCientCounter6(&delegate);
    counter = groupCientCounter6.GetCounter(false);
    NL_TEST_ASSERT(inSuite, counter == (UINT32_MAX - 4 + GROUP_MSG_COUNTER_MIN_INCREMENT));
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
    NL_TEST_DEF("Counter Trust first",    CounterTrustFirstTest),
    NL_TEST_DEF("Reorder Fabric Removal", ReorderFabricRemovalTest),
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
