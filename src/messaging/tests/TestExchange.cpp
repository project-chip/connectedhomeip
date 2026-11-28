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
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <messaging/tests/ReliableMessageContextTestAccess.h>
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

// Sends one message that expects a response on `ec`, then returns.
static CHIP_ERROR SendExpectingResponse(ExchangeContext * ec)
{
    return ec->SendMessage(Protocols::SecureChannel::Id, kMsgType_TEST1,
                           System::PacketBufferHandle::New(System::PacketBuffer::kMaxSize),
                           SendFlags(SendMessageFlags::kExpectResponse));
}

TEST_F(TestExchange, GivenUpRetransmittingOnMRPSessionAfterRetransmissionsStop)
{
    MockExchangeDelegate delegate;
    delegate.mKeepExchangeAliveOnMessageReceipt = false;

    // Nothing reaches the peer, so the message stays unacknowledged.
    auto & loopback             = GetLoopback();
    loopback.mNumMessagesToDrop = Testing::LoopbackTransport::kUnlimitedMessageCount;
    auto resetLoopback          = ScopeExit([&]() { loopback.Reset(); });

    ExchangeContext * ec = NewUnauthenticatedExchangeToBob(&delegate);
    ASSERT_NE(ec, nullptr);
    ASSERT_TRUE(ec->GetSessionHandle()->AllowsMRP());
    auto closeExchange = ScopeExit([&]() { ec->Close(); });

    EXPECT_EQ(SendExpectingResponse(ec), CHIP_NO_ERROR);
    EXPECT_FALSE(ec->HasGivenUpRetransmittingUnackedMessage());

    // MRP runs out of retransmissions; the response timer is still running.
    GetExchangeManager().GetReliableMessageMgr()->ClearRetransTable(ec->GetReliableMessageContext());
    EXPECT_TRUE(ec->HasGivenUpRetransmittingUnackedMessage());
}

TEST_F(TestExchange, LateAckForGivenUpMessageIsRecognized)
{
    MockExchangeDelegate delegate;
    delegate.mKeepExchangeAliveOnMessageReceipt = false;

    // Nothing reaches the peer, so the message stays unacknowledged.
    auto & loopback             = GetLoopback();
    loopback.mNumMessagesToDrop = Testing::LoopbackTransport::kUnlimitedMessageCount;
    auto resetLoopback          = ScopeExit([&]() { loopback.Reset(); });

    ExchangeContext * ec = NewUnauthenticatedExchangeToBob(&delegate);
    ASSERT_NE(ec, nullptr);
    auto closeExchange = ScopeExit([&]() { ec->Close(); });

    EXPECT_EQ(SendExpectingResponse(ec), CHIP_NO_ERROR);
    auto * mrpMgr = GetExchangeManager().GetReliableMessageMgr();
    uint32_t messageCounter;
    ASSERT_TRUE(mrpMgr->TestGetRetransMessageCounter(ec->GetReliableMessageContext(), messageCounter));

    // MRP stops retransmitting; the peer's ack for that message arrives later.
    mrpMgr->ClearRetransTable(ec->GetReliableMessageContext());
    ASSERT_TRUE(ec->HasGivenUpRetransmittingUnackedMessage());

    Testing::ReliableMessageContextTestAccess(ec->GetReliableMessageContext()).ReceiveAck(messageCounter);

    // The peer did receive it, so it was not given up on after all.
    EXPECT_FALSE(ec->HasGivenUpRetransmittingUnackedMessage());
}

