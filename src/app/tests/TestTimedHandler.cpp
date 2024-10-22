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
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/support/UnitTestUtils.h>
#include <protocols/interaction_model/Constants.h>
#include <pw_unit_test/framework.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

namespace chip {
namespace app {

using namespace Messaging;
using namespace Protocols::InteractionModel;

namespace {

class TestTimedHandler : public chip::Test::AppContext
{
public:
    void TestFollowingMessageFastEnough(MsgType aMsgType);
    void TestFollowingMessageTooSlow(MsgType aMsgType);
    void GenerateTimedRequest(uint16_t aTimeoutValue, System::PacketBufferHandle & aPayload);
};

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

void TestTimedHandler::GenerateTimedRequest(uint16_t aTimeoutValue, System::PacketBufferHandle & aPayload)
{
    aPayload = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize);
    ASSERT_FALSE(aPayload.IsNull());

    System::PacketBufferTLVWriter writer;
    writer.Init(std::move(aPayload));

    TimedRequestMessage::Builder builder;
    EXPECT_EQ(builder.Init(&writer), CHIP_NO_ERROR);

    builder.TimeoutMs(aTimeoutValue);
    EXPECT_EQ(builder.GetError(), CHIP_NO_ERROR);

    EXPECT_EQ(writer.Finalize(&aPayload), CHIP_NO_ERROR);
}

void TestTimedHandler::TestFollowingMessageFastEnough(MsgType aMsgType)
{

    System::PacketBufferHandle payload;
    GenerateTimedRequest(500, payload);

    TestExchangeDelegate delegate;
    ExchangeContext * exchange = NewExchangeToAlice(&delegate);
    ASSERT_NE(exchange, nullptr);

    EXPECT_FALSE(delegate.mNewMessageReceived);

    delegate.mKeepExchangeOpen = true;

    EXPECT_EQ(exchange->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse), CHIP_NO_ERROR);

    DrainAndServiceIO();
    EXPECT_TRUE(delegate.mNewMessageReceived);
    EXPECT_TRUE(delegate.mLastMessageWasStatus);
    EXPECT_EQ(delegate.mError, CHIP_NO_ERROR);

    // Send an empty payload, which will error out but not with the
    // TIMEOUT status we expect if we miss our timeout.
    payload = MessagePacketBuffer::New(0);
    ASSERT_FALSE(payload.IsNull());

    delegate.mKeepExchangeOpen   = false;
    delegate.mNewMessageReceived = false;

    EXPECT_EQ(exchange->SendMessage(aMsgType, std::move(payload), SendMessageFlags::kExpectResponse), CHIP_NO_ERROR);

    DrainAndServiceIO();
    EXPECT_TRUE(delegate.mNewMessageReceived);
    EXPECT_TRUE(delegate.mLastMessageWasStatus);
    EXPECT_NE(StatusIB(delegate.mError).mStatus, Status::Timeout);
}

TEST_F(TestTimedHandler, TestInvokeFastEnough)
{
    TestFollowingMessageFastEnough(MsgType::InvokeCommandRequest);
}

TEST_F(TestTimedHandler, TestWriteFastEnough)
{
    TestFollowingMessageFastEnough(MsgType::WriteRequest);
}

void TestTimedHandler::TestFollowingMessageTooSlow(MsgType aMsgType)
{

    System::PacketBufferHandle payload;
    GenerateTimedRequest(50, payload);

    TestExchangeDelegate delegate;
    ExchangeContext * exchange = NewExchangeToAlice(&delegate);
    ASSERT_NE(exchange, nullptr);

    EXPECT_FALSE(delegate.mNewMessageReceived);

    delegate.mKeepExchangeOpen = true;

    EXPECT_EQ(exchange->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse), CHIP_NO_ERROR);

    DrainAndServiceIO();
    EXPECT_TRUE(delegate.mNewMessageReceived);
    EXPECT_TRUE(delegate.mLastMessageWasStatus);
    EXPECT_EQ(delegate.mError, CHIP_NO_ERROR);

    // Sleep for > 50ms so we miss our time window.
    chip::test_utils::SleepMillis(75);

    // Send an empty payload, which will error out but not with the
    // TIMEOUT status we expect if we miss our timeout.
    payload = MessagePacketBuffer::New(0);
    EXPECT_FALSE(payload.IsNull());

    delegate.mKeepExchangeOpen   = false;
    delegate.mNewMessageReceived = false;

    EXPECT_EQ(exchange->SendMessage(aMsgType, std::move(payload), SendMessageFlags::kExpectResponse), CHIP_NO_ERROR);

    DrainAndServiceIO();
    EXPECT_TRUE(delegate.mNewMessageReceived);
    EXPECT_TRUE(delegate.mLastMessageWasStatus);
    EXPECT_EQ(StatusIB(delegate.mError).mStatus, Status::Timeout);
}

TEST_F(TestTimedHandler, TestInvokeTooSlow)
{
    TestFollowingMessageTooSlow(MsgType::InvokeCommandRequest);
}

// TEST(TestTimedHandler, TestTimedHandler::TestWriteTooSlow)
TEST_F(TestTimedHandler, TestWriteTooSlow)
{
    TestFollowingMessageTooSlow(MsgType::WriteRequest);
}

TEST_F(TestTimedHandler, TestInvokeNeverComes)
{

    System::PacketBufferHandle payload;
    GenerateTimedRequest(50, payload);

    TestExchangeDelegate delegate;
    ExchangeContext * exchange = NewExchangeToAlice(&delegate);
    ASSERT_NE(exchange, nullptr);

    EXPECT_FALSE(delegate.mNewMessageReceived);

    EXPECT_EQ(exchange->SendMessage(MsgType::TimedRequest, std::move(payload), SendMessageFlags::kExpectResponse), CHIP_NO_ERROR);

    DrainAndServiceIO();
    EXPECT_TRUE(delegate.mNewMessageReceived);
    EXPECT_TRUE(delegate.mLastMessageWasStatus);
    EXPECT_EQ(delegate.mError, CHIP_NO_ERROR);

    // Do nothing else; exchange on the server remains open.  We are testing to
    // see whether shutdown cleans it up properly.
}

} // namespace app
} // namespace chip
