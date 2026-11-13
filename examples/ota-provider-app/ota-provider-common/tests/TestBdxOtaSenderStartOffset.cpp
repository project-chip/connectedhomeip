/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <ota-provider-common/BdxOtaSender.h>

#include <cstdlib>
#include <cstring>
#include <ios>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <memory>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/secure_channel/StatusReport.h>
#include <pw_unit_test/framework.h>
#include <string>
#include <unistd.h>
#include <vector>

using namespace ::chip;
using namespace ::chip::bdx;

namespace {

constexpr uint16_t kLocalSessionId                = 31;
constexpr uint16_t kPeerSessionId                 = 41;
constexpr NodeId kRequesterNodeId                 = 0x3333;
constexpr uint16_t kMaxBlockSize                  = 1024;
constexpr size_t kImageSize                       = size_t{ 3 } * kMaxBlockSize;
constexpr char kFileDesignator[]                  = "image.ota";
constexpr System::Clock::Timeout kBdxTimeout      = System::Clock::Seconds16(300);
constexpr System::Clock::Timeout kBdxPollInterval = System::Clock::Milliseconds32(20);

class TestableBdxOtaSender : public BdxOtaSender
{
public:
    using BdxOtaSender::OnMessageReceived;
    using BdxOtaSender::PollForOutput;
    TransferSession & Transfer() { return mTransfer; }

    // Lets a test name an image that cannot be sized the same way on every platform.
    std::streamoff ImageSize(const std::string & path) override
    {
        return mUnsizable ? std::streamoff{ -1 } : BdxOtaSender::ImageSize(path);
    }
    bool mUnsizable = false;
};

class TestBdxOtaSenderStartOffset : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        ASSERT_SUCCESS(GetSecureSessionManager().InjectCaseSessionWithTestKey(
            mSession, kLocalSessionId, kPeerSessionId, GetBobFabric()->GetNodeId(), kRequesterNodeId, GetBobFabricIndex(),
            Transport::PeerAddress::UDP(Inet::IPAddress::Any), CryptoContext::SessionRole::kResponder));

        for (size_t i = 0; i < kImageSize; i++)
        {
            mImage[i] = static_cast<uint8_t>(i % 251);
        }
        int fd = mkstemp(mImagePath);
        ASSERT_GE(fd, 0);
        ASSERT_EQ(write(fd, mImage, kImageSize), static_cast<ssize_t>(kImageSize));
        close(fd);

        mSender = std::make_unique<TestableBdxOtaSender>();
        mSender->SetFileDesignatorMap({ { kFileDesignator, mImagePath } });
        ASSERT_SUCCESS(mSender->InitializeTransfer(GetBobFabricIndex(), kRequesterNodeId));
        ASSERT_SUCCESS(mSender->PrepareForTransfer(&GetSystemLayer(), TransferRole::kSender,
                                                   BitFlags<TransferControlFlags>(TransferControlFlags::kReceiverDrive),
                                                   kMaxBlockSize, kBdxTimeout, kBdxPollInterval));
        mExchange = GetExchangeManager().NewContext(mSession.Get().Value(), mSender.get());
        ASSERT_NE(mExchange, nullptr);
    }

    void TearDown() override
    {
        if (mExchange != nullptr)
        {
            mExchange->Abort();
        }
        mSender.reset();
        mSession.Release();
        unlink(mImagePath);
        LoopbackMessagingContext::TearDown();
    }

protected:
    CHIP_ERROR Deliver(MessageType type, const BdxMessage & message)
    {
        Encoding::LittleEndian::PacketBufferWriter writer(System::PacketBufferHandle::New(message.MessageSize()));
        message.WriteToBuffer(writer);
        PayloadHeader payloadHeader;
        payloadHeader.SetMessageType(Protocols::BDX::Id, to_underlying(type));
        return mSender->OnMessageReceived(mExchange, payloadHeader, writer.Finalize());
    }

    CHIP_ERROR DeliverReceiveInit(uint64_t startOffset)
    {
        TransferInit init;
        init.TransferCtlOptions.ClearAll().Set(TransferControlFlags::kReceiverDrive, true);
        init.Version        = 1;
        init.MaxBlockSize   = kMaxBlockSize;
        init.StartOffset    = startOffset;
        init.FileDesLength  = static_cast<uint16_t>(strlen(kFileDesignator));
        init.FileDesignator = reinterpret_cast<const uint8_t *>(kFileDesignator);
        return Deliver(MessageType::ReceiveInit, init);
    }

    TransferSession::OutputEvent NextOutgoingMessage()
    {
        TransferSession::OutputEvent event;
        mSender->Transfer().PollOutput(event, System::SystemClock().GetMonotonicTimestamp());
        return event;
    }

    uint16_t NextStatusCode()
    {
        TransferSession::OutputEvent event = NextOutgoingMessage();
        VerifyOrReturnValue(event.EventType == TransferSession::OutputEventType::kMsgToSend, 0);
        VerifyOrReturnValue(event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport), 0);
        Protocols::SecureChannel::StatusReport report;
        VerifyOrReturnValue(report.Parse(std::move(event.MsgData)) == CHIP_NO_ERROR, 0);
        return report.GetProtocolCode();
    }

    char mImagePath[40] = "/tmp/chip-bdx-ota-sender-XXXXXX";
    uint8_t mImage[kImageSize];
    std::unique_ptr<TestableBdxOtaSender> mSender;
    Messaging::ExchangeContext * mExchange = nullptr;
    SessionHolder mSession;
};

