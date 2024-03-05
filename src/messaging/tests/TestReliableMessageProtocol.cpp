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

#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <lib/support/UnitTestUtils.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDConfigurationData.h> // nogncheck
#endif

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;
using namespace chip::System::Clock::Literals;

const char PAYLOAD[] = "Hello!";

// The CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST can be set to non-zero value
// to boost the retransmission timeout for a high latency network like Thread to
// avoid spurious retransmits.
//
// This adds extra I/O time to account for this. See the documentation for
// CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST for more details.
constexpr auto retryBoosterTimeout = CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS * CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST;

class TestContext : public chip::Test::LoopbackMessagingContext
{
public:
    // Performs setup for each individual test in the test suite
    CHIP_ERROR SetUp() override
    {
        ReturnErrorOnFailure(chip::Test::LoopbackMessagingContext::SetUp());
        GetSessionAliceToBob()->AsSecureSession()->SetRemoteSessionParameters(GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
        GetSessionBobToAlice()->AsSecureSession()->SetRemoteSessionParameters(GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
        return CHIP_NO_ERROR;
    }
};

class MockAppDelegate : public UnsolicitedMessageHandler, public ExchangeDelegate
{
public:
    MockAppDelegate(TestContext & ctx) : mTestContext(ctx) {}

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

        if (mTestSuite != nullptr)
        {
            NL_TEST_ASSERT(mTestSuite, buffer->TotalLength() == sizeof(PAYLOAD));
            NL_TEST_ASSERT(mTestSuite, memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()) == 0);
        }
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
            mTestContext.GetExchangeManager().GetReliableMessageMgr()->StartTimer();
        }
    }

    bool IsOnMessageReceivedCalled = false;
    bool mReceivedPiggybackAck     = false;
    bool mRetainExchange           = false;
    bool mResponseTimedOut         = false;
    ExchangeContext * mExchange    = nullptr;
    nlTestSuite * mTestSuite       = nullptr;

private:
    TestContext & mTestContext;
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
        if (mTestSuite != nullptr)
        {
            NL_TEST_ASSERT(mTestSuite, buffer->TotalLength() == sizeof(PAYLOAD));
            NL_TEST_ASSERT(mTestSuite, memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()) == 0);
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    virtual ExchangeMessageDispatch & GetMessageDispatch() override { return mMessageDispatch; }

    bool IsOnMessageReceivedCalled = false;
    MockSessionEstablishmentExchangeDispatch mMessageDispatch;
    nlTestSuite * mTestSuite = nullptr;
};

struct BackoffComplianceTestVector
{
    uint8_t sendCount;
    System::Clock::Timeout backoffBase;
    System::Clock::Timeout backoffMin;
    System::Clock::Timeout backoffMax;
};

struct BackoffComplianceTestVector theBackoffComplianceTestVector[] = {
    {
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
        .backoffMax  = System::Clock::Timeout(660),
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
        .backoffMax  = System::Clock::Timeout(1690),
    },
    {
        .sendCount   = 5,
        .backoffBase = System::Clock::Timeout(300),
        .backoffMin  = System::Clock::Timeout(2162),
        .backoffMax  = System::Clock::Timeout(2704),
    },
    {
        .sendCount   = 6,
        .backoffBase = System::Clock::Timeout(300),
        .backoffMin  = System::Clock::Timeout(2162),
        .backoffMax  = System::Clock::Timeout(2704),
    },
    {
        .sendCount   = 0,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(4400),
        .backoffMax  = System::Clock::Timeout(5500),
    },
    {
        .sendCount   = 1,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(4400),
        .backoffMax  = System::Clock::Timeout(5500),
    },
    {
        .sendCount   = 2,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(7040),
        .backoffMax  = System::Clock::Timeout(8800),
    },
    {
        .sendCount   = 3,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(11264),
        .backoffMax  = System::Clock::Timeout(14081),
    },
    {
        .sendCount   = 4,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(18022),
        .backoffMax  = System::Clock::Timeout(22529),
    },
    {
        .sendCount   = 5,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(28835),
        .backoffMax  = System::Clock::Timeout(36045),
    },
    {
        .sendCount   = 6,
        .backoffBase = System::Clock::Timeout(4000),
        .backoffMin  = System::Clock::Timeout(28835),
        .backoffMax  = System::Clock::Timeout(36045),
    },
};

} // namespace

