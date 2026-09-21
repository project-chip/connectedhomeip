/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include <errno.h>
#include <utility>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

#if CHIP_CRYPTO_PSA
#include "psa/crypto.h"
#endif

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;

class MockExchangeDelegate;

struct TestExchange : public Testing::LoopbackMessagingContext
{
    void SetUp() override
    {
#if CHIP_CRYPTO_PSA
        ASSERT_EQ(psa_crypto_init(), PSA_SUCCESS);
#endif
        Testing::LoopbackMessagingContext::SetUp();
    }

    template <typename AfterRequestChecker, typename AfterResponseChecker>
    void DoRoundTripTest(MockExchangeDelegate & delegate1, MockExchangeDelegate & delegate2, uint8_t requestMessageType,
                         uint8_t responseMessageType, AfterRequestChecker && afterRequestChecker,
                         AfterResponseChecker && afterResponseChecker);
};

enum : uint8_t
{
    kMsgType_TEST1 = 0xf0,
    kMsgType_TEST2 = 0xf1,
};

class MockExchangeDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
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
        ++mReceivedMessageCount;
        if (mKeepExchangeAliveOnMessageReceipt)
        {
            ec->WillSendMessage();
            mExchange = ec;
        }
        else
        {
            // Exchange will be closing, so don't hold on to a reference to it.
            mExchange = nullptr;
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    ExchangeMessageDispatch & GetMessageDispatch() override
    {
        if (mMessageDispatch != nullptr)
        {
            return *mMessageDispatch;
        }

        return ExchangeDelegate::GetMessageDispatch();
    }

    uint32_t mReceivedMessageCount             = 0;
    bool mKeepExchangeAliveOnMessageReceipt    = true;
    ExchangeContext * mExchange                = nullptr;
    ExchangeMessageDispatch * mMessageDispatch = nullptr;
};

// Helper used by several tests.  Registers delegate2 as an unsolicited message
// handler, sends a message of type requestMessageType via an exchange that has
// delegate1 as delegate, responds with responseMessageType.
template <typename AfterRequestChecker, typename AfterResponseChecker>
void TestExchange::DoRoundTripTest(MockExchangeDelegate & delegate1, MockExchangeDelegate & delegate2, uint8_t requestMessageType,
                                   uint8_t responseMessageType, AfterRequestChecker && afterRequestChecker,
                                   AfterResponseChecker && afterResponseChecker)
{
    ExchangeContext * ec1 = NewExchangeToBob(&delegate1);
    ASSERT_NE(ec1, nullptr);

    CHIP_ERROR err =
        GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::Id, requestMessageType, &delegate2);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // To simplify things, skip MRP for all our messages, and make sure we are
    // always expecting responses.
    constexpr auto sendFlags =
        SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck, Messaging::SendMessageFlags::kExpectResponse);

    err = ec1->SendMessage(Protocols::SecureChannel::Id, requestMessageType,
                           System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize), sendFlags);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    afterRequestChecker();

    ExchangeContext * ec2 = delegate2.mExchange;
    err                   = ec2->SendMessage(Protocols::SecureChannel::Id, responseMessageType,
                                             System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize), sendFlags);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    DrainAndServiceIO();

    afterResponseChecker();

    ec1->Close();
    ec2->Close();

    Messaging::UnsolicitedMessageHandler * removedHandler = nullptr;
    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::Id, kMsgType_TEST1,
                                                                          &removedHandler);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(removedHandler, &delegate2);
}

TEST_F(TestExchange, CheckBasicMessageRoundTrip)
{
    MockExchangeDelegate delegate1;
    MockExchangeDelegate delegate2;
    DoRoundTripTest(
        delegate1, delegate2, kMsgType_TEST1, kMsgType_TEST2,
        [&] {
            EXPECT_EQ(delegate1.mReceivedMessageCount, 0u);
            EXPECT_EQ(delegate2.mReceivedMessageCount, 1u);
        },
        [&] {
            EXPECT_EQ(delegate1.mReceivedMessageCount, 1u);
            EXPECT_EQ(delegate2.mReceivedMessageCount, 1u);
        });
}

