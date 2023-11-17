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
#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>
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

const CATValues kPeer1CATs = { { 0xABCD0001, 0xABCE0100, 0xABCD0020 } };
const CATValues kPeer2CATs = { { 0xABCD0012, kUndefinedCAT, kUndefinedCAT } };

void TestBasicFunctionality(nlTestSuite * inSuite, void * inContext)
{
    SecureSessionTable connections;
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);
    clock.SetMonotonic(100_ms64);
    CATValues peerCATs;
    Optional<SessionHandle> sessions[CHIP_CONFIG_SECURE_SESSION_POOL_SIZE];

    // First node, peer session id 1, local session id 2
    auto optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 2, kLocalNodeId, kCasePeer1NodeId,
                                                                     kPeer1CATs, 1, kFabricIndex, GetDefaultMRPConfig());
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
                                                                kPeer2CATs, 3, kFabricIndex, GetDefaultMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetSecureSessionType() == SecureSession::Type::kCASE);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetPeerNodeId() == kCasePeer2NodeId);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetLocalNodeId() == kLocalNodeId);
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->GetPeer() == ScopedNodeId(kCasePeer2NodeId, kFabricIndex));
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->GetLocalScopedNodeId() == ScopedNodeId(kLocalNodeId, kFabricIndex));
    NL_TEST_ASSERT(inSuite, optionalSession.Value()->AsSecureSession()->GetLastActivityTime() == 100_ms64);
    peerCATs = optionalSession.Value()->AsSecureSession()->GetPeerCATs();
    NL_TEST_ASSERT(inSuite, memcmp(&peerCATs, &kPeer2CATs, sizeof(CATValues)) == 0);

    //
    // Fill up the session table.
    //
    for (int i = 2; i < CHIP_CONFIG_SECURE_SESSION_POOL_SIZE; ++i)
    {
        sessions[i] = connections.CreateNewSecureSessionForTest(
            SecureSession::Type::kCASE, static_cast<uint16_t>(static_cast<uint16_t>(i) + 6u), kLocalNodeId, kCasePeer2NodeId,
            kPeer2CATs, 3, kFabricIndex, GetDefaultMRPConfig());
        NL_TEST_ASSERT(inSuite, sessions[i].HasValue());
    }

    // #endif
    System::Clock::Internal::SetSystemClockForTesting(realClock);
}

void TestFindByKeyId(nlTestSuite * inSuite, void * inContext)
{
    SecureSessionTable connections;
    System::Clock::Internal::MockClock clock;
    System::Clock::ClockBase * realClock = &System::SystemClock();
    System::Clock::Internal::SetSystemClockForTesting(&clock);

    // First node, peer session id 1, local session id 2
    auto optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 2, kLocalNodeId, kCasePeer1NodeId,
                                                                     kPeer1CATs, 1, kFabricIndex, GetDefaultMRPConfig());
    NL_TEST_ASSERT(inSuite, optionalSession.HasValue());

    NL_TEST_ASSERT(inSuite, !connections.FindSecureSessionByLocalKey(1).HasValue());
    NL_TEST_ASSERT(inSuite, connections.FindSecureSessionByLocalKey(2).HasValue());

    // Second node, peer session id 3, local session id 4
    optionalSession = connections.CreateNewSecureSessionForTest(SecureSession::Type::kCASE, 4, kLocalNodeId, kCasePeer2NodeId,
                                                                kPeer2CATs, 3, kFabricIndex, GetDefaultMRPConfig());
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

int Initialize(void * apSuite)
{
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    return SUCCESS;
}

int Finalize(void * aContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

} // namespace

// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("BasicFunctionality", TestBasicFunctionality),
    NL_TEST_DEF("FindByKeyId", TestFindByKeyId),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestPeerConnectionsFn()
{
    nlTestSuite theSuite = { "Transport-SecureSessionTable", &sTests[0], Initialize, Finalize };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestPeerConnectionsFn)
