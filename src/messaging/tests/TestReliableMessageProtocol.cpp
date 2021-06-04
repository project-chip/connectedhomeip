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

namespace {

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::Messaging;
using namespace chip::Protocols;

using TestContext = chip::Test::MessagingContext;

TestContext sContext;

const char PAYLOAD[] = "Hello!";

class OutgoingTransport : public Transport::Base
{
public:
    CHIP_ERROR SendMessage(const PeerAddress & address, System::PacketBufferHandle && msgBuf) override
    {
        mSendMessageCount++;

        if (mNumMessagesToDrop == 0)
        {
            System::PacketBufferHandle receivedMessage = msgBuf.CloneData();
            HandleMessageReceived(address, std::move(receivedMessage));
        }
        else
        {
            mNumMessagesToDrop--;
            mDroppedMessageCount++;
        }

        return CHIP_NO_ERROR;
    }

    bool CanSendToPeer(const PeerAddress & address) override { return true; }

    uint32_t mNumMessagesToDrop   = 0;
    uint32_t mDroppedMessageCount = 0;
    uint32_t mSendMessageCount    = 0;
};

TransportMgrBase gTransportMgr;
OutgoingTransport gLoopback;

class MockAppDelegate : public ExchangeDelegate
{
public:
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;
        if (mDropAckResponse)
        {
            ec->GetReliableMessageContext()->SetAckPending(false);
        }
        ec->Close();
        if (mTestSuite != nullptr)
        {
            NL_TEST_ASSERT(mTestSuite, buffer->TotalLength() == sizeof(PAYLOAD));
            NL_TEST_ASSERT(mTestSuite, memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()) == 0);
        }
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    bool IsOnMessageReceivedCalled = false;
    bool mDropAckResponse          = false;
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
    void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                           System::PacketBufferHandle && buffer) override
    {
        IsOnMessageReceivedCalled = true;
        ec->Close();
        if (mTestSuite != nullptr)
        {
            NL_TEST_ASSERT(mTestSuite, buffer->TotalLength() == sizeof(PAYLOAD));
            NL_TEST_ASSERT(mTestSuite, memcmp(buffer->Start(), PAYLOAD, buffer->TotalLength()) == 0);
        }
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
    gLoopback.mSendMessageCount    = 0;
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
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // sleep another 65 ms to trigger second re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount >= 3);
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
    gLoopback.mSendMessageCount    = 0;
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
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mNumMessagesToDrop == 0);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 2);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);

    // sleep another 65 ms to trigger second re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was NOT dropped, and the retransmit table is empty, as we should have gotten an ack
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount >= 3);
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
    gLoopback.mSendMessageCount    = 0;
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
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

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
    gLoopback.mSendMessageCount    = 0;
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
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount >= 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 1);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    mockReceiver.mTestSuite = nullptr;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    rm->ClearRetransTable(rc);
}

void CheckResendApplicationMessageWithLostAcks(nlTestSuite * inSuite, void * inContext)
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

    mockReceiver.mDropAckResponse = true;

    // Let's not drop any messages. We'll drop acks for this test.
    gLoopback.mSendMessageCount    = 0;
    gLoopback.mNumMessagesToDrop   = 0;
    gLoopback.mDroppedMessageCount = 0;

    // Ensure the retransmit table is empty right now
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);

    err = exchange->SendMessage(Echo::MsgType::EchoRequest, std::move(buffer));
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
    exchange->Close();

    // Ensure the message was not dropped, and was added to retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 1);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was also not dropped, and is still there in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount == 2);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);
    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 1);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    // Let's not drop the ack on the next retry
    mockReceiver.mDropAckResponse = false;

    // 1 tick is 64 ms, sleep 65 ms to trigger first re-transmit
    test_os_sleep_ms(65);
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the message was retransmitted, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount >= 3);
    NL_TEST_ASSERT(inSuite, gLoopback.mDroppedMessageCount == 0);

    // TODO - Enable test for lost CRMP ack messages
    // The following check is commented out because of https://github.com/project-chip/connectedhomeip/issues/7292
    //    NL_TEST_ASSERT(inSuite, rm->TestGetCountRetransTable() == 0);
    NL_TEST_ASSERT(inSuite, mockReceiver.IsOnMessageReceivedCalled);

    mockReceiver.mTestSuite = nullptr;

    err = ctx.GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(Echo::MsgType::EchoRequest);
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);

    rm->ClearRetransTable(rc);
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
    gLoopback.mSendMessageCount    = 0;
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
    ReliableMessageMgr::Timeout(&ctx.GetSystemLayer(), rm, CHIP_SYSTEM_NO_ERROR);

    // Ensure the retransmit message was not dropped, and is no longer in the retransmit table
    NL_TEST_ASSERT(inSuite, gLoopback.mSendMessageCount >= 2);
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
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendApplicationMessageWithLostAcks", CheckResendApplicationMessageWithLostAcks),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckResendSessionEstablishmentMessageWithPeerExchange", CheckResendSessionEstablishmentMessageWithPeerExchange),
    NL_TEST_DEF("Test ReliableMessageMgr::CheckSendStandaloneAckMessage", CheckSendStandaloneAckMessage),

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
