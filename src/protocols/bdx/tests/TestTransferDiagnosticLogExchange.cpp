#include <pw_unit_test/framework.h>

#include <functional>
#include <optional>

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
        case TransferSession::OutputEventType::kAcceptReceived: {
            TransferSession::BlockData block;
            block.Data   = kBlock;
            block.Length = sizeof(kBlock);
            ReturnErrorOnFailure(mTransfer.PrepareBlock(block));
            return SendOutput();
        }
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

    static constexpr uint8_t kFileDesignator[] = { 'l', 'o', 'g' };
    static constexpr uint8_t kBlock[]          = { 1, 2, 3, 4 };
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
    CHIP_ERROR OnTransferData(BDXTransferProxy * transfer, const ByteSpan & data) override { return transfer->Continue(); }
    CHIP_ERROR OnTransferEnd(BDXTransferProxy * transfer, CHIP_ERROR error) override
    {
        mTransfer = nullptr;
        mTransferEndCount++;
        return CHIP_NO_ERROR;
    }

    BDXTransferProxy * mTransfer = nullptr;
    int mTransferEndCount        = 0;
};

class TestTransferDiagnosticLogExchange : public Testing::LoopbackMessagingContext
{
};

TEST_F(TestTransferDiagnosticLogExchange, DeliversAbortStatusReportSentWhileBlockAckIsUnacknowledged)
{
    LogReceiver receiver;
    BDXTransferServer server;
    server.SetDelegate(&receiver);
    ASSERT_SUCCESS(server.Init(&GetSystemLayer(), &GetExchangeManager()));

    LogSender sender;
    sender.mOnBlockAck = [&] {
        GetLoopback().mNumMessagesToDrop = 1;
        ASSERT_NE(receiver.mTransfer, nullptr);
        EXPECT_SUCCESS(receiver.mTransfer->Reject(CHIP_ERROR_INVALID_ARGUMENT));
    };

    Messaging::ExchangeContext * exchange = NewExchangeToAlice(&sender);
    ASSERT_NE(exchange, nullptr);
    ASSERT_SUCCESS(sender.Start(exchange));

    GetIOContext().DriveIOUntil(System::Clock::Seconds16(5),
                                [&] { return sender.mStatusReceived.has_value() && receiver.mTransferEndCount > 0; });

    EXPECT_EQ(GetLoopback().mDroppedMessageCount, 1u);
    EXPECT_TRUE(sender.mStatusReceived.has_value());
    EXPECT_EQ(sender.mStatusReceived.value_or(StatusCode::kUnknown), StatusCode::kBadMessageContents);
    EXPECT_EQ(receiver.mTransferEndCount, 1);

    if (sender.mExchange != nullptr)
    {
        sender.mExchange->Close();
    }
    GetIOContext().DriveIOUntil(System::Clock::Seconds16(5), [&] {
        return !GetLoopback().HasPendingMessages() && GetExchangeManager().GetReliableMessageMgr()->TestGetCountRetransTable() == 0;
    });
    EXPECT_EQ(receiver.mTransferEndCount, 1);
    server.Shutdown();
}

} // namespace
