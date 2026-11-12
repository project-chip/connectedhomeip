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

#include <cstring>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/tests/MessagingContext.h>
#include <protocols/Protocols.h>
#include <protocols/bdx/AsyncTransferFacilitator.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/StatusCode.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemPacketBuffer.h>

namespace {

using namespace chip;
using namespace chip::bdx;
using namespace chip::Messaging;

constexpr uint16_t kMaxBlockSize          = 512;
constexpr uint64_t kStartOffset           = 1024;
constexpr System::Clock::Timeout kTimeout = System::Clock::Seconds16(30);
constexpr char kFileDesignator[]          = "test-image.bin";

// The BDX Initiator (Receiver) side.  It keeps the Responder's reply to the ReceiveInit.
class TestInitiator : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        mReplyHeader = payloadHeader;
        mReply       = std::move(payload);
        return CHIP_NO_ERROR;
    }

    void OnResponseTimeout(ExchangeContext * ec) override {}

    PayloadHeader mReplyHeader;
    System::PacketBufferHandle mReply;
};

// The BDX Responder (Sender) side under test.  It records the event it gets instead of handling it, so each test
// answers the event by calling NotifyEventHandled itself.
class TestResponder : public AsyncResponder, public UnsolicitedMessageHandler
{
public:
    TestResponder(ExchangeManager & exchangeManager, System::Layer & layer) : mExchangeManager(exchangeManager), mLayer(layer)
    {
        EXPECT_SUCCESS(mExchangeManager.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this));
    }

    ~TestResponder() override
    {
        EXPECT_SUCCESS(mExchangeManager.UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id));
    }

    TransferSession & Transfer() { return mTransfer; }

    TransferSession::OutputEventType mLastEvent = TransferSession::OutputEventType::kNone;
    bool mDestroySelfCalled                     = false;
    CHIP_ERROR mTransferEndErrorAtDestroySelf   = CHIP_NO_ERROR;

private:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override
    {
        if (payloadHeader.HasMessageType(MessageType::ReceiveInit))
        {
            ReturnErrorOnFailure(
                Init(&mLayer, ec, TransferRole::kSender, TransferControlFlags::kReceiverDrive, kMaxBlockSize, kTimeout));
        }
        return AsyncTransferFacilitator::OnMessageReceived(ec, payloadHeader, std::move(payload));
    }

    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override { mLastEvent = event.EventType; }

    void DestroySelf() override
    {
        mDestroySelfCalled             = true;
        mTransferEndErrorAtDestroySelf = mTransferEndError;
    }

    ExchangeManager & mExchangeManager;
    System::Layer & mLayer;
};

class TestAsyncResponder : public chip::Testing::LoopbackMessagingContext
{
protected:
    // Sends a ReceiveInit from Bob to Alice, where a TestResponder must be registered to handle it.
    CHIP_ERROR SendReceiveInit()
    {
        TransferSession initiatorSession;
        TransferSession::TransferInitData initData;
        initData.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
        initData.MaxBlockSize     = kMaxBlockSize;
        initData.StartOffset      = kStartOffset;
        initData.FileDesignator   = Uint8::from_const_char(kFileDesignator);
        initData.FileDesLength    = static_cast<uint16_t>(strlen(kFileDesignator));
        ReturnErrorOnFailure(initiatorSession.StartTransfer(TransferRole::kReceiver, initData, kTimeout));

        TransferSession::OutputEvent event;
        initiatorSession.GetNextAction(event);
        VerifyOrReturnError(event.EventType == TransferSession::OutputEventType::kMsgToSend, CHIP_ERROR_INCORRECT_STATE);

        ExchangeContext * ec = NewExchangeToAlice(&mInitiator);
        VerifyOrReturnError(ec != nullptr, CHIP_ERROR_NO_MEMORY);
        CHIP_ERROR err = ec->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                         SendMessageFlags::kExpectResponse);
        if (err != CHIP_NO_ERROR)
        {
            ec->Close();
            return err;
        }

