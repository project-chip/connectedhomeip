/*
 *
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

#include <protocols/bdx/BusyResponder.h>

#include <lib/support/BufferWriter.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/StatusCode.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <pw_unit_test/framework.h>
#include <transport/SessionManager.h>

#include <cstring>

using namespace chip;
using namespace chip::bdx;
using namespace chip::Messaging;
using chip::Protocols::SecureChannel::GeneralStatusCode;

namespace {

constexpr char kFileDesignator[] = "image.ota";

// A BDX responder that is in the middle of a transfer: it hands every new ReceiveInit to a BusyResponder.
class BusyBdxServer : public UnsolicitedMessageHandler
{
public:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = &mResponder;
        return CHIP_NO_ERROR;
    }

    BusyResponder mResponder;
};

// The initiator's end of an exchange: records the reply it gets.
class Initiator : public ExchangeDelegate
{
public:
    ~Initiator() override
    {
        // An exchange still waiting for a reply would otherwise call back into a destroyed delegate.
        if (mExchange != nullptr)
        {
            mExchange->Abort();
        }
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        mExchange = nullptr;
        mReplies++;
        mReplyIsStatusReport = payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport);
        return mReport.Parse(std::move(payload));
    }

    void OnResponseTimeout(ExchangeContext * ec) override
    {
        mExchange = nullptr;
        mTimedOut = true;
    }

    void OnExchangeClosing(ExchangeContext * ec) override { mExchange = nullptr; }

    ExchangeContext * mExchange = nullptr;
    uint32_t mReplies           = 0;
    bool mReplyIsStatusReport   = false;
    bool mTimedOut              = false;
    Protocols::SecureChannel::StatusReport mReport;
};

class TestBusyResponder : public chip::Testing::LoopbackMessagingContext
{
public:
    void SetUp() override
    {
        LoopbackMessagingContext::SetUp();
        ASSERT_SUCCESS(GetExchangeManager().RegisterUnsolicitedMessageHandlerForType(MessageType::ReceiveInit, &mServer));
    }

    void TearDown() override
    {
        EXPECT_SUCCESS(GetExchangeManager().UnregisterUnsolicitedMessageHandlerForType(MessageType::ReceiveInit));
        LoopbackMessagingContext::TearDown();
    }

    // Opens an exchange to the server and sends it a ReceiveInit, as an OTA requestor starting a download does.
    void SendReceiveInit(Initiator & initiator)
    {
        ExchangeContext * ec = NewExchangeToBob(&initiator);
        ASSERT_NE(ec, nullptr);

        ReceiveInit init;
        init.TransferCtlOptions.Set(TransferControlFlags::kReceiverDrive);
        init.Version        = 1;
        init.MaxBlockSize   = 1024;
        init.FileDesignator = reinterpret_cast<const uint8_t *>(kFileDesignator);
        init.FileDesLength  = static_cast<uint16_t>(strlen(kFileDesignator));

        const size_t size = init.MessageSize();
        Encoding::LittleEndian::PacketBufferWriter writer(MessagePacketBuffer::New(size), size);
        init.WriteToBuffer(writer);
        System::PacketBufferHandle message = writer.Finalize();
        ASSERT_FALSE(message.IsNull());
        CHIP_ERROR err =
            ec->SendMessage(MessageType::ReceiveInit, std::move(message), SendFlags(SendMessageFlags::kExpectResponse));
        if (err != CHIP_NO_ERROR)
        {
            ec->Close();
        }
        ASSERT_SUCCESS(err);
        initiator.mExchange = ec;
    }

    static void ExpectResponderBusy(const Initiator & initiator)
    {
        EXPECT_EQ(initiator.mReplies, 1u);
        EXPECT_TRUE(initiator.mReplyIsStatusReport);
        EXPECT_FALSE(initiator.mTimedOut);
        EXPECT_EQ(initiator.mReport.GetGeneralCode(), GeneralStatusCode::kFailure);
        EXPECT_EQ(initiator.mReport.GetProtocolId(), Protocols::BDX::Id);
        EXPECT_EQ(initiator.mReport.GetProtocolCode(), to_underlying(StatusCode::kResponderBusy));
    }

    BusyBdxServer mServer;
};

TEST_F(TestBusyResponder, AnswersReceiveInitWithResponderBusy)
{
    Initiator initiator;
    SendReceiveInit(initiator);
    DrainAndServiceIO();

    ExpectResponderBusy(initiator);
}

TEST_F(TestBusyResponder, LeavesNoExchangeOpen)
{
    Initiator initiator;
    SendReceiveInit(initiator);
    DrainAndServiceIO();

    EXPECT_EQ(GetExchangeManager().GetNumActiveExchanges(), 0u);
}

TEST_F(TestBusyResponder, AnswersOverlappingReceiveInits)
{
    Initiator first;
    Initiator second;
    SendReceiveInit(first);
    SendReceiveInit(second);
    DrainAndServiceIO();

    ExpectResponderBusy(first);
    ExpectResponderBusy(second);
}

} // namespace
