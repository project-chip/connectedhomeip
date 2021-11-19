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
 *      This file implements unit tests for the ExchangeManager implementation.
 */

#include "TestMessagingLayer.h"

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <transport/SessionManager.h>
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

using TestContext = chip::Test::MessagingContext;

enum : uint8_t
{
    kMsgType_TEST1 = 1,
    kMsgType_TEST2 = 2,
};

TestContext sContext;

TransportMgr<Test::LoopbackTransport> gTransportMgr;
Test::IOContext gIOContext;

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    bool IsOnMessageReceivedCalled = false;
};

class WaitForTimeoutDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override { IsOnResponseTimeoutCalled = true; }

    bool IsOnResponseTimeoutCalled = false;
};

void CheckNewContextTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * ec1 = ctx.NewExchangeToBob(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, ec1 != nullptr);
    NL_TEST_ASSERT(inSuite, ec1->IsInitiator() == true);
    NL_TEST_ASSERT(inSuite, ec1->GetExchangeId() != 0);
    auto sessionPeerToLocal = ctx.GetSecureSessionManager().GetSecureSession(ec1->GetSessionHandle());
    NL_TEST_ASSERT(inSuite, sessionPeerToLocal->GetPeerNodeId() == ctx.GetBobNodeId());
    NL_TEST_ASSERT(inSuite, sessionPeerToLocal->GetPeerSessionId() == ctx.GetBobKeyId());
    NL_TEST_ASSERT(inSuite, ec1->GetDelegate() == &mockAppDelegate);

    ExchangeContext * ec2 = ctx.NewExchangeToAlice(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, ec2 != nullptr);
    NL_TEST_ASSERT(inSuite, ec2->GetExchangeId() > ec1->GetExchangeId());
    auto sessionLocalToPeer = ctx.GetSecureSessionManager().GetSecureSession(ec2->GetSessionHandle());
    NL_TEST_ASSERT(inSuite, sessionLocalToPeer->GetPeerNodeId() == ctx.GetAliceNodeId());
    NL_TEST_ASSERT(inSuite, sessionLocalToPeer->GetPeerSessionId() == ctx.GetAliceKeyId());

    ec1->Close();
    ec2->Close();
}

void CheckSessionExpirationBasics(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate sendDelegate;
    ExchangeContext * ec1 = ctx.NewExchangeToBob(&sendDelegate);

    // Expire the session this exchange is supposedly on.
    ctx.GetExchangeManager().ExpireExchangesForSession(ec1->GetSessionHandle());

    MockAppDelegate receiveDelegate;
    CHIP_ERROR err =
        ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, kMsgType_TEST1, &receiveDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST1, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                           SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, !receiveDelegate.IsOnMessageReceivedCalled);
    ec1->Close();

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, kMsgType_TEST1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void CheckSessionExpirationTimeout(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    WaitForTimeoutDelegate sendDelegate;
    ExchangeContext * ec1 = ctx.NewExchangeToBob(&sendDelegate);

    ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST1, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kExpectResponse).Set(Messaging::SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, !sendDelegate.IsOnResponseTimeoutCalled);

    // Expire the session this exchange is supposedly on.  This should close the
    // exchange.
    ctx.GetExchangeManager().ExpireExchangesForSession(ec1->GetSessionHandle());
    NL_TEST_ASSERT(inSuite, sendDelegate.IsOnResponseTimeoutCalled);
}

void CheckUmhRegistrationTest(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR err;
    MockAppDelegate mockAppDelegate;

    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::Echo::Id, kMsgType_TEST1, &mockAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::Echo::Id);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::Echo::Id, kMsgType_TEST1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::Echo::Id, kMsgType_TEST2);
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
}

void CheckExchangeMessages(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR err;

    // create solicited exchange
    MockAppDelegate mockSolicitedAppDelegate;
    ExchangeContext * ec1 = ctx.NewExchangeToAlice(&mockSolicitedAppDelegate);

    // create unsolicited exchange
    MockAppDelegate mockUnsolicitedAppDelegate;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, kMsgType_TEST1,
                                                                            &mockUnsolicitedAppDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // send a malicious packet
    ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST2, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, !mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    ec1 = ctx.NewExchangeToAlice(&mockSolicitedAppDelegate);

    // send a good packet
    ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST1, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, kMsgType_TEST1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ExchangeMgr::NewContext",               CheckNewContextTest),
    NL_TEST_DEF("Test ExchangeMgr::CheckUmhRegistrationTest", CheckUmhRegistrationTest),
    NL_TEST_DEF("Test ExchangeMgr::CheckExchangeMessages",    CheckExchangeMessages),
    NL_TEST_DEF("Test OnConnectionExpired basics",            CheckSessionExpirationBasics),
    NL_TEST_DEF("Test OnConnectionExpired timeout handling",  CheckSessionExpirationTimeout),

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
    // Initialize System memory and resources
    VerifyOrReturnError(chip::Platform::MemoryInit() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gIOContext.Init() == CHIP_NO_ERROR, FAILURE);
    VerifyOrReturnError(gTransportMgr.Init("LOOPBACK") == CHIP_NO_ERROR, FAILURE);

    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&gTransportMgr, &gIOContext) == CHIP_NO_ERROR, FAILURE);

    return SUCCESS;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    gIOContext.Shutdown();
    chip::Platform::MemoryShutdown();
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

CHIP_REGISTER_TEST_SUITE(TestExchangeMgr);
