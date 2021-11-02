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
 *      the SecureSessionTable class within the transport layer
 *
 */
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/SecureSessionTable.h>

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

const PeerAddress kPeer1Addr = AddressFromString("fe80::1");
const PeerAddress kPeer2Addr = AddressFromString("fe80::2");
const PeerAddress kPeer3Addr = AddressFromString("fe80::3");

const NodeId kPeer1NodeId = 123;
const NodeId kPeer2NodeId = 6;
const NodeId kPeer3NodeId = 81;

void TestBasicFunctionality(nlTestSuite * inSuite, void * inContext)
{
    SecureSession * statePtr;
    SecureSessionTable<2, Time::Source::kTest> connections;
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(100);

    // Node ID 1, peer key 1, local key 2
    statePtr = connections.CreateNewSecureSession(2, kPeer1NodeId, 1, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);

    // Node ID 2, peer key 3, local key 4
    statePtr = connections.CreateNewSecureSession(4, kPeer2NodeId, 3, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerNodeId() == kPeer2NodeId);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == 100);

    // Insufficient space for new connections. Object is max size 2
    statePtr = connections.CreateNewSecureSession(6, kPeer3NodeId, 5, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr == nullptr);
}

void TestFindByKeyId(nlTestSuite * inSuite, void * inContext)
{
    SecureSession * statePtr;
    SecureSessionTable<2, Time::Source::kTest> connections;

    // Node ID 1, peer key 1, local key 2
    statePtr = connections.CreateNewSecureSession(2, kPeer1NodeId, 1, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);

    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(1));
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(2));

    // Node ID 2, peer key 3, local key 4
    statePtr = connections.CreateNewSecureSession(4, kPeer2NodeId, 3, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);

    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(3));
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(4));
}

struct ExpiredCallInfo
{
    int callCount                   = 0;
    NodeId lastCallNodeId           = 0;
    PeerAddress lastCallPeerAddress = PeerAddress::Uninitialized();
};

void TestExpireConnections(nlTestSuite * inSuite, void * inContext)
{
    ExpiredCallInfo callInfo;
    SecureSession * statePtr;
    SecureSessionTable<2, Time::Source::kTest> connections;

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(100);

    // Node ID 1, peer key 1, local key 2
    statePtr = connections.CreateNewSecureSession(2, kPeer1NodeId, 1, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    statePtr->SetPeerAddress(kPeer1Addr);

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(200);
    // Node ID 2, peer key 3, local key 4
    statePtr = connections.CreateNewSecureSession(4, kPeer2NodeId, 3, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    statePtr->SetPeerAddress(kPeer2Addr);

    // cannot add before expiry
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(300);
    statePtr = connections.CreateNewSecureSession(6, kPeer3NodeId, 5, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr == nullptr);

    // at time 300, this expires ip addr 1
    connections.ExpireInactiveSessions(150, [&callInfo](const SecureSession & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallNodeId == kPeer1NodeId);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeerAddress == kPeer1Addr);
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(2));

    // now that the connections were expired, we can add peer3
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(300);
    // Node ID 3, peer key 5, local key 6
    statePtr = connections.CreateNewSecureSession(6, kPeer3NodeId, 5, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    statePtr->SetPeerAddress(kPeer3Addr);

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(400);
    NL_TEST_ASSERT(inSuite, statePtr = connections.FindSecureSessionByLocalKey(4));

    connections.MarkSessionActive(statePtr);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == connections.GetTimeSource().GetCurrentMonotonicTimeMs());

    // At this time:
    //   Peer 3 active at time 300
    //   Peer 2 active at time 400

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(500);
    callInfo.callCount = 0;
    connections.ExpireInactiveSessions(150, [&callInfo](const SecureSession & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });

    // peer 2 stays active
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallNodeId == kPeer3NodeId);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeerAddress == kPeer3Addr);
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(2));
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(4));
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(6));

    // Node ID 1, peer key 1, local key 2
    statePtr = connections.CreateNewSecureSession(2, kPeer1NodeId, 1, 0 /* fabricIndex */);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(2));
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(4));
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(6));

    // peer 1 and 2 are active
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(1000);
    callInfo.callCount = 0;
    connections.ExpireInactiveSessions(100, [&callInfo](const SecureSession & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 2); // everything expired
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(2));
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(4));
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(6));
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicFunctionality", TestBasicFunctionality),
    NL_TEST_DEF("FindByKeyId", TestFindByKeyId),
    NL_TEST_DEF("ExpireConnections", TestExpireConnections),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestPeerConnectionsFn(void)
{
    nlTestSuite theSuite = { "Transport-SecureSessionTable", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPeerConnectionsFn)