TEST_F(TestExchange, LateAckAfterRetransmitSendFailureIsRecognized)
{
    MockExchangeDelegate delegate;
    delegate.mKeepExchangeAliveOnMessageReceipt = false;

    ExchangeContext * ec = NewUnauthenticatedExchangeToBob(&delegate);
    ASSERT_NE(ec, nullptr);
    auto closeExchange = ScopeExit([&]() { ec->Close(); });

    // Retransmit quickly.
    ec->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(
        SessionParameters(ReliableMessageProtocolConfig(System::Clock::Milliseconds32(50), System::Clock::Milliseconds32(50))));

    // The first transmission goes out (and is lost); the retransmission then
    // fails with an error that is not transient, so MRP stops retransmitting.
    auto & loopback                         = GetLoopback();
    loopback.mNumMessagesToDrop             = 1;
    loopback.mNumMessagesToAllowBeforeError = 1;
    loopback.mMessageSendError              = CHIP_ERROR_INTERNAL;
    auto resetLoopback                      = ScopeExit([&]() { loopback.Reset(); });

    EXPECT_EQ(SendExpectingResponse(ec), CHIP_NO_ERROR);
    auto * mrpMgr = GetExchangeManager().GetReliableMessageMgr();
    uint32_t messageCounter;
    ASSERT_TRUE(mrpMgr->TestGetRetransMessageCounter(ec->GetReliableMessageContext(), messageCounter));

    GetIOContext().DriveIOUntil(System::Clock::Milliseconds32(2000), [&] { return mrpMgr->TestGetCountRetransTable() == 0; });
    ASSERT_EQ(mrpMgr->TestGetCountRetransTable(), 0);
    ASSERT_TRUE(ec->HasGivenUpRetransmittingUnackedMessage());

    // The first transmission did reach the peer after all; its ack arrives late.
    Testing::ReliableMessageContextTestAccess(ec->GetReliableMessageContext()).ReceiveAck(messageCounter);
    EXPECT_FALSE(ec->HasGivenUpRetransmittingUnackedMessage());
}

TEST_F(TestExchange, LateAckForOtherMessageIsIgnored)
{
    MockExchangeDelegate delegate;
    delegate.mKeepExchangeAliveOnMessageReceipt = false;

    auto & loopback             = GetLoopback();
    loopback.mNumMessagesToDrop = Testing::LoopbackTransport::kUnlimitedMessageCount;
    auto resetLoopback          = ScopeExit([&]() { loopback.Reset(); });

    ExchangeContext * ec = NewUnauthenticatedExchangeToBob(&delegate);
    ASSERT_NE(ec, nullptr);
    auto closeExchange = ScopeExit([&]() { ec->Close(); });

    EXPECT_EQ(SendExpectingResponse(ec), CHIP_NO_ERROR);
    auto * mrpMgr = GetExchangeManager().GetReliableMessageMgr();
    uint32_t messageCounter;
    ASSERT_TRUE(mrpMgr->TestGetRetransMessageCounter(ec->GetReliableMessageContext(), messageCounter));

    mrpMgr->ClearRetransTable(ec->GetReliableMessageContext());

    // An ack for some other message counter does not count.
    Testing::ReliableMessageContextTestAccess(ec->GetReliableMessageContext()).ReceiveAck(messageCounter + 1);
    EXPECT_TRUE(ec->HasGivenUpRetransmittingUnackedMessage());
}

TEST_F(TestExchange, NeverGivenUpRetransmittingOnSessionWithoutMRP)
{
    MockExchangeDelegate delegate;
    delegate.mKeepExchangeAliveOnMessageReceipt = false;

    // A session to a TCP peer address does not use MRP, so nothing tracks an
    // acknowledgement for what is sent on it.
    Transport::PeerAddress tcpPeer  = Transport::PeerAddress::TCP(GetBobAddress().GetIPAddress(), GetBobAddress().GetPort());
    Optional<SessionHandle> session = GetSecureSessionManager().CreateUnauthenticatedSession(tcpPeer, GetDefaultMRPConfig());
    ASSERT_TRUE(session.HasValue());
    ASSERT_FALSE(session.Value()->AllowsMRP());

    ExchangeContext * ec = GetExchangeManager().NewContext(session.Value(), &delegate);
    ASSERT_NE(ec, nullptr);
    ec->SetResponseTimeout(System::Clock::Seconds16(30));

    EXPECT_EQ(SendExpectingResponse(ec), CHIP_NO_ERROR);
    // The message is still in flight: it was never "given up on".
    EXPECT_FALSE(ec->HasGivenUpRetransmittingUnackedMessage());

    ec->Abort();
    GetLoopback().Reset();
    DrainAndServiceIO();
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
