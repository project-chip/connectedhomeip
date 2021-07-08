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
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <transport/PeerConnections.h>

#include <nlunit-test.h>

namespace {

using namespace chip;
using namespace chip::Transport;

constexpr PeerId kPeer1 = PeerId{ 123, kUndefinedFabricId };
constexpr PeerId kPeer2 = PeerId{ 6, kUndefinedFabricId };
constexpr PeerId kPeer3 = PeerId{ 81, kUndefinedFabricId };

void TestBasicFunctionality(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    Transport::PeerCache<5> peers;
    PeerConnectionState * statePtr;
    PeerConnections<2, Time::Source::kTest> connections;
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(100);

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer1), kUndefinedFabricIndex, 1, 2, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerInfo().GetPeer() == kPeer1);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerKeyID() == 1);
    NL_TEST_ASSERT(inSuite, statePtr->GetLocalKeyID() == 2);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == 100);

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer2), kUndefinedFabricIndex, 3, 4, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, statePtr != nullptr);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerInfo().GetPeer() == kPeer2);
    NL_TEST_ASSERT(inSuite, statePtr->GetPeerKeyID() == 3);
    NL_TEST_ASSERT(inSuite, statePtr->GetLocalKeyID() == 4);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == 100);

    // Insufficient space for new connections. Object is max size 2
    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer3), kUndefinedFabricIndex, 5, 6, statePtr);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void TestFindByPeer(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    Transport::PeerCache<5> peers;
    PeerConnectionState * statePtr;
    PeerConnections<3, Time::Source::kTest> connections;

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer1), kUndefinedFabricIndex, 1, 2, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer2), kUndefinedFabricIndex, 3, 4, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer1), kUndefinedFabricIndex, 5, 6, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer1, 1));
    NL_TEST_ASSERT(inSuite, statePtr->GetLocalKeyID() == 2);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer1, 5));
    NL_TEST_ASSERT(inSuite, statePtr->GetLocalKeyID() == 6);

    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer2, 3));
    NL_TEST_ASSERT(inSuite, statePtr->GetLocalKeyID() == 4);
}

struct ExpiredCallInfo
{
    int callCount = 0;
    PeerId lastCallPeer;
};

void TestExpireConnections(nlTestSuite * inSuite, void * inContext)
{
    CHIP_ERROR err;
    ExpiredCallInfo callInfo;
    Transport::PeerCache<5> peers;
    PeerConnectionState * statePtr;
    PeerConnections<2, Time::Source::kTest> connections;

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(100);

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer1), kUndefinedFabricIndex, 1, 2, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(200);
    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer2), kUndefinedFabricIndex, 3, 4, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // cannot add before expiry
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(300);
    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer3), kUndefinedFabricIndex, 5, 6, statePtr);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    // at time 300, this expires ip addr 1
    connections.ExpireInactiveConnections(150, [&callInfo](const PeerConnectionState & state) {
        callInfo.callCount++;
        callInfo.lastCallPeer = state.GetPeerInfo().GetPeer();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeer == kPeer1);
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1, 1));

    // now that the connections were expired, we can add peer3
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(300);
    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer3), kUndefinedFabricIndex, 7, 8, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(400);
    NL_TEST_ASSERT(inSuite, statePtr = connections.FindPeerConnectionState(kPeer2, 3));

    connections.MarkConnectionActive(statePtr);
    NL_TEST_ASSERT(inSuite, statePtr->GetLastActivityTimeMs() == connections.GetTimeSource().GetCurrentMonotonicTimeMs());

    // At this time:
    //   Peer 3 active at time 300
    //   Peer 2 active at time 400

    connections.GetTimeSource().SetCurrentMonotonicTimeMs(500);
    callInfo.callCount = 0;
    connections.ExpireInactiveConnections(150, [&callInfo](const PeerConnectionState & state) {
        callInfo.callCount++;
        callInfo.lastCallPeer = state.GetPeerInfo().GetPeer();
    });

    // peer 2 stays active
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeer == kPeer3);
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1, 1));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(kPeer2, 3));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3, 5));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3, 7));

    err = connections.CreateNewPeerConnectionState(*peers.FindOrAllocateEntry(kPeer1), kUndefinedFabricIndex, 9, 10, statePtr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1, 1));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(kPeer1, 9));
    NL_TEST_ASSERT(inSuite, connections.FindPeerConnectionState(kPeer2, 3));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3, 5));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3, 7));

    // peer 1 and 2 are active
    connections.GetTimeSource().SetCurrentMonotonicTimeMs(1000);
    callInfo.callCount = 0;
    connections.ExpireInactiveConnections(100, [&callInfo](const PeerConnectionState & state) {
        callInfo.callCount++;
        callInfo.lastCallPeer = state.GetPeerInfo().GetPeer();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 2); // everything expired
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1, 1));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer1, 9));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer2, 3));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3, 5));
    NL_TEST_ASSERT(inSuite, !connections.FindPeerConnectionState(kPeer3, 7));
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicFunctionality", TestBasicFunctionality),
    NL_TEST_DEF("FindByPeer", TestFindByPeer),
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
