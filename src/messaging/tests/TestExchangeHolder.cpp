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

#include "messaging/ExchangeDelegate.h"
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeHolder.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>

namespace chip {
namespace Protocols {

//
// Let's create a mock protocol that encapsulates a 3 message exchange to test out the ExchangeHolder
// and the various states the underlying exchange might be set to, altering the clean-up behavior
// the holder will execute depending on those states.
//
namespace MockProtocol {
static constexpr Id Id(VendorId::TestVendor1, 1);

enum class MessageType : uint8_t
{
    kMsg1 = 0x01,
    kMsg2 = 0x02,
    kMsg3 = 0x03
};
} // namespace MockProtocol

template <>
struct MessageTypeTraits<MockProtocol::MessageType>
{
    static constexpr const Protocols::Id & ProtocolId() { return MockProtocol::Id; }
};

} // namespace Protocols
} // namespace chip

namespace {

using namespace chip;
using namespace chip::Messaging;
using namespace chip::System;
using namespace chip::Protocols;

using TestContext = Test::LoopbackMessagingContext;

TestContext * gCtx = nullptr;

class MockProtocolResponder : public ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    enum class BehaviorModifier
    {
        kNone,
        kHoldMsg1,
    };

    MockProtocolResponder(BehaviorModifier modifier = BehaviorModifier::kNone) : mExchangeCtx(*this)
    {
        VerifyOrDie(gCtx != nullptr);
        mBehaviorModifier = modifier;
        gCtx->GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::MockProtocol::Id, this);
    }

    ~MockProtocolResponder()
    {
        gCtx->GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::MockProtocol::Id);
    }

    bool DidInteractionSucceed() { return mInteractionSucceeded; }

private:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override;

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    ExchangeHolder mExchangeCtx;
    BehaviorModifier mBehaviorModifier = BehaviorModifier::kNone;
    bool mInteractionSucceeded         = false;
};

class MockProtocolInitiator : public ExchangeDelegate
{
public:
    enum class BehaviorModifier
    {
        kNone,
        kHoldMsg2,
    };

    MockProtocolInitiator(BehaviorModifier modifier = BehaviorModifier::kNone) : mExchangeCtx(*this)
    {
        mBehaviorModifier = modifier;
    }

    CHIP_ERROR StartInteraction(SessionHandle & sessionHandle);

    bool DidInteractionSucceed() { return mInteractionSucceeded; }

private:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override;

    void OnResponseTimeout(ExchangeContext * ec) override {}

    ExchangeHolder mExchangeCtx;
    BehaviorModifier mBehaviorModifier = BehaviorModifier::kNone;
    bool mInteractionSucceeded         = false;
};

CHIP_ERROR MockProtocolResponder::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                    System::PacketBufferHandle && buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (payloadHeader.HasMessageType(chip::Protocols::MockProtocol::MessageType::kMsg1))
    {
        //
        // This is the first message in the exchange - let's have our holder start managing the exchange by grabbing it.
        //
        mExchangeCtx.Grab(ec);

        if (mBehaviorModifier != BehaviorModifier::kHoldMsg1)
        {
            PacketBufferHandle respBuffer = MessagePacketBuffer::New(0);
            VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);
            ReturnErrorOnFailure(mExchangeCtx->SendMessage(chip::Protocols::MockProtocol::MessageType::kMsg2, std::move(respBuffer),
                                                           SendMessageFlags::kExpectResponse));
        }
        else
        {
            mExchangeCtx->WillSendMessage();
        }
    }
    else if (payloadHeader.HasMessageType(chip::Protocols::MockProtocol::MessageType::kMsg3))
    {
        mInteractionSucceeded = true;
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

    return err;
}

CHIP_ERROR MockProtocolInitiator::StartInteraction(SessionHandle & sessionHandle)
{
    PacketBufferHandle buffer = MessagePacketBuffer::New(0);
    VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

    auto exchange = gCtx->GetExchangeManager().NewContext(sessionHandle, this);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    //
    // This is the first exchange in this interaction - let's have our holder start managing the exchange by grabbing it.
    //
    mExchangeCtx.Grab(exchange);

    ReturnErrorOnFailure(mExchangeCtx->SendMessage(chip::Protocols::MockProtocol::MessageType::kMsg1, std::move(buffer),
                                                   SendMessageFlags::kExpectResponse));

    return CHIP_NO_ERROR;
}

CHIP_ERROR MockProtocolInitiator::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                    System::PacketBufferHandle && buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (payloadHeader.HasMessageType(chip::Protocols::MockProtocol::MessageType::kMsg2))
    {
        if (mBehaviorModifier != BehaviorModifier::kHoldMsg2)
        {
            PacketBufferHandle respBuffer = MessagePacketBuffer::New(0);
            VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);
            ReturnErrorOnFailure(mExchangeCtx->SendMessage(chip::Protocols::MockProtocol::MessageType::kMsg3, std::move(respBuffer),
                                                           SendMessageFlags::kNone));

            mInteractionSucceeded = true;
        }
        else
        {
            mExchangeCtx->WillSendMessage();
        }
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

    return err;
}

