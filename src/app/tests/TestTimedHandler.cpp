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

#include <app/InteractionModelEngine.h>
#include <app/MessageDef/TimedRequestMessage.h>
#include <app/StatusResponse.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/TLV.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <protocols/interaction_model/Constants.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

#include <nlunit-test.h>

using TestContext = chip::Test::AppContext;

namespace chip {
namespace app {

using namespace Messaging;
using namespace Protocols::InteractionModel;

class TestTimedHandler
{
public:
    static void TestInvokeFastEnough(nlTestSuite * aSuite, void * aContext);
    static void TestWriteFastEnough(nlTestSuite * aSuite, void * aContext);

    static void TestInvokeTooSlow(nlTestSuite * aSuite, void * aContext);
    static void TestWriteTooSlow(nlTestSuite * aSuite, void * aContext);

    static void TestInvokeNeverComes(nlTestSuite * aSuite, void * aContext);

private:
    static void TestFollowingMessageFastEnough(nlTestSuite * aSuite, void * aContext, MsgType aMsgType);
    static void TestFollowingMessageTooSlow(nlTestSuite * aSuite, void * aContext, MsgType aMsgType);

    static void GenerateTimedRequest(nlTestSuite * aSuite, uint16_t aTimeoutValue, System::PacketBufferHandle & aPayload);
};

namespace {

class TestExchangeDelegate : public Messaging::ExchangeDelegate
{
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * aExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override
    {
        mNewMessageReceived   = true;
        mLastMessageWasStatus = aPayloadHeader.HasMessageType(MsgType::StatusResponse);
        if (mLastMessageWasStatus)
        {
            CHIP_ERROR statusError = CHIP_NO_ERROR;
            mError                 = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError);
            if (mError == CHIP_NO_ERROR)
            {
                mError = statusError;
            }
        }
        if (mKeepExchangeOpen)
        {
            aExchangeContext->WillSendMessage();
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext *) override {}

public:
    bool mKeepExchangeOpen     = false;
    bool mNewMessageReceived   = false;
    bool mLastMessageWasStatus = false;
    CHIP_ERROR mError          = CHIP_NO_ERROR;
};

} // anonymous namespace

void TestTimedHandler::GenerateTimedRequest(nlTestSuite * aSuite, uint16_t aTimeoutValue, System::PacketBufferHandle & aPayload)
{
    aPayload = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    NL_TEST_ASSERT(aSuite, !aPayload.IsNull());

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    TimedRequestMessage::Builder builder;
    CHIP_ERROR err = builder.Init(&writer);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    builder.TimeoutMs(aTimeoutValue);
    NL_TEST_ASSERT(aSuite, builder.GetError() == CHIP_NO_ERROR);

    err = writer.Finalize(&aPayload);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);
}

void TestTimedHandler::TestFollowingMessageFastEnough(nlTestSuite * aSuite, void * aContext, MsgType aMsgType)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);

    System::PacketBufferHandle payload;
    GenerateTimedRequest(aSuite, 500, payload);

    TestExchangeDelegate delegate;
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&delegate);
    NL_TEST_ASSERT(aSuite, exchange != nullptr);

    NL_TEST_ASSERT(aSuite, !delegate.mNewMessageReceived);

    delegate.mKeepExchangeOpen = true;

    CHIP_ERROR err = exchange->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(aSuite, delegate.mNewMessageReceived);
    NL_TEST_ASSERT(aSuite, delegate.mLastMessageWasStatus);
    NL_TEST_ASSERT(aSuite, delegate.mError == CHIP_NO_ERROR);

    // Send an empty payload, which will error out but not with the
    // UNSUPPORTED_ACCESS status we expect if we miss our timeout.
    payload = MessagePacketBuffer::New(0);
    NL_TEST_ASSERT(aSuite, !payload.IsNull());

    delegate.mKeepExchangeOpen   = false;
    delegate.mNewMessageReceived = false;

    err = exchange->SendMessage(aMsgType, std::move(payload), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(aSuite, delegate.mNewMessageReceived);
    NL_TEST_ASSERT(aSuite, delegate.mLastMessageWasStatus);
    NL_TEST_ASSERT(aSuite, StatusIB(delegate.mError).mStatus != Status::UnsupportedAccess);
}

