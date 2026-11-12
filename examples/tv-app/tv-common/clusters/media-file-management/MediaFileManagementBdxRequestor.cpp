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

#include "MediaFileManagementBdxRequestor.h"

#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MediaFileManagement {

using chip::bdx::StatusCode;
using chip::bdx::TransferControlFlags;
using chip::bdx::TransferRole;
using chip::bdx::TransferSession;

namespace {

// Receiver-drive BDX, matching the client's advertised sender-drive-capable set.
constexpr uint16_t kMaxBlockSize                    = 1024;
constexpr System::Clock::Timeout kBdxTimeout        = System::Clock::Seconds16(5 * 60);
constexpr System::Clock::Timeout kBdxPollIntervalMs = System::Clock::Milliseconds32(50);

} // namespace

CHIP_ERROR MediaFileManagementBdxRequestor::StartDownload(ScopedNodeId peer, std::string designator, std::string targetPath,
                                                          uint64_t expectedSize)
{
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_BUSY);
    VerifyOrReturnError(!designator.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mPeer         = peer;
    mDesignator   = std::move(designator);
    mTargetPath   = std::move(targetPath);
    mExpectedSize = expectedSize;
    mBytesWritten = 0;
    mInitialized  = true;

    // Open a CASE session back to the client; BeginTransfer runs on success.
    Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(peer, &mOnConnected, &mOnConnectionFailure);
    return CHIP_NO_ERROR;
}

void MediaFileManagementBdxRequestor::OnDeviceConnected(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                        const SessionHandle & sessionHandle)
{
    auto * self = static_cast<MediaFileManagementBdxRequestor *>(context);
    // The transfer may have been aborted while the CASE session was still being
    // established; Reset() clears mInitialized. Do not start a transfer for a
    // download that is no longer wanted.
    VerifyOrReturn(self->mInitialized,
                   ChipLogProgress(BDX, "MediaFileManagementBdxRequestor: session established after abort, ignoring"));
    CHIP_ERROR err = self->BeginTransfer(exchangeMgr, sessionHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "MediaFileManagementBdxRequestor: BeginTransfer failed: %" CHIP_ERROR_FORMAT, err.Format());
        self->Reset();
    }
}

void MediaFileManagementBdxRequestor::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peer, CHIP_ERROR error)
{
    auto * self = static_cast<MediaFileManagementBdxRequestor *>(context);
    ChipLogError(BDX, "MediaFileManagementBdxRequestor: CASE session to peer failed: %" CHIP_ERROR_FORMAT, error.Format());
    self->Reset();
}

CHIP_ERROR MediaFileManagementBdxRequestor::BeginTransfer(Messaging::ExchangeManager & exchangeMgr,
                                                          const SessionHandle & sessionHandle)
{
    mOutFile.open(mTargetPath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    VerifyOrReturnError(mOutFile.is_open(), CHIP_ERROR_OPEN_FAILED);

    TransferSession::TransferInitData initData;
    initData.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initData.MaxBlockSize     = kMaxBlockSize;
    initData.FileDesLength    = static_cast<uint16_t>(mDesignator.size());
    initData.FileDesignator   = reinterpret_cast<const uint8_t *>(mDesignator.data());

    Messaging::ExchangeContext * exchangeCtx = exchangeMgr.NewContext(sessionHandle, this);
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);
    mExchangeCtx = exchangeCtx;

    CHIP_ERROR err = Initiator::InitiateTransfer(&DeviceLayer::SystemLayer(), TransferRole::kReceiver, initData, kBdxTimeout,
                                                 kBdxPollIntervalMs);
    if (err != CHIP_NO_ERROR)
    {
        exchangeCtx->Close();
        mExchangeCtx = nullptr;
        return err;
    }
    return CHIP_NO_ERROR;
}

