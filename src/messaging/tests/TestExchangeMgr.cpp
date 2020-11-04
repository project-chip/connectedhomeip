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
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;

using TestContext = chip::Test::IOContext;

TestContext sContext;

constexpr NodeId kSourceNodeId      = 123654;
constexpr NodeId kDestinationNodeId = 111222333;

class LoopbackTransport : public Transport::Base
{
public:
    /// Transports are required to have a constructor that takes exactly one argument
    CHIP_ERROR Init(const char * unused) { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(const PacketHeader & header, Header::Flags payloadFlags, const PeerAddress & address,
                           System::PacketBuffer * msgBuf) override
    {
        HandleMessageReceived(header, address, msgBuf);
        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }
};

class MockAppDelegate : public ExchangeContextDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                           System::PacketBuffer * buffer) override
    {}
    void OnResponseTimeout(ExchangeContext * ec) override {}
} gMockAppDelegate;

void CheckSimpleInitTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    SecureSessionMgr<LoopbackTransport> conn;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = conn.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), "LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&conn);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckNewContextTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    SecureSessionMgr<LoopbackTransport> conn;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = conn.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), "LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&conn);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeContext * ec1 = exchangeMgr.NewContext(kSourceNodeId, (ExchangeContextDelegate *) 0x1234);
    NL_TEST_ASSERT(inSuite, ec1 != nullptr);
    NL_TEST_ASSERT(inSuite, ec1->IsInitiator() == true);
    NL_TEST_ASSERT(inSuite, ec1->GetExchangeId() != 0);
    NL_TEST_ASSERT(inSuite, ec1->GetPeerNodeId() == kSourceNodeId);
    NL_TEST_ASSERT(inSuite, ec1->GetDelegate() == (void *) 0x1234);

    ExchangeContext * ec2 = exchangeMgr.NewContext(kDestinationNodeId, (ExchangeContextDelegate *) 0x2345);
    NL_TEST_ASSERT(inSuite, ec2 != nullptr);
    NL_TEST_ASSERT(inSuite, ec2->GetExchangeId() > ec1->GetExchangeId());
    NL_TEST_ASSERT(inSuite, ec2->GetPeerNodeId() == kDestinationNodeId);
}

void CheckFindContextTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    SecureSessionMgr<LoopbackTransport> conn;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = conn.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), "LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&conn);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeContext * ec = exchangeMgr.NewContext(kDestinationNodeId, nullptr);
    NL_TEST_ASSERT(inSuite, ec != nullptr);

    bool result = exchangeMgr.FindContext(kDestinationNodeId, nullptr, true);
    NL_TEST_ASSERT(inSuite, result == true);

    result = exchangeMgr.FindContext(kDestinationNodeId, nullptr, false);
    NL_TEST_ASSERT(inSuite, result == false);
}

void CheckUmhRegistrationTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    SecureSessionMgr<LoopbackTransport> conn;
    CHIP_ERROR err;

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    err = conn.Init(kSourceNodeId, ctx.GetInetLayer().SystemLayer(), "LOOPBACK");
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ExchangeManager exchangeMgr;
    err = exchangeMgr.Init(&conn);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = exchangeMgr.RegisterUnsolicitedMessageHandler(0x0001, &gMockAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = exchangeMgr.RegisterUnsolicitedMessageHandler(0x0002, 0x0001, &gMockAppDelegate);
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

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ExchangeMgr::Init",                     CheckSimpleInitTest),
    NL_TEST_DEF("Test ExchangeMgr::NewContext",               CheckNewContextTest),
    NL_TEST_DEF("Test ExchangeMgr::FindContext",              CheckFindContextTest),
    NL_TEST_DEF("Test ExchangeMgr::CheckUmhRegistrationTest", CheckUmhRegistrationTest),

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
