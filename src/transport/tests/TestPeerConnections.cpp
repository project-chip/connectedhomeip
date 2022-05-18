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
using namespace chip::System::Clock::Literals;

PeerAddress AddressFromString(const char * str)
{
    Inet::IPAddress addr;

    VerifyOrDie(Inet::IPAddress::FromString(str, addr));

    return PeerAddress::UDP(addr);
}

const PeerAddress kPeer1Addr    = AddressFromString("fe80::1");
const PeerAddress kPeer2Addr    = AddressFromString("fe80::2");
const PeerAddress kPasePeerAddr = AddressFromString("fe80::3");

const NodeId kLocalNodeId      = 0xC439A991071292DB;
const NodeId kCasePeer1NodeId  = 123;
const NodeId kCasePeer2NodeId  = 6;
const FabricIndex kFabricIndex = 8;

const NodeId kPasePeerNodeId = kUndefinedNodeId; // PASE is always undefined

const CATValues kPeer1CATs = { { 0xABCD0001, 0xABCE0100, 0xABCD0020 } };
const CATValues kPeer2CATs = { { 0xABCD0012, kUndefinedCAT, kUndefinedCAT } };
const CATValues kPeer3CATs;

void TestBasicFunctionality(nlTestSuite * inSuite, void * inContext)
{
    SecureSessionTable<2> connections;
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);
    clock.SetMonotonic(100_ms64);
    CATValues peerCATs;

    // First node, peer session id 1, local session id 2
    auto optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 2, kLocalNodeId, kCasePeer1NodeId,
                                                                     kPeer1CATs, 1, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetSecureSessionType() == SecureSession::Type::kCASE);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetPeerNodeId() == kCasePeer1NodeId);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetLocalNodeId() == kLocalNodeId);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->GetPeer() == ScopedNodeId(kCasePeer1NodeId, kFabricIndex));
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->GetLocalScopedNodeId() == ScopedNodeId(kLocalNodeId, kFabricIndex));
    peerCATs = optionalSession.Value()->AsSecureSession()->GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kPeer1CATs, sizeof(CATValues)) == 0);

    // Second node, peer session id 3, local session id 4
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 4, kLocalNodeId, kCasePeer2NodeId,
                                                                kPeer2CATs, 3, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetSecureSessionType() == SecureSession::Type::kCASE);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetPeerNodeId() == kCasePeer2NodeId);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetLocalNodeId() == kLocalNodeId);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->GetPeer() == ScopedNodeId(kCasePeer2NodeId, kFabricIndex));
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->GetLocalScopedNodeId() == ScopedNodeId(kLocalNodeId, kFabricIndex));
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetLastActivityTime() == 100_ms64);
    peerCATs = optionalSession.Value()->AsSecureSession()->GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kPeer2CATs, sizeof(CATValues)) == 0);

    // Insufficient space for new connections. Object is max size 2
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kPASE, 6, kLocalNodeId, kPasePeerNodeId,
                                                                kPeer3CATs, 5, kUndefinedFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, !optionalSession.HasValue());
    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

void TestFindByKeyId(nlTestSuite * inSuite, void * inContext)
{
    SecureSessionTable<2> connections;
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    // First node, peer session id 1, local session id 2
    auto optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 2, kLocalNodeId, kCasePeer1NodeId,
                                                                     kPeer1CATs, 1, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());

    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(1).HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(2).HasValue());

    // Second node, peer session id 3, local session id 4
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 4, kLocalNodeId, kCasePeer2NodeId,
                                                                kPeer2CATs, 3, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());

    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(3).HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(4).HasValue());

    System::Clock::Internal::SetSystemClockForTesting(realClock);
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
    SecureSessionTable<2> connections;

    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    clock.SetMonotonic(100_ms64);

    // First node, peer session id 1, local session id 2
    auto optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 2, kLocalNodeId, kCasePeer1NodeId,
                                                                     kPeer1CATs, 1, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    optionalSession.Value()->AsSecureSession()->SetPeerAddress(kPeer1Addr);

    clock.SetMonotonic(200_ms64);
    // Second node, peer session id 3, local session id 4
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 4, kLocalNodeId, kCasePeer2NodeId,
                                                                kPeer2CATs, 3, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    optionalSession.Value()->AsSecureSession()->SetPeerAddress(kPeer2Addr);

    // cannot add before expiry
    clock.SetMonotonic(300_ms64);
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kPASE, 6, kLocalNodeId, kPasePeerNodeId,
                                                                kPeer3CATs, 5, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, !optionalSession.HasValue());

    // at time 300, this expires ip addr 1
    connections.ExpireInactiveSessions(150_ms64, [&callInfo](const SecureSession & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallNodeId == kCasePeer1NodeId);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeerAddress == kPeer1Addr);
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(2).HasValue());

    // now that the connections were expired, we can add peer3
    clock.SetMonotonic(300_ms64);
    // Third node (PASE session), peer session id 5, local session id 6
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kPASE, 6, kLocalNodeId, kPasePeerNodeId,
                                                                kPeer3CATs, 5, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    optionalSession.Value()->AsSecureSession()->SetPeerAddress(kPasePeerAddr);

    clock.SetMonotonic(400_ms64);
    optionalSession = connections.FindSecureSessionByLocalKey(4);
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());

    optionalSession.Value()->AsSecureSession()->MarkActive();
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetLastActivityTime() == clock.GetMonotonicTimestamp());

    // At this time:
    //   Peer 3 active at time 300
    //   Peer 2 active at time 400

    clock.SetMonotonic(500_ms64);
    callInfo.callCount = 0;
    connections.ExpireInactiveSessions(150_ms64, [&callInfo](const SecureSession & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });

    // peer 2 stays active
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 1);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallNodeId == kPasePeerNodeId);
    NL_TEST_ASSERT(inSuite, callInfo.lastCallPeerAddress == kPasePeerAddr);
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(2).HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(4).HasValue());
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(6).HasValue());

    // First node, peer session id 1, local session id 2
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 2, kLocalNodeId, kCasePeer1NodeId,
                                                                kPeer1CATs, 1, kFabricIndex, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(2).HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(4).HasValue());
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(6).HasValue());

    // peer 1 and 2 are active
    clock.SetMonotonic(1000_ms64);
    callInfo.callCount = 0;
    connections.ExpireInactiveSessions(100_ms64, [&callInfo](const SecureSession & state) {
        callInfo.callCount++;
        callInfo.lastCallNodeId      = state.GetPeerNodeId();
        callInfo.lastCallPeerAddress = state.GetPeerAddress();
    });
    NL_TEST_ASSERT(inSuite, callInfo.callCount == 2); // everything expired
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(2).HasValue());
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(4).HasValue());
    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(6).HasValue());

    System::Clock::Internal::SetSystemClockForTesting(realClock);
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
