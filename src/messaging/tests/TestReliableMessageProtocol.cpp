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
 *      This file implements unit tests for the ReliableMessageProtocol
 *      implementation.
 */
#include <queue>

#include <errno.h>

#include <pw_unit_test/framework.h>

#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDConfigurationData.h> // nogncheck
#endif

#if CHIP_CRYPTO_PSA
#include "psa/crypto.h"
#endif

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;
using namespace chip::System::Clock::Literals;

const char PAYLOAD[] = "Hello!";

class TestReliablityAnalyticDelegate : public ReliableMessageAnalyticsDelegate
{
public:
    virtual void OnTransmitEvent(const TransmitEvent & event) override { mTransmitEvents.push(event); }
    std::queue<ReliableMessageAnalyticsDelegate::TransmitEvent> mTransmitEvents;
};

class TestReliableMessageProtocol : public chip::Test::LoopbackMessagingContext
{
public:
    // Performs setup for each individual test in the test suite
    void SetUp() override
    {
#if CHIP_CRYPTO_PSA
        ASSERT_EQ(psa_crypto_init(), PSA_SUCCESS);
#endif
        chip::Test::LoopbackMessagingContext::SetUp();
        GetSessionAliceToBob()->AsSecureSession()->SetRemoteSessionParameters(GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
        GetSessionBobToAlice()->AsSecureSession()->SetRemoteSessionParameters(GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
    }
};

class MockAppDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
{
public:
    MockAppDelegate(TestReliableMessageProtocol & ctx) : mTestReliableMessageProtocol(ctx) {}

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        // Handle messages by myself
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;

        if (ec->HasSessionHandle() && ec->GetSessionHolder()->IsSecureSession())
        {
            mLastSubjectDescriptor = ec->GetSessionHolder()->AsSecureSession()->GetSubjectDescriptor();
        }

        if (payloadHeader.IsAckMsg())
        {
            mReceivedPiggybackAck = true;
        }
        if (mDropAckResponse)
        {
            auto * rc = ec->GetReliableMessageContext();
            if (rc->HasPiggybackAckPending())
            {
                // Make sure we don't accidentally retransmit and end up acking
                // the retransmit.
                rc->GetReliableMessageMgr()->StopTimer();
                (void) rc->TakePendingPeerAckMessageCounter();
            }
        }

        if (mExchange != ec)
        {
            CloseExchangeIfNeeded();
        }

        if (!mRetainExchange)
        {
            ec = nullptr;
        }
        else
        {
            ec->WillSendMessage();
        }
        mExchange = ec;

        EXPECT_EQ(buffer->TotalLength(), sizeof(PAYLOAD));
        EXPECT_EQ(memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()), 0);

        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override { mResponseTimedOut = true; }

    void CloseExchangeIfNeeded()
    {
        if (mExchange != nullptr)
        {
            mExchange->Close();
            mExchange = nullptr;
        }
    }

    void SetDropAckResponse(bool dropResponse)
    {
        mDropAckResponse = dropResponse;
        if (!mDropAckResponse)
        {
            // Restart the MRP retransmit timer, now that we are not going to be
            // dropping acks anymore, so we send out pending retransmits, if
            // any, as needed.
            mTestReliableMessageProtocol.GetExchangeManager().GetReliableMessageMgr()->StartTimer();
        }
    }

    Access::SubjectDescriptor mLastSubjectDescriptor{};

    bool IsOnMessageReceivedCalled = false;
    bool mReceivedPiggybackAck     = false;
    bool mRetainExchange           = false;
    bool mResponseTimedOut         = false;
    ExchangeContext * mExchange    = nullptr;

private:
    TestReliableMessageProtocol & mTestReliableMessageProtocol;
    bool mDropAckResponse = false;
};

class MockSessionEstablishmentExchangeDispatch : public Messaging::ApplicationExchangeDispatch
{
public:
    bool IsReliableTransmissionAllowed() const override { return mRetainMessageOnSend; }

    bool MessagePermitted(Protocols::Id protocol, uint8_t type) override { return true; }

    bool IsEncryptionRequired() const override { return mRequireEncryption; }

    bool mRetainMessageOnSend = true;

    bool mRequireEncryption = false;
};

class MockSessionEstablishmentDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        // Handle messages by myself
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;
        EXPECT_EQ(buffer->TotalLength(), sizeof(PAYLOAD));
        EXPECT_EQ(memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()), 0);
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    virtual ExchangeMessageDispatch & GetMessageDispatch() override { return mMessageDispatch; }

    bool IsOnMessageReceivedCalled = false;
    MockSessionEstablishmentExchangeDispatch mMessageDispatch;
};

struct BackoffComplianceTestVector
{
    uint8_t sendCount;
    System::Clock::Timeout backoffBase;
    System::Clock::Timeout backoffMin;
    System::Clock::Timeout backoffMax;
};