TEST_F(TestBdxOtaSenderStartOffset, ServesTheFirstBlockFromTheRequestedStartOffset)
{
    constexpr size_t kStartOffset = kMaxBlockSize + 100;
    ASSERT_SUCCESS(DeliverReceiveInit(kStartOffset));
    mSender->PollForOutput();
    mSender->PollForOutput();
    DrainAndServiceIO();

    BlockQuery query;
    ASSERT_SUCCESS(Deliver(MessageType::BlockQuery, query));
    mSender->PollForOutput();

    TransferSession::OutputEvent event = NextOutgoingMessage();
    ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kMsgToSend);
    ASSERT_TRUE(event.msgTypeData.HasMessageType(MessageType::Block));
    Block block;
    ASSERT_SUCCESS(block.Parse(std::move(event.MsgData)));
    ASSERT_EQ(block.DataLength, size_t{ kMaxBlockSize });
    EXPECT_EQ(memcmp(block.Data, &mImage[kStartOffset], block.DataLength), 0);
}

TEST_F(TestBdxOtaSenderStartOffset, RejectsAStartOffsetAtOrPastTheEndOfTheImage)
{
    ASSERT_SUCCESS(DeliverReceiveInit(kImageSize));
    mSender->PollForOutput();

    EXPECT_EQ(NextStatusCode(), to_underlying(StatusCode::kStartOffsetNotSupported));
}

TEST_F(TestBdxOtaSenderStartOffset, ReportsAnUnreadableImageAsAnUnknownFileDesignator)
{
    unlink(mImagePath);
    ASSERT_SUCCESS(DeliverReceiveInit(kMaxBlockSize));
    mSender->PollForOutput();

    EXPECT_EQ(NextStatusCode(), to_underlying(StatusCode::kFileDesignatorUnknown));
}

// An image that cannot be sized reports -1, which must not become a huge unsigned size that passes the end-of-image check.
TEST_F(TestBdxOtaSenderStartOffset, ReportsAnUnsizableImageAsAnUnknownFileDesignator)
{
    mSender->mUnsizable = true;
    ASSERT_SUCCESS(DeliverReceiveInit(kMaxBlockSize));
    mSender->PollForOutput();

    EXPECT_EQ(NextStatusCode(), to_underlying(StatusCode::kFileDesignatorUnknown));
}

// The requestor restarts from offset 0 on a new exchange right after the provider rejects the offset, without a new QueryImage.
TEST_F(TestBdxOtaSenderStartOffset, ServesTheSameRequestorAgainFromTheStartAfterRejectingAnOffset)
{
    ASSERT_SUCCESS(DeliverReceiveInit(kImageSize));
    mSender->PollForOutput(); // rejects the offset
    mSender->PollForOutput(); // sends the StatusReport
    mSender->PollForOutput(); // ends the transfer
    DrainAndServiceIO();
    mExchange = nullptr; // the StatusReport closed it

    mExchange = GetExchangeManager().NewContext(mSession.Get().Value(), mSender.get());
    ASSERT_NE(mExchange, nullptr);
    ASSERT_SUCCESS(DeliverReceiveInit(0));
    mSender->PollForOutput();

    TransferSession::OutputEvent event = NextOutgoingMessage();
    ASSERT_EQ(event.EventType, TransferSession::OutputEventType::kMsgToSend);
    EXPECT_TRUE(event.msgTypeData.HasMessageType(MessageType::ReceiveAccept));
}

// The requestor restarts as soon as it receives the StatusReport, which can be before the sender polls again.
TEST_F(TestBdxOtaSenderStartOffset, ServesARestartArrivingRightAfterTheRejection)
{
    ASSERT_SUCCESS(DeliverReceiveInit(kImageSize));
    mSender->PollForOutput(); // rejects the offset
    mSender->PollForOutput(); // sends the StatusReport

    mExchange = GetExchangeManager().NewContext(mSession.Get().Value(), mSender.get());
    ASSERT_NE(mExchange, nullptr);
    ASSERT_SUCCESS(DeliverReceiveInit(0));

    // A transfer that is still ending would answer with a StatusReport instead.
    EXPECT_EQ(NextOutgoingMessage().EventType, TransferSession::OutputEventType::kInitReceived);
}

// A sender armed again for a restarting requestor does not keep other requestors from their own QueryImage.
TEST_F(TestBdxOtaSenderStartOffset, ArmedAgainSenderIsTakenOverByAnotherRequestor)
{
    ASSERT_SUCCESS(DeliverReceiveInit(kImageSize));
    mSender->PollForOutput();
    mSender->PollForOutput();
    mSender->PollForOutput();
    DrainAndServiceIO();
    mExchange = nullptr; // the StatusReport closed it

    EXPECT_SUCCESS(mSender->InitializeTransfer(GetBobFabricIndex(), kRequesterNodeId + 1));
}

} // namespace
