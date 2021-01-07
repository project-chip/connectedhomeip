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
 *      This file implements unit tests for the ReliableMessageProtocol
 *      implementation.
 */

#include "TestMessagingLayer.h"

#include <core/CHIPCore.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageManager.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::IOContext;

TestContext sContext;

const char PAYLOAD[]                = "Hello!";
constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

int gSendMessageCount = 0;

class OutgoingTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        const uint16_t headerSize = header.EncodeSizeBytes();

        VerifyOrReturnError(msgBuf->EnsureReservedSize(headerSize), CHIP_ERROR_NO_MEMORY);

        msgBuf->SetStart(msgBuf->Start() - headerSize);

        uint16_t actualEncodedHeaderSize;
        ReturnErrorOnFailure(header.Encode(msgBuf->Start(), msgBuf->DataLength(), &actualEncodedHeaderSize));

        gSendMessageCount++;

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

void test_os_sleep_ms(uint64_t millisecs)
{
    struct timespec sleep_time;
    uint64_t s = millisecs / 1000;

    millisecs -= s * 1000;
    sleep_time.tv_sec  = static_cast<time_t>(s);
    sleep_time.tv_nsec = static_cast<long>(millisecs * 1000000);

    nanosleep(&sleep_time, nullptr);
}

class ReliableMessageDelegateObject : public ReliableMessageDelegate
{
public:
    ~ReliableMessageDelegateObject() override {}

    /* Application callbacks */
    void OnThrottleRcvd(uint32_t pauseTime) override {}
    void OnDelayedDeliveryRcvd(uint32_t pauseTime) override {}
    void OnSendError(CHIP_ERROR err) override { SendErrorCalled = true; }
    void OnAckRcvd() override {}

    bool SendErrorCalled = false;
};

void CheckAddClearRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<OutgoingTransport> transportMgr;
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

    ExchangeContext * exchange = exchangeMgr.NewContext(kDestinationNodeId, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageManager::RetransTableEntry * entry;

    rm->AddToRetransTable(rc, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    rm->ClearRetransmitTable(*entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void CheckFailRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    TransportMgr<OutgoingTransport> transportMgr;
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

    ExchangeContext * exchange = exchangeMgr.NewContext(kDestinationNodeId, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageManager::RetransTableEntry * entry;
    ReliableMessageDelegateObject delegate;
    rc->SetDelegate(&delegate);
    rm->AddToRetransTable(rc, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    rm->FailRetransmitTableEntries(rc, CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
}

void CheckResendMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    uint16_t payload_len = sizeof(PAYLOAD);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::System::PacketBuffer::NewWithAvailableSize(payload_len);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    memmove(buffer->Start(), PAYLOAD, payload_len);
    buffer->SetDataLength(payload_len);

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);
    CHIP_ERROR err = CHIP_NO_ERROR;

    TransportMgr<OutgoingTransport> transportMgr;
    SecureSessionMgr secureSessionMgr;

    err = transportMgr.Init("LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = secureSessionMgr.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), &transportMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&secureSessionMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing1(Optional<NodeId>::Value(kSourceNodeId), 1, 2);
    Optional<Transport::PeerAddress> peer(Transport::PeerAddress::UDP(addr, CHIP_PORT));

    err = secureSessionMgr.NewPairing(peer, kDestinationNodeId, &pairing1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SecurePairingUsingTestSecret pairing2(Optional<NodeId>::Value(kDestinationNodeId), 2, 1);
    err = secureSessionMgr.NewPairing(peer, kSourceNodeId, &pairing2);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    MockAppDelegate mockSender;

    ExchangeContext * exchange = exchangeMgr.NewContext(kDestinationNodeId, &mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
        3, // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
    });

    gSendMessageCount = 0;

    err = exchange->SendMessage(kProtocol_Echo, kEchoMessageType_EchoRequest, std::move(buffer),
                                Messaging::SendFlags(Messaging::SendMessageFlags::kSendFlag_None));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gSendMessageCount == 2);

    // sleep another 65 ms to trigger second re-transmit
    test_os_sleep_ms(65);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gSendMessageCount == 3);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ReliableMessageManager::CheckAddClearRetrans", CheckAddClearRetrans),
    NL_TEST_DEF("Test ReliableMessageManager::CheckFailRetrans", CheckFailRetrans),
    NL_TEST_DEF("Test ReliableMessageManager::CheckResendMessage", CheckResendMessage),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-ReliableMessageProtocol",
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
int TestReliableMessageProtocol()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