class TestReliableMessageProtocol
{
public:
    static void CheckAddClearRetrans(nlTestSuite * inSuite, void * inContext);
    static void CheckResendApplicationMessage(nlTestSuite * inSuite, void * inContext);
    static void CheckCloseExchangeAndResendApplicationMessage(nlTestSuite * inSuite, void * inContext);
    static void CheckFailedMessageRetainOnSend(nlTestSuite * inSuite, void * inContext);
    static void CheckResendApplicationMessageWithPeerExchange(nlTestSuite * inSuite, void * inContext);
    static void CheckResendSessionEstablishmentMessageWithPeerExchange(nlTestSuite * inSuite, void * inContext);
    static void CheckDuplicateMessage(nlTestSuite * inSuite, void * inContext);
    static void CheckDuplicateMessageClosedExchange(nlTestSuite * inSuite, void * inContext);
    static void CheckDuplicateOldMessageClosedExchange(nlTestSuite * inSuite, void * inContext);
    static void CheckReceiveAfterStandaloneAck(nlTestSuite * inSuite, void * inContext);
    static void CheckPiggybackAfterPiggyback(nlTestSuite * inSuite, void * inContext);
    static void CheckSendUnsolicitedStandaloneAckMessage(nlTestSuite * inSuite, void * inContext);
    static void CheckSendStandaloneAckMessage(nlTestSuite * inSuite, void * inContext);
    static void CheckMessageAfterClosed(nlTestSuite * inSuite, void * inContext);
    static void CheckUnencryptedMessageReceiveFailure(nlTestSuite * inSuite, void * inContext);
    static void CheckLostResponseWithPiggyback(nlTestSuite * inSuite, void * inContext);
    static void CheckLostStandaloneAck(nlTestSuite * inSuite, void * inContext);
    static void CheckIsPeerActiveNotInitiator(nlTestSuite * inSuite, void * inContext);
    static void CheckGetBackoff(nlTestSuite * inSuite, void * inContext);
    static void CheckApplicationResponseDelayed(nlTestSuite * inSuite, void * inContext);
    static void CheckApplicationResponseNeverComes(nlTestSuite * inSuite, void * inContext);
};

