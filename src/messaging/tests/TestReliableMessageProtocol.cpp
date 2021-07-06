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

#include "TestMessagingLayer.h"

#include <core/CHIPCore.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/Protocols.h>
#include <protocols/echo/Echo.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

#include <errno.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <messaging/tests/MessagingContext.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

TestContext sContext;

const char PAYLOAD[] = "Hello!";

TransportMgrBase gTransportMgr;
Test::LoopbackTransport gLoopback;

class MockAppDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
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
            if (rc->IsAckPending())
            {
                (void) rc->TakePendingPeerAckId();
            }
        }

        if (mExchange != ec)
        {
            CloseExchangeIfNeeded();
        }

        if (!mRetainExchange)
        {
            ec->Close();
            ec = nullptr;
        }
        mExchange = ec;

        if (mTestSuite != nullptr)
        {
            NL_TEST_ASSERT(mTestSuite, buffer->TotalLength() == sizeof(PAYLOAD));
            NL_TEST_ASSERT(mTestSuite, memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()) == 0);
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    void CloseExchangeIfNeeded()
    {
        if (mExchange != nullptr)
        {
            mExchange->Close();
            mExchange = nullptr;
        }
    }

    bool IsOnMessageReceivedCalled = false;
    bool mReceivedPiggybackAck     = false;
    bool mDropAckResponse          = false;
    bool mRetainExchange           = false;
    ExchangeContext * mExchange    = nullptr;
    nlTestSuite * mTestSuite       = nullptr;
};

class MockSessionEstablishmentExchangeDispatch : public Messaging::ExchangeMessageDispatch
{
public:
    CHIP_ERROR PrepareMessage(SecureSessionHandle session, PayloadHeader & payloadHeader, System::PacketBufferHandle && message,
                              EncryptedPacketBufferHandle & preparedMessage) override
    {
        PacketHeader packetHeader;

        ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));
        ReturnErrorOnFailure(packetHeader.EncodeBeforeData(message));

        preparedMessage = EncryptedPacketBufferHandle::MarkEncrypted(std::move(message));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendPreparedMessage(SecureSessionHandle session, const EncryptedPacketBufferHandle & preparedMessage) const override
    {
        return gTransportMgr.SendMessage(Transport::PeerAddress(), preparedMessage.CastToWritable());
    }

    bool IsReliableTransmissionAllowed() const override { return mRetainMessageOnSend; }

    bool MessagePermitted(uint16_t protocol, uint8_t type) override { return true; }

    bool mRetainMessageOnSend = true;
};

class MockSessionEstablishmentDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;
        ec->Close();
        if (mTestSuite != nullptr)
        {
            NL_TEST_ASSERT(mTestSuite, buffer->TotalLength() == sizeof(PAYLOAD));
            NL_TEST_ASSERT(mTestSuite, memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()) == 0);
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    virtual ExchangeMessageDispatch * GetMessageDispatch(ReliableMessageMgr * rmMgr, SecureSessionMgr * sessionMgr) override
    {
        return &mMessageDispatch;
    }

    bool IsOnMessageReceivedCalled = false;
    MockSessionEstablishmentExchangeDispatch mMessageDispatch;
    nlTestSuite * mTestSuite = nullptr;
};

void test_os_sleep_ms(uint64_t millisecs)
{
    struct timespec sleep_time;
    uint64_t s = millisecs / 1000;

    millisecs -= s * 1000;
    sleep_time.tv_sec  = static_cast<time_t>(s);
    sleep_time.tv_nsec = static_cast<long>(millisecs * 1000000);

    nanosleep(&sleep_time, nullptr);
}

void CheckAddClearRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
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

void CheckFailRetrans(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    ReliableMessageMgr::RetransTableEntry * entry;
    rm->AddToRetransTable(rc, &entry);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    rm->FailRetransTableEntries(rc, CHIP_NO_ERROR);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    exchange->Close();
}

void CheckResendApplicationMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender;
    // TODO: temporarily create a SecureSessionHandle from node id, will be fix in PR 3602
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    // Let's drop the initial message
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 2;
    gLoopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // sleep another 65 ms to trigger second re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount >= 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    rm->ClearRetransTable(rc);
    exchange->Close();
}

void CheckCloseExchangeAndResendApplicationMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockSender;
    // TODO: temporarily create a SecureSessionHandle from node id, will be fixed in PR 3602
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    // Let's drop the initial message
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 2;
    gLoopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    exchange->Close();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // sleep another 65 ms to trigger second re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount >= 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    rm->ClearRetransTable(rc);
}

void CheckFailedMessageRetainOnSend(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    err = mockSender.mMessageDispatch.Init();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockSender.mMessageDispatch.mRetainMessageOnSend = false;

    // Let's drop the initial message
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 1;
    gLoopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ensure the message was dropped
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit table is empty, as we did not provide a message to retain
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    exchange->Close();

    rm->ClearRetransTable(rc);
}

void CheckResendApplicationMessageWithPeerExchange(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    // Let's drop the initial message
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 1;
    gLoopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    exchange->Close();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount >= 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    mockReceiver.mTestSuite = nullptr;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    rm->ClearRetransTable(rc);
}

