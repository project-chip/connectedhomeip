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
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageManager.h>
#include <protocols/CHIPProtocols.h>
#include <support/CodeUtils.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::messaging;

using TestContext = chip::Test::IOContext;

TestContext sContext;

ReliableMessageManager manager;

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
    auto & m          = manager;
    m.Init(ctx.GetSystemLayer());
    ReliableMessageContext rc;
    rc.Init(&m);
    ReliableMessageManager::RetransTableEntry * entry;
    m.AddToRetransTable(&rc, nullptr, 1, 0, &entry);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);
    m.ClearRetransmitTable(*entry);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 0);
    m.Shutdown();
}

void CheckFailRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    auto & m          = manager;
    m.Init(ctx.GetSystemLayer());
    ReliableMessageContext rc;
    rc.Init(&m);
    ReliableMessageDelegateObject delegate;
    rc.SetDelegate(&delegate);
    ReliableMessageManager::RetransTableEntry * entry;
    auto buf = System::PacketBuffer::New();
    m.AddToRetransTable(&rc, buf, 1, 0, &entry);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    m.FailRetransmitTableEntries(&rc, CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
    m.Shutdown();
}

void CheckRetransExpire(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    auto & m          = manager;
    m.TestSetIntervalShift(4); // 16ms per tick
    m.Init(ctx.GetSystemLayer());
    ReliableMessageContext rc;
    rc.Init(&m);
    ReliableMessageDelegateObject delegate;
    rc.SetDelegate(&delegate);
    rc.SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
        2, // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
    });
    ReliableMessageManager::RetransTableEntry * entry;
    auto buf = System::PacketBuffer::New();
    m.AddToRetransTable(&rc, buf, 1, 0, &entry);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);

    test_os_sleep_ms(20);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), &m, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);
    // 1st retrans

    test_os_sleep_ms(20);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), &m, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    // 2nd retrans

    test_os_sleep_ms(20);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), &m, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
    // send error

    m.Shutdown();
}

void CheckDelayDelivery(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    auto & m          = manager;
    m.TestSetIntervalShift(4); // 16ms per tick
    m.Init(ctx.GetSystemLayer());
    ReliableMessageContext rc;
    rc.Init(&m);
    ReliableMessageDelegateObject delegate;
    rc.SetDelegate(&delegate);
    rc.SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRANS_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_ACK_TIMEOUT_TICK
        1, // CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS
    });
    ReliableMessageManager::RetransTableEntry * entry;
    auto buf = System::PacketBuffer::New();
    m.AddToRetransTable(&rc, buf, 1, 0, &entry);
    m.ProcessDelayedDeliveryMessage(&rc, 64);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);

    test_os_sleep_ms(50);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), &m, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);
    // not send, delayed

    test_os_sleep_ms(50);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), &m, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !delegate.SendErrorCalled);
    // 1st retrans

    test_os_sleep_ms(20);
    ReliableMessageManager::Timeout(&ctx.GetSystemLayer(), &m, CHIP_SYSTEM_NO_ERROR);
    NL_TEST_ASSERT(inSuite, m.TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, delegate.SendErrorCalled);
    // send error

    m.Shutdown();
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

namespace chip {
namespace messaging {

// Stub implementation
CHIP_ERROR ReliableMessageManager::SendMessage(ReliableMessageContext * context, System::PacketBuffer * msgBuf, uint16_t sendFlags)
{
    return CHIP_NO_ERROR;
}
CHIP_ERROR ReliableMessageManager::SendMessage(ReliableMessageContext * context, uint32_t profileId, uint8_t msgType,
                                               System::PacketBuffer * msgBuf, BitFlags<uint16_t, SendMessageFlags> sendFlags)
{
    return CHIP_NO_ERROR;
}
void ReliableMessageManager::FreeContext(ReliableMessageContext *) {}

} // namespace messaging
} // namespace chip

/**
 *  Main
 */
int TestReliableMessageProtocol()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
