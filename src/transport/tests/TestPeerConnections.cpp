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
#include <transport/SessionManager.h>

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

const PeerAddress kPeer1Addr = AddressFromString("fe80::1");
const PeerAddress kPeer2Addr = AddressFromString("fe80::2");

const NodeId kPeer1NodeId = 123;
const NodeId kPeer2NodeId = 6;

const SecureSession::Type kPeer1SessionType = SecureSession::Type::kCASE;
const SecureSession::Type kPeer2SessionType = SecureSession::Type::kCASE;

const CATValues kPeer1CATs = { { 0xABCD0001, 0xABCE0100, 0xABCD0020 } };
const CATValues kPeer2CATs = { { 0xABCD0012, kUndefinedCAT, kUndefinedCAT } };

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
const PeerAddress kPeer3Addr                = AddressFromString("fe80::3");
const NodeId kPeer3NodeId                   = 81;
const SecureSession::Type kPeer3SessionType = SecureSession::Type::kPASE;
const CATValues kPeer3CATs;
#endif

void TestBasicFunctionality(nlTestSuite * inSuite, void * inContext)
{
    SecureSessionTable connections;
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);
    clock.SetMonotonic(100_ms64);
    CATValues peerCATs;
    Optional<SessionHandle> sessions[CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE];

    // Node ID 1, peer key 1, local key 2
    sessions[0] = connections.CreateNewSecureSessionForTest(kPeer1SessionType, 2, kPeer1NodeId, kPeer1CATs, 1, 0 /* fabricIndex */,
                                                            GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, sessions[0].HasValue());
    NL_TEST_ASSERT(inSuite, sessions[0].Value()->AsSecureSession()->GetSecureSessionType() == kPeer1SessionType);
    NL_TEST_ASSERT(inSuite, sessions[0].Value()->AsSecureSession()->GetPeerNodeId() == kPeer1NodeId);
    peerCATs = sessions[0].Value()->AsSecureSession()->GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kPeer1CATs, sizeof(CATValues)) == 0);

    // Node ID 2, peer key 3, local key 4
    sessions[1] = connections.CreateNewSecureSessionForTest(kPeer2SessionType, 4, kPeer2NodeId, kPeer2CATs, 3, 0 /* fabricIndex */,
                                                            GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, sessions[1].HasValue());
    NL_TEST_ASSERT(inSuite, sessions[1].Value()->AsSecureSession()->GetSecureSessionType() == kPeer2SessionType);
    NL_TEST_ASSERT(inSuite, sessions[1].Value()->AsSecureSession()->GetPeerNodeId() == kPeer2NodeId);
    NL_TEST_ASSERT(inSuite, sessions[1].Value()->AsSecureSession()->GetLastActivityTime() == 100_ms64);
    peerCATs = sessions[1].Value()->AsSecureSession()->GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kPeer2CATs, sizeof(CATValues)) == 0);

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    // If not using a heap, we can fill the SecureSessionTable
    for (uint16_t i = 2; i < CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE; ++i)
    {
        sessions[i] = connections.CreateNewSecureSessionForTest(kPeer2SessionType, static_cast<uint16_t>(i + 6u), kPeer2NodeId,
                                                                kPeer2CATs, 3, 0 /* fabricIndex */, GetLocalMRPConfig());
        NL_TEST_ASSERT(inSuite, sessions[i].HasValue());
    }

    // Insufficient space for new connections.
    auto optionalSession = connections.CreateNewSecureSessionForTest(kPeer3SessionType, 6, kPeer3NodeId, kPeer3CATs, 5,
                                                                     0 /* fabricIndex */, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, !optionalSession.HasValue());
#endif
    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

void TestFindByKeyId(nlTestSuite * inSuite, void * inContext)
{
    SessionManager sessionManager;
    SecureSessionTable connections;
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    // Node ID 1, peer key 1, local key 2
    auto optionalSession = connections.CreateNewSecureSessionForTest(kPeer1SessionType, 2, kPeer1NodeId, kPeer1CATs, 1,
                                                                     0 /* fabricIndex */, GetLocalMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());

    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(1).HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(2).HasValue());

    // Node ID 2, peer key 3, local key 4
    optionalSession = connections.CreateNewSecureSessionForTest(kPeer2SessionType, 4, kPeer2NodeId, kPeer2CATs, 3,
                                                                0 /* fabricIndex */, GetLocalMRPConfig());
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

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicFunctionality", TestBasicFunctionality),
    NL_TEST_DEF("FindByKeyId", TestFindByKeyId),
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
