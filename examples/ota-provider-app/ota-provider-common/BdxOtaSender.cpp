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
#include <protocols/bdx/BdxTransferSession.h>

#include <fstream>

using chip::bdx::StatusCode;
using chip::bdx::TransferControlFlags;
using chip::bdx::TransferSession;

BdxOtaSender::BdxOtaSender()
{
    memset(mFilepath, 0, kFilepathMaxLength);
}

void BdxOtaSender::SetFilepath(const char * path)
{
    if (path != nullptr)
    {
        chip::Platform::CopyString(mFilepath, path);
    }
    else
    {
        memset(mFilepath, 0, kFilepathMaxLength);
    }
}

void BdxOtaSender::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %d", static_cast<uint16_t>(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kMsgToSend:
        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "%s: mExchangeCtx is null", __FUNCTION__));
        err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "SendMessage failed: %s", chip::ErrorStr(err));
        }
        break;
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

        // TODO: This should be a utility function in TransferSession
        if (mTransfer.GetTransferLength() > 0 && mNumBytesSent + blockSize > mTransfer.GetTransferLength())
        {
            // cast should be safe because of condition above
            bytesToRead = static_cast<uint16_t>(mTransfer.GetTransferLength() - mNumBytesSent);
        }

        chip::System::PacketBufferHandle blockBuf = chip::System::PacketBufferHandle::New(bytesToRead);
        if (blockBuf.IsNull())
        {
            // TODO: AbortTransfer() needs to support GeneralStatusCode failures as well as BDX specific errors.
            mTransfer.AbortTransfer(StatusCode::kUnknown);
        }

        std::ifstream otaFile(mFilepath, std::ifstream::in);
        VerifyOrReturn(otaFile.good(), ChipLogError(BDX, "%s: file read failed", __FUNCTION__));
        otaFile.seekg(mNumBytesSent);
        otaFile.read(reinterpret_cast<char *>(blockBuf->Start()), bytesToRead);
        VerifyOrReturn(otaFile.good(), ChipLogError(BDX, "%s: file read failed", __FUNCTION__));

        blockData.Data   = blockBuf->Start();
        blockData.Length = otaFile.gcount();
        blockData.IsEof  = (otaFile.gcount() < blockSize) ||
            (mNumBytesSent + static_cast<uint64_t>(otaFile.gcount()) == mTransfer.GetTransferLength() || (otaFile.peek() == EOF));
        mNumBytesSent = static_cast<uint32_t>(mNumBytesSent + otaFile.gcount());

        VerifyOrReturn(CHIP_NO_ERROR == mTransfer.PrepareBlock(blockData),
                       ChipLogError(BDX, "%s: PrepareBlock failed: %s", __FUNCTION__, chip::ErrorStr(err)));
        break;
    }
    case TransferSession::OutputEventType::kAckReceived:
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        ChipLogDetail(BDX, "Transfer completed, got AckEOF");
        Reset();
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        Reset();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "InternalError");
        Reset();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
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
    memset(mFilepath, 0, kFilepathMaxLength);
}
