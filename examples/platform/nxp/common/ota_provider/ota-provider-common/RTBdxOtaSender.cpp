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

#include "RTBdxOtaSender.h"

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/CHIPMemString.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <protocols/bdx/BdxTransferSession.h>

#include "fwk_filesystem.h"
#include "ota-fw-placeholder.bin.h"
#include <fstream>

using chip::bdx::StatusCode;
using chip::bdx::TransferControlFlags;
using chip::bdx::TransferSession;

chip::NXP::App::RTBdxOtaSender::RTBdxOtaSender()
{
    memset(mFilePath, 0, kFilepathMaxLength);
}

void chip::NXP::App::RTBdxOtaSender::SetFilepath(const char * path)
{
    if (path != nullptr)
    {
        chip::Platform::CopyString(mFilePath, path);
    }
    else
    {
        memset(mFilePath, 0, kFilepathMaxLength);
    }
}

CHIP_ERROR chip::NXP::App::RTBdxOtaSender::InitializeTransfer(chip::FabricIndex fabricIndex, chip::NodeId nodeId)
{
    if (mInitialized)
    {
        // Reset stale connection from the Same Node if exists
        if ((mFabricIndex.HasValue() && mFabricIndex.Value() == fabricIndex) && (mNodeId.HasValue() && mNodeId.Value() == nodeId))
        {
            Reset();
        }
        // Prevent a new node connection since another is active
        else if ((mFabricIndex.HasValue() && mFabricIndex.Value() != fabricIndex) ||
                 (mNodeId.HasValue() && mNodeId.Value() != nodeId))
        {
            return CHIP_ERROR_BUSY;
        }
        else
        {
            return CHIP_ERROR_INTERNAL;
        }
    }
    mFabricIndex.SetValue(fabricIndex);
    mNodeId.SetValue(nodeId);
    mInitialized = true;
    return CHIP_NO_ERROR;
}

void chip::NXP::App::RTBdxOtaSender::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
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
        VerifyOrReturn(mExchangeCtx != nullptr);
        err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                        sendFlags);

        if (err == CHIP_NO_ERROR)
        {
            if (!sendFlags.Has(chip::Messaging::SendMessageFlags::kExpectResponse))
            {
                // After sending the StatusReport, exchange context gets closed so, set mExchangeCtx to null
                mExchangeCtx = nullptr;
            }
        }
        else
        {
            ChipLogError(BDX, "SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            Reset();
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
        acceptData.Length       = ota_fw_placeholder_bin_len;
        VerifyOrReturn(mTransfer.AcceptTransfer(acceptData) == CHIP_NO_ERROR,
                       ChipLogError(BDX, "AcceptTransfer failed: %" CHIP_ERROR_FORMAT, err.Format()));
        break;
    }
    case TransferSession::OutputEventType::kQueryReceived: {
        TransferSession::BlockData blockData;
        uint16_t blockSize   = mTransfer.GetTransferBlockSize();
        uint16_t bytesToRead = blockSize;

        // TODO: This should be a utility function in TransferSession
        if (ota_fw_placeholder_bin_len > 0 && mNumBytesSent + blockSize > ota_fw_placeholder_bin_len)
        {
            // cast should be safe because of condition above
            bytesToRead = static_cast<uint16_t>(ota_fw_placeholder_bin_len - mNumBytesSent);
        }

        chip::System::PacketBufferHandle blockBuf = chip::System::PacketBufferHandle::New(bytesToRead);
        if (blockBuf.IsNull())
        {
            // TODO(#13981): AbortTransfer() needs to support GeneralStatusCode failures as well as BDX specific errors.
            mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }

        FS_ReadBufferFromFileLocation(mFilePath, reinterpret_cast<uint8_t *>(blockBuf->Start()), bytesToRead, mNumBytesSent);

        blockData.Data   = blockBuf->Start();
        blockData.Length = static_cast<size_t>(bytesToRead);
        blockData.IsEof  = (blockData.Length < blockSize) ||
            (mNumBytesSent + static_cast<uint64_t>(blockData.Length) == ota_fw_placeholder_bin_len ||
             (mNumBytesSent + bytesToRead == ota_fw_placeholder_bin_len));
        mNumBytesSent = static_cast<uint32_t>(mNumBytesSent + blockData.Length);

        err = mTransfer.PrepareBlock(blockData);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "PrepareBlock failed: %" CHIP_ERROR_FORMAT, err.Format());
            mTransfer.AbortTransfer(StatusCode::kUnknown);
        }
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
        ChipLogError(BDX, "Unsupported event type");
    }
}

void chip::NXP::App::RTBdxOtaSender::Reset()
{
    mFabricIndex.ClearValue();
    mNodeId.ClearValue();
    mTransfer.Reset();
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }

    mInitialized  = false;
    mNumBytesSent = 0;
}
