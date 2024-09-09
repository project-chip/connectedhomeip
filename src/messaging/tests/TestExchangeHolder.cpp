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

#include <pw_unit_test/framework.h>

#include "messaging/ExchangeDelegate.h"
#include "system/SystemClock.h"
#include <lib/core/StringBuilderAdapters.h>
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

using TestExchangeHolder = chip::Test::LoopbackMessagingContext;

class MockProtocolResponder : public ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    enum class BehaviorModifier : uint8_t
    {
        kNone                          = 0x00,
        kHoldMsg2                      = 0x01,
        kErrMsg2                       = 0x02,
        kExpireSessionBeforeMsg2Send   = 0x04,
        kExpireSessionAfterMsg2Send    = 0x08,
        kExpireSessionAfterMsg3Receive = 0x10,
    };

    template <typename... Args>
    MockProtocolResponder(TestExchangeHolder & ctx, BehaviorModifier modifier1, Args &&... args) :
        mExchangeCtx(*this), mBehaviorModifier(modifier1, std::forward<Args>(args)...), testExchangeHolder(ctx)
    {
        testExchangeHolder.GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::MockProtocol::Id,
                                                                                             this);
        ChipLogDetail(ExchangeManager, "[%p] MockProtocolResponder: %p", this, &mExchangeCtx);
    }

    MockProtocolResponder(TestExchangeHolder & ctx, BehaviorModifier modifier = BehaviorModifier::kNone) :
        mExchangeCtx(*this), testExchangeHolder(ctx)
    {
        mBehaviorModifier.Set(modifier);
        testExchangeHolder.GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::MockProtocol::Id,
                                                                                             this);
        ChipLogDetail(ExchangeManager, "[%p] MockProtocolResponder: %p", this, &mExchangeCtx);
    }

    ~MockProtocolResponder()
    {
        ChipLogDetail(ExchangeManager, "[%p] ~MockProtocolResponder", this);
        testExchangeHolder.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::MockProtocol::Id);
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
    BitFlags<BehaviorModifier> mBehaviorModifier = BehaviorModifier::kNone;
    bool mInteractionSucceeded                   = false;
    TestExchangeHolder & testExchangeHolder;
};

class MockProtocolInitiator : public ExchangeDelegate
{
public:
    enum class BehaviorModifier : uint8_t
    {
        kNone                        = 0x00,
        kHoldMsg3                    = 0x01,
        kErrMsg1                     = 0x02,
        kErrMsg3                     = 0x04,
        kDontSendMsg1                = 0x08,
        kExpireSessionBeforeMsg1Send = 0x10,
        kExpireSessionAfterMsg1Send  = 0x12,
        kExpireSessionBeforeMsg3Send = 0x14,
        kExpireSessionAfterMsg3Send  = 0x18,
    };

    MockProtocolInitiator(TestExchangeHolder & ctx, BehaviorModifier modifier = BehaviorModifier::kNone) :
        mExchangeCtx(*this), testExchangeHolder(ctx)
    {
        mBehaviorModifier.Set(modifier);
        ChipLogDetail(ExchangeManager, "[%p] MockProtocolInitiator: %p", this, &mExchangeCtx);
    }

    template <typename... Args>
    MockProtocolInitiator(TestExchangeHolder & ctx, BehaviorModifier modifier1, Args &&... args) :
        mExchangeCtx(*this), mBehaviorModifier(modifier1, std::forward<Args>(args)...), testExchangeHolder(ctx)
    {
        ChipLogDetail(ExchangeManager, "[%p] MockProtocolInitiator: %p", this, &mExchangeCtx);
    }

    ~MockProtocolInitiator() { ChipLogDetail(ExchangeManager, "[%p] ~MockProtocolInitiator", this); }

    CHIP_ERROR StartInteraction(SessionHandle & sessionHandle);

    bool DidInteractionSucceed() { return mInteractionSucceeded; }

private:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override;

    void OnResponseTimeout(ExchangeContext * ec) override {}

    ExchangeHolder mExchangeCtx;
    BitFlags<BehaviorModifier> mBehaviorModifier = BehaviorModifier::kNone;
    bool mInteractionSucceeded                   = false;
    TestExchangeHolder & testExchangeHolder;
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

