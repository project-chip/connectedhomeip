/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for the MessageCounterSyncMgr implementation.
 */

#include "TestMessagingLayer.h"

#include <core/CHIPCore.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <support/logging/CHIPLogging.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

TestContext sContext;

constexpr NodeId kSourceNodeId        = 123654;
constexpr NodeId kDestinationNodeId   = 111222333;
constexpr NodeId kTestPeerGroupKeyId  = 0x4000;
constexpr NodeId kTestLocalGroupKeyId = 0x5000;

const char PAYLOAD[] = "Hello!";

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

class TestExchangeMgr : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           System::PacketBufferHandle msgBuf, SecureSessionMgr * mgr) override
    {
        NL_TEST_ASSERT(mSuite, header.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, header.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, msgBuf->DataLength() == kMsgCounterChallengeSize);

        ReceiveHandlerCallCount++;
    }

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override {}

    void OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr) override {}

    nlTestSuite * mSuite        = nullptr;
    int ReceiveHandlerCallCount = 0;
};

class MockAppDelegate : public ExchangeDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle msgBuf) override
    {
        IsOnMessageReceivedCalled = true;

        NL_TEST_ASSERT(mSuite, payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq));
        NL_TEST_ASSERT(mSuite, packetHeader.GetSourceNodeId() == Optional<NodeId>::Value(kSourceNodeId));
        NL_TEST_ASSERT(mSuite, packetHeader.GetDestinationNodeId() == Optional<NodeId>::Value(kDestinationNodeId));
        NL_TEST_ASSERT(mSuite, msgBuf->DataLength() == kMsgCounterChallengeSize);
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    nlTestSuite * mSuite           = nullptr;
    bool IsOnMessageReceivedCalled = false;
};

TransportMgr<LoopbackTransport> gTransportMgr;

void CheckSendMsgCounterSyncReq(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    TestExchangeMgr testExchangeMgr;

    testExchangeMgr.mSuite = inSuite;
    ctx.GetSecureSessionManager().SetDelegate(&testExchangeMgr);

    MessageCounterSyncMgr * sm = ctx.GetExchangeManager().GetMessageCounterSyncMgr();
    NL_TEST_ASSERT(inSuite, sm != nullptr);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SecurePairingUsingTestSecret pairingLocalToPeer(kTestPeerGroupKeyId, kTestLocalGroupKeyId);

    err = ctx.GetSecureSessionManager().NewPairing(peer, kDestinationNodeId, &pairingLocalToPeer,
                                                   SecureSessionMgr::PairingDirection::kInitiator, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairingPeerToLocal(kTestLocalGroupKeyId, kTestPeerGroupKeyId);

    err = ctx.GetSecureSessionManager().NewPairing(peer, kSourceNodeId, &pairingPeerToLocal,
                                                   SecureSessionMgr::PairingDirection::kResponder, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle session(kDestinationNodeId, 0x4000, 0);

    // Should be able to send a message to itself by just calling send.
    testExchangeMgr.ReceiveHandlerCallCount = 0;

    err = sm->SendMsgCounterSyncReq(session);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, testExchangeMgr.ReceiveHandlerCallCount == 1);
}

void CheckReceiveMsgCounterSyncReq(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);

    CHIP_ERROR err = CHIP_NO_ERROR;
    MockAppDelegate mockAppDelegate;

    mockAppDelegate.mSuite = inSuite;

    MessageCounterSyncMgr * sm = ctx.GetExchangeManager().GetMessageCounterSyncMgr();
    NL_TEST_ASSERT(inSuite, sm != nullptr);

    // Register to receive unsolicited Secure Channel Request messages from the exchange manager.
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::SecureChannel::Id, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    SecurePairingUsingTestSecret pairingLocalToPeer(kTestPeerGroupKeyId, kTestLocalGroupKeyId);

    err = ctx.GetSecureSessionManager().NewPairing(peer, kDestinationNodeId, &pairingLocalToPeer,
                                                   SecureSessionMgr::PairingDirection::kInitiator, 0);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairingPeerToLocal(kTestLocalGroupKeyId, kTestPeerGroupKeyId);

    err = ctx.GetSecureSessionManager().NewPairing(peer, kSourceNodeId, &pairingPeerToLocal,
                                                   SecureSessionMgr::PairingDirection::kResponder, 1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecureSessionHandle session(kDestinationNodeId, 0x4000, 0);

    err = sm->SendMsgCounterSyncReq(session);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, mockAppDelegate.IsOnMessageReceivedCalled == true);
}

void CheckAddRetransTable(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    MessageCounterSyncMgr * sm = ctx.GetExchangeManager().GetMessageCounterSyncMgr();
    NL_TEST_ASSERT(inSuite, sm != nullptr);

    System::PacketBufferHandle buffer = MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err =
        sm->AddToRetransmissionTable(Protocols::Echo::Id, static_cast<uint8_t>(Protocols::Echo::MsgType::EchoRequest),
                                     Messaging::SendFlags(Messaging::SendMessageFlags::kNone), std::move(buffer), exchange);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckAddToReceiveTable(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    MessageCounterSyncMgr * sm = ctx.GetExchangeManager().GetMessageCounterSyncMgr();
    NL_TEST_ASSERT(inSuite, sm != nullptr);

    System::PacketBufferHandle buffer = MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    PacketHeader packetHeader;
    PayloadHeader payloadHeader;

    CHIP_ERROR err = sm->AddToReceiveTable(packetHeader, payloadHeader, { chip::kTestDeviceNodeId, 0, 0 }, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test MessageCounterSyncMgr::ReceiveMsgCounterSyncReq", CheckReceiveMsgCounterSyncReq),
    NL_TEST_DEF("Test MessageCounterSyncMgr::SendMsgCounterSyncReq", CheckSendMsgCounterSyncReq),
    NL_TEST_DEF("Test MessageCounterSyncMgr::AddToRetransTable", CheckAddRetransTable),
    NL_TEST_DEF("Test MessageCounterSyncMgr::AddToReceiveTable", CheckAddToReceiveTable),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-MessageCounterSyncMgr",
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
    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    err = gTransportMgr.Init("LOOPBACK");
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    err = reinterpret_cast<TestContext *>(aContext)->Init(&sSuite, &gTransportMgr);
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
int TestMessageCounterSyncMgr()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
