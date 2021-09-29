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

#include <fstream>

using namespace chip::bdx;

uint32_t numBlocksRead   = 0;
const char outFilePath[] = "test-ota-out.txt";

void BdxDownloader::SetInitialExchange(chip::Messaging::ExchangeContext * ec)
{
    mExchangeCtx = ec;
}

void BdxDownloader::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %d", static_cast<uint16_t>(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        if (mIsTransferComplete)
        {
            ChipLogDetail(BDX, "Transfer complete! Contents written/appended to %s", outFilePath);
            mTransfer.Reset();
            mIsTransferComplete = false;
        }
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        chip::Messaging::SendFlags sendFlags;
        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "%s: mExchangeContext is null", __FUNCTION__));
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
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "%s: SendMessage failed: %s", __FUNCTION__, chip::ErrorStr(err)));
        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived:
        VerifyOrReturn(CHIP_NO_ERROR == mTransfer.PrepareBlockQuery(),
                       ChipLogError(BDX, "%s: PrepareBlockQuery failed", __FUNCTION__));
        break;
    case TransferSession::OutputEventType::kBlockReceived: {
        ChipLogDetail(BDX, "Got block length %zu", event.blockdata.Length);

        // TODO: something more elegant than appending to a local file
        // TODO: while convenient, we should not do a synchronous block write in our example application - this is bad practice
        std::ofstream otaFile(outFilePath, std::ifstream::out | std::ifstream::ate | std::ifstream::app);
        otaFile.write(reinterpret_cast<const char *>(event.blockdata.Data), event.blockdata.Length);

        if (event.blockdata.IsEof)
        {
            err = mTransfer.PrepareBlockAck();
            VerifyOrReturn(err == CHIP_NO_ERROR,
                           ChipLogError(BDX, "%s: PrepareBlockAck failed: %s", __FUNCTION__, chip::ErrorStr(err)));
            mIsTransferComplete = true;
        }
        else
        {
            err = mTransfer.PrepareBlockQuery();
            VerifyOrReturn(err == CHIP_NO_ERROR,
                           ChipLogError(BDX, "%s: PrepareBlockQuery failed: %s", __FUNCTION__, chip::ErrorStr(err)));
        }
        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "InternalError");
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "%s: unexpected event type", __FUNCTION__);
    }
}
