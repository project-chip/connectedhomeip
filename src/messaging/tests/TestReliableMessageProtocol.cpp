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
#include <support/CodeUtils.h>
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

using TestContext = chip::Test::IOContext;

TestContext sContext;

constexpr NodeId kSourceNodeId = 123654;

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, const PeerAddress & address, System::PacketBuffer * msgBuf) override
    {
        System::PacketBufferHandle msg_ForNow;
        msg_ForNow.Adopt(msgBuf);
        HandleMessageReceived(header, address, std::move(msg_ForNow));
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

    ExchangeContext * exchange = exchangeMgr.NewContext(kSourceNodeId, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageManager::RetransTableEntry * entry;
    PayloadHeader header;

    rm->AddToRetransTable(rc, header, 1, nullptr, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    rm->ClearRetransmitTable(*entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void CheckFailRetrans(nlTestSuite * inSuite, void * inContext)
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

    ExchangeContext * exchange = exchangeMgr.NewContext(kSourceNodeId, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageManager::RetransTableEntry * entry;
    PayloadHeader header;
    ReliableMessageDelegateObject delegate;
    rc->SetDelegate(&delegate);
    auto buf = System::PacketBuffer::New();
    rm->AddToRetransTable(rc, header, 1, buf.Release_ForNow(), &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    rm->FailRetransmitTableEntries(rc, CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
}

void CheckRetransExpire(nlTestSuite * inSuite, void * inContext)
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

    ExchangeContext * exchange = exchangeMgr.NewContext(kSourceNodeId, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageDelegateObject delegate;
    rc->SetDelegate(&delegate);
    rc->SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
        2, // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
    });

    ReliableMessageManager::RetransTableEntry * entry;
    PayloadHeader header;
    auto buf = System::PacketBuffer::New();
    rm->AddToRetransTable(rc, header, 1, buf.Release_ForNow(), &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    test_os_sleep_ms(20);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    // 1st retrans

    test_os_sleep_ms(20);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    // 2nd retrans

    test_os_sleep_ms(40);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
    // send error
}

void CheckDelayDelivery(nlTestSuite * inSuite, void * inContext)
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

    ExchangeContext * exchange = exchangeMgr.NewContext(kSourceNodeId, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageManager * rm = exchangeMgr.GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageDelegateObject delegate;
    rc->SetDelegate(&delegate);
    rc->SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
    });

    ReliableMessageManager::RetransTableEntry * entry;
    PayloadHeader header;
    auto buf = System::PacketBuffer::New();
    rm->AddToRetransTable(rc, header, 1, buf.Release_ForNow(), &entry);
    rm->ProcessDelayedDeliveryMessage(64, kSourceNodeId);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    test_os_sleep_ms(50);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    // not send, delayed

    test_os_sleep_ms(50);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    // 1st retrans

    test_os_sleep_ms(50);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
    // send error
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
    NL_TEST_DEF("Test ReliableMessageManager::CheckRetransExpire", CheckRetransExpire),
    NL_TEST_DEF("Test ReliableMessageManager::CheckDelayDelivery", CheckDelayDelivery),

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