void CheckDuplicateMessageClosedExchange(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 0;
    gLoopback.mDroppedMessageCount = 0;

    // Drop the ack, and also close the peer exchange
    mockReceiver.mDropAckResponse = true;
    mockReceiver.mRetainExchange  = false;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    exchange->Close();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // Let's not drop the duplicate message
    mockReceiver.mDropAckResponse = false;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    mockReceiver.CloseExchangeIfNeeded();
}

void CheckResendSessionEstablishmentMessageWithPeerExchange(nlTestSuite * inSuite, void * inContext)
{
    // Making this static to reduce stack usage, as some platforms have limits on stack size.
    static TestContext ctx;

    CHIP_ERROR err = ctx.Init(inSuite, &gTransportMgr);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    ctx.SetSourceNodeId(kAnyNodeId);
    ctx.SetDestinationNodeId(kAnyNodeId);
    ctx.SetLocalKeyId(0);
    ctx.SetPeerKeyId(0);
    ctx.SetAdminId(kUndefinedAdminId);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    MockSessionEstablishmentDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockSessionEstablishmentDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_MRP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_MRP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    err = mockSender.mMessageDispatch.Init();
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Let's drop the initial message
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 1;
    gLoopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    exchange->Close();

    // Ensure the message was dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_NO_ERROR);

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount >= 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    mockReceiver.mTestSuite = nullptr;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    rm->ClearRetransTable(rc);
    ctx.Shutdown();

    // This test didn't use the global test context because the session establishment messages
    // do not carry encryption key IDs (as the messages are not encrypted), or node IDs (as these
    // are not assigned yet). A temporary context is created with default values for these
    // parameters.
    // Let's reset the state of transport manager so that other tests are not impacted
    // as those could be using the global test context.
    TestContext & inctx = *static_cast<TestContext *>(inContext);
    gTransportMgr.SetSecureSessionMgr(&inctx.GetSecureSessionManager());
}

void CheckDuplicateMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    rc->SetConfig({
        1, // CHIP_CONFIG_RMP_DEFAULT_INITIAL_RETRY_INTERVAL
        1, // CHIP_CONFIG_RMP_DEFAULT_ACTIVE_RETRY_INTERVAL
    });

    // Let's not drop the message. Expectation is that it is received by the peer, but the ack is dropped
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 0;
    gLoopback.mDroppedMessageCount = 0;

    // Drop the ack, and keep the exchange around to receive the duplicate message
    mockReceiver.mDropAckResponse = true;
    mockReceiver.mRetainExchange  = true;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    exchange->Close();

    // Ensure the message was sent
    // The ack was dropped, and message was added to the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Let's not drop the duplicate message
    mockReceiver.mDropAckResponse = false;
    mockReceiver.mRetainExchange  = false;

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was sent and the ack was sent
    // and retransmit table was cleared
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    mockReceiver.CloseExchangeIfNeeded();
}

void CheckReceiveAfterStandaloneAck(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // We send a message, have it get received by the peer, then an ack is
    // returned, then a reply is returned.  We need to keep both exchanges alive
    // for that (so we can send the response from the receiver and so the
    // initial sender exchange can get it).
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 0;
    gLoopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange   = true;
    mockSender.mRetainExchange     = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we have not seen an ack yet.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    ReliableMessageContext * receiverRc = mockReceiver.mExchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, receiverRc->IsAckPending());

    // Send the standalone ack.
    receiverRc->SendStandaloneAckMessage();

    // Ensure the ack was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

    // Ensure that we have not gotten any app-level responses so far.
    NL_TEST_ASSERT(inSuite, !mockSender.IsOnMessageReceivedCalled);

    // And that we have now gotten our ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // Now send a message from the other side.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    // Make sure we don't leave retransmits sitting around.
    receiverRc->GetReliableMessageMgr()->ClearRetransTable(receiverRc);
    mockReceiver.mExchange->Close();

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);

    // Make sure we don't leave retransmits sitting around.
    rm->ClearRetransTable(exchange->GetReliableMessageContext());
    exchange->Close();

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void CheckNoPiggybackAfterPiggyback(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
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
    gLoopback.mSentMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 0;
    gLoopback.mDroppedMessageCount = 0;
    mockReceiver.mRetainExchange   = true;
    mockSender.mRetainExchange     = true;

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

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

    mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer),
                                        SendFlags(SendMessageFlags::kExpectResponse).Set(SendMessageFlags::kNoAutoRequestAck));

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

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

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

    // And that it was received.
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // And that we are not expecting an ack.
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    // Send the final response.
    buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    // Make sure we don't leave retransmits sitting around.
    receiverRc->GetReliableMessageMgr()->ClearRetransTable(receiverRc);
    mockReceiver.mExchange->Close();

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSentMessageCount == 4);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

    // Ensure that we have received that response and it did not have a piggyback
    // ack.
    NL_TEST_ASSERT(inSuite, mockSender.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockSender.mReceivedPiggybackAck);

    // Make sure we don't leave retransmits sitting around.
    rm->ClearRetransTable(exchange->GetReliableMessageContext());
    exchange->Close();

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
}