void TestTimedHandler::TestInvokeFastEnough(nlTestSuite * aSuite, void * aContext)
{
    TestFollowingMessageFastEnough(aSuite, aContext, MsgType::InvokeCommandRequest);
}

void TestTimedHandler::TestWriteFastEnough(nlTestSuite * aSuite, void * aContext)
{
    TestFollowingMessageFastEnough(aSuite, aContext, MsgType::WriteRequest);
}

void TestTimedHandler::TestFollowingMessageTooSlow(nlTestSuite * aSuite, void * aContext, MsgType aMsgType)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);

    System::PacketBufferHandle payload;
    GenerateTimedRequest(aSuite, 50, payload);

    TestExchangeDelegate delegate;
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&delegate);
    NL_TEST_ASSERT(aSuite, exchange != nullptr);

    NL_TEST_ASSERT(aSuite, !delegate.mNewMessageReceived);

    delegate.mKeepExchangeOpen = true;

    CHIP_ERROR err = exchange->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(aSuite, delegate.mNewMessageReceived);
    NL_TEST_ASSERT(aSuite, delegate.mLastMessageWasStatus);
    NL_TEST_ASSERT(aSuite, delegate.mError == CHIP_NO_ERROR);

    // Sleep for > 50ms so we miss our time window.
    chip::test_utils::SleepMillis(75);

    // Send an empty payload, which will error out but not with the
    // UNSUPPORTED_ACCESS status we expect if we miss our timeout.
    payload = MessagePacketBuffer::New(0);
    NL_TEST_ASSERT(aSuite, !payload.IsNull());

    delegate.mKeepExchangeOpen   = false;
    delegate.mNewMessageReceived = false;

    err = exchange->SendMessage(aMsgType, std::move(payload), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(aSuite, delegate.mNewMessageReceived);
    NL_TEST_ASSERT(aSuite, delegate.mLastMessageWasStatus);
    NL_TEST_ASSERT(aSuite, StatusIB(delegate.mError).mStatus == Status::UnsupportedAccess);
}

void TestTimedHandler::TestInvokeTooSlow(nlTestSuite * aSuite, void * aContext)
{
    TestFollowingMessageTooSlow(aSuite, aContext, MsgType::InvokeCommandRequest);
}

void TestTimedHandler::TestWriteTooSlow(nlTestSuite * aSuite, void * aContext)
{
    TestFollowingMessageTooSlow(aSuite, aContext, MsgType::WriteRequest);
}

void TestTimedHandler::TestInvokeNeverComes(nlTestSuite * aSuite, void * aContext)
{
    TestContext & ctx = *static_cast<TestContext *>(aContext);

    System::PacketBufferHandle payload;
    GenerateTimedRequest(aSuite, 50, payload);

    TestExchangeDelegate delegate;
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&delegate);
    NL_TEST_ASSERT(aSuite, exchange != nullptr);

    NL_TEST_ASSERT(aSuite, !delegate.mNewMessageReceived);

    CHIP_ERROR err = exchange->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(aSuite, err == CHIP_NO_ERROR);

    ctx.DrainAndServiceIO();
    NL_TEST_ASSERT(aSuite, delegate.mNewMessageReceived);
    NL_TEST_ASSERT(aSuite, delegate.mLastMessageWasStatus);
    NL_TEST_ASSERT(aSuite, delegate.mError == CHIP_NO_ERROR);

    // Do nothing else; exchange on the server remains open.  We are testing to
    // see whether shutdown cleans it up properly.
}

} // namespace app
} // namespace chip

namespace {

/**
 *   Test Suite. It lists all the test functions.
 */

// clang-format off
const nlTest sTests[] =
{
        NL_TEST_DEF("TimedHandlerTestInvokeFastEnough", chip::app::TestTimedHandler::TestInvokeFastEnough),
        NL_TEST_DEF("TimedHandlerTestInvokeTooSlow", chip::app::TestTimedHandler::TestInvokeTooSlow),
        NL_TEST_DEF("TimedHandlerTestInvokeNeverComes", chip::app::TestTimedHandler::TestInvokeNeverComes),
        NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "TestTimedHandler",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

} // namespace

int TestTimedHandler()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestTimedHandler)