void TestReliableMessageProtocol::CheckAddClearRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate mockAppDelegate(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageMgr::RetransTableEntry * entry;

    rm->AddToRetransTable(rc, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    rm->ClearRetransTable(*entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

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
void TestReliableMessageProtocol::CheckResendApplicationMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);
    BackoffComplianceTestVector * expectedBackoff;
    System::Clock::Timestamp now, startTime;
    System::Clock::Timeout timeoutTime, margin;
    margin = System::Clock::Timeout(15);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender(ctx);
    // TODO: temporarily create a SessionHandle from node id, will be fix in PR 3602
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        System::Clock::Timestamp(300), // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        System::Clock::Timestamp(300), // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 4;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // Ensure the exchange stays open after we send (unlike the CheckCloseExchangeAndResendApplicationMessage case), by claiming to
    // expect a response.
    startTime = System::SystemClock().GetMonotonicTimestamp();
    err       = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the initial message was dropped and was added to retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 3);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Wait for the initial message to fail (should take 330-413ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32 + retryBoosterTimeout, [&] { return loopback.mSentMessageCount >= 2; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #1  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[0];
    NL_TEST_ASSERT(inSuite, timeoutTime >= expectedBackoff->backoffMin - margin);

    startTime = System::SystemClock().GetMonotonicTimestamp();
    ctx.DrainAndServiceIO();

    // Ensure the 1st retry was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Wait for the 1st retry to fail (should take 330-413ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32 + retryBoosterTimeout, [&] { return loopback.mSentMessageCount >= 3; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #2  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[1];
    NL_TEST_ASSERT(inSuite, timeoutTime >= expectedBackoff->backoffMin - margin);

    startTime = System::SystemClock().GetMonotonicTimestamp();
    ctx.DrainAndServiceIO();

    // Ensure the 2nd retry was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 3);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Wait for the 2nd retry to fail (should take 528-660ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32 + retryBoosterTimeout, [&] { return loopback.mSentMessageCount >= 4; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #3  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[2];
    NL_TEST_ASSERT(inSuite, timeoutTime >= expectedBackoff->backoffMin - margin);

    startTime = System::SystemClock().GetMonotonicTimestamp();
    ctx.DrainAndServiceIO();

    // Ensure the 3rd retry was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 4);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 4);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Wait for the 3rd retry to fail (should take 845-1056ms)
    ctx.GetIOContext().DriveIOUntil(1500_ms32 + retryBoosterTimeout, [&] { return loopback.mSentMessageCount >= 5; });
    now         = System::SystemClock().GetMonotonicTimestamp();
    timeoutTime = now - startTime;
    ChipLogProgress(Test, "Attempt #4  Timeout : %" PRIu32 "ms", timeoutTime.count());
    expectedBackoff = &theBackoffComplianceTestVector[3];
    NL_TEST_ASSERT(inSuite, timeoutTime >= expectedBackoff->backoffMin - margin);

    // Trigger final transmission
    ctx.DrainAndServiceIO();

    // Ensure the last retransmission was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount >= 5);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 4);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    exchange->Close();
}

void TestReliableMessageProtocol::CheckCloseExchangeAndResendApplicationMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender(ctx);
    // TODO: temporarily create a SessionHandle from node id, will be fixed in PR 3602
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 2;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Wait for the first re-transmit (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit message was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Wait for the second re-transmit (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit message was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount >= 3);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckFailedMessageRetainOnSend(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    mockSender.mMessageDispatch.mRetainMessageOnSend = false;
    // Let's drop the initial message
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was dropped
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);

    // Wait for the first re-transmit (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit table is empty, as we did not provide a message to retain
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckUnencryptedMessageReceiveFailure(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    MockSessionEstablishmentDelegate mockReceiver;
    CHIP_ERROR err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Expect the received messages to be encrypted
    mockReceiver.mMessageDispatch.mRequireEncryption = true;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = ctx.NewUnauthenticatedExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // We are sending a malicious packet, doesn't expect an ack
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Test that the message was actually sent (and not dropped)
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    // Test that the message was dropped by the receiver
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckResendApplicationMessageWithPeerExchange(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);

    // Wait for the first re-transmit (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount >= 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    mockReceiver.mTestSuite = nullptr;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestReliableMessageProtocol::CheckDuplicateMessageClosedExchange(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    }));

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Drop the ack, and also close the peer exchange
    mockReceiver.SetDropAckResponse(true);
    mockReceiver.mRetainExchange = false;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Let's not drop the duplicate message
    mockReceiver.SetDropAckResponse(false);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Wait for the first re-transmit and ack (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckDuplicateOldMessageClosedExchange(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    }));

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Drop the ack, and also close the peer exchange
    mockReceiver.SetDropAckResponse(true);
    mockReceiver.mRetainExchange = false;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Now send CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2 messages to make
    // sure our original message is out of the message counter window.  These
    // messages can be sent withour MRP, because we are not expecting acks for
    // them anyway.
    size_t extraMessages = CHIP_CONFIG_MESSAGE_COUNTER_WINDOW_SIZE + 2;
    for (size_t i = 0; i < extraMessages; ++i)
    {
        buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
        NL_TEST_ASSERT(inSuite, !buffer.IsNull());

        ExchangeContext * newExchange = ctx.NewExchangeToAlice(&mockSender);
        NL_TEST_ASSERT(inSuite, newExchange != nullptr);

        mockReceiver.mRetainExchange = false;

        // Ensure the retransmit table has our one message right now
        NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

        // Send without MRP.
        err = newExchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kNoAutoRequestAck);
        NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
        ctx.DrainAndServiceIO();

        // Ensure the message was sent, but not added to the retransmit table.
        NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1 + (i + 1));
        NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
        NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    }

    // Let's not drop the duplicate message's ack.
    mockReceiver.SetDropAckResponse(false);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Wait for the first re-transmit and ack (should take 64ms)
    rm->StartTimer();
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3 + extraMessages; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 3 + extraMessages);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckResendSessionEstablishmentMessageWithPeerExchange(nlTestSuite * inSuite, void * inContext)
{
    // Making this static to reduce stack usage, as some platforms have limits on stack size.
    static Test::MessagingContext ctx;

    TestContext & inctx = *static_cast<TestContext *>(inContext);

    CHIP_ERROR err = ctx.InitFromExisting(inctx);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    MockSessionEstablishmentDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = ctx.NewUnauthenticatedExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsUnauthenticatedSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    // Let's drop the initial message
    auto & loopback               = inctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    inctx.DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);

    // Wait for the first re-transmit (should take 64ms)
    inctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    inctx.DrainAndServiceIO();

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount >= 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    mockReceiver.mTestSuite = nullptr;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.ShutdownAndRestoreExisting(inctx);
}

