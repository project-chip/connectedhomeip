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
 *      This file implements unit tests for the ExchangeManager implementation.
 */

#include "TestMessagingLayer.h"

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
#include <utility>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;

using TestContext = chip::Test::IOContext;

TestContext sContext;

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        HandleMessageReceived(header, address, std::move(msgBuf));
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class MockAppDelegate : public ExchangeDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBufferHandle buffer) override
    {
        IsOnMessageReceivedCalled = true;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    bool IsOnMessageReceivedCalled = false;
};

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&secureSessionMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

class TestSessMgrCallback : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           System::PacketBufferHandle msgBuf, SecureSessionMgr * mgr) override
    {
        ReceiveHandlerCallCount++;
    }

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override
    {
        mSecureSession = session;
        NewConnectionHandlerCallCount++;
    }
    void OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr) override {}

    SecureSessionHandle mSecureSession;
    int ReceiveHandlerCallCount       = 0;
    int NewConnectionHandlerCallCount = 0;
};

void CheckNewContextTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    CHIP_ERROR err;

    TestSessMgrCallback callback;
    secureSessionMgr.SetDelegate(&callback);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    SecurePairingUsingTestSecret pairing1(Optional<NodeId>::Value(kSourceNodeId), 1, 2);
    Optional<Transport::PeerAddress> peer1(Transport::PeerAddress::UDP(addr, 1));
    err = secureSessionMgr.NewPairing(peer1, kDestinationNodeId, &pairing1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    SecureSessionHandle sessionFromSourceToDestination = callback.mSecureSession;

    SecurePairingUsingTestSecret pairing2(Optional<NodeId>::Value(kDestinationNodeId), 2, 1);
    Optional<Transport::PeerAddress> peer2(Transport::PeerAddress::UDP(addr, 2));
    err = secureSessionMgr.NewPairing(peer2, kSourceNodeId, &pairing2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    SecureSessionHandle sessionFromDestinationToSource = callback.mSecureSession;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&secureSessionMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    MockAppDelegate mockAppDelegate;

    ExchangeContext * ec1 = exchangeMgr.NewContext(sessionFromDestinationToSource, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, ec1 != nullptr);
    NL_TEST_ASSERT(inSuite, ec1->IsInitiator() == true);
    NL_TEST_ASSERT(inSuite, ec1->GetExchangeId() != 0);
    NL_TEST_ASSERT(inSuite, ec1->GetSecureSession() == sessionFromDestinationToSource);
    NL_TEST_ASSERT(inSuite, ec1->GetDelegate() == &mockAppDelegate);

    ExchangeContext * ec2 = exchangeMgr.NewContext(sessionFromSourceToDestination, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, ec2 != nullptr);
    NL_TEST_ASSERT(inSuite, ec2->GetExchangeId() > ec1->GetExchangeId());
    NL_TEST_ASSERT(inSuite, ec2->GetSecureSession() == sessionFromSourceToDestination);
}

void CheckUmhRegistrationTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&secureSessionMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    MockAppDelegate mockAppDelegate;

    err = exchangeMgr.RegisterUnsolicitedMessageHandler(0x0001, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = exchangeMgr.RegisterUnsolicitedMessageHandler(0x0002, 0x0001, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = exchangeMgr.UnregisterUnsolicitedMessageHandler(0x0001);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = exchangeMgr.UnregisterUnsolicitedMessageHandler(0x0002);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    err = exchangeMgr.UnregisterUnsolicitedMessageHandler(0x0002, 0x0001);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = exchangeMgr.UnregisterUnsolicitedMessageHandler(0x0002, 0x0002);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void CheckExchangeMessages(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<LoopbackTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;
    CHIP_ERROR err;

    TestSessMgrCallback callback;
    secureSessionMgr.SetDelegate(&callback);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    SecurePairingUsingTestSecret pairing1(Optional<NodeId>::Value(kSourceNodeId), 1, 2);
    Optional<Transport::PeerAddress> peer1(Transport::PeerAddress::UDP(addr, 1));
    err = secureSessionMgr.NewPairing(peer1, kDestinationNodeId, &pairing1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    SecureSessionHandle sessionFromSourceToDestination = callback.mSecureSession;

    SecurePairingUsingTestSecret pairing2(Optional<NodeId>::Value(kDestinationNodeId), 2, 1);
    Optional<Transport::PeerAddress> peer2(Transport::PeerAddress::UDP(addr, 2));
    err = secureSessionMgr.NewPairing(peer2, kSourceNodeId, &pairing2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&secureSessionMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // create solicited exchange
    MockAppDelegate mockSolicitedAppDelegate;
    ExchangeContext * ec1 = exchangeMgr.NewContext(sessionFromSourceToDestination, &mockSolicitedAppDelegate);

    // create unsolicited exchange
    MockAppDelegate mockUnsolicitedAppDelegate;
    err = exchangeMgr.RegisterUnsolicitedMessageHandler(0x0001, 0x0001, &mockUnsolicitedAppDelegate);

    // send a malicious packet
    ec1->SendMessage(0x0001, 0x0002, System::PacketBuffer::New(), SendFlags(Messaging::SendMessageFlags::kNone));
    NL_TEST_ASSERT(inSuite, !mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    // send a good packet
    ec1->SendMessage(0x0001, 0x0001, System::PacketBuffer::New(), SendFlags(Messaging::SendMessageFlags::kNone));
    NL_TEST_ASSERT(inSuite, mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ExchangeMgr::Init",                     CheckSimpleInitTest),
    NL_TEST_DEF("Test ExchangeMgr::NewContext",               CheckNewContextTest),
    NL_TEST_DEF("Test ExchangeMgr::CheckUmhRegistrationTest", CheckUmhRegistrationTest),
    NL_TEST_DEF("Test ExchangeMgr::CheckExchangeMessages",    CheckExchangeMessages),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-ExchangeManager",
    &sTests[0],
    Initialize,
    Finalize
};
// clang-format on

/**
 *  Initialize the test suite.
 */
int Initialize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite);
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

/**
 *  Main
 */
int TestExchangeMgr()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
