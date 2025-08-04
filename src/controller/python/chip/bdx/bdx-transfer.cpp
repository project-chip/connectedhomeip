/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <controller/python/chip/bdx/bdx-transfer.h>

#include <protocols/bdx/BdxTransferSession.h>

namespace chip {
namespace bdx {
namespace {

constexpr uint32_t kMaxBdxBlockSize               = 1024;
constexpr System::Clock::Timeout kBdxPollInterval = System::Clock::Milliseconds32(50);
constexpr System::Clock::Timeout kBdxTimeout      = System::Clock::Seconds16(5 * 60);

} // namespace

void BdxTransfer::SetDelegate(BdxTransfer::Delegate * delegate)
{
    mDelegate = delegate;
}

CHIP_ERROR BdxTransfer::AcceptAndReceiveData()
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    mAwaitingAccept = false;

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = TransferControlFlags::kSenderDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset  = mTransfer.GetStartOffset();
    acceptData.Length       = mTransfer.GetTransferLength();
    return mTransfer.AcceptTransfer(acceptData);
}

CHIP_ERROR BdxTransfer::AcceptAndSendData(const ByteSpan & data_to_send)
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    mAwaitingAccept = false;
    mData.assign(data_to_send.begin(), data_to_send.end());
    mDataCount = data_to_send.size();

    TransferSession::TransferAcceptData acceptData;
    acceptData.ControlMode  = TransferControlFlags::kReceiverDrive;
    acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
    acceptData.StartOffset  = mTransfer.GetStartOffset();
    acceptData.Length       = mTransfer.GetTransferLength();
    return mTransfer.AcceptTransfer(acceptData);
}

CHIP_ERROR BdxTransfer::Reject()
{
    VerifyOrReturnError(mAwaitingAccept, CHIP_ERROR_INCORRECT_STATE);
    mAwaitingAccept = false;
    return mTransfer.RejectTransfer(StatusCode::kTransferFailedUnknownError);
}

void BdxTransfer::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    ChipLogDetail(BDX, "Received event %s", event.ToString(event.EventType));

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kInitReceived:
        mAwaitingAccept = true;
        mDelegate->InitMessageReceived(this, event.transferInitData);
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Received StatusReport %x", ::chip::to_underlying(event.statusData.statusCode));
        EndSession(ChipError(ChipError::SdkPart::kIMClusterStatus, static_cast<uint8_t>(event.statusData.statusCode)));
        break;
    case TransferSession::OutputEventType::kInternalError:
        EndSession(CHIP_ERROR_INTERNAL);
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        EndSession(CHIP_ERROR_TIMEOUT);
        break;
    case TransferSession::OutputEventType::kBlockReceived:
        if (mDelegate)
        {
            ByteSpan data(event.blockdata.Data, event.blockdata.Length);
            mDelegate->DataReceived(this, data);
            mTransfer.PrepareBlockAck();
        }
        else
        {
            ChipLogError(BDX, "Block received without a delegate!");
        }
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        SendMessage(event);
        if (event.msgTypeData.HasMessageType(MessageType::BlockAckEOF))
        {
            // TODO: Ending the session here means the StandaloneAck for the BlockAckEOF message hasn't been received.
            EndSession(CHIP_NO_ERROR);
        }
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        EndSession(CHIP_NO_ERROR);
        break;
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
        mDataTransferredCount = std::min<size_t>(mDataTransferredCount + event.bytesToSkip.BytesToSkip, mDataCount);
        SendBlock();
        break;
    case TransferSession::OutputEventType::kQueryReceived:
        SendBlock();
        break;
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kNone:
        // Nothing to do.
        break;
    default:
        // Should never happen.
        ChipLogError(BDX, "Unhandled BDX transfer session event type %d.", static_cast<int>(event.EventType));
        chipDie();
        break;
    }
}

void BdxTransfer::EndSession(CHIP_ERROR result)
{
    if (mDelegate)
    {
        mDelegate->TransferCompleted(this, result);
    }
    ResetTransfer();
    if (mExchangeCtx)
    {
        mExchangeCtx->Close();
    }
}

void BdxTransfer::OnExchangeClosing(Messaging::ExchangeContext * exchangeContext)
{
    mExchangeCtx = nullptr;
}

CHIP_ERROR BdxTransfer::SendMessage(TransferSession::OutputEvent & event)
{
    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ::chip::Messaging::SendFlags sendFlags;
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport) &&
        !event.msgTypeData.HasMessageType(MessageType::BlockAckEOF))
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }
    return mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, event.MsgData.Retain(),
                                     sendFlags);
}

CHIP_ERROR BdxTransfer::SendBlock()
{
    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t dataRemaining = mDataCount - mDataTransferredCount;
    TransferSession::BlockData block;
    block.Data   = mData.data() + mDataTransferredCount;
    block.Length = std::min<size_t>(mTransfer.GetTransferBlockSize(), dataRemaining);
    block.IsEof  = block.Length == dataRemaining;
    ReturnErrorOnFailure(mTransfer.PrepareBlock(block));
    mDataTransferredCount += block.Length;
    ScheduleImmediatePoll();
    return CHIP_NO_ERROR;
}

CHIP_ERROR BdxTransfer::OnMessageReceived(chip::Messaging::ExchangeContext * exchangeContext,
                                          const chip::PayloadHeader & payloadHeader, chip::System::PacketBufferHandle && payload)
{
    bool has_send_init    = payloadHeader.HasMessageType(MessageType::SendInit);
    bool has_receive_init = payloadHeader.HasMessageType(MessageType::ReceiveInit);
    if (has_send_init || has_receive_init)
    {
        FabricIndex fabricIndex = exchangeContext->GetSessionHandle()->GetFabricIndex();
        NodeId peerNodeId       = exchangeContext->GetSessionHandle()->GetPeer().GetNodeId();
        VerifyOrReturnError(fabricIndex != kUndefinedFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(peerNodeId != kUndefinedNodeId, CHIP_ERROR_INVALID_ARGUMENT);

        TransferControlFlags flags;
        TransferRole role;
        if (has_send_init)
        {
            flags = TransferControlFlags::kSenderDrive;
            role  = TransferRole::kReceiver;
        }
        else
        {
            flags = TransferControlFlags::kReceiverDrive;
            role  = TransferRole::kSender;
        }
        ReturnLogErrorOnFailure(
            Responder::PrepareForTransfer(mSystemLayer, role, flags, kMaxBdxBlockSize, kBdxTimeout, kBdxPollInterval));
    }

    return Responder::OnMessageReceived(exchangeContext, payloadHeader, std::move(payload));
}

} // namespace bdx
} // namespace chip
