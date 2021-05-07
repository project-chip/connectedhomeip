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
 *      This file implements unit tests for the ReliableMessageProtocol
 *      implementation.
 */

#include "TestMessagingLayer.h"

#include <core/CHIPCore.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

TestContext sContext;

const char PAYLOAD[] = "Hello!";

int gSendMessageCount = 0;

class OutgoingTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBufferHandle msgBuf) override
    {
        ReturnErrorOnFailure(header.EncodeBeforeData(msgBuf));

        gSendMessageCount++;

        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

TransportMgr<OutgoingTransport> gTransportMgr;

class MockAppDelegate : public ExchangeDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle buffer) override
    {
        IsOnMessageReceivedCalled = true;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    bool IsOnMessageReceivedCalled = false;
};

void test_os_sleep_ms(uint64_t millisecs)
{
#ifdef __MBED__
    usleep(millisecs * 1000);
#else
    struct timespec sleep_time;
    uint64_t s = millisecs / 1000;

    millisecs -= s * 1000;
    sleep_time.tv_sec  = static_cast<time_t>(s);
    sleep_time.tv_nsec = static_cast<long>(millisecs * 1000000);

    nanosleep(&sleep_time, nullptr);
#endif
}

class ReliableMessageDelegateObject : public ReliableMessageDelegate
{
public:
    ~ReliableMessageDelegateObject() override {}

    /* Application callbacks */
    void OnSendError(CHIP_ERROR err) override { SendErrorCalled = true; }
    void OnAckRcvd() override {}

    bool SendErrorCalled = false;
};

void CheckAddClearRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageMgr::RetransTableEntry * entry;

    rm->AddToRetransTable(rc, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    rm->ClearRetransTable(*entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void CheckFailRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageMgr::RetransTableEntry * entry;
    ReliableMessageDelegateObject delegate;
    rc->SetDelegate(&delegate);
    rm->AddToRetransTable(rc, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    rm->FailRetransTableEntries(rc, CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
}

void CheckResendMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    IPAddress addr;
    IPAddress::FromString("127.0.0.1", addr);

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender;
    // TODO: temprary create a SecureSessionHandle from node id, will be fix in PR 3602
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
        1, // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
        3, // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
    });

    gSendMessageCount = 0;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer),
                                Messaging::SendFlags(Messaging::SendMessageFlags::kNone));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gSendMessageCount == 2);

    // sleep another 65 ms to trigger second re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, gSendMessageCount == 3);
}

void CheckSendStandaloneAckMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageDelegateObject delegate;
    rc->SetDelegate(&delegate);

    NL_TEST_ASSERT(inSuite, rc->SendStandaloneAckMessage() == CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ReliableMessageMgr::CheckAddClearRetrans", CheckAddClearRetrans),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckFailRetrans", CheckFailRetrans),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendMessage", CheckResendMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckSendStandaloneAckMessage", CheckSendStandaloneAckMessage),

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
    chip::Platform::MemoryShutdown();
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

CHIP_REGISTER_TEST_SUITE(TestReliableMessageProtocol)
