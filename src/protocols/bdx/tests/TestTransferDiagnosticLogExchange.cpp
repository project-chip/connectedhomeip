/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <functional>
#include <optional>
#include <vector>

#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/bdx/BdxTransferDiagnosticLog.h>
#include <protocols/bdx/BdxTransferServer.h>
#include <system/SystemClock.h>

using namespace ::chip;
using namespace ::chip::bdx;
using namespace ::chip::Protocols;

namespace {

constexpr uint16_t kMaxBlockSize = 512;

class LogSender : public Messaging::ExchangeDelegate
{
    static constexpr uint8_t kFileDesignator[] = { 'l', 'o', 'g' };
    static constexpr uint8_t kBlock[]          = { 1, 2, 3, 4 };

public:
    CHIP_ERROR Start(Messaging::ExchangeContext * exchange)
    {
        mExchange = exchange;
        TransferSession::TransferInitData initData;
        initData.TransferCtlFlags = TransferControlFlags::kSenderDrive;
        initData.MaxBlockSize     = kMaxBlockSize;
        initData.FileDesLength    = static_cast<uint16_t>(sizeof(kFileDesignator));
        initData.FileDesignator   = kFileDesignator;
        ReturnErrorOnFailure(mTransfer.StartTransfer(TransferRole::kSender, initData, System::Clock::Seconds16(10)));
        return SendOutput();
    }

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        ec->WillSendMessage();
        ReturnErrorOnFailure(
            mTransfer.HandleMessageReceived(payloadHeader, std::move(payload), System::SystemClock().GetMonotonicTimestamp()));

        TransferSession::OutputEvent event;
        mTransfer.PollOutput(event, System::SystemClock().GetMonotonicTimestamp());
        switch (event.EventType)
        {
        case TransferSession::OutputEventType::kAcceptReceived:
            return SendBlock();
        case TransferSession::OutputEventType::kAckReceived:
            mOnBlockAck();
            break;
        case TransferSession::OutputEventType::kStatusReceived:
            mStatusReceived = event.statusData.statusCode;
            break;
        default:
            break;
        }
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(Messaging::ExchangeContext * ec) override {}
    void OnExchangeClosing(Messaging::ExchangeContext * ec) override { mExchange = nullptr; }

    // Sends one block. When this is called while handling a message, the exchange piggybacks its pending ack on the block.
    CHIP_ERROR SendBlock()
    {
        TransferSession::BlockData block;
        block.Data   = kBlock;
        block.Length = sizeof(kBlock);
        ReturnErrorOnFailure(mTransfer.PrepareBlock(block));
        return SendOutput();
    }

    std::function<void()> mOnBlockAck;
    std::optional<StatusCode> mStatusReceived;
    Messaging::ExchangeContext * mExchange = nullptr;

private:
    CHIP_ERROR SendOutput()
    {
        TransferSession::OutputEvent event;
        mTransfer.PollOutput(event, System::SystemClock().GetMonotonicTimestamp());
        VerifyOrReturnError(event.EventType == TransferSession::OutputEventType::kMsgToSend, CHIP_ERROR_INCORRECT_STATE);
        return mExchange->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                      Messaging::SendMessageFlags::kExpectResponse);
    }

    TransferSession mTransfer;
};

class LogReceiver : public BDXTransferServerDelegate
{
public:
    CHIP_ERROR OnTransferBegin(BDXTransferProxy * transfer) override
    {
        mTransfer = transfer;
        return transfer->Accept();
    }
    CHIP_ERROR OnTransferData(BDXTransferProxy * transfer, const ByteSpan & data) override
    {
        if (++mBlocksReceived == mRejectAtBlock)
        {
            return transfer->Reject(CHIP_ERROR_INVALID_ARGUMENT);
        }
        return transfer->Continue();
    }
    CHIP_ERROR OnTransferEnd(BDXTransferProxy * transfer, CHIP_ERROR error) override
    {
        mTransfer = nullptr;
        mTransferEndCount++;
        return CHIP_NO_ERROR;
    }

    BDXTransferProxy * mTransfer = nullptr;
    int mTransferEndCount        = 0;
    int mBlocksReceived          = 0;
    // The block that is rejected instead of acknowledged. Zero means none.
    int mRejectAtBlock = 0;
};

