/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "MediaFileManagementBdxServer.h"

#include <lib/support/BitFlags.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/Constants.h>

#include <cstring>
#include <fstream>
#include <limits>

namespace matter {
namespace casting {
namespace core {

using chip::ScopedNodeId;
using chip::bdx::StatusCode;
using chip::bdx::TransferControlFlags;
using chip::bdx::TransferSession;

void MediaFileManagementBdxServer::Allow(ScopedNodeId peer, std::string designator, std::string path)
{
    mGrants[std::move(designator)] = Grant{ peer, std::move(path) };
}

void MediaFileManagementBdxServer::Revoke(ScopedNodeId peer)
{
    for (auto it = mGrants.begin(); it != mGrants.end();)
    {
        if (it->second.peer == peer)
        {
            it = mGrants.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

CHIP_ERROR MediaFileManagementBdxServer::Arm()
{
    // Advertise only synchronous receiver-drive transfers, matching OTA.
    chip::BitFlags<TransferControlFlags> flags(TransferControlFlags::kReceiverDrive);
    constexpr uint16_t kMaxBlockSize                          = 1024;
    constexpr chip::System::Clock::Timeout kBdxTimeout        = chip::System::Clock::Seconds16(5 * 60);
    constexpr chip::System::Clock::Timeout kBdxPollIntervalMs = chip::System::Clock::Milliseconds32(50);
    return PrepareForTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kSender, flags, kMaxBlockSize,
                              kBdxTimeout, kBdxPollIntervalMs);
}

void MediaFileManagementBdxServer::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "MediaFileManagementBdxServer: event %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        chip::Messaging::SendFlags sendFlags;
        if (!event.msgTypeData.HasMessageType(chip::Protocols::SecureChannel::MsgType::StatusReport))
        {
            sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
        }
        VerifyOrReturn(mExchangeCtx != nullptr);
        CHIP_ERROR err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                   std::move(event.MsgData), sendFlags);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "MediaFileManagementBdxServer: SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            Reset();
        }
        else if (!sendFlags.Has(chip::Messaging::SendMessageFlags::kExpectResponse))
        {
            // The final StatusReport closes the exchange.
            mExchangeCtx = nullptr;
        }
        break;
    }
    case TransferSession::OutputEventType::kInitReceived: {
        // Record the requested designator.
        uint16_t fdl       = 0;
        const uint8_t * fd = mTransfer.GetFileDesignator(fdl);
        VerifyOrReturn(fdl < chip::bdx::kMaxFileDesignatorLen,
                       ChipLogError(BDX, "MediaFileManagementBdxServer: designator too long (%u)", fdl));
        memcpy(mFileDesignator, fd, fdl);
        mFileDesignator[fdl] = 0;

        // Authorize: the designator must have been armed, and for the node that
        // is now pulling it (identified by the CASE session peer).
        VerifyOrReturn(mExchangeCtx != nullptr && mExchangeCtx->HasSessionHandle());
        const ScopedNodeId requester = mExchangeCtx->GetSessionHandle()->GetPeer();
        const auto entry             = mGrants.find(mFileDesignator);
        if (entry == mGrants.cend() || entry->second.peer != requester)
        {
            ChipLogError(BDX, "MediaFileManagementBdxServer: rejecting unauthorized designator %s", mFileDesignator);
            VerifyOrReturn(mTransfer.AbortTransfer(StatusCode::kFileDesignatorUnknown) == CHIP_NO_ERROR,
                           ChipLogError(BDX, "MediaFileManagementBdxServer: AbortTransfer failed"));
            return;
        }
        mActivePath   = entry->second.path;
        mNumBytesSent = 0;

        TransferSession::TransferAcceptData acceptData;
        acceptData.ControlMode  = TransferControlFlags::kReceiverDrive;
        acceptData.MaxBlockSize = mTransfer.GetTransferBlockSize();
        acceptData.StartOffset  = mTransfer.GetStartOffset();
        acceptData.Length       = mTransfer.GetTransferLength();
        VerifyOrReturn(mTransfer.AcceptTransfer(acceptData) == CHIP_NO_ERROR,
                       ChipLogError(BDX, "MediaFileManagementBdxServer: AcceptTransfer failed"));
        break;
    }
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kQueryWithSkipReceived: {
        uint16_t blockSize   = mTransfer.GetTransferBlockSize();
        uint16_t bytesToRead = blockSize;
        uint64_t bytesToSkip = 0;
        if (event.EventType == TransferSession::OutputEventType::kQueryWithSkipReceived)
        {
            bytesToSkip = event.bytesToSkip.BytesToSkip;
        }
        uint64_t seekOffset = mNumBytesSent + bytesToSkip;

        if ((mTransfer.GetTransferLength() > 0) && ((seekOffset + blockSize) > mTransfer.GetTransferLength()))
        {
            bytesToRead = static_cast<uint16_t>(mTransfer.GetTransferLength() - seekOffset);
        }

        chip::System::PacketBufferHandle blockBuf = chip::System::PacketBufferHandle::New(bytesToRead);
        if (blockBuf.IsNull())
        {
            TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kUnknown);
            return;
        }