struct BackoffComplianceTestVector theBackoffComplianceTestVector[] = { {
                                                                            .sendCount   = 0,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(330),
                                                                            .backoffMax  = System::Clock::Timeout(413),
                                                                        },
                                                                        {
                                                                            .sendCount   = 1,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(330),
                                                                            .backoffMax  = System::Clock::Timeout(413),
                                                                        },
                                                                        {
                                                                            .sendCount   = 2,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(528),
                                                                            .backoffMax  = System::Clock::Timeout(661),
                                                                        },
                                                                        {
                                                                            .sendCount   = 3,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(844),
                                                                            .backoffMax  = System::Clock::Timeout(1057),
                                                                        },
                                                                        {
                                                                            .sendCount   = 4,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(1351),
                                                                            .backoffMax  = System::Clock::Timeout(1691),
                                                                        },
                                                                        {
                                                                            .sendCount   = 5,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(2162),
                                                                            .backoffMax  = System::Clock::Timeout(2705),
                                                                        },
                                                                        {
                                                                            .sendCount   = 6,
                                                                            .backoffBase = System::Clock::Timeout(300),
                                                                            .backoffMin  = System::Clock::Timeout(2162),
                                                                            .backoffMax  = System::Clock::Timeout(2705),
                                                                        },
                                                                        {
                                                                            .sendCount   = 0,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(4400),
                                                                            .backoffMax  = System::Clock::Timeout(5503),
                                                                        },
                                                                        {
                                                                            .sendCount   = 1,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(4400),
                                                                            .backoffMax  = System::Clock::Timeout(5503),
                                                                        },
                                                                        {
                                                                            .sendCount   = 2,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(7040),
                                                                            .backoffMax  = System::Clock::Timeout(8805),
                                                                        },
                                                                        {
                                                                            .sendCount   = 3,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(11264),
                                                                            .backoffMax  = System::Clock::Timeout(14088),
                                                                        },
                                                                        {
                                                                            .sendCount   = 4,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(18022),
                                                                            .backoffMax  = System::Clock::Timeout(22541),
                                                                        },
                                                                        {
                                                                            .sendCount   = 5,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(28835),
                                                                            .backoffMax  = System::Clock::Timeout(36065),
                                                                        },
                                                                        {
                                                                            .sendCount   = 6,
                                                                            .backoffBase = System::Clock::Timeout(4000),
                                                                            .backoffMin  = System::Clock::Timeout(28835),
                                                                            .backoffMax  = System::Clock::Timeout(36065),
                                                                        },
                                                                        {
                                                                            // test theoretical worst-case 1-hour interval
                                                                            .sendCount   = 4,
                                                                            .backoffBase = System::Clock::Timeout(3'600'000),
                                                                            .backoffMin  = System::Clock::Timeout(16'220'160),
                                                                            .backoffMax  = System::Clock::Timeout(20'286'001),
                                                                        } };

void CheckGetBackoffImpl(System::Clock::Timeout additionalMRPBackoffTime)
{
    ReliableMessageMgr::SetAdditionalMRPBackoffTime(MakeOptional(additionalMRPBackoffTime));

    // Run 3x iterations to thoroughly test random jitter always results in backoff within bounds.
    for (uint32_t j = 0; j < 3; j++)
    {
        for (const auto & test : theBackoffComplianceTestVector)
        {
            System::Clock::Timeout backoff      = ReliableMessageMgr::GetBackoff(test.backoffBase, test.sendCount);
            System::Clock::Timeout extraBackoff = additionalMRPBackoffTime;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
            // If running as an ICD, increase maxBackoff to account for the polling interval
            extraBackoff += ICDConfigurationData::GetInstance().GetFastPollingInterval();
#endif

            ChipLogProgress(Test, "Backoff base %" PRIu32 " extra %" PRIu32 " # %d: %" PRIu32, test.backoffBase.count(),
                            extraBackoff.count(), test.sendCount, backoff.count());

            EXPECT_GE(backoff, test.backoffMin + extraBackoff);
            EXPECT_LE(backoff, test.backoffMax + extraBackoff);
        }
    }

    ReliableMessageMgr::SetAdditionalMRPBackoffTime(NullOptional);
}

} // namespace

TEST_F(TestReliableMessageProtocol, CheckAddClearRetrans)
{
    MockAppDelegate mockAppDelegate(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockAppDelegate);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm     = GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    ASSERT_NE(rm, nullptr);
    ASSERT_NE(rc, nullptr);

    ReliableMessageMgr::RetransTableEntry * entry;

    rm->AddToRetransTable(rc, &entry);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    rm->ClearRetransTable(*entry);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    exchange->Close();
}

/**
 * Tests MRP retransmission logic with the following scenario:
 *
 *      DUT = sender, PEER = remote device
 *
 * 1) DUT configured to use sleepy peer parameters of active = 64ms, idle = 64ms
 * 2) DUT sends message attempt #1 to PEER
 *      - Force PEER to drop message
 *      - Observe DUT timeout with no ack
 *      - Confirm MRP backoff interval is correct
 * 3) DUT resends message attempt #2 to PEER
 *      - Force PEER to drop message
 *      - Observe DUT timeout with no ack
 *      - Confirm MRP backoff interval is correct
 * 4) DUT resends message attempt #3 to PEER
 *      - Force PEER to drop message
 *      - Observe DUT timeout with no ack
 *      - Confirm MRP backoff interval is correct
 * 5) DUT resends message attempt #4 to PEER
 *      - Force PEER to drop message
 *      - Observe DUT timeout with no ack
 *      - Confirm MRP backoff interval is correct
 * 6) DUT resends message attempt #5 to PEER
 *      - PEER to acknowledge message
 *      - Observe DUT signal successful reliable transmission
 */