TEST_F(TestExchange, CheckBasicExchangeMessageDispatch)
{
    class MockMessageDispatch : public ExchangeMessageDispatch
    {
        bool MessagePermitted(Protocols::Id protocol, uint8_t type) override
        {
            // Only allow TEST1 messages.
            return protocol == Protocols::SecureChannel::Id && type == kMsgType_TEST1;
        }
    };

    MockMessageDispatch dispatch;

    {
        // Allowed response.
        MockExchangeDelegate delegate1;
        delegate1.mMessageDispatch = &dispatch;
        MockExchangeDelegate delegate2;

        DoRoundTripTest(
            delegate1, delegate2, kMsgType_TEST1, kMsgType_TEST1,
            [&] {
                EXPECT_EQ(delegate1.mReceivedMessageCount, 0u);
                EXPECT_EQ(delegate2.mReceivedMessageCount, 1u);
            },
            [&] {
                EXPECT_EQ(delegate1.mReceivedMessageCount, 1u);
                EXPECT_EQ(delegate2.mReceivedMessageCount, 1u);
            });
    }

    {
        // Disallowed response.
        MockExchangeDelegate delegate1;
        delegate1.mMessageDispatch = &dispatch;
        MockExchangeDelegate delegate2;

        DoRoundTripTest(
            delegate1, delegate2, kMsgType_TEST1, kMsgType_TEST2,
            [&] {
                EXPECT_EQ(delegate1.mReceivedMessageCount, 0u);
                EXPECT_EQ(delegate2.mReceivedMessageCount, 1u);
            },
            [&] {
                EXPECT_EQ(delegate1.mReceivedMessageCount, 0u);
                EXPECT_EQ(delegate2.mReceivedMessageCount, 1u);
            });
    }
}

// A responder exchange that has latched WillSendMessage() and then fails to send its
// reply must still be released when its owner walks away.  CASESession::HandleSigma3a
// latches WillSendMessage() and the reply goes out later from HandleSigma3c; if the link
// drops in between, that send fails, and PairingSession::DiscardExchange() then drops
// its handle.  The exchange was left still expecting to send, so it never closed
// itself: it held its SessionHolder ref for good and pinned an
// UnauthenticatedSessionTable entry, which FindLeastRecentUsedEntry() only reclaims
// once the refcount is 0.  With every entry in the pool pinned, SessionManager dropped
// each inbound Sigma1 with "UnauthenticatedSession exhausted" and the node could not
// establish CASE again short of a reboot.
TEST_F(TestExchange, CheckResponderExchangeReleasedWhenReplySendFails)
{
    MockExchangeDelegate delegate1;
    MockExchangeDelegate delegate2;

    ExchangeContext * ec1 = NewExchangeToBob(&delegate1);
    ASSERT_NE(ec1, nullptr);

    ASSERT_EQ(
        GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::Id, kMsgType_TEST1, &delegate2),
        CHIP_NO_ERROR);

    constexpr auto sendFlags =
        SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck, Messaging::SendMessageFlags::kExpectResponse);

    ASSERT_EQ(ec1->SendMessage(Protocols::SecureChannel::Id, kMsgType_TEST1,
                               System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize), sendFlags),
              CHIP_NO_ERROR);
    DrainAndServiceIO();

    // The responder kept the exchange alive to reply later, as HandleSigma3a does.
    ExchangeContext * ec2 = delegate2.mExchange;
    ASSERT_NE(ec2, nullptr);
    EXPECT_TRUE(ec2->IsSendExpected());

    // Its reply now fails to go out, as SendStatusReport does over a dropped link.
    ec2->InjectFailure(ExchangeContext::InjectedFailureType::kFailOnSend);
    EXPECT_NE(ec2->SendMessage(Protocols::SecureChannel::Id, kMsgType_TEST2,
                               System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                               SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck)),
              CHIP_NO_ERROR);
    DrainAndServiceIO();

    // A failed send leaves the exchange still expecting to send, so it cannot close
    // itself: this is what used to make it outlive its owner.
    ASSERT_NE(delegate2.mExchange, nullptr);
    EXPECT_TRUE(ec2->IsSendExpected());
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 2u);

    // The owner walks away, as PairingSession::DiscardExchange() does.
    ec2->SetDelegate(nullptr);
    ec2->AbandonPendingSend();

    // ec2 has been released by now, so it must not be touched again here.
    ec1->Close();
    DrainAndServiceIO();

    // Nothing was pending on the responder exchange, so it must be back in the pool
    // rather than holding its session open forever.
    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);

    Messaging::UnsolicitedMessageHandler * removedHandler = nullptr;
    EXPECT_EQ(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::Id, kMsgType_TEST1,
                                                                              &removedHandler),
              CHIP_NO_ERROR);
    EXPECT_EQ(removedHandler, &delegate2);
}

// A crude test to exercise VerifyOrDieWithObject() in ObjectPool and
// the resulting DumpToLog() call on the ExchangeContext.
// TODO: Find a way to automate this test without killing the process.
// TEST_F(TestExchange, DumpExchangePoolToLog)
// {
//     MockExchangeDelegate delegate;
//     ObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> pool;
//     pool.CreateObject(&GetExchangeManager(), static_cast<uint16_t>(1234), GetSessionAliceToBob(), true, &delegate);
// }

} // namespace