        std::ifstream file(mActivePath.c_str(), std::ifstream::in | std::ios::binary);
        if (!file.good())
        {
            ChipLogError(BDX, "MediaFileManagementBdxServer: cannot open %s", mActivePath.c_str());
            TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kFileDesignatorUnknown);
            return;
        }
        if (seekOffset > static_cast<uint64_t>(std::numeric_limits<std::streamoff>::max()))
        {
            TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kLengthTooLarge);
            return;
        }
        file.seekg(static_cast<std::streamoff>(seekOffset));
        file.read(reinterpret_cast<char *>(blockBuf->Start()), bytesToRead);
        if (!(file.good() || file.eof()))
        {
            ChipLogError(BDX, "MediaFileManagementBdxServer: read failed on %s", mActivePath.c_str());
            TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kFileDesignatorUnknown);
            return;
        }

        TransferSession::BlockData blockData;
        blockData.Data   = blockBuf->Start();
        blockData.Length = static_cast<size_t>(file.gcount());
        blockData.IsEof  = (blockData.Length < blockSize) ||
            (seekOffset + static_cast<uint64_t>(blockData.Length) == mTransfer.GetTransferLength()) || (file.peek() == EOF);
        mNumBytesSent = seekOffset + blockData.Length;

        CHIP_ERROR err = mTransfer.PrepareBlock(blockData);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "MediaFileManagementBdxServer: PrepareBlock failed: %" CHIP_ERROR_FORMAT, err.Format());
            TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kUnknown);
        }
        break;
    }
    case TransferSession::OutputEventType::kAckReceived:
        break;
    case TransferSession::OutputEventType::kAckEOFReceived:
        ChipLogProgress(BDX, "MediaFileManagementBdxServer: transfer complete");
        Reset();
        break;
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "MediaFileManagementBdxServer: StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        Reset();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "MediaFileManagementBdxServer: internal error");
        Reset();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "MediaFileManagementBdxServer: transfer timed out");
        Reset();
        break;
    case TransferSession::OutputEventType::kAcceptReceived:
    case TransferSession::OutputEventType::kBlockReceived:
    default:
        ChipLogError(BDX, "MediaFileManagementBdxServer: unexpected event");
        break;
    }
}

void MediaFileManagementBdxServer::Reset()
{
    ResetTransfer();
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }
    mActivePath.clear();
    mNumBytesSent = 0;
    memset(mFileDesignator, 0, sizeof(mFileDesignator));
}

void MediaFileManagementBdxServer::AbortTransfer()
{
    TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kUnknown);
    PollForOutput();
}

} // namespace core
} // namespace casting
} // namespace matter