        DrainAndServiceIO();
        return CHIP_NO_ERROR;
    }

    void ExpectBdxStatusReport(StatusCode expectedCode)
    {
        ASSERT_TRUE(mInitiator.mReplyHeader.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport));

        Protocols::SecureChannel::StatusReport report;
        ASSERT_SUCCESS(report.Parse(std::move(mInitiator.mReply)));
        EXPECT_EQ(report.GetGeneralCode(), Protocols::SecureChannel::GeneralStatusCode::kFailure);
        EXPECT_EQ(report.GetProtocolId(), Protocols::BDX::Id);
        EXPECT_EQ(report.GetProtocolCode(), to_underlying(expectedCode));
    }

    // A fixture member, so that it outlives any exchange still pointing at it when the messaging layer shuts down.
    TestInitiator mInitiator;
};

TEST_F(TestAsyncResponder, StatusCodeOverloadAbortsWithThatStatus)
{
    TestResponder responder(GetExchangeManager(), GetSystemLayer());
    ASSERT_SUCCESS(SendReceiveInit());
    ASSERT_EQ(responder.mLastEvent, TransferSession::OutputEventType::kInitReceived);

    responder.NotifyEventHandled(TransferSession::OutputEventType::kInitReceived, StatusCode::kStartOffsetNotSupported);
    DrainAndServiceIO();

    EXPECT_TRUE(responder.mDestroySelfCalled);
    EXPECT_EQ(responder.mTransferEndErrorAtDestroySelf, GetChipErrorFromBdxStatusCode(StatusCode::kStartOffsetNotSupported));
    // No CHIP_ERROR maps to START_OFFSET_NOT_SUPPORTED, so only the StatusCode overload can put it on the wire.
    ExpectBdxStatusReport(StatusCode::kStartOffsetNotSupported);
}

TEST_F(TestAsyncResponder, ChipErrorOverloadAbortsWithMappedStatus)
{
    TestResponder responder(GetExchangeManager(), GetSystemLayer());
    ASSERT_SUCCESS(SendReceiveInit());
    ASSERT_EQ(responder.mLastEvent, TransferSession::OutputEventType::kInitReceived);

    responder.NotifyEventHandled(TransferSession::OutputEventType::kInitReceived, CHIP_ERROR_NO_MEMORY);
    DrainAndServiceIO();

    EXPECT_TRUE(responder.mDestroySelfCalled);
    // CHIP_ERROR_NO_MEMORY has no BDX status of its own, so the peer gets UNKNOWN.  The recorded error must still be
    // CHIP_ERROR_NO_MEMORY, not GetChipErrorFromBdxStatusCode(kUnknown), which is CHIP_ERROR_BAD_REQUEST.
    EXPECT_EQ(responder.mTransferEndErrorAtDestroySelf, CHIP_ERROR_NO_MEMORY);
    ExpectBdxStatusReport(StatusCode::kUnknown);
}

TEST_F(TestAsyncResponder, ChipErrorOverloadWithNoErrorContinuesTransfer)
{
    TestResponder responder(GetExchangeManager(), GetSystemLayer());
    ASSERT_SUCCESS(SendReceiveInit());
    ASSERT_EQ(responder.mLastEvent, TransferSession::OutputEventType::kInitReceived);

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = TransferControlFlags::kReceiverDrive;
    acceptData.MaxBlockSize = responder.Transfer().GetTransferBlockSize();
    acceptData.StartOffset  = responder.Transfer().GetStartOffset();
    acceptData.Length       = responder.Transfer().GetTransferLength();
    ASSERT_SUCCESS(responder.Transfer().AcceptTransfer(acceptData));

    responder.NotifyEventHandled(TransferSession::OutputEventType::kInitReceived, CHIP_NO_ERROR);
    DrainAndServiceIO();

    EXPECT_FALSE(responder.mDestroySelfCalled);
    EXPECT_TRUE(mInitiator.mReplyHeader.HasMessageType(MessageType::ReceiveAccept));
}

} // namespace
