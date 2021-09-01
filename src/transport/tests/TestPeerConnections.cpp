/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a process to effect a functional test for
 *      the PeerConnections class within the transport layer
 *
 */
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/PeerConnections.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Transport;

PeerAddress AddressFromString(const char * str)
{
    Inet::IPAddress addr;

    VerifyOrDie(Inet::IPAddress::FromString(str, addr));

    return PeerAddress::UDP(addr);
}

const PeerAddress kPeer1Addr = AddressFromString("10.1.2.3");
const PeerAddress kPeer2Addr = AddressFromString("10.0.0.32");
const PeerAddress kPeer3Addr = AddressFromString("100.200.0.1");

const NodeId kPeer1NodeId = 123;
const NodeId kPeer2NodeId = 6;
const NodeId kPeer3NodeId = 81;

void TestBasicFunctionality(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    PeerConnectionState * statePtr;
    PeerConnections<2, Time::Source::kTest> connections;
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(100);

    err = connections.CreateNewPeerConnectionState(kPeer1Addr, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = connections.CreateNewPeerConnectionState(kPeer2Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer2Addr);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == 100);

    // Insufficient space for new connections. Object is max size 2
    err = connections.CreateNewPeerConnectionState(kPeer3Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void TestFindByAddress(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    PeerConnectionState * statePtr;
    PeerConnections<3, Time::Source::kTest> connections;

    PeerConnectionState * state1 = nullptr;
    PeerConnectionState * state2 = nullptr;
    PeerConnectionState * state3 = nullptr;

    err = connections.CreateNewPeerConnectionState(kPeer1Addr, &state1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = connections.CreateNewPeerConnectionState(kPeer1Addr, &state2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = connections.CreateNewPeerConnectionState(kPeer2Addr, &state3);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, state1 != state2);
    NL_TEST_ASSERT(inSuite, state1 != state3);
    NL_TEST_ASSERT(inSuite, state2 != state3);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer1Addr, nullptr));
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer1Addr);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer1Addr, statePtr));
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer1Addr);

    NL_TEST_ASSERT(inSuite, (statePtr = connections.FindPeerConnectionState(kPeer1Addr, statePtr)) == nullptr);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer2Addr, nullptr));
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer2Addr);
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3Addr, nullptr));
}

void TestFindByNodeId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    PeerConnectionState * statePtr;
    PeerConnections<3, Time::Source::kTest> connections;

    err = connections.CreateNewPeerConnectionState(kPeer1Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    statePtr->SetPeerNodeId(kPeer1NodeId);

    err = connections.CreateNewPeerConnectionState(kPeer2Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    statePtr->SetPeerNodeId(kPeer2NodeId);

    err = connections.CreateNewPeerConnectionState(kPeer2Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    statePtr->SetPeerNodeId(kPeer1NodeId);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer1NodeId, nullptr));
    char buf[100];
    statePtr->GetPeerAddress().ToString(buf);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer1Addr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerNodeId() == kPeer1NodeId);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer1NodeId, statePtr));
    statePtr->GetPeerAddress().ToString(buf);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer2Addr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerNodeId() == kPeer1NodeId);

    NL_TEST_ASSERT(inSuite, (statePtr = connections.FindPeerConnectionState(kPeer1NodeId, statePtr)) == nullptr);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer2NodeId, nullptr));
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerAddress() == kPeer2Addr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerNodeId() == kPeer2NodeId);

    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3NodeId, nullptr));
}