void MediaFileManagementBdxRequestor::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "MediaFileManagementBdxRequestor: event %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        // As the receiver, the last message we send is BlockAckEOF; there is no
        // further reply, so the transfer is complete once it goes out.
        const bool isBlockAckEof = event.msgTypeData.HasMessageType(bdx::MessageType::BlockAckEOF);
        Messaging::SendFlags sendFlags;
        if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport) && !isBlockAckEof)
        {
            sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
        }
        VerifyOrReturn(mExchangeCtx != nullptr);
        CHIP_ERROR err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                   std::move(event.MsgData), sendFlags);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "MediaFileManagementBdxRequestor: SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            Reset();
        }
        else if (isBlockAckEof)
        {
            // The receiver has acknowledged EOF: the download is finished. Flag
            // (but do not fail, the bytes are already acked) a short write or a
            // size mismatch against the metadata the command advertised.
            if (!mOutFile.good())
            {
                ChipLogError(BDX, "MediaFileManagementBdxRequestor: write error on %s", mTargetPath.c_str());
            }
            else if (mExpectedSize != 0 && mBytesWritten != mExpectedSize)
            {
                ChipLogError(BDX, "MediaFileManagementBdxRequestor: size mismatch, got %llu expected %llu",
                             static_cast<unsigned long long>(mBytesWritten), static_cast<unsigned long long>(mExpectedSize));
            }
            else
            {
                ChipLogProgress(BDX, "MediaFileManagementBdxRequestor: download complete, %llu bytes -> %s",
                                static_cast<unsigned long long>(mBytesWritten), mTargetPath.c_str());
            }
            mExchangeCtx = nullptr;
            Reset();
        }
        else if (!sendFlags.Has(Messaging::SendMessageFlags::kExpectResponse))
        {
            mExchangeCtx = nullptr;
        }
        break;
    }
    case TransferSession::OutputEventType::kAcceptReceived: {
        // Transfer accepted: as the receiver we drive it by querying the first block.
        CHIP_ERROR err = mTransfer.PrepareBlockQuery();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "MediaFileManagementBdxRequestor: PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, err.Format());
            Reset();
        }
        break;
    }
    case TransferSession::OutputEventType::kBlockReceived: {
        const TransferSession::BlockData & block = event.blockdata;
        if (block.Length > 0)
        {
            mOutFile.write(reinterpret_cast<const char *>(block.Data), static_cast<std::streamsize>(block.Length));
            mBytesWritten += block.Length;
        }

        if (block.IsEof)
        {
            mOutFile.flush();
            CHIP_ERROR err = mTransfer.PrepareBlockAck();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(BDX, "MediaFileManagementBdxRequestor: PrepareBlockAck failed: %" CHIP_ERROR_FORMAT, err.Format());
                Reset();
            }
        }
        else
        {
            CHIP_ERROR err = mTransfer.PrepareBlockQuery();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(BDX, "MediaFileManagementBdxRequestor: PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, err.Format());
                Reset();
            }
        }
        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "MediaFileManagementBdxRequestor: StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        Reset();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "MediaFileManagementBdxRequestor: internal error");
        Reset();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "MediaFileManagementBdxRequestor: transfer timed out");
        Reset();
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "MediaFileManagementBdxRequestor: unexpected event");
        break;
    }
}

void MediaFileManagementBdxRequestor::Reset()
{
    ResetTransfer();
    if (mExchangeCtx != nullptr)
    {
        mExchangeCtx->Close();
        mExchangeCtx = nullptr;
    }
    if (mOutFile.is_open())
    {
        mOutFile.close();
    }
    mDesignator.clear();
    mTargetPath.clear();
    mExpectedSize = 0;
    mBytesWritten = 0;
    mInitialized  = false;
}

void MediaFileManagementBdxRequestor::AbortTransfer()
{
    if (mInitialized)
    {
        TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kUnknown);
        PollForOutput();
    }
}

} // namespace MediaFileManagement
} // namespace Clusters
} // namespace app
} // namespace chip