void TestExchangeHolder(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    gCtx = &ctx;

    auto sessionHandle = ctx.GetSessionAliceToBob();

    //
    // #1: Initiator >--- Msg1 --X  Responder.
    //
    // Initiator sends Msg1 to Responder, but we set it up such that Responder doesn't actually
    // receive the message.
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's waiting for
    // a response.
    //
    {
        ChipLogProgress(ExchangeManager, "-------- #1: Initiator >-- Msg1 --X Responder ---------");

        {
            MockProtocolInitiator initiator;
            MockProtocolResponder responder;

            auto err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        }

        //
        // Service IO AFTER the objects above cease to exist to prevent Msg1 from getting to Responder. This also
        // flush any pending messages in the queue.
        //
        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(inSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    //
    // #2: Initiator --- Msg1 -->  Responder (WillSend)
    //
    // Initiator sends Msg1 to Responder, which is received successfully. However, Responder
    // doesn't send a response right away (calls WillSendMessage() on the EC).
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's waiting for
    // a response, and so should the Responder's holder since it has yet to send a message.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #2: Initiator >-- Msg1 --> Responder (WillSend) ---------");

            MockProtocolInitiator initiator;
            MockProtocolResponder responder(MockProtocolResponder::BehaviorModifier::kHoldMsg1);

            auto err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();
        }

        NL_TEST_ASSERT(inSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    //
    // #3:            Initiator --- Msg1 -->  Responder
    //     (WillSend) Initiator <-- Msg2 <--  Responder
    //
    // Initiator receives Msg2 back from Responder, but calls WillSend on that EC.
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's waiting
    // to send a response, and Responder's holder should abort as well since it's waiting for a response.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #3: (WillSend) Initiator <-- Msg2 <-- Responder ---------");

            MockProtocolInitiator initiator(MockProtocolInitiator::BehaviorModifier::kHoldMsg2);
            MockProtocolResponder responder;

            auto err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();
        }

        NL_TEST_ASSERT(inSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    //
    // #4:            Initiator --- Msg1 -->  Responder
    //                Initiator <-- Msg2 <--  Responder
    //                Initiator >-- Msg3 -->  Responder
    //
    // Initiator sends final message in exchange to Responder, which is received successfully.
    //
    // Then, destroy both objects. Initiator's holder should NOT abort the underlying exchange since
    // it has sent the final message in the exchange, while responder's holder should NOT abor the underlying
    // exchange either since it is not going to send any further messages on the exchange.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #4: Initiator >-- Msg3 -->  Responder ---------");

            MockProtocolInitiator initiator;
            MockProtocolResponder responder;

            auto err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();
        }

        NL_TEST_ASSERT(inSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    //
    // #5:            Initiator --- Msg1 -->  Responder (WillSend)
    //                Initiator --- Msg1 -->  Responder (WillSend)
    //
    // Similar to #2, except we have Initiator start the interaction again. This validates
    // ExchangeHolder::Grab in correctly aborting a previous exchange and acquiring a new one.
    //
    // Then, destroy both objects. Both holders should abort the exchange (see #2).
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #5: Initiator >-- Msg1 -->  Responder (WillSend) X2 ---------");

            MockProtocolInitiator initiator;
            MockProtocolResponder responder(MockProtocolResponder::BehaviorModifier::kHoldMsg1);

            auto err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();

            err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();
        }

        ctx.DrainAndServiceIO();
        NL_TEST_ASSERT(inSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }

    //
    // #6:            Initiator --- Msg1 -->  Responder
    //                Initiator <-- Msg2 <--  Responder
    //                Initiator >-- Msg3 -->  Responder
    //
    //                X2
    //
    // Similar to #4, except we do the entire interaction twice. This validates
    // ExchangeHolder::Grab in correctly releasing a reference to a previous exchange (but not aborting it)
    // and acquiring a new one.
    //
    // Then, destroy both objects. Both holders should release their reference without aborting.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #6: Initiator >-- Msg3 -->  Responder X2 ---------");

            MockProtocolInitiator initiator;
            MockProtocolResponder responder;

            auto err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();

            err = initiator.StartInteraction(sessionHandle);
            NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

            ctx.DrainAndServiceIO();
        }

        NL_TEST_ASSERT(inSuite, ctx.GetExchangeManager().GetNumActiveExchanges() == 0);
    }
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestExchangeHolder", TestExchangeHolder),

    NL_TEST_SENTINEL()
};
// clang-format on

// clang-format off
nlTestSuite sSuite =
{
    "Test-TestExchangeHolder",
    &sTests[0],
    TestContext::Initialize,
    TestContext::Finalize
};
// clang-format on

} // anonymous namespace

/**
 *  Main
 */
int TestExchangeHolder()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestExchangeHolder);