void TestReliableMessageProtocol::CheckDuplicateMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        64_ms32, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    }));

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Drop the ack, and keep the exchange around to receive the duplicate message
    mockReceiver.SetDropAckResponse(true);
    mockReceiver.mRetainExchange = true;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Let's not drop the duplicate message
    mockReceiver.SetDropAckResponse(false);
    mockReceiver.mRetainExchange = false;

    // Wait for the first re-transmit and ack (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 3; });
    ctx.DrainAndServiceIO();

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    mockReceiver.CloseExchangeIfNeeded();
}

void TestReliableMessageProtocol::CheckReceiveAfterStandaloneAck(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // We send a message, have it get received by the peer, then an ack is
    // returned, then a reply is returned.  We need to keep the receiver
    // exchange alive until it does the message send (so we can send the
    // response from the receiver and so the initial sender exchange can get
    // it).
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not seen an ack yet.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, receiverRc->IsAckPending());

    // Send the standalone ack.
    receiverRc->SendStandaloneAckMessage();
    ctx.DrainAndServiceIO();

    // Ensure the ack was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // Ensure that we have not gotten any app-level responses so far.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // And that we have now gotten our ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the response and its ack was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 4);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckPiggybackAfterPiggyback(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // We send a message, have it get received by the peer, have the peer return
    // a piggybacked ack.  Then we send a second message this time _not_
    // requesting an ack, get a response, and see whether an ack was
    // piggybacked.  We need to keep both exchanges alive for that (so we can
    // send the response from the receiver and so the initial sender exchange
    // can get it).
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;
    mockSender.mRetainExchange    = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not seen an ack yet.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, receiverRc->IsAckPending());

    // Ensure that we have not gotten any app-level responses or acks so far.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.mReceivedPiggybackAck);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err =
        mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer),
                                            SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response and it had a piggyback ack.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, mockSender.mReceivedPiggybackAck);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Now send a new message to the other side, but don't ask for an ack.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer),
                                SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we are not expecting an ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // Send the final response.  At this point we don't need to keep the
    // exchanges alive anymore.
    mockReceiver.mRetainExchange = false;
    mockSender.mRetainExchange   = false;

    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the response and its ack was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 5);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response and it had a piggyback ack.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, mockSender.mReceivedPiggybackAck);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckSendUnsolicitedStandaloneAckMessage(nlTestSuite * inSuite, void * inContext)
{
    /**
     * Tests sending a standalone ack message that is:
     * 1) Unsolicited.
     * 2) Requests an ack.
     *
     * This is not a thing that would normally happen, but a malicious entity
     * could absolutely do this.
     */
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData("", 0);
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // We send a message, have it get received by the peer, expect an ack from
    // the peer.
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;

    // Purposefully sending a standalone ack that requests an ack!
    err = exchange->SendMessage(SecureChannel::MsgType::StandaloneAck, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    // Needs a manual close, because SendMessage does not close for standalone acks.
    exchange->Close();
    ctx.DrainAndServiceIO();

    // Ensure the message and its ack were sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that nothing is waiting for acks.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckSendStandaloneAckMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate mockAppDelegate(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    NL_TEST_ASSERT(inSuite, rc->SendStandaloneAckMessage() == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Need manual close because standalone acks don't close exchanges.
    exchange->Close();
}

void TestReliableMessageProtocol::CheckMessageAfterClosed(nlTestSuite * inSuite, void * inContext)
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

    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    // We need to keep both exchanges alive for the thing we are testing here.
    mockReceiver.mRetainExchange = true;
    mockSender.mRetainExchange   = true;

    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockReceiver.mReceivedPiggybackAck);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockReceiver.mReceivedPiggybackAck);

    // And that we have not seen an ack yet.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, receiverRc->IsAckPending());

    // Ensure that we have not gotten any app-level responses or acks so far.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.mReceivedPiggybackAck);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response and it had a piggyback ack.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, mockSender.mReceivedPiggybackAck);
    // And that we are now waiting for an ack for the response.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockReceiver.mReceivedPiggybackAck     = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Now send a second message to the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent (and the ack for it was also sent).
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 4);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was not received (because the exchange is closed on the
    // receiver).
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);

    // And that we are not expecting an ack; acks should have been flushed
    // immediately on the receiver, due to the exchange being closed.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckLostResponseWithPiggyback(nlTestSuite * inSuite, void * inContext)
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
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

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
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not gotten any app-level responses or acks so far.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    // Should have pending ack here.
    NL_TEST_ASSERT(inSuite, receiverRc->IsAckPending());
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
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    // Stop keeping receiver exchange alive.
    mockReceiver.mRetainExchange = true;

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the response was sent but dropped.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);

    // Ensure that we have not received that response.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.mReceivedPiggybackAck);
    // We now have our un-acked message still waiting to retransmit and the
    // message that the other side sent is waiting for an ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 2);

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockReceiver.mReceivedPiggybackAck     = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Wait for re-transmit from sender and ack (should take 64ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    ctx.DrainAndServiceIO();

    // We resent our first message, which did not make it to the app-level
    // listener on the receiver (because it's a duplicate) but did trigger a
    // standalone ack.
    //
    // Now the annoying part is that depending on how long we _actually_ slept
    // we might have also triggered the retransmit from the other side, even
    // though we did not want to.  Handle both cases here.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 4 || loopback.mSentMessageCount == 6);
    if (loopback.mSentMessageCount == 4)
    {
        // Just triggered the retransmit from the sender.
        NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
        NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);
        NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
        NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    }
    else
    {
        // Also triggered the retransmit from the receiver.
        NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
        NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
        NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
        NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    }

    // Wait for re-transmit from receiver (should take 256ms)
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mSentMessageCount >= 6; });
    ctx.DrainAndServiceIO();

    // And now we've definitely resent our response message, which should show
    // up as an app-level message and trigger a standalone ack.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 6);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);

    // Should be all done now.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckIsPeerActiveNotInitiator(nlTestSuite * inSuite, void * inContext)
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

    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        1000_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        1000_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 1;
    loopback.mDroppedMessageCount = 0;

    mockReceiver.mRetainExchange = true;
    mockSender.mRetainExchange   = true;

    NL_TEST_ASSERT(inSuite, !exchange->HasReceivedAtLeastOneMessage());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Verify that the first message is dropped
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);

    // Make sure retransmit was not done before the idle restrans interval hits
    ctx.GetIOContext().DriveIOUntil(500_ms32, [&] { return loopback.mSentMessageCount >= 1; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, !exchange->HasReceivedAtLeastOneMessage());

    // // Make sure nothing happened
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);

    // // Retrasnmit message
    ctx.GetIOContext().DriveIOUntil(2000_ms32, [&] { return loopback.mSentMessageCount >= 2; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, !exchange->HasReceivedAtLeastOneMessage());

    // // Make sure nothing happened
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // // Verify that the receiver considers the sender is active
    NL_TEST_ASSERT(inSuite, !exchange->HasReceivedAtLeastOneMessage());
    NL_TEST_ASSERT(inSuite, mockReceiver.mExchange->HasReceivedAtLeastOneMessage());

    mockReceiver.mExchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        1000_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        100_ms32,  // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    mockReceiver.mRetainExchange = false;
    mockSender.mRetainExchange   = false;

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    // Make receiver message fail once
    loopback.mNumMessagesToDrop = 1;

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Make sure nothing happened
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // // Retrasnmit message
    ctx.GetIOContext().DriveIOUntil(500_ms32, [&] { return loopback.mSentMessageCount >= 4; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 5);
}

void TestReliableMessageProtocol::CheckLostStandaloneAck(nlTestSuite * inSuite, void * inContext)
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
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // We send a message, the other side sends a standalone ack first (which is
    // lost), then an application response, then we respond to that response.
    // We need to keep both exchanges alive for that (so we can send the
    // response from the receiver and so the initial sender exchange can send a
    // response to that).
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = 0;
    loopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange  = true;
    mockSender.mRetainExchange    = true;

    // And ensure the ack heading back our way is dropped.
    mockReceiver.SetDropAckResponse(true);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not gotten any app-level responses or acks so far.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    // Ack should have been dropped.
    NL_TEST_ASSERT(inSuite, !receiverRc->IsAckPending());

    // Don't drop any more acks.
    mockReceiver.SetDropAckResponse(false);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer),
                                              SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response and had a piggyback ack.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, mockSender.mReceivedPiggybackAck);
    // We now have just the received message waiting for an ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // And receiver still has no ack pending.
    NL_TEST_ASSERT(inSuite, !receiverRc->IsAckPending());

    // Reset various state so we can measure things again.
    mockReceiver.IsOnMessageReceivedCalled = false;
    mockReceiver.mReceivedPiggybackAck     = false;
    mockSender.IsOnMessageReceivedCalled   = false;
    mockSender.mReceivedPiggybackAck       = false;

    // Stop retaining the recipient exchange.
    mockReceiver.mRetainExchange = false;

    // Now send a new message to the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message and the standalone ack to it were sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 4);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, mockReceiver.mReceivedPiggybackAck);

    // At this point all our exchanges and reliable message contexts should be
    // dead, so we can't test anything about their state.

    // And that there are no un-acked messages left.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void TestReliableMessageProtocol::CheckGetBackoff(nlTestSuite * inSuite, void * inContext)
{
    // Run 3x iterations to thoroughly test random jitter always results in backoff within bounds.
    for (uint32_t j = 0; j < 3; j++)
    {
        for (const auto & test : theBackoffComplianceTestVector)
        {
            System::Clock::Timeout backoff = ReliableMessageMgr::GetBackoff(test.backoffBase, test.sendCount);
            ChipLogProgress(Test, "Backoff base %" PRIu32 " # %d: %" PRIu32, test.backoffBase.count(), test.sendCount,
                            backoff.count());

            NL_TEST_ASSERT(inSuite, backoff >= test.backoffMin);

            auto maxBackoff = test.backoffMax + retryBoosterTimeout;
#if CHIP_CONFIG_ENABLE_ICD_SERVER == 1
            // If running as an ICD, increase maxBackoff to account for the polling interval
            maxBackoff += ICDConfigurationData::GetInstance().GetSlowPollingInterval();
#endif
            NL_TEST_ASSERT(inSuite, backoff <= maxBackoff);
        }
    }
}