TEST_F(TestReliableMessageProtocol, CheckResendApplicationMessage)
{
    BackoffComplianceTestVector * expectedBackoff;
    System::Clock::Timestamp now, startTime;
    System::Clock::Timeout timeoutTime, margin;
    margin = System::Clock::Timeout(15);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender(*this);
    // TODO: temporarily create a SessionHandle from node id, will be fix in PR 3602
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        System::Clock::Timestamp(300), // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        System::Clock::Timestamp(300), // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 4;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // Ensure the exchange stays open after we send (unlike the CheckCloseExchangeAndResendApplicationMessage case), by claiming to
    // expect a response.
    startTime = System::SystemClock().GetMonotonicTimestamp();
    err       = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kExpectResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the initial message was dropped and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the initial message to fail (should take 330-413ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #1  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[0];
    EXPECT_GE(timeoutTime, expectedBackoff->backoffMin - margin);

    startTime = System::SystemClock().GetMonotonicTimestamp();
    DrainAndServiceIO();

    // Ensure the 1st retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 2u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the 1st retry to fail (should take 330-413ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #2  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[1];
    EXPECT_GE(timeoutTime, expectedBackoff->backoffMin - margin);

    startTime = System::SystemClock().GetMonotonicTimestamp();
    DrainAndServiceIO();

    // Ensure the 2nd retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 3u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the 2nd retry to fail (should take 528-660ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #3  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[2];
    EXPECT_GE(timeoutTime, expectedBackoff->backoffMin - margin);

    startTime = System::SystemClock().GetMonotonicTimestamp();
    DrainAndServiceIO();

    // Ensure the 3rd retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 4u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 4u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the 3rd retry to fail (should take 845-1056ms)
    GetIOContext().DriveIOUntil(1500_ms32, [&] { return loopback.mSentMessageCount >= 5; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #4  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[3];
    EXPECT_GE(timeoutTime, expectedBackoff->backoffMin - margin);

    // Trigger final transmission
    DrainAndServiceIO();

    // Ensure the last retransmission was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    EXPECT_GE(loopback.mSentMessageCount, 5u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 4u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    exchange->Close();
}

TEST_F(TestReliableMessageProtocol, CheckCloseExchangeAndResendApplicationMessage)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender(*this);
    // TODO: temporarily create a SessionHandle from node id, will be fixed in PR 3602
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 2;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the first re-transmit (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    // Ensure the retransmit message was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 2u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the second re-transmit (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    DrainAndServiceIO();

    // Ensure the retransmit message was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    EXPECT_GE(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 2u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckFailedMessageRetainOnSend)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    mockSender.mMessageDispatch.mRetainMessageOnSend = false;
    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was dropped
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);

    // Wait for the first re-transmit (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    // Ensure the retransmit table is empty, as we did not provide a message to retain
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckUnencryptedMessageReceiveFailure)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockSessionEstablishmentDelegate mockReceiver;
    CHIP_ERROR err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Expect the received messages to be encrypted
    mockReceiver.mMessageDispatch.mRequireEncryption = true;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = NewUnauthenticatedExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // We are sending a malicious packet, doesn't expect an ack
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kNoAutoRequestAck));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Test that the message was actually sent (and not dropped)
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    // Test that the message was dropped by the receiver
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckResendApplicationMessageWithPeerExchange)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);

    // Wait for the first re-transmit (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    EXPECT_GE(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestReliableMessageProtocol, CheckDuplicateMessageClosedExchange)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    }));

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Drop the ack, and also close the peer exchange
    mockReceiver.SetDropAckResponse(true);
    mockReceiver.mRetainExchange = false;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Let's not drop the duplicate message
    mockReceiver.SetDropAckResponse(false);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Wait for the first re-transmit and ack (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    DrainAndServiceIO();

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckDuplicateOldMessageClosedExchange)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    }));

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Drop the ack, and also close the peer exchange
    mockReceiver.SetDropAckResponse(true);
    mockReceiver.mRetainExchange = false;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Now send CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2 messages to make
    // sure our original message is out of the message counter window.  These
    // messages can be sent withour MRP, because we are not expecting acks for
    // them anyway.
    size_t extraMessages = CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2;
    for (size_t i = 0; i < extraMessages; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
        EXPECT_FALSE(buffer.IsNull());

        ExchangeContext * newExchange = NewExchangeToAlice(&mockSender);
        ASSERT_NE(newExchange, nullptr);

        mockReceiver.mRetainExchange = false;

        // Ensure the retransmit table has our one message right now
        EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

        // Send without MRP.
        err = newExchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kNoAutoRequestAck);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        DrainAndServiceIO();

        // Ensure the message was sent, but not added to the retransmit table.
        EXPECT_EQ(loopback.mSentMessageCount, 1u + (i + 1u));
        EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
        EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    }

    // Let's not drop the duplicate message's ack.
    mockReceiver.SetDropAckResponse(false);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Wait for the first re-transmit and ack (should take 64ms)
    rm->StartTimer();
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3 + extraMessages; });
    DrainAndServiceIO();

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    EXPECT_EQ(loopback.mSentMessageCount, 3u + extraMessages);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckResendSessionEstablishmentMessageWithPeerExchange)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    ASSERT_FALSE(buffer.IsNull());

    MockSessionEstablishmentDelegate mockReceiver;
    CHIP_ERROR err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = NewUnauthenticatedExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);

    // Wait for the first re-transmit (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    EXPECT_GE(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestReliableMessageProtocol, CheckDuplicateMessage)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    }));

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Drop the ack, and keep the exchange around to receive the duplicate message
    mockReceiver.SetDropAckResponse(true);
    mockReceiver.mRetainExchange = true;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    // Let's not drop the duplicate message
    mockReceiver.SetDropAckResponse(false);
    mockReceiver.mRetainExchange = false;

    // Wait for the first re-transmit and ack (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    DrainAndServiceIO();

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    mockReceiver.CloseExchangeIfNeeded();
}

