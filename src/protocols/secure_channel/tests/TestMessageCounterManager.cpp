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
 *      This file implements unit tests for the MessageCounterManager implementation.
 */

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::LoopbackMessagingContext;

const char PAYLOAD[] = "Hello!";

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && msgBuf) override
    {
        ++ReceiveHandlerCallCount;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    int ReceiveHandlerCallCount = 0;
};

void MessageCounterSyncProcess(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    SessionHandle localSession = ctx.GetSessionBobToAlice();
    SessionHandle peerSession  = ctx.GetSessionAliceToBob();

    Transport::SecureSession * localState = ctx.GetSecureSessionManager().GetSecureSession(localSession);
    Transport::SecureSession * peerState  = ctx.GetSecureSessionManager().GetSecureSession(peerSession);

    localState->GetSessionMessageCounter().GetPeerMessageCounter().Reset();
    err = ctx.GetMessageCounterManager().SendMsgCounterSyncReq(localSession, localState);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    MessageCounter & peerCounter      = peerState->GetSessionMessageCounter().GetLocalMessageCounter();
    PeerMessageCounter & localCounter = localState->GetSessionMessageCounter().GetPeerMessageCounter();
    NL_TEST_ASSERT(inSuite, localCounter.IsSynchronized());
    NL_TEST_ASSERT(inSuite, localCounter.GetCounter() == peerCounter.Value());
}

void CheckReceiveMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    CHIP_ERROR err    = CHIP_NO_ERROR;

    SessionHandle peerSession            = ctx.GetSessionAliceToBob();
    Transport::SecureSession * peerState = ctx.GetSecureSessionManager().GetSecureSession(peerSession);
    peerState->GetSessionMessageCounter().GetPeerMessageCounter().Reset();

    MockAppDelegate callback;
    ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(chip::Protocols::Echo::MsgType::EchoRequest, &callback);

    uint16_t payload_len              = sizeof(PAYLOAD);
    System::PacketBufferHandle msgBuf = MessagePacketBuffer::NewWithData(PAYLOAD, payload_len);
    NL_TEST_ASSERT(inSuite, !msgBuf.IsNull());

    Messaging::ExchangeContext * ec = ctx.NewExchangeToAlice(nullptr);
    NL_TEST_ASSERT(inSuite, ec != nullptr);

    err = ec->SendMessage(chip::Protocols::Echo::MsgType::EchoRequest, std::move(msgBuf),
                          Messaging::SendFlags{ Messaging::SendMessageFlags::kNoAutoRequestAck });
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, peerState->GetSessionMessageCounter().GetPeerMessageCounter().IsSynchronized());
    NL_TEST_ASSERT(inSuite, callback.ReceiveHandlerCallCount == 1);
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test MessageCounterManager::MessageCounterSyncProcess", MessageCounterSyncProcess),
    NL_TEST_DEF("Test MessageCounterManager::ReceiveMessage", CheckReceiveMessage),
    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-MessageCounterManager",
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
    auto * ctx = static_cast<TestContext *>(aContext);
    VerifyOrReturnError(ctx->Init(&sSuite) == CHIP_NO_ERROR, FAILURE);

    return SUCCESS;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    reinterpret_cast<TestContext *>(aContext)->Shutdown();
    return SUCCESS;
}

} // namespace

/**
 *  Main
 */
int TestMessageCounterManager()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestMessageCounterManager);