void TestReliableMessageProtocol::CheckApplicationResponseDelayed(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ctx.ExpireSessionBobToAlice();
    ctx.ExpireSessionAliceToBob();
    err = ctx.CreateCASESessionBobToAlice();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = ctx.CreateCASESessionAliceToBob();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite      = inSuite;
    mockReceiver.mRetainExchange = true;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    constexpr uint32_t kMaxMRPTransmits = 5; // Counting the initial message.

    // Let's drop all but the last MRP transmit.
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = kMaxMRPTransmits - 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    exchange->SetResponseTimeout(3000_ms32);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == kMaxMRPTransmits - 2);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // Wait for all but the last retransmit to happen.
    ctx.GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mDroppedMessageCount >= kMaxMRPTransmits - 1; });
    ctx.DrainAndServiceIO();

    // Ensure that nothing has been sent yet.
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // Now allow through the next message (our last retransmit), but make sure
    // there is no standalone ack for it.
    mockReceiver.SetDropAckResponse(true);
    ctx.GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits; });
    ctx.DrainAndServiceIO();

    // Verify that message was sent and received but nothing else has been sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);    // We have no ack yet.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got the message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

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
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    err = mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // At this point, we should have two MRP contexts pending.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 2);    // We have no ack yet.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // Now wait for all but the last retransmit to happen from the other side.
    ctx.GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits - 1; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    // We might have timed our MRP resends out, or not, but the other side is waiting for an ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1 || rm->TestGetCountRetransTable() == 2);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // Now wait for us to time out our MRP context for sure.
    ctx.GetIOContext().DriveIOUntil(5000_ms32, [&] { return rm->TestGetCountRetransTable() == 1; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);    // We timed out our MRP context.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We did not get a response.
    NL_TEST_ASSERT(inSuite, !mockSender.mResponseTimedOut);          // We did not time out yet.

    // Now wait for the last retransmit (and our ack) to to happen.
    ctx.GetIOContext().DriveIOUntil(5000_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits + 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);    // Everything has been acked.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);   // We got the response.
    NL_TEST_ASSERT(inSuite, !mockSender.mResponseTimedOut);          // We did not time out yet.

    mockReceiver.mTestSuite = nullptr;

    // Ensure that we did not mark any sessions defunct.
    NL_TEST_ASSERT(inSuite, !ctx.GetSessionBobToAlice()->AsSecureSession()->IsDefunct());
    NL_TEST_ASSERT(inSuite, !ctx.GetSessionAliceToBob()->AsSecureSession()->IsDefunct());

    // Reset our sessions, so other tests get the usual PASE session
    ctx.ExpireSessionBobToAlice();
    ctx.ExpireSessionAliceToBob();
    err = ctx.CreateSessionBobToAlice();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = ctx.CreateSessionAliceToBob();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestReliableMessageProtocol::CheckApplicationResponseNeverComes(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    CHIP_ERROR err = CHIP_NO_ERROR;

    // Make sure we are using CASE sessions, because there is no defunct-marking for PASE.
    ctx.ExpireSessionBobToAlice();
    ctx.ExpireSessionAliceToBob();
    err = ctx.CreateCASESessionBobToAlice();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = ctx.CreateCASESessionAliceToBob();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    MockAppDelegate mockReceiver(ctx);
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender(ctx);
    ExchangeContext * exchange = ctx.NewExchangeToAlice(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    exchange->GetSessionHandle()->AsSecureSession()->SetRemoteSessionParameters(ReliableMessageProtocolConfig({
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_IDLE_RETRY_INTERVAL
        30_ms32, // CHIP_CONFIG_MRP_LOCAL_ACTIVE_RETRY_INTERVAL
    }));

    constexpr uint32_t kMaxMRPTransmits = 5; // Counting the initial message.

    // Let's drop all but the last MRP transmit.
    auto & loopback               = ctx.GetLoopback();
    loopback.mSentMessageCount    = 0;
    loopback.mNumMessagesToDrop   = kMaxMRPTransmits - 1;
    loopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    exchange->SetResponseTimeout(2500_ms32);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendMessageFlags::kExpectResponse);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    ctx.DrainAndServiceIO();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == kMaxMRPTransmits - 2);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // Wait for all but the last retransmit to happen.
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return loopback.mDroppedMessageCount >= kMaxMRPTransmits - 1; });
    ctx.DrainAndServiceIO();

    // Ensure that nothing has been sent yet.
    NL_TEST_ASSERT(inSuite, loopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // Now allow through the next message (our last retransmit), but make sure
    // there is no standalone ack for it.
    mockReceiver.SetDropAckResponse(true);
    ctx.GetIOContext().DriveIOUntil(500_ms32, [&] { return loopback.mSentMessageCount >= kMaxMRPTransmits; });
    ctx.DrainAndServiceIO();

    // Verify that message was sent and received but nothing else has been sent.
    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);    // We have no ack yet.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got the message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We did not get a response.

    // Ensure there will be no more weirdness with acks and that our MRP timer is restarted properly.
    mockReceiver.SetDropAckResponse(false);

    // Now wait for us to time out our MRP context.
    ctx.GetIOContext().DriveIOUntil(1000_ms32, [&] { return rm->TestGetCountRetransTable() == 0; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);    // We timed out our MRP context.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We did not get a response.
    NL_TEST_ASSERT(inSuite, !mockSender.mResponseTimedOut);          // We did not time out yet.

    // Now wait for our exchange to time out.
    ctx.GetIOContext().DriveIOUntil(3000_ms32, [&] { return mockSender.mResponseTimedOut; });
    ctx.DrainAndServiceIO();

    NL_TEST_ASSERT(inSuite, loopback.mSentMessageCount == kMaxMRPTransmits);
    NL_TEST_ASSERT(inSuite, loopback.mDroppedMessageCount == kMaxMRPTransmits - 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);    // We timed this out long ago.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled); // Other side got original message.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);  // We never got a response.
    NL_TEST_ASSERT(inSuite, mockSender.mResponseTimedOut);           // We tiemd out

    mockReceiver.mTestSuite = nullptr;

    // We should have marked out session defunct.
    NL_TEST_ASSERT(inSuite, ctx.GetSessionBobToAlice()->AsSecureSession()->IsDefunct());
    // Other side had no reason to mark its session defunct.
    NL_TEST_ASSERT(inSuite, !ctx.GetSessionAliceToBob()->AsSecureSession()->IsDefunct());

    // Reset our sessions, so other tests get the usual PASE session
    ctx.ExpireSessionBobToAlice();
    ctx.ExpireSessionAliceToBob();
    err = ctx.CreateSessionBobToAlice();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    err = ctx.CreateSessionAliceToBob();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

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

const nlTest sTests[] = {
    NL_TEST_DEF("Test ReliableMessageMgr::CheckAddClearRetrans", TestReliableMessageProtocol::CheckAddClearRetrans),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendApplicationMessage",
                TestReliableMessageProtocol::CheckResendApplicationMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckCloseExchangeAndResendApplicationMessage",
                TestReliableMessageProtocol::CheckCloseExchangeAndResendApplicationMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckFailedMessageRetainOnSend",
                TestReliableMessageProtocol::CheckFailedMessageRetainOnSend),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendApplicationMessageWithPeerExchange",
                TestReliableMessageProtocol::CheckResendApplicationMessageWithPeerExchange),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendSessionEstablishmentMessageWithPeerExchange",
                TestReliableMessageProtocol::CheckResendSessionEstablishmentMessageWithPeerExchange),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckDuplicateMessage", TestReliableMessageProtocol::CheckDuplicateMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckDuplicateMessageClosedExchange",
                TestReliableMessageProtocol::CheckDuplicateMessageClosedExchange),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckDuplicateOldMessageClosedExchange",
                TestReliableMessageProtocol::CheckDuplicateOldMessageClosedExchange),
    NL_TEST_DEF("Test that a reply after a standalone ack comes through correctly",
                TestReliableMessageProtocol::CheckReceiveAfterStandaloneAck),
    NL_TEST_DEF("Test that a reply to a non-MRP message piggybacks an ack if there were MRP things happening on the context before",
                TestReliableMessageProtocol::CheckPiggybackAfterPiggyback),
    NL_TEST_DEF("Test sending an unsolicited ack-soliciting 'standalone ack' message",
                TestReliableMessageProtocol::CheckSendUnsolicitedStandaloneAckMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckSendStandaloneAckMessage",
                TestReliableMessageProtocol::CheckSendStandaloneAckMessage),
    NL_TEST_DEF("Test command, response, default response, with receiver closing exchange after sending response",
                TestReliableMessageProtocol::CheckMessageAfterClosed),
    NL_TEST_DEF("Test that unencrypted message is dropped if exchange requires encryption",
                TestReliableMessageProtocol::CheckUnencryptedMessageReceiveFailure),
    NL_TEST_DEF("Test that dropping an application-level message with a piggyback ack works ok once both sides retransmit",
                TestReliableMessageProtocol::CheckLostResponseWithPiggyback),
    NL_TEST_DEF("Test that an application-level response-to-response after a lost standalone ack to the initial message works",
                TestReliableMessageProtocol::CheckLostStandaloneAck),
    NL_TEST_DEF("Test Is Peer Active Retry logic", TestReliableMessageProtocol::CheckIsPeerActiveNotInitiator),
    NL_TEST_DEF("Test MRP backoff algorithm", TestReliableMessageProtocol::CheckGetBackoff),
    // TODO: Re-enable this test, after changing test to use Mock clock / DriveIO rather than DriveIOUntil.
    // Issue: https://github.com/project-chip/connectedhomeip/issues/32440
    // NL_TEST_DEF("Test an application response that comes after MRP retransmits run out",
    //            TestReliableMessageProtocol::CheckApplicationResponseDelayed),
    NL_TEST_DEF("Test an application response that never comes, so MRP retransmits run out and then exchange times out",
                TestReliableMessageProtocol::CheckApplicationResponseNeverComes),
    NL_TEST_SENTINEL(),
};

// clang-format off
nlTestSuite sSuite = {
    "Test-CHIP-ReliableMessageProtocol",
    &sTests[0],
    TestContext::nlTestSetUpTestSuite,
    TestContext::nlTestTearDownTestSuite,
    TestContext::nlTestSetUp,
    TestContext::nlTestTearDown,
};
// clang-format on

int TestReliableMessageProtocolSuite()
{
    return chip::ExecuteTestsWithContext<TestContext>(&sSuite);
}

CHIP_REGISTER_TEST_SUITE(TestReliableMessageProtocolSuite)