void TestFindByKeyId(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    PeerConnectionState * statePtr;
    PeerConnections<2, Time::Source::kTest> connections;

    // No Node ID, peer key 1, local key 2
    err = connections.CreateNewPeerConnectionState(Optional<NodeId>::Missing(), 1, 2, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Lookup using no node, and peer key
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(Optional<NodeId>::Missing(), 1, nullptr));
    // Lookup using no node, and local key
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Missing(), 2, nullptr));

    // Lookup using no node, and incorrect peer key
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(Optional<NodeId>::Missing(), 2, nullptr));

    // Lookup using no node, and incorrect local key
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Missing(), 1, nullptr));

    // Lookup using a node ID, and peer key
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(Optional<NodeId>::Value(kPeer1NodeId), 1, nullptr));

    // Lookup using a node ID, and local key
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(kPeer1NodeId), 2, nullptr));

    // Some Node ID, peer key 3, local key 4
    err = connections.CreateNewPeerConnectionState(Optional<NodeId>::Value(kPeer1NodeId), 3, 4, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Lookup using correct node (or no node), and correct keys
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(Optional<NodeId>::Value(kPeer1NodeId), 3, nullptr));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(kPeer1NodeId), 4, nullptr));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(Optional<NodeId>::Missing(), 3, nullptr));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Missing(), 4, nullptr));

    // Lookup using incorrect keys
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(Optional<NodeId>::Value(kPeer1NodeId), 4, nullptr));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(kPeer1NodeId), 3, nullptr));

    // Lookup using incorrect node, but correct keys
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(Optional<NodeId>::Value(kPeer2NodeId), 3, nullptr));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(kPeer2NodeId), 4, nullptr));
}

struct ExpiredCallInfo
{
    int callCount                   = 0;
    NodeId lastCallNodeId           = 0;
    PeerAddress lastCallPeerAddress = PeerAddress::Uninitialized();
};

void TestExpireConnections(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ExpiredCallInfo callInfo;
    PeerConnectionState * statePtr;
    PeerConnections<2, Time::Source::kTest> connections;

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(100);

    err = connections.CreateNewPeerConnectionState(kPeer1Addr, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(200);
    err = connections.CreateNewPeerConnectionState(kPeer2Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    statePtr->SetPeerNodeId(kPeer2NodeId);

    // cannot add before expiry
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(300);
    err = connections.CreateNewPeerConnectionState(kPeer3Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // at time 300, this expires ip addr 1
    connections.ExpireInactiveConnections(150, [&callInfo](const PeerConnectionState & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallNodeId == kUndefinedNodeId);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeerAddress == kPeer1Addr);
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1NodeId, nullptr));

    // now that the connections were expired, we can add peer3
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(300);
    err = connections.CreateNewPeerConnectionState(kPeer3Addr, &statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    statePtr->SetPeerNodeId(kPeer3NodeId);

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(400);
    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer2NodeId, nullptr));

    connections.MarkConnectionActive(statePtr);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == connections.GetTimeSource().GetCurrentMonotonicTimeMs());

    // At this time:
    //   Peer 3 active at time 300
    //   Peer 2 active at time 400

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(500);
    callInfo.callCount = 0;
    connections.ExpireInactiveConnections(150, [&callInfo](const PeerConnectionState & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });

    // peer 2 stays active
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallNodeId == kPeer3NodeId);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeerAddress == kPeer3Addr);
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1Addr, nullptr));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(kPeer2Addr, nullptr));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3Addr, nullptr));

    err = connections.CreateNewPeerConnectionState(kPeer1Addr, nullptr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(kPeer1Addr, nullptr));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(kPeer2Addr, nullptr));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3Addr, nullptr));

    // peer 1 and 2 are active
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(1000);
    callInfo.callCount = 0;
    connections.ExpireInactiveConnections(100, [&callInfo](const PeerConnectionState & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 2); // everything expired
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1Addr, nullptr));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer2Addr, nullptr));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3Addr, nullptr));
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicFunctionality", TestBasicFunctionality),
    NL_TEST_DEF("FindByPeerAddress", TestFindByAddress),
    NL_TEST_DEF("FindByNodeId", TestFindByNodeId),
    NL_TEST_DEF("FindByKeyId", TestFindByKeyId),
    NL_TEST_DEF("ExpireConnections", TestExpireConnections),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestPeerConnectionsFn(void)
{
    nlTestSuite theSuite = { "Transport-PeerConnections", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPeerConnectionsFn)