        if (!mBehaviorModifier.Has(BehaviorModifier::kHoldMsg2))
        {
            PacketBufferHandle respBuffer = MessagePacketBuffer::New(0);
            VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

            if (mBehaviorModifier.Has(BehaviorModifier::kErrMsg2))
            {
                mExchangeCtx->InjectFailure(ExchangeContext::InjectedFailureType::kFailOnSend);
            }

            if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionBeforeMsg2Send))
            {
                mExchangeCtx->GetSessionHolder().Release();
                mExchangeCtx->OnSessionReleased();
            }

            if (mExchangeCtx)
            {
                err = mExchangeCtx->SendMessage(chip::Protocols::MockProtocol::MessageType::kMsg2, std::move(respBuffer),
                                                SendMessageFlags::kExpectResponse);
                if (mExchangeCtx)
                {
                    mExchangeCtx->ClearInjectedFailures();
                }

                ReturnErrorOnFailure(err);
            }

            if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionAfterMsg2Send))
            {
                mExchangeCtx->GetSessionHolder().Release();
                mExchangeCtx->OnSessionReleased();
            }
        }
        else
        {
            mExchangeCtx->WillSendMessage();
        }
    }
    else if (payloadHeader.HasMessageType(chip::Protocols::MockProtocol::MessageType::kMsg3))
    {
        if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionAfterMsg3Receive))
        {
            mExchangeCtx->GetSessionHolder().Release();
            mExchangeCtx->OnSessionReleased();
        }

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

    auto exchange = testExchangeHolder.GetExchangeManager().NewContext(sessionHandle, this);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    //
    // This is the first exchange in this interaction - let's have our holder start managing the exchange by grabbing it.
    //
    mExchangeCtx.Grab(exchange);

    if (mBehaviorModifier.Has(BehaviorModifier::kErrMsg1))
    {
        mExchangeCtx->InjectFailure(ExchangeContext::InjectedFailureType::kFailOnSend);
    }

    if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionBeforeMsg1Send))
    {
        mExchangeCtx->GetSessionHolder().Release();
        mExchangeCtx->OnSessionReleased();
    }

    if (!mBehaviorModifier.Has(BehaviorModifier::kDontSendMsg1))
    {
        auto err = mExchangeCtx->SendMessage(chip::Protocols::MockProtocol::MessageType::kMsg1, std::move(buffer),
                                             SendMessageFlags::kExpectResponse);
        if (mExchangeCtx)
        {
            mExchangeCtx->ClearInjectedFailures();
        }

        ReturnErrorOnFailure(err);
    }

    if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionAfterMsg1Send))
    {
        mExchangeCtx->GetSessionHolder().Release();
        mExchangeCtx->OnSessionReleased();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MockProtocolInitiator::OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                    System::PacketBufferHandle && buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (payloadHeader.HasMessageType(chip::Protocols::MockProtocol::MessageType::kMsg2))
    {
        if (!mBehaviorModifier.Has(BehaviorModifier::kHoldMsg3))
        {
            if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionBeforeMsg3Send))
            {
                mExchangeCtx->GetSessionHolder().Release();
                mExchangeCtx->OnSessionReleased();
            }

            PacketBufferHandle respBuffer = MessagePacketBuffer::New(0);
            VerifyOrReturnError(!buffer.IsNull(), CHIP_ERROR_NO_MEMORY);

            if (mBehaviorModifier.Has(BehaviorModifier::kErrMsg3))
            {
                mExchangeCtx->InjectFailure(ExchangeContext::InjectedFailureType::kFailOnSend);
            }

            if (mExchangeCtx)
            {
                err = mExchangeCtx->SendMessage(chip::Protocols::MockProtocol::MessageType::kMsg3, std::move(respBuffer),
                                                SendMessageFlags::kNone);
                if (mExchangeCtx)
                {
                    mExchangeCtx->ClearInjectedFailures();
                }

                ReturnErrorOnFailure(err);
            }

            if (mBehaviorModifier.Has(BehaviorModifier::kExpireSessionAfterMsg3Send))
            {
                mExchangeCtx->GetSessionHolder().Release();
                mExchangeCtx->OnSessionReleased();
            }

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

TEST_F(TestExchangeHolder, TestExchangeHolder)
{
    auto sessionHandle = GetSessionAliceToBob();

    SetMRPMode(chip::Test::MessagingContext::MRPMode::kResponsive);

    //
    // #1: Initiator (AllocExchange)
    //
    // The initiator just allocated the exchange, but doesn't send a message on it.
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it still owns
    // it.
    //
    {
        ChipLogProgress(ExchangeManager, "-------- #1: Initiator (AllocExchange) ----------");

        {
            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kDontSendMsg1);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #2: Initiator --X Msg1
    //
    // Inject a failure to transmit Msg1. This should retain the WillSendMessage flag on the initiator's exchange.
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's still has the
    // WillSendMessage flag on it.
    //
    //
    {
        ChipLogProgress(ExchangeManager, "-------- #2: Initiator --X (SendErr) Msg1 --------- ");

        {
            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kErrMsg1);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_NE(err, CHIP_NO_ERROR);
        }

        //
        // Service IO AFTER the objects above cease to exist to prevent Msg1 from getting to Responder. This also
        // flush any pending messages in the queue.
        //
        DrainAndServiceIO();
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #3: Initiator --X (Session Released Before) -- Msg1
    //
    // Inject a release of the session associated with the exchange on the initiator before sending Msg1. This
    // should just close out the exchange without releasing the ref.
    //
    // Then, destroy both objects. The initiator's holder should correctly abort the exchange since WillSendMessage
    // should still be present on the EC.
    //
    {
        ChipLogProgress(ExchangeManager, "-------- #3: Initiator --X (SessionReleased before) Msg1 --------- ");

        {
            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kExpireSessionBeforeMsg1Send);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_NE(err, CHIP_NO_ERROR);
        }

        //
        // Service IO AFTER the objects above cease to exist to prevent Msg1 from getting to Responder. This also
        // flush any pending messages in the queue.
        //
        DrainAndServiceIO();
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #4: Initiator --X (SendErr + Session Released After) -- Msg1
    //
    // Inject an error at Msg1 transmission followed by the release of a session (scenario in #21544). This should
    // just close out the exchange without releasing the ref since the WillSendMessage flag should still be set.
    //
    // Then, destroy both objects. The initiator's holder should correctly abort the exchange since WillSendMessage
    // should still be present on the EC.
    //
    {
        ChipLogProgress(ExchangeManager, "-------- #4: Initiator --X (SendErr + SessionReleased after) Msg1 --------- ");

        {
            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kExpireSessionAfterMsg1Send,
                                            MockProtocolInitiator::BehaviorModifier::kErrMsg1);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_NE(err, CHIP_NO_ERROR);
        }

        //
        // Service IO AFTER the objects above cease to exist to prevent Msg1 from getting to Responder. This also
        // flush any pending messages in the queue.
        //
        DrainAndServiceIO();
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #5: Initiator >--- Msg1 --X  Responder.
    //
    // Initiator sends Msg1 to Responder, but we set it up such that Responder doesn't actually
    // receive the message.
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's waiting for
    // a response.
    //
    {
        ChipLogProgress(ExchangeManager, "-------- #5: Initiator >-- Msg1 --X Responder ---------");

        {
            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);
        }

        //
        // Service IO AFTER the objects above cease to exist to prevent Msg1 from getting to Responder. This also
        // flush any pending messages in the queue.
        //
        DrainAndServiceIO();
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #6: Initiator --- Msg1 -->  Responder (WillSend)
    //
    // Initiator sends Msg1 to Responder, which is received successfully. However, Responder
    // doesn't send a response right away (calls WillSendMessage() on the EC).
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's waiting for
    // a response, and so should the Responder's holder since it has yet to send a message.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #6: Initiator >-- Msg1 --> Responder (WillSend) ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this, MockProtocolResponder::BehaviorModifier::kHoldMsg2);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #7:            Initiator --- Msg1           -->  Responder
    //                              Msg2 (SendErr) X--  Responder
    //
    //  Inject an error in the responder when attempting to send Msg2.
    //
    //  Then, destroy both objects. The holder on the responder should abort the exchange since
    //  the transmission failed, and the ref is still with the holder. The holder on the initiator
    //  should abort the exchange since it is waiting for a response.
    //
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #7: Msg2 (SendFailure) X-- Responder ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this, MockProtocolResponder::BehaviorModifier::kErrMsg2);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #8:            Initiator --- Msg1                          -->  Responder
    //                              Msg2 (SessionReleased before) X--  Responder
    //
    // Release the session right before sending Msg2 on the responder. This should abort the underlying exchange
    // immediately since neither WillSendMessage or ResponseExpected flags are set.
    //
    // Then, destroy both objects. The holders on both should just null out their internal reference to the EC.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #8: Msg2 (SessionReleased Before) X-- Responder ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this, MockProtocolResponder::BehaviorModifier::kExpireSessionBeforeMsg2Send);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #9:            Initiator --- Msg1                                    -->  Responder
    //                              Msg2 (SendErr + SessionReleased after)  X--  Responder
    //
    // Trigger a send error when sending Msg2 from the responder, and release the session immediately after. This should still
    // preserve the WillSendMessage flags on the exchange and just close out the EC without releasing the ref.
    //
    // Then, destroy both objects. The holders on both should abort their respective ECs.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #9: Msg2 (SendErr + SessionReleased after) X-- Responder ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this, MockProtocolResponder::BehaviorModifier::kErrMsg2,
                                            MockProtocolResponder::BehaviorModifier::kExpireSessionAfterMsg2Send);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #10:            Initiator --- Msg1 -->  Responder
    //      (WillSend) Initiator <-- Msg2 <--  Responder
    //
    // Initiator receives Msg2 back from Responder, but calls WillSend on that EC.
    //
    // Then, destroy both objects. Initiator's holder should correctly abort the exchange since it's waiting
    // to send a response, and Responder's holder should abort as well since it's waiting for a response.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #10: (WillSend) Initiator <-- Msg2 <-- Responder ---------");

            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kHoldMsg3);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #11:           Initiator                          --- Msg1 -->  Responder
    //                Initiator                          <-- Msg2 <--  Responder
    //                Initiator (SessionReleased before) X-- Msg3
    //
    // Release the session right before the initiator sends Msg3. This should abort the underlying EC immediately on the initiator.
    //
    // Then destroy both objects. Both holders on the initiator and responder should be pointing to null.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #11: Initiator --X (SessionReleased before) Msg3 ------------");

            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kExpireSessionBeforeMsg3Send);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_NE(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #12:            Initiator                                     --- Msg1 -->  Responder
    //                 Initiator                                     <-- Msg2 <--  Responder
    //                 Initiator X (SendErr + SessionReleased after) -- Msg3
    //
    // Trigger a send error on the initiator when sending Msg3, followed by a session release. Since a send was initiated, the ref
    // is with the initiator's holder and the EC will just close itself out without removing the ref.
    //
    // Then, destroy both objects. The responder's holder will have a null ref but the initiator's holder will have a non-null ref,
    // and should abort it.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #12: Initiator --X (SendErr + SessionReleased after) Msg3 ------------");

            MockProtocolInitiator initiator(*this, MockProtocolInitiator::BehaviorModifier::kErrMsg3,
                                            MockProtocolInitiator::BehaviorModifier::kExpireSessionAfterMsg3Send);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #13:            Initiator --- Msg1 -->  Responder
    //                 Initiator <-- Msg2 <--  Responder
    //                 Initiator >-- Msg3 -->  Responder
    //
    // Initiator sends final message in exchange to Responder, which is received successfully.
    //
    // Then, destroy both objects. Initiator's holder should NOT abort the underlying exchange since
    // it has sent the final message in the exchange, while responder's holder should NOT abor the underlying
    // exchange either since it is not going to send any further messages on the exchange.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #13: Initiator >-- Msg3 -->  Responder ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #14:           Initiator --- Msg1 -->  Responder
    //                Initiator <-- Msg2 <--  Responder
    //                Initiator >-- Msg3 -->  Responder (SessionReleased)
    //
    // Released the session right on reception of Msg3 on the responder. Since there no responses expected or send expected,
    // the EC aborts immediately.
    //
    // Then, destroy both objects. Both holders should be point to null and should do nothing.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #14: Initiator >-- Msg3 -->  Responder (SessionReleased) ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this, MockProtocolResponder::BehaviorModifier::kExpireSessionAfterMsg3Receive);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();

            //
            // Because of the session expiration right after Msg3 is received, it causes an abort of the underlying EC
            // on the reponder side. This means that Msg3 won't be ACK'ed. Msg3 on the initiator side remains un-acked. Since
            // the exchange was just closed and not aborted on the initiator side, it is still sitting in the retransmission table
            // and consequently, the exchange still has a ref-count of 1. If we were to just check the number of active
            // exchanges, it would still show 1 active exchange.
            //
            // This will only be released once the re-transmission table
            // entry has been removed. To make this happen, drive the IO forward enough that a single re-transmission happens. This
            // will result in a duplicate message ACK being delivered by the responder, causing the EC to finally get released.
            //
            GetIOContext().DriveIOUntil(System::Clock::Seconds16(5),
                                        [&]() { return GetExchangeManager().GetNumActiveExchanges() == 0; });
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #15:           Initiator --- Msg1 -->  Responder (WillSend)
    //                Initiator --- Msg1 -->  Responder (WillSend)
    //
    // Similar to #6, except we have Initiator start the interaction again. This validates
    // ExchangeHolder::Grab in correctly aborting a previous exchange and acquiring a new one.
    //
    // Then, destroy both objects. Both holders should abort the exchange (see #6).
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #15: Initiator >-- Msg1 -->  Responder (WillSend) X2 ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this, MockProtocolResponder::BehaviorModifier::kHoldMsg2);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();

            err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        DrainAndServiceIO();
        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }

    //
    // #16:           Initiator --- Msg1 -->  Responder
    //                Initiator <-- Msg2 <--  Responder
    //                Initiator >-- Msg3 -->  Responder
    //
    //                X2
    //
    // We do the entire interaction twice. This validates ExchangeHolder::Grab in correctly releasing a reference
    // to a previous exchange (but not aborting it) and acquiring a new one.
    //
    // Then, destroy both objects. Both holders should release their reference without aborting.
    //
    {
        {
            ChipLogProgress(ExchangeManager, "-------- #16: Initiator >-- Msg3 -->  Responder X2 ---------");

            MockProtocolInitiator initiator(*this);
            MockProtocolResponder responder(*this);

            auto err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();

            err = initiator.StartInteraction(sessionHandle);
            EXPECT_EQ(err, CHIP_NO_ERROR);

            DrainAndServiceIO();
        }

        EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
    }
}
} // anonymous namespace