void CheckSendStandaloneAckMessage(nlTestSuite * inSuite, void * inContext)
{
    TestContext & ctx = *reinterpret_cast<TestContext *>(inContext);

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    MockAppDelegate mockAppDelegate;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockAppDelegate);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    ReliableMessageMgr * rm     = ctx.GetExchangeManager().GetReliableMessageMgr();
    ReliableMessageContext * rc = exchange->GetReliableMessageContext();
    NL_TEST_ASSERT(inSuite, rm != nullptr);
    NL_TEST_ASSERT(inSuite, rc != nullptr);

    NL_TEST_ASSERT(inSuite, rc->SendStandaloneAckMessage() == CHIP_NO_ERROR);

    exchange->Close();
}

void CheckMessageAfterClosed(nlTestSuite * inSuite, void * inContext)
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

    ctx.GetInetLayer().SystemLayer()->Init(nullptr);

    chip::System::PacketBufferHandle buffer = chip::MessagePacketBuffer::NewWithData(PAYLOAD, sizeof(PAYLOAD));
    NL_TEST_ASSERT(inSuite, !buffer.IsNull());

    CHIP_ERROR err = CHIP_NO_ERROR;

    MockAppDelegate mockReceiver;
    err = ctx.GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest, &mockReceiver);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    mockReceiver.mTestSuite = inSuite;

    MockAppDelegate mockSender;
    ExchangeContext * exchange = ctx.NewExchangeToPeer(&mockSender);
    NL_TEST_ASSERT(inSuite, exchange != nullptr);

    mockSender.mTestSuite = inSuite;

    ReliableMessageMgr * rm = ctx.GetExchangeManager().GetReliableMessageMgr();
    NL_TEST_ASSERT(inSuite, rm != nullptr);

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    gLoopback.mSendMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 0;
    gLoopback.mDroppedMessageCount = 0;
    // We need to keep both exchanges alive for the thing we are testing here.
    mockReceiver.mRetainExchange = true;
    mockSender.mRetainExchange   = true;

    NL_TEST_ASSERT(inSuite, !mockReceiver.IsOnMessageReceivedCalled);
    NL_TEST_ASSERT(inSuite, !mockReceiver.mReceivedPiggybackAck);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer), SendFlags(SendMessageFlags::kExpectResponse));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    // Ensure the message was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

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

    mockReceiver.mExchange->SendMessage(Echo::MsgType::EchoResponse, std::move(buffer));
    mockReceiver.mExchange->Close();

    // Ensure the response was sent.
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

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
    exchange->Close();

    // Ensure the message was sent (and the ack for it was also sent).
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 4);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

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

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("Test ReliableMessageMgr::CheckAddClearRetrans", CheckAddClearRetrans),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckFailRetrans", CheckFailRetrans),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendApplicationMessage", CheckResendApplicationMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckCloseExchangeAndResendApplicationMessage", CheckCloseExchangeAndResendApplicationMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckFailedMessageRetainOnSend", CheckFailedMessageRetainOnSend),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendApplicationMessageWithPeerExchange", CheckResendApplicationMessageWithPeerExchange),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendSessionEstablishmentMessageWithPeerExchange", CheckResendSessionEstablishmentMessageWithPeerExchange),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckDuplicateMessage", CheckDuplicateMessage),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckDuplicateMessageClosedExchange", CheckDuplicateMessageClosedExchange),
    NL_TEST_DEF("Test that a reply after a standalone ack comes through correctly", CheckReceiveAfterStandaloneAck),
    NL_TEST_DEF("Test that a reply to a non-MRP message does not piggyback an ack even if there were MRP things happening on the context before", CheckNoPiggybackAfterPiggyback),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckSendStandaloneAckMessage", CheckSendStandaloneAckMessage),
    NL_TEST_DEF("Test command, response, default response, with receiver closing exchange after sending response", CheckMessageAfterClosed),

    NL_TEST_SENTINEL()
};
// clang-format on

int Initialize(void * aContext);
int Finalize(void * aContext);

// clang-format off
nlTestSuite sSuite =
{
    "Test-CHIP-ReliableMessageProtocol",
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
    CHIP_ERROR err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
        return FAILURE;

    gTransportMgr.Init(&gLoopback);

    auto * ctx = reinterpret_cast<TestContext *>(aContext);
    err        = ctx->Init(&sSuite, &gTransportMgr);
    if (err != CHIP_NO_ERROR)
    {
        return FAILURE;
    }

    gTransportMgr.SetSecureSessionMgr(&ctx->GetSecureSessionManager());
    return SUCCESS;
}

/**
 *  Finalize the test suite.
 */
int Finalize(void * aContext)
{
    CHIP_ERROR err = reinterpret_cast<TestContext *>(aContext)->Shutdown();
    chip::Platform::MemoryShutdown();
    return (err == CHIP_NO_ERROR) ? SUCCESS : FAILURE;
}

} // namespace

/**
 *  Main
 */
int TestReliableMessageProtocol()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, &sContext);

    return (nlTestRunnerStats(&sSuite));
}