class TestTransferDiagnosticLogExchange : public Testing::LoopbackMessagingContext, public Testing::LoopbackTransportDelegate
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        // The loopback transport outlives a test, so clear what an earlier test left in its drop counters.
        GetLoopback().mNumMessagesToDrop   = 0;
        GetLoopback().mDroppedMessageCount = 0;
        GetLoopback().SetLoopbackTransportDelegate(this);
        mServer.SetDelegate(&mReceiver);
        ASSERT_SUCCESS(mServer.Init(&GetSystemLayer(), &GetExchangeManager()));
        mServerInitialized = true;
    }

    // Runs even when an ASSERT in the test body returned early.
    void TearDown() override
    {
        if (mSender.mExchange != nullptr)
        {
            mSender.mExchange->Close();
        }
        if (mServerInitialized)
        {
            // Let MRP drain so nothing leaks into the next test.
            GetIOContext().DriveIOUntil(System::Clock::Seconds16(5), [&] {
                return !GetLoopback().HasPendingMessages() &&
                    GetExchangeManager().GetReliableMessageMgr()->TestGetCountRetransTable() == 0;
            });
            mServer.Shutdown();
        }
        GetLoopback().SetLoopbackTransportDelegate(nullptr);
        LoopbackMessagingContext::TearDown();
    }

    // LoopbackTransportDelegate. Records the size of each message put on the wire, in order.
    void WillSendMessage(const Transport::PeerAddress & peer, const System::PacketBufferHandle & message) override
    {
        mSentSizes.push_back(message->DataLength());
    }

    // The dropped message is the one WillSendMessage saw last.
    void OnMessageDropped() override
    {
        mDroppedSize = mSentSizes.back();
        if (mSentSizes.size() >= 2)
        {
            mSizeBeforeDropped = mSentSizes[mSentSizes.size() - 2];
        }
    }

    // Starts the transfer and waits until the sender has received the StatusReport and the receiver has ended the transfer.
    void RunUntilAbortDelivered()
    {
        Messaging::ExchangeContext * exchange = NewExchangeToAlice(&mSender);
        ASSERT_NE(exchange, nullptr);
        ASSERT_SUCCESS(mSender.Start(exchange));

        GetIOContext().DriveIOUntil(System::Clock::Seconds16(5),
                                    [&] { return mSender.mStatusReceived.has_value() && mReceiver.mTransferEndCount > 0; });
        EXPECT_TRUE(mSender.mStatusReceived.has_value() && mReceiver.mTransferEndCount > 0)
            << "Abort did not complete within the wait";
    }

    // Starts a transfer in which the receiver aborts as soon as the sender acknowledges the first block, and the sender
    // loses its standalone MRP ack of the receiver's BlockAck. The receiver therefore aborts while its exchange still waits
    // for that ack.
    void RunAbortWhileBlockAckIsUnacknowledged()
    {
        mSender.mOnBlockAck = [this] {
            // The sender sends nothing after the BlockAck, so the next message on the wire is its standalone ack of it.
            GetLoopback().mNumMessagesToDrop = 1;
            if (mReceiver.mTransfer == nullptr)
            {
                ADD_FAILURE() << "No transfer in progress when the block was acknowledged";
                return;
            }
            EXPECT_SUCCESS(mReceiver.mTransfer->Reject(CHIP_ERROR_INVALID_ARGUMENT));
        };
        RunUntilAbortDelivered();
    }

    LogReceiver mReceiver;
    BDXTransferServer mServer;
    LogSender mSender;
    bool mServerInitialized = false;
    std::vector<size_t> mSentSizes;
    size_t mDroppedSize       = 0;
    size_t mSizeBeforeDropped = 0;
};

// Defined first so that nothing a failing test leaves behind can affect it.
TEST_F(TestTransferDiagnosticLogExchange, DeliversAbortStatusReportWhenPeerPiggybacksBlockAck)
{
    // The sender answers the receiver's BlockAck with a second block, which carries the ack of the BlockAck. The receiver
    // aborts at that block, so its exchange is not waiting for an ack and the abort is not held back.
    mReceiver.mRejectAtBlock = 2;
    mSender.mOnBlockAck      = [this] { EXPECT_SUCCESS(mSender.SendBlock()); };
    RunUntilAbortDelivered();

    // Init, Accept, Block, BlockAck, Block and StatusReport: every ack was piggybacked and none was dropped.
    EXPECT_EQ(mSentSizes.size(), 6u);
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 0u);
    EXPECT_TRUE(mSender.mStatusReceived.has_value());
    EXPECT_EQ(mSender.mStatusReceived.value_or(StatusCode::kUnknown), StatusCode::kBadMessageContents);
    EXPECT_EQ(mReceiver.mTransferEndCount, 1);
}

TEST_F(TestTransferDiagnosticLogExchange, DeliversAbortStatusReportSentWhileBlockAckIsUnacknowledged)
{
    RunAbortWhileBlockAckIsUnacknowledged();

    // Exactly one message was dropped, and it was the sender's standalone ack (smaller than the BlockAck that preceded it),
    // not a retransmission of the BlockAck.
    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
    EXPECT_GT(mSizeBeforeDropped, 0u);
    EXPECT_LT(mDroppedSize, mSizeBeforeDropped);
    EXPECT_TRUE(mSender.mStatusReceived.has_value());
    EXPECT_EQ(mSender.mStatusReceived.value_or(StatusCode::kUnknown), StatusCode::kBadMessageContents);
    EXPECT_EQ(mReceiver.mTransferEndCount, 1);
}

} // namespace
