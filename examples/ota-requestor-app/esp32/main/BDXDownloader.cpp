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

#include "BDXDownloader.h"

#include <lib/core/CHIPError.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/bdx/BdxTransferSession.h>

using namespace chip::bdx;

void BdxDownloader::SetInitialExchange(chip::Messaging::ExchangeContext * ec)
{
    mExchangeCtx = ec;
}

void BdxDownloader::SetCallbacks(BdxDownloaderCallbacks callbacks)
{
    mOnBlockReceivedCallback    = callbacks.onBlockReceived;
    mOnTransferCompleteCallback = callbacks.onTransferComplete;
    mOnTransferFailedCallback   = callbacks.onTransferFailed;
}

void BdxDownloader::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        if (mIsTransferComplete)
        {
            if (mOnTransferCompleteCallback != nullptr && mOnTransferCompleteCallback->mCall != nullptr)
            {
                mOnTransferCompleteCallback->mCall(mOnTransferCompleteCallback->mContext);
            }
            else
            {
                ChipLogError(BDX, "onTransferComplete Callback not set");
            }
            mTransfer.Reset();
            mIsTransferComplete = false;
        }
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        chip::Messaging::SendFlags sendFlags;
        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "mExchangeContext is null, cannot proceed"));
        if (event.msgTypeData.MessageType == static_cast<uint8_t>(MessageType::ReceiveInit))
        {
            sendFlags.Set(chip::Messaging::SendMessageFlags::kFromInitiator);
        }
        if (event.msgTypeData.MessageType != static_cast<uint8_t>(MessageType::BlockAckEOF))
        {
            sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
        }
        err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                        sendFlags);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format()));
        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived:
        VerifyOrReturn(CHIP_NO_ERROR == mTransfer.PrepareBlockQuery(), ChipLogError(BDX, "PrepareBlockQuery failed"));
        break;
    case TransferSession::OutputEventType::kBlockReceived: {
        ChipLogDetail(BDX, "Got block length %zu", event.blockdata.Length);

        // TODO: while convenient, we should not do a synchronous block write in our example application - this is bad practice
        if (mOnBlockReceivedCallback != nullptr && mOnBlockReceivedCallback->mCall != nullptr)
        {
            mOnBlockReceivedCallback->mCall(mOnBlockReceivedCallback->mContext, event.blockdata);
        }
        else
        {
            ChipLogError(BDX, "onBlockReceived Callback not set");
        }
        if (event.blockdata.IsEof)
        {
            err = mTransfer.PrepareBlockAck();
            VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "PrepareBlockAck failed: %" CHIP_ERROR_FORMAT, err.Format()));
            mIsTransferComplete = true;
        }
        else
        {
            err = mTransfer.PrepareBlockQuery();
            VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, err.Format()));
        }
        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        if (mOnTransferFailedCallback != nullptr && mOnTransferFailedCallback->mCall != nullptr)
        {
            mOnTransferFailedCallback->mCall(mOnTransferFailedCallback->mContext, kErrorBdxDownloaderStatusReceived);
        }
        else
        {
            ChipLogError(BDX, "onTransferFailed Callback not set");
        }
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "InternalError");
        if (mOnTransferFailedCallback != nullptr && mOnTransferFailedCallback->mCall != nullptr)
        {
            mOnTransferFailedCallback->mCall(mOnTransferFailedCallback->mContext, kErrorBdxDownloaderInternal);
        }
        else
        {
            ChipLogError(BDX, "onTransferFailed Callback not set");
        }
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        if (mOnTransferFailedCallback != nullptr && mOnTransferFailedCallback->mCall != nullptr)
        {
            mOnTransferFailedCallback->mCall(mOnTransferFailedCallback->mContext, kErrorBdxDownloaderTimeOut);
        }
        else
        {
            ChipLogError(BDX, "onTransferFailed Callback not set");
        }
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "Unexpected BDX event type: %" PRIu16, static_cast<uint16_t>(event.EventType));
    }
}
