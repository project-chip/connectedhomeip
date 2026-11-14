/*
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

// Drives a BdxTransfer through the start of a transfer that the peer initiates with a ReceiveInit, and checks where in the data
// it starts sending.

#include <controller/python/matter/bdx/bdx-transfer.h>

#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/secure_channel/StatusReport.h>
#include <pw_unit_test/framework.h>

#include <cstring>
#include <optional>
#include <vector>

using namespace ::chip;
using namespace ::chip::bdx;

namespace {

constexpr uint16_t kLocalSessionId = 31;
constexpr uint16_t kPeerSessionId  = 41;
constexpr NodeId kInitiatorNodeId  = 0x3333;
constexpr uint16_t kBlockSize      = 64;
constexpr size_t kDataSize         = 300;
constexpr char kFileDesignator[]   = "file";

// Exposes the TransferSession, so a test can take each message the transfer prepares before the transfer would send it.
class TestableBdxTransfer : public BdxTransfer
{
public:
    using BdxTransfer::BdxTransfer;
    TransferSession & Transfer() { return mTransfer; }
};

class RecordingDelegate : public BdxTransfer::Delegate
{
public:
    void InitMessageReceived(BdxTransfer * transfer, TransferSession::TransferInitData initData) override { mInitReceived = true; }
    void DataReceived(BdxTransfer * transfer, const ByteSpan & block) override {}
    void TransferCompleted(BdxTransfer * transfer, CHIP_ERROR result) override {}

    bool mInitReceived = false;
};

class TestBdxTransferStartOffset : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        ASSERT_SUCCESS(GetSecureSessionManager().InjectCaseSessionWithTestKey(
            mSession, kLocalSessionId, kPeerSessionId, GetBobFabric()->GetNodeId(), kInitiatorNodeId, GetBobFabricIndex(),
            Transport::PeerAddress::UDP(Inet::IPAddress::Any), CryptoContext::SessionRole::kResponder));

        for (size_t i = 0; i < kDataSize; i++)
        {
            mData[i] = static_cast<uint8_t>(i);
        }

        mTransfer.emplace(&GetSystemLayer());
        mTransfer->SetDelegate(&mDelegate);
        mExchange = GetExchangeManager().NewContext(mSession.Get().Value(), &*mTransfer);
        ASSERT_NE(mExchange, nullptr);
    }

    void TearDown() override
    {
        // Aborting the exchange notifies its delegate, so the transfer has to outlive it.
        if (mExchange != nullptr)
        {
            mExchange->Abort();
            mExchange = nullptr;
        }
        mTransfer.reset();
        mSession.Release();
        LoopbackMessagingContext::TearDown();
    }

protected:
    CHIP_ERROR Deliver(MessageType type, const BdxMessage & message)
    {
        Encoding::LittleEndian::PacketBufferWriter writer(System::PacketBufferHandle::New(message.MessageSize()));
        message.WriteToBuffer(writer);
        PayloadHeader payloadHeader;
        payloadHeader.SetMessageType(Protocols::BDX::Id, to_underlying(type));
        return mTransfer->OnMessageReceived(mExchange, payloadHeader, writer.Finalize());
    }

    TransferSession::OutputEvent NextOutput()
    {
        TransferSession::OutputEvent event;
        mTransfer->Transfer().PollOutput(event, System::SystemClock().GetMonotonicTimestamp());
        return event;
    }

    // Delivers a ReceiveInit asking for the data from startOffset on, and hands it to the transfer's delegate.
    void ReceiveInit(uint64_t startOffset)
    {
        TransferInit init;
        init.TransferCtlOptions.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);
        init.Version        = 1;
        init.MaxBlockSize   = kBlockSize;
        init.StartOffset    = startOffset;
        init.FileDesLength  = static_cast<uint16_t>(strlen(kFileDesignator));
        init.FileDesignator = reinterpret_cast<const uint8_t *>(kFileDesignator);
        ASSERT_SUCCESS(Deliver(MessageType::ReceiveInit, init));

        TransferSession::OutputEvent event = NextOutput();
        ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kInitReceived);
        mTransfer->HandleTransferSessionOutput(event);
        ASSERT_TRUE(mDelegate.mInitReceived);
    }

    // After an accept, takes the ReceiveAccept, asks for the first block and returns the data it carries.
    void QueryFirstBlock(std::vector<uint8_t> & blockData, bool & isEof)
    {
        TransferSession::OutputEvent event = NextOutput();
        ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kMsgToSend);
        ASSERT_TRUE(event.msgTypeData.HasMessageType(MessageType::ReceiveAccept));

        BlockQuery query;
        query.BlockCounter = 0;
        ASSERT_SUCCESS(Deliver(MessageType::BlockQuery, query));
        event = NextOutput();
        ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kQueryReceived);
        mTransfer->HandleTransferSessionOutput(event);

        event = NextOutput();
        ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kMsgToSend);
        isEof = event.msgTypeData.HasMessageType(MessageType::BlockEOF);
        ASSERT_TRUE(isEof || event.msgTypeData.HasMessageType(MessageType::Block));
        DataBlock block;
        ASSERT_SUCCESS(block.Parse(std::move(event.MsgData)));
        blockData.assign(block.Data, block.Data + block.DataLength);
    }

    uint8_t mData[kDataSize];
    RecordingDelegate mDelegate;
    std::optional<TestableBdxTransfer> mTransfer;
    Messaging::ExchangeContext * mExchange = nullptr;
    SessionHolder mSession;
};

TEST_F(TestBdxTransferStartOffset, ServesTheFirstBlockFromTheStartOffset)
{
    constexpr uint64_t kStartOffset = 100;
    ReceiveInit(kStartOffset);
    ASSERT_SUCCESS(mTransfer->AcceptAndSendData(ByteSpan(mData)));

    std::vector<uint8_t> blockData;
    bool isEof = true;
    QueryFirstBlock(blockData, isEof);

    EXPECT_FALSE(isEof);
    EXPECT_EQ(blockData, std::vector<uint8_t>(&mData[kStartOffset], &mData[kStartOffset + kBlockSize]));
}

TEST_F(TestBdxTransferStartOffset, StartOffsetAtTheEndSendsAnEmptyBlockEOF)
{
    ReceiveInit(kDataSize);
    ASSERT_SUCCESS(mTransfer->AcceptAndSendData(ByteSpan(mData)));

    std::vector<uint8_t> blockData;
    bool isEof = false;
    QueryFirstBlock(blockData, isEof);

    EXPECT_TRUE(isEof);
    EXPECT_TRUE(blockData.empty());
}

TEST_F(TestBdxTransferStartOffset, RejectsAStartOffsetPastTheEnd)
{
    ReceiveInit(kDataSize + 1);
    EXPECT_NE(mTransfer->AcceptAndSendData(ByteSpan(mData)), CHIP_NO_ERROR);

    TransferSession::OutputEvent event = NextOutput();
    ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kMsgToSend);
    ASSERT_TRUE(event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport));
    Protocols::SecureChannel::StatusReport report;
    ASSERT_SUCCESS(report.Parse(std::move(event.MsgData)));
    EXPECT_EQ(report.GetProtocolCode(), to_underlying(StatusCode::kStartOffsetNotSupported));
}

} // namespace
