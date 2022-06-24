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

#include <lib/core/CHIPCore.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>
#include <utility>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;

using TestContext = Test::LoopbackMessagingContext;

enum : uint8_t
{
    kMsgType_TEST1 = 1,
    kMsgType_TEST2 = 2,
};

class MockAppDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

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
    NL_TEST_ASSERT(inSuite, ec1->GetSessionHandle() == ctx.GetSessionAliceToBob());
    NL_TEST_ASSERT(inSuite, ec1->GetDelegate() == &mockAppDelegate);

    ExchangeContext * ec2 = ctx.NewExchangeToAlice(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, ec2 != nullptr);
    NL_TEST_ASSERT(inSuite, ec2->GetExchangeId() > ec1->GetExchangeId());
    NL_TEST_ASSERT(inSuite, ec2->GetSessionHandle() == ctx.GetSessionBobToAlice());

    ec1->Close();
    ec2->Close();
}

void CheckSessionExpirationBasics(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate sendDelegate;
    ExchangeContext * ec1 = ctx.NewExchangeToBob(&sendDelegate);

    // Expire the session this exchange is supposedly on.
    ec1->GetSessionHandle()->AsSecureSession()->MarkForEviction();

    MockAppDelegate receiveDelegate;
    CHIP_ERROR err =
        ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, kMsgType_TEST1, &receiveDelegate);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST1, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                           SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, err != CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, !receiveDelegate.IsOnMessageReceivedCalled);
    ec1->Close();

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, kMsgType_TEST1);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // recreate closed session.
    NL_TEST_ASSERT(inSuite, ctx.CreateSessionAliceToBob() == CHIP_NO_ERROR);
}

void CheckSessionExpirationTimeout(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    WaitForTimeoutDelegate sendDelegate;
    ExchangeContext * ec1 = ctx.NewExchangeToBob(&sendDelegate);

    ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST1, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kExpectResponse).Set(Messaging::SendMessageFlags::kNoAutoRequestAck));

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, !sendDelegate.IsOnResponseTimeoutCalled);

    // Expire the session this exchange is supposedly on.  This should close the exchange.
    ec1->GetSessionHandle()->AsSecureSession()->MarkForEviction();
    NL_TEST_ASSERT(inSuite, sendDelegate.IsOnResponseTimeoutCalled);

    // recreate closed session.
    NL_TEST_ASSERT(inSuite, ctx.CreateSessionAliceToBob() == CHIP_NO_ERROR);
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

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(inSuite, !mockUnsolicitedAppDelegate.IsOnMessageReceivedCalled);

    ec1 = ctx.NewExchangeToAlice(&mockSolicitedAppDelegate);

    // send a good packet
    ec1->SendMessage(Protocols::BDX::Id, kMsgType_TEST1, System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                     SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));

    ctx.DrainAndServiceIO();
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

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-ExchangeManager",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // namespace

/**
 *  Main
 */
int TestExchangeMgr()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestExchangeMgr);