TEST_F(TestReliableMessageProtocol, CheckReceiveAfterStandaloneAck)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // We send a message, have it get received by the peer, then an ack is
    // returned, then a reply is returned.  We need to keep the receiver
    // exchange alive until it does the message send (so we can send the
    // response from the receiver and so the initial sender exchange can get
    // it).
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not seen an ack yet.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    EXPECT_TRUE(receiverRc->IsAckPending());

    // Send the standalone ack.
    receiverRc->SendStandaloneAckMessage();
    DrainAndServiceIO();

    // Ensure the ack was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // Ensure that we have not gotten any app-level responses so far.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);

    // And that we have now gotten our ack.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the response and its ack was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 4u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // Ensure that we have received that response.
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckPiggybackAfterPiggyback)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // We send a message, have it get received by the peer, have the peer return
    // a piggybacked ack.  Then we send a second message this time _not_
    // requesting an ack, get a response, and see whether an ack was
    // piggybacked.  We need to keep both exchanges alive for that (so we can
    // send the response from the receiver and so the initial sender exchange
    // can get it).
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;
    mockSender.mRetainExchange    = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not seen an ack yet.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    EXPECT_TRUE(receiverRc->IsAckPending());

    // Ensure that we have not gotten any app-level responses or acks so far.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.mReceivedPiggybackAck);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err =
        mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer),
                                            SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the response was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // Ensure that we have received that response and it had a piggyback ack.
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_TRUE(mockSender.mReceivedPiggybackAck);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Now send a new message to the other side, but don't ask for an ack.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer),
                                SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    // And that we are not expecting an ack.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // Send the final response.  At this point we don't need to keep the
    // exchanges alive anymore.
    mockReceiver.mRetainExchange = false;
    mockSender.mRetainExchange   = false;

    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the response and its ack was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 5u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // Ensure that we have received that response and it had a piggyback ack.
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_TRUE(mockSender.mReceivedPiggybackAck);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckSendUnsolicitedStandaloneAckMessage)
{
    /**
     * Tests sending a standalone ack message that is:
     * 1) Unsolicited.
     * 2) Requests an ack.
     *
     * This is not a thing that would normally happen, but a malicious entity
     * could absolutely do this.
     */
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData("", 0);
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // We send a message, have it get received by the peer, expect an ack from
    // the peer.
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Purposefully sending a standalone ack that requests an ack!
    err = exchange->SendMessage(SecureChannel::MsgType::StandaloneAck, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    // Needs a manual close, because SendMessage does not close for standalone acks.
    exchange->Close();
    DrainAndServiceIO();

    // Ensure the message and its ack were sent.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that nothing is waiting for acks.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckSendStandaloneAckMessage)
{
    MockAppDelegate mockAppDelegate(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockAppDelegate);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm     = GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    ASSERT_NE(rm, nullptr);
    ASSERT_NE(rc, nullptr);

    EXPECT_EQ(rc->SendStandaloneAckMessage(), CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Need manual close because standalone acks don't close exchanges.
    exchange->Close();
}

TEST_F(TestReliableMessageProtocol, CheckMessageAfterClosed)
{
    /**
     * This test performs the following sequence of actions, where all messages
     * are sent with MRP enabled:
     *
     * 1) Initiator sends message to responder.
     * 2) Responder responds to the message (piggybacking an ack) and closes
     *    the exchange.
     * 3) Initiator sends a response to the response on the same exchange, again
     *    piggybacking an ack.
     *
     * This is basically the "command, response, status response" flow, with the
     * responder closing the exchange after it sends the response.
     */

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    // We need to keep both exchanges alive for the thing we are testing here.
    mockReceiver.mRetainExchange = true;
    mockSender.mRetainExchange   = true;

    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockReceiver.mReceivedPiggybackAck);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockReceiver.mReceivedPiggybackAck);

    // And that we have not seen an ack yet.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    EXPECT_TRUE(receiverRc->IsAckPending());

    // Ensure that we have not gotten any app-level responses or acks so far.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.mReceivedPiggybackAck);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the response was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // Ensure that we have received that response and it had a piggyback ack.
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_TRUE(mockSender.mReceivedPiggybackAck);
    // And that we are now waiting for an ack for the response.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockReceiver.mReceivedPiggybackAck     = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Now send a second message to the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent (and the ack for it was also sent).
    EXPECT_EQ(loopback.mSentMessageCount, 4u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was not received (because the exchange is closed on the
    // receiver).
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);

    // And that we are not expecting an ack; acks should have been flushed
    // immediately on the receiver, due to the exchange being closed.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckLostResponseWithPiggyback)
{
    /**
     * This tests the following scenario:
     * 1) A reliable message is sent from initiator to responder.
     * 2) The responder sends a response with a piggybacked ack, which is lost.
     * 3) Initiator resends the message.
     * 4) Responder responds to the resent message with a standalone ack.
     * 5) The responder retransmits the application-level response.
     * 4) The initiator should receive the application-level response.
     */
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // Make sure that we resend our message before the other side does.
    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // We send a message, the other side sends an application-level response
    // (which is lost), then we do a retransmit that is acked, then the other
    // side does a retransmit.  We need to keep the receiver exchange alive (so
    // we can send the response from the receiver), but don't need anything
    // special for the sender exchange, because it will be waiting for the
    // application-level response.
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not gotten any app-level responses or acks so far.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    // Should have pending ack here.
    EXPECT_TRUE(receiverRc->IsAckPending());
    // Make sure receiver resends after sender does, and there's enough of a gap
    // that we are very unlikely to actually trigger the resends on the receiver
    // when we trigger the resends on the sender.
    mockReceiver.mExchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        256_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        256_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Now send a message from the other side, but drop it.
    loopback.mNumMessagesToDrop = 1;

    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    // Stop keeping receiver exchange alive.
    mockReceiver.mRetainExchange = true;

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the response was sent but dropped.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);

    // Ensure that we have not received that response.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.mReceivedPiggybackAck);
    // We now have our un-acked message still waiting to retransmit and the
    // message that the other side sent is waiting for an ack.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 2);

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockReceiver.mReceivedPiggybackAck     = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Wait for re-transmit from sender and ack (should take 64ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    DrainAndServiceIO();

    // We resent our first message, which did not make it to the app-level
    // listener on the receiver (because it's a duplicate) but did trigger a
    // standalone ack.
    //
    // Now the annoying part is that depending on how long we _actually_ slept
    // we might have also triggered the retransmit from the other side, even
    // though we did not want to.  Handle both cases here.
    EXPECT_TRUE(loopback.mSentMessageCount == 4 || loopback.mSentMessageCount == 6);
    if (loopback.mSentMessageCount == 4)
    {
        // Just triggered the retransmit from the sender.
        EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
        EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);
        EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
        EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    }
    else
    {
        // Also triggered the retransmit from the receiver.
        EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
        EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);
        EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
        EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
    }

    // Wait for re-transmit from receiver (should take 256ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 6; });
    DrainAndServiceIO();

    // And now we've definitely resent our response message, which should show
    // up as an app-level message and trigger a standalone ack.
    EXPECT_EQ(loopback.mSentMessageCount, 6u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);

    // Should be all done now.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckIsPeerActiveNotInitiator)
{
    /**
     * This tests the following scenario:
     * 1) A reliable message expecting a response is sent from the initiator to responder which is lost
     * 2) Initiator resends the message at the IdleRetrans interval
     * 3) Responder receives the message and sends a standalone ack
     * 4) Responder sends a response and fails
     * 5) Responder retries at the ActiveRestrans interval
     * 6) Initiator receives the response
     */

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        1000_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        1000_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    mockReceiver.mRetainExchange = true;
    mockSender.mRetainExchange   = true;

    EXPECT_FALSE(exchange->HasReceivedAtLeastOneMessage());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Verify that the first message is dropped
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);

    // Make sure retransmit was not done before the idle restrans interval hits
    GetIOContext().DriveIOUntil(500_ms32, [&] { return loopback.mSentMessageCount >= 1; });
    DrainAndServiceIO();

    EXPECT_FALSE(exchange->HasReceivedAtLeastOneMessage());

    // // Make sure nothing happened
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);

    // // Retrasnmit message
    GetIOContext().DriveIOUntil(2000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    EXPECT_FALSE(exchange->HasReceivedAtLeastOneMessage());

    // // Make sure nothing happened
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    // // Verify that the receiver considers the sender is active
    EXPECT_FALSE(exchange->HasReceivedAtLeastOneMessage());
    EXPECT_TRUE(mockReceiver.mExchange->HasReceivedAtLeastOneMessage());

    mockReceiver.mExchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        1000_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        100_ms32,  // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    mockReceiver.mRetainExchange = false;
    mockSender.mRetainExchange   = false;

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    // Make receiver message fail once
    loopback.mNumMessagesToDrop = 1;

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Make sure nothing happened
    EXPECT_EQ(loopback.mDroppedMessageCount, 2u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);

    // // Retrasnmit message
    GetIOContext().DriveIOUntil(500_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    DrainAndServiceIO();

    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_EQ(loopback.mSentMessageCount, 5u);
}

TEST_F(TestReliableMessageProtocol, CheckLostStandaloneAck)
{
    /**
     * This tests the following scenario:
     * 1) A reliable message is sent from initiator to responder.
     * 2) The responder sends a standalone ack, which is lost.
     * 3) The responder sends an application-level response.
     * 4) The initiator sends a reliable response to the app-level response.
     *
     * This should succeed, with all application-level messages being delivered
     * and no crashes.
     */
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    // We send a message, the other side sends a standalone ack first (which is
    // lost), then an application response, then we respond to that response.
    // We need to keep both exchanges alive for that (so we can send the
    // response from the receiver and so the initial sender exchange can send a
    // response to that).
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;
    mockSender.mRetainExchange    = true;

    // And ensure the ack heading back our way is dropped.
    mockReceiver.SetDropAckResponse(true);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not gotten any app-level responses or acks so far.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    // Ack should have been dropped.
    EXPECT_FALSE(receiverRc->IsAckPending());

    // Don't drop any more acks.
    mockReceiver.SetDropAckResponse(false);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer),
                                              SendFlags(SendMessageFlags::kExpectResponse));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the response was sent.
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // Ensure that we have received that response and had a piggyback ack.
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);
    EXPECT_TRUE(mockSender.mReceivedPiggybackAck);
    // We now have just the received message waiting for an ack.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // And receiver still has no ack pending.
    EXPECT_FALSE(receiverRc->IsAckPending());

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockReceiver.mReceivedPiggybackAck     = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Stop retaining the recipient exchange.
    mockReceiver.mRetainExchange = false;

    // Now send a new message to the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message and the standalone ack to it were sent.
    EXPECT_EQ(loopback.mSentMessageCount, 4u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    // And that it was received.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_TRUE(mockReceiver.mReceivedPiggybackAck);

    // At this point all our exchanges and reliable message contexts should be
    // dead, so we can't test anything about their state.

    // And that there are no un-acked messages left.
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);
}

