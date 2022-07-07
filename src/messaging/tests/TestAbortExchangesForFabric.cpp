/*
 *    Copyright (c) 2022 Project CHIP Authors
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
 *      This file implements unit tests for aborting existing exchanges (except
 *      one) for a fabric.
 */

#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/echo/Echo.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>

namespace {

using namespace chip;
using namespace chip::Messaging;
using namespace chip::System;
using namespace chip::Protocols;

using TestContext = Test::LoopbackMessagingContext;

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        mOnMessageReceivedCalled = true;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    bool mOnMessageReceivedCalled = false;
};

void CheckAbortAllButOneExchange(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    // We want to have two sessions using the same fabric id that we use for
    // creating our exchange contexts.  That lets us test exchanges on the same
    // session as the "special exchange" as well as on other sessions.
    auto & sessionManager = ctx.GetSecureSessionManager();

    // Use key ids that are not going to collide with anything else that ctx is
    // doing.
    // TODO: These should really be CASE sessions...
    SessionHolder session1;
    CHIP_ERROR err =
        sessionManager.InjectPaseSessionWithTestKey(session1, 100, ctx.GetBobFabric()->GetNodeId(), 101, ctx.GetAliceFabricIndex(),
                                                    ctx.GetBobAddress(), CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder session1Reply;
    err = sessionManager.InjectPaseSessionWithTestKey(session1Reply, 101, ctx.GetAliceFabric()->GetNodeId(), 100,
                                                      ctx.GetBobFabricIndex(), ctx.GetAliceAddress(),
                                                      CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // TODO: Ideally this would go to a different peer, but we don't have that
    // set up right now: only Alice and Bob have useful node ids and whatnot.
    SessionHolder session2;
    err =
        sessionManager.InjectPaseSessionWithTestKey(session2, 200, ctx.GetBobFabric()->GetNodeId(), 201, ctx.GetAliceFabricIndex(),
                                                    ctx.GetBobAddress(), CryptoContext::SessionRole::kInitiator);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    SessionHolder session2Reply;
    err = sessionManager.InjectPaseSessionWithTestKey(session2Reply, 201, ctx.GetAliceFabric()->GetNodeId(), 200,
                                                      ctx.GetBobFabricIndex(), ctx.GetAliceAddress(),
                                                      CryptoContext::SessionRole::kResponder);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    auto & exchangeMgr = ctx.GetExchangeManager();

    MockAppDelegate delegate;
    Echo::EchoServer server;
    err = server.Init(&exchangeMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    auto & loopback = ctx.GetLoopback();

    auto trySendMessage = [&](ExchangeContext * exchange, SendMessageFlags flags) {
        PacketBufferHandle buffer = MessagePacketBuffer::New(0);
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());
        return exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), flags);
    };

    auto sendAndDropMessage = [&](ExchangeContext * exchange, SendMessageFlags flags) {
        // Send a message on the given exchange with the given flags, make sure
        // it's not delivered.
        loopback.mNumMessagesToDrop   = 1;
        loopback.mDroppedMessageCount = 0;

        err = trySendMessage(exchange, flags);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(inSuite, !delegate.mOnMessageReceivedCalled);
        NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    };

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();

    // We want to test three possible exchange states:
    // 1) Closed but waiting for ack.
    // 2) Waiting for a response.
    // 3) Waiting for a send.
    auto * waitingForAck1 = exchangeMgr.NewContext(session1.Get().Value(), &delegate);
    NL_TEST_ASSERT(inSuite, waitingForAck1 != nullptr);
    sendAndDropMessage(waitingForAck1, SendMessageFlags::kNone);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    auto * waitingForAck2 = exchangeMgr.NewContext(session2.Get().Value(), &delegate);
    NL_TEST_ASSERT(inSuite, waitingForAck2 != nullptr);
    sendAndDropMessage(waitingForAck2, SendMessageFlags::kNone);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 2);

    auto * waitingForIncomingMessage1 = exchangeMgr.NewContext(session1.Get().Value(), &delegate);
    NL_TEST_ASSERT(inSuite, waitingForIncomingMessage1 != nullptr);
    sendAndDropMessage(waitingForIncomingMessage1, SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 3);

    auto * waitingForIncomingMessage2 = exchangeMgr.NewContext(session2.Get().Value(), &delegate);
    NL_TEST_ASSERT(inSuite, waitingForIncomingMessage2 != nullptr);
    sendAndDropMessage(waitingForIncomingMessage2, SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 4);

    auto * waitingForSend1 = exchangeMgr.NewContext(session1.Get().Value(), &delegate);
    NL_TEST_ASSERT(inSuite, waitingForSend1 != nullptr);
    waitingForSend1->WillSendMessage();

    auto * waitingForSend2 = exchangeMgr.NewContext(session2.Get().Value(), &delegate);
    NL_TEST_ASSERT(inSuite, waitingForSend2 != nullptr);
    waitingForSend2->WillSendMessage();

    // Grab handles to our sessions now, before we evict things.
    const auto & sessionHandle1 = session1.Get();
    const auto & sessionHandle2 = session2.Get();

    NL_TEST_ASSERT(inSuite, session1);
    NL_TEST_ASSERT(inSuite, session2);
    auto * specialExhange = exchangeMgr.NewContext(session1.Get().Value(), &delegate);
    specialExhange->AbortAllOtherCommunicationOnFabric();

    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, !session1);
    NL_TEST_ASSERT(inSuite, !session2);

    NL_TEST_ASSERT(inSuite, exchangeMgr.NewContext(sessionHandle1.Value(), &delegate) == nullptr);
    NL_TEST_ASSERT(inSuite, exchangeMgr.NewContext(sessionHandle2.Value(), &delegate) == nullptr);

    // Make sure we can't send messages on any of the other exchanges.
    NL_TEST_ASSERT(inSuite, trySendMessage(waitingForSend1, SendMessageFlags::kExpectResponse) != CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, trySendMessage(waitingForSend2, SendMessageFlags::kExpectResponse) != CHIP_NO_ERROR);

    // Make sure we can send a message on the special exchange.
    NL_TEST_ASSERT(inSuite, !delegate.mOnMessageReceivedCalled);
    err = trySendMessage(specialExhange, SendMessageFlags::kNone);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Should be waiting for an ack now.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ctx.DrainAndServiceIO();

    // Should not get an app-level response, since we are not expecting one.
    NL_TEST_ASSERT(inSuite, !delegate.mOnMessageReceivedCalled);
    // We should have gotten our ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    waitingForSend1->Close();
    waitingForSend2->Close();
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test aborting all but one exchange",         CheckAbortAllButOneExchange),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "Test-AbortExchangesForFabric",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // anonymous namespace

/**
 *  Main
 */
int TestAbortExchangesForFabric()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestAbortExchangesForFabric);
