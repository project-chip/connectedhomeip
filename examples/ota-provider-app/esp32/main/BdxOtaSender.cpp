/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMemString.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/bdx/BdxTransferSession.h>

using chip::bdx::StatusCode;
using chip::bdx::TransferControlFlags;
using chip::bdx::TransferSession;

void BdxOtaSender::SetCallbacks(BdxOtaSenderCallbacks callbacks)
{
    mOnBlockQueryCallback       = callbacks.onBlockQuery;
    mOnTransferCompleteCallback = callbacks.onTransferComplete;
    mOnTransferFailedCallback   = callbacks.onTransferFailed;
}

void BdxOtaSender::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        chip::Messaging::SendFlags sendFlags;
        if (!event.msgTypeData.HasMessageType(chip::Protocols::SecureChannel::MsgType::StatusReport))
        {
            // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and the
            // end of the transfer.
            sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
        }
        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "%s: mExchangeCtx is null", __FUNCTION__));
        err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                        sendFlags);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "SendMessage failed: %s", chip::ErrorStr(err));
        }
        break;
    }
    case TransferSession::OutputEventType::kInitReceived: {
        // TransferSession will automatically reject a transfer if there are no
        // common supported control modes. It will also default to the smaller
        // block size.
        TransferSession::TransferAcceptData acceptData;
        acceptData.ControlMode  = TransferControlFlags::kReceiverDrive; // OTA must use receiver drive
        acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
        acceptData.StartOffset  = mTransfer.GetStartOffset();
        acceptData.Length       = mTransfer.GetTransferLength();
        VerifyOrReturn(mTransfer.AcceptTransfer(acceptData) == CHIP_NO_ERROR,
                       ChipLogError(BDX, "%s: %s", __FUNCTION__, chip::ErrorStr(err)));
        break;
    }
    case TransferSession::OutputEventType::kQueryReceived: {
        TransferSession::BlockData blockData;
        uint16_t blockSize   = mTransfer.GetTransferBlockSize();
        uint16_t bytesToRead = blockSize;

        chip::System::PacketBufferHandle blockBuf = chip::System::PacketBufferHandle::New(bytesToRead);
        if (blockBuf.IsNull())
        {
            // TODO: AbortTransfer() needs to support GeneralStatusCode failures as well as BDX specific errors.
            mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }

        if (mOnBlockQueryCallback != nullptr && mOnBlockQueryCallback->mCall != nullptr)
        {
            if (CHIP_NO_ERROR !=
                mOnBlockQueryCallback->mCall(mOnBlockQueryCallback->mContext, blockBuf, blockData.Length, blockData.IsEof,
                                             mNumBytesSent))
            {
                ChipLogError(BDX, "onBlockQuery Callback failed");
                mTransfer.AbortTransfer(StatusCode::kUnknown);
                return;
            }
        }
        else
        {
            ChipLogError(BDX, "onBlockQuery Callback not set");
            mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }

        blockData.Data = blockBuf->Start();
        mNumBytesSent  = static_cast<uint32_t>(mNumBytesSent + blockData.Length);

        VerifyOrReturn(CHIP_NO_ERROR == mTransfer.PrepareBlock(blockData),
                       ChipLogError(BDX, "%s: PrepareBlock failed: %s", __FUNCTION__, chip::ErrorStr(err)));
        break;
    }
    case TransferSession::OutputEventType::kAckReceived:
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        ChipLogDetail(BDX, "Transfer completed, got AckEOF");
        if (mOnTransferCompleteCallback != nullptr && mOnTransferCompleteCallback->mCall != nullptr)
        {
            mOnTransferCompleteCallback->mCall(mOnTransferCompleteCallback->mContext);
        }
        else
        {
            ChipLogError(BDX, "onTransferComplete Callback not set");
        }
        Reset();
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        if (mOnTransferFailedCallback != nullptr && mOnTransferFailedCallback->mCall != nullptr)
        {
            mOnTransferFailedCallback->mCall(mOnTransferFailedCallback->mContext, kErrorBdxSenderStatusReceived);
        }
        else
        {
            ChipLogError(BDX, "onTransferFailed Callback not set");
        }
        Reset();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "InternalError");
        if (mOnTransferFailedCallback != nullptr && mOnTransferFailedCallback->mCall != nullptr)
        {
            mOnTransferFailedCallback->mCall(mOnTransferFailedCallback->mContext, kErrorBdxSenderInternal);
        }
        {
            ChipLogError(BDX, "onTransferFailed Callback not set");
        }
        Reset();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        if (mOnTransferFailedCallback != nullptr && mOnTransferFailedCallback->mCall != nullptr)
        {
            mOnTransferFailedCallback->mCall(mOnTransferFailedCallback->mContext, kErrorBdxSenderTimeOut);
        }
        {
            ChipLogError(BDX, "onTransferFailed Callback not set");
        }
        Reset();
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        // TransferSession should prevent this case from happening.
        ChipLogError(BDX, "%s: unsupported event type", __FUNCTION__);
    }
}

void BdxOtaSender::Reset()
{
    mTransfer.Reset();
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
    }
    mNumBytesSent = 0;
}

uint16_t BdxOtaSender::GetTransferBlockSize(void)
{
    return mTransfer.GetTransferBlockSize();
}

uint64_t BdxOtaSender::GetTransferLength()
{
    return mTransfer.GetTransferLength();
}