TEST_F(TestReliableMessageProtocol, CheckGetBackoff)
{
    CheckGetBackoffImpl(System::Clock::kZero);
}

TEST_F(TestReliableMessageProtocol, CheckGetBackoffAdditionalTime)
{
    CheckGetBackoffImpl(System::Clock::Seconds32(1));
}

TEST_F(TestReliableMessageProtocol, CheckApplicationResponseDelayed)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    err = CreateCASESessionBobToAlice();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = CreateCASESessionAliceToBob();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    mockReceiver.mRetainExchange = true;

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    constexpr uint32_t kMaxMRPTransmits = 5; // Counting the initial message.

    // Let's drop all but the last MRP transmit.
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = kMaxMRPTransmits - 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    exchange->SetResponseTimeout(3000_ms32);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kExpectResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, kMaxMRPTransmits - 2u);
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);

    // Wait for all but the last retransmit to happen.
    GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mDroppedMessageCount >= kMaxMRPTransmits - 1; });
    DrainAndServiceIO();

    // Ensure that nothing has been sent yet.
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits - 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);

    // Now allow through the next message (our last retransmit), but make sure
    // there is no standalone ack for it.
    mockReceiver.SetDropAckResponse(true);
    GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits; });
    DrainAndServiceIO();

    // Verify that message was sent and received but nothing else has been sent.
    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);        // We have no ack yet.
    ASSERT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got the message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // It was not a commissioning CASE session so that is lined-up properly.
    EXPECT_FALSE(mockReceiver.mLastSubjectDescriptor.isCommissioning);

    // Ensure there will be no more weirdness with acks and that our MRP timer is restarted properly.
    mockReceiver.SetDropAckResponse(false);

    // Now send a response, but drop all but the last MRP retransmit.
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = kMaxMRPTransmits - 1;
    loopback.mDroppedMessageCount = 0;

    mockReceiver.mExchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // At this point, we should have two MRP contexts pending.
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 2);        // We have no ack yet.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // Now wait for all but the last retransmit to happen from the other side.
    GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits - 1; });
    DrainAndServiceIO();

    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    // We might have timed our MRP resends out, or not, but the other side is waiting for an ack.
    EXPECT_TRUE(rm->TestGetCountRetransTable() == 1 || rm->TestGetCountRetransTable() == 2);
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // Now wait for us to time out our MRP context for sure.
    GetIOContext().DriveIOUntil(5000_ms32, [&] { return rm->TestGetCountRetransTable() == 1; });
    DrainAndServiceIO();

    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);        // We timed out our MRP context.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We did not get a response.
    EXPECT_FALSE(mockSender.mResponseTimedOut);          // We did not time out yet.

    // Now wait for the last retransmit (and our ack) to to happen.
    GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits; });
    DrainAndServiceIO();

    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits + 1);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);        // Everything has been acked.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    EXPECT_TRUE(mockSender.IsOnMessageReceivedCalled);   // We got the response.
    EXPECT_FALSE(mockSender.mResponseTimedOut);          // We did not time out yet.

    // Ensure that we did not mark any sessions defunct.
    EXPECT_FALSE(GetSessionBobToAlice()->AsSecureSession()->IsDefunct());
    EXPECT_FALSE(GetSessionAliceToBob()->AsSecureSession()->IsDefunct());

    // Reset our sessions, so other tests get the usual PASE session
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    err = CreateSessionBobToAlice();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = CreateSessionAliceToBob();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestReliableMessageProtocol, CheckApplicationResponseNeverComes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    err = CreateCASESessionBobToAlice();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = CreateCASESessionAliceToBob();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockAppDelegate mockReceiver(*this);
    err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    constexpr uint32_t kMaxMRPTransmits = 5; // Counting the initial message.

    // Let's drop all but the last MRP transmit.
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = kMaxMRPTransmits - 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    exchange->SetResponseTimeout(2500_ms32);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kExpectResponse);
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, kMaxMRPTransmits - 2u);
    EXPECT_EQ(loopback.mSentMessageCount, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);

    // Wait for all but the last retransmit to happen.
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mDroppedMessageCount >= kMaxMRPTransmits - 1; });
    DrainAndServiceIO();

    // Ensure that nothing has been sent yet.
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits - 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_FALSE(mockReceiver.IsOnMessageReceivedCalled);
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);

    // Now allow through the next message (our last retransmit), but make sure
    // there is no standalone ack for it.
    mockReceiver.SetDropAckResponse(true);
    GetIOContext().DriveIOUntil(500_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits; });
    DrainAndServiceIO();

    // Verify that message was sent and received but nothing else has been sent.
    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);        // We have no ack yet.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got the message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // Ensure there will be no more weirdness with acks and that our MRP timer is restarted properly.
    mockReceiver.SetDropAckResponse(false);

    // Now wait for us to time out our MRP context.
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return rm->TestGetCountRetransTable() == 0; });
    DrainAndServiceIO();

    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);        // We timed out our MRP context.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We did not get a response.
    EXPECT_FALSE(mockSender.mResponseTimedOut);          // We did not time out yet.

    // Now wait for our exchange to time out.
    GetIOContext().DriveIOUntil(3000_ms32, [&] { return mockSender.mResponseTimedOut; });
    DrainAndServiceIO();

    EXPECT_EQ(loopback.mSentMessageCount, kMaxMRPTransmits);
    EXPECT_EQ(loopback.mDroppedMessageCount, kMaxMRPTransmits - 1);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);        // We timed this out long ago.
    EXPECT_TRUE(mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    EXPECT_FALSE(mockSender.IsOnMessageReceivedCalled);  // We never got a response.
    EXPECT_TRUE(mockSender.mResponseTimedOut);           // We tiemd out

    // We should have marked out session defunct.
    EXPECT_TRUE(GetSessionBobToAlice()->AsSecureSession()->IsDefunct());
    // Other side had no reason to mark its session defunct.
    EXPECT_FALSE(GetSessionAliceToBob()->AsSecureSession()->IsDefunct());

    // Reset our sessions, so other tests get the usual PASE session
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    err = CreateSessionBobToAlice();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = CreateSessionAliceToBob();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    err = GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
TEST_F(TestReliableMessageProtocol, CheckReliableMessageAnalyticsForTransmitEventualSuccessForEsablishedCase)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    err = CreateCASESessionBobToAlice();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = CreateCASESessionAliceToBob();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);
    TestReliablityAnalyticDelegate testAnalyticsDelegate;
    rm->RegisterAnalyticsDelegate(&testAnalyticsDelegate);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    const auto expectedFabricIndex = exchange->GetSessionHandle()->GetFabricIndex();
    const auto expectedNodeId      = exchange->GetSessionHandle()->AsSecureSession()->GetPeerNodeId();

    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 4;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the initial message was dropped and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the initial message to fail (should take 330-413ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    // Ensure the 1st retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 2u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    DrainAndServiceIO();

    // Ensure the 2nd retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 3u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    DrainAndServiceIO();

    // Ensure the 3rd retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 4u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 0u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 4u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Trigger final transmission
    GetIOContext().DriveIOUntil(1500_ms32, [&] { return loopback.mSentMessageCount >= 5; });
    DrainAndServiceIO();

    // Ensure the last retransmission was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    EXPECT_GE(loopback.mSentMessageCount, 5u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 4u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    ASSERT_EQ(testAnalyticsDelegate.mTransmitEvents.size(), 6u);
    auto firstTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(firstTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(firstTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(firstTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kInitialSend);
    EXPECT_EQ(firstTransmitEvent.retransmissionCount, std::nullopt);
    // We have no way of validating the first messageCounter since this is a randomly generated value, but it should
    // remain constant for all subsequent transmit events in this test.
    const uint32_t messageCounter = firstTransmitEvent.messageCounter;

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto secondTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(secondTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(secondTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(secondTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(secondTransmitEvent.retransmissionCount, 1);
    EXPECT_EQ(messageCounter, secondTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto thirdTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(thirdTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(thirdTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(thirdTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(thirdTransmitEvent.retransmissionCount, 2);
    EXPECT_EQ(messageCounter, thirdTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto fourthTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(fourthTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(fourthTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(fourthTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(fourthTransmitEvent.retransmissionCount, 3);
    EXPECT_EQ(messageCounter, fourthTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto fifthTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(fifthTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(fifthTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(fifthTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(fifthTransmitEvent.retransmissionCount, 4);
    EXPECT_EQ(messageCounter, fifthTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto sixthTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(sixthTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(sixthTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(sixthTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kAcknowledged);
    EXPECT_EQ(sixthTransmitEvent.retransmissionCount, std::nullopt);
    EXPECT_EQ(messageCounter, sixthTransmitEvent.messageCounter);
}

TEST_F(TestReliableMessageProtocol, CheckReliableMessageAnalyticsForTransmitFailureForEsablishedCase)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ExpireSessionBobToAlice();
    ExpireSessionAliceToBob();
    err = CreateCASESessionBobToAlice();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    err = CreateCASESessionAliceToBob();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);
    TestReliablityAnalyticDelegate testAnalyticsDelegate;
    rm->RegisterAnalyticsDelegate(&testAnalyticsDelegate);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    const auto expectedFabricIndex = exchange->GetSessionHandle()->GetFabricIndex();
    const auto expectedNodeId      = exchange->GetSessionHandle()->AsSecureSession()->GetPeerNodeId();

    // Let's drop the initial message
    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 5;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Ensure the initial message was dropped and was added to retransmit table
    EXPECT_EQ(loopback.mNumMessagesToDrop, 4u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 1u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Wait for the initial message to fail (should take 330-413ms)
    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    DrainAndServiceIO();

    // Ensure the 1st retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 3u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 2u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    DrainAndServiceIO();

    // Ensure the 2nd retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 3u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 3u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    DrainAndServiceIO();

    // Ensure the 3rd retry was dropped, and is still there in the retransmit table
    EXPECT_EQ(loopback.mSentMessageCount, 4u);
    EXPECT_EQ(loopback.mNumMessagesToDrop, 1u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 4u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);

    // Trigger final transmission
    GetIOContext().DriveIOUntil(1500_ms32, [&] { return loopback.mSentMessageCount >= 5; });
    DrainAndServiceIO();

    // Ensure the last retransmission was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    EXPECT_GE(loopback.mSentMessageCount, 5u);
    EXPECT_EQ(rm->TestGetCountRetransTable(), 1);
    EXPECT_EQ(loopback.mDroppedMessageCount, 5u);

    // Now wait for our exchange to time out.
    GetIOContext().DriveIOUntil(3000_ms32, [&] { return rm->TestGetCountRetransTable() == 0; });
    DrainAndServiceIO();
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    ASSERT_EQ(testAnalyticsDelegate.mTransmitEvents.size(), 6u);
    auto firstTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(firstTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(firstTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(firstTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kInitialSend);
    EXPECT_EQ(firstTransmitEvent.retransmissionCount, std::nullopt);
    // We have no way of validating the first messageCounter since this is a randomly generated value, but it should
    // remain constant for all subsequent transmit events in this test.
    const uint32_t messageCounter = firstTransmitEvent.messageCounter;

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto secondTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(secondTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(secondTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(secondTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(secondTransmitEvent.retransmissionCount, 1);
    EXPECT_EQ(messageCounter, secondTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto thirdTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(thirdTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(thirdTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(thirdTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(thirdTransmitEvent.retransmissionCount, 2);
    EXPECT_EQ(messageCounter, thirdTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto fourthTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(fourthTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(fourthTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(fourthTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(fourthTransmitEvent.retransmissionCount, 3);
    EXPECT_EQ(messageCounter, fourthTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto fifthTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(fifthTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(fifthTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(fifthTransmitEvent.retransmissionCount, 4);
    EXPECT_EQ(fifthTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kRetransmission);
    EXPECT_EQ(messageCounter, fifthTransmitEvent.messageCounter);

    testAnalyticsDelegate.mTransmitEvents.pop();
    auto sixthTransmitEvent = testAnalyticsDelegate.mTransmitEvents.front();
    EXPECT_EQ(sixthTransmitEvent.nodeId, expectedNodeId);
    EXPECT_EQ(sixthTransmitEvent.fabricIndex, expectedFabricIndex);
    EXPECT_EQ(sixthTransmitEvent.retransmissionCount, std::nullopt);
    EXPECT_EQ(sixthTransmitEvent.eventType, ReliableMessageAnalyticsDelegate::EventType::kFailed);
    EXPECT_EQ(messageCounter, sixthTransmitEvent.messageCounter);
}

TEST_F(TestReliableMessageProtocol, CheckReliableMessageAnalyticsForTransmitEsablishedPase)
{
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockAppDelegate mockSender(*this);
    ExchangeContext * exchange = NewExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    TestReliablityAnalyticDelegate testAnalyticsDelegate;
    rm->RegisterAnalyticsDelegate(&testAnalyticsDelegate);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    ASSERT_TRUE(exchange->GetSessionHandle()->AsSecureSession()->IsPASESession());

    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Test that the message was actually sent (and not dropped)
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    ASSERT_EQ(testAnalyticsDelegate.mTransmitEvents.size(), 0u);
}

TEST_F(TestReliableMessageProtocol, CheckReliableMessageAnalyticsForTransmitUnauthenticatedExchange)
{
    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    EXPECT_FALSE(buffer.IsNull());

    MockSessionEstablishmentDelegate mockReceiver;
    CHIP_ERROR err = GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    EXPECT_EQ(err, CHIP_NO_ERROR);

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = NewUnauthenticatedExchangeToAlice(&mockSender);
    ASSERT_NE(exchange, nullptr);

    ReliableMessageMgr * rm = GetExchangeManager().GetReliableMessageMgr();
    ASSERT_NE(rm, nullptr);

    TestReliablityAnalyticDelegate testAnalyticsDelegate;
    rm->RegisterAnalyticsDelegate(&testAnalyticsDelegate);

    exchange->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    auto & loopback               = GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    EXPECT_EQ(err, CHIP_NO_ERROR);
    DrainAndServiceIO();

    // Test that the message was actually sent (and not dropped)
    EXPECT_EQ(loopback.mSentMessageCount, 2u);
    EXPECT_EQ(loopback.mDroppedMessageCount, 0u);

    EXPECT_EQ(rm->TestGetCountRetransTable(), 0);

    ASSERT_EQ(testAnalyticsDelegate.mTransmitEvents.size(), 0u);
}
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

/**
 * TODO: A test that we should have but can't write with the existing
 * infrastructure we have:
 *
 * 1. A sends message 1 to B
 * 2. B is slow to respond, A does a resend and the resend is delayed in the network.
 * 3. B responds with message 2, which acks message 1.
 * 4. A sends message 3 to B
 * 5. B sends standalone ack to message 3, which is lost
 * 6. The duplicate message from step 3 is delivered and triggers a standalone ack.
 * 7. B responds with message 4, which should carry a piggyback ack for message 3
 *    (this is the part that needs testing!)
 * 8. A sends message 5 to B.
 */
