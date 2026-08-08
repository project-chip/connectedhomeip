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

#include "MediaFileManagementBdxClient.h"

#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/Constants.h>

namespace matter {
namespace casting {
namespace core {

using chip::ScopedNodeId;
using chip::SessionHandle;
using chip::bdx::StatusCode;
using chip::bdx::TransferControlFlags;
using chip::bdx::TransferRole;
using chip::bdx::TransferSession;

namespace {

// Receiver-drive BDX, matching the tv-app's advertised sender-drive-capable set.
constexpr uint16_t kMaxBlockSize                          = 1024;
constexpr chip::System::Clock::Timeout kBdxTimeout        = chip::System::Clock::Seconds16(5 * 60);
constexpr chip::System::Clock::Timeout kBdxPollIntervalMs = chip::System::Clock::Milliseconds32(50);

} // namespace

CHIP_ERROR MediaFileManagementBdxClient::StartDownload(ScopedNodeId peer, std::string designator, std::string targetPath,
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

    // Open a CASE session to the tv-app; BeginTransfer runs on success.
    chip::Server::GetInstance().GetCASESessionManager()->FindOrEstablishSession(peer, &mOnConnected, &mOnConnectionFailure);
    return CHIP_NO_ERROR;
}

void MediaFileManagementBdxClient::OnDeviceConnected(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                                     const SessionHandle & sessionHandle)
{
    auto * self    = static_cast<MediaFileManagementBdxClient *>(context);
    CHIP_ERROR err = self->BeginTransfer(exchangeMgr, sessionHandle);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "MediaFileManagementBdxClient: BeginTransfer failed: %" CHIP_ERROR_FORMAT, err.Format());
        self->Reset();
    }
}

void MediaFileManagementBdxClient::OnDeviceConnectionFailure(void * context, const ScopedNodeId & peer, CHIP_ERROR error)
{
    auto * self = static_cast<MediaFileManagementBdxClient *>(context);
    ChipLogError(BDX, "MediaFileManagementBdxClient: CASE session to peer failed: %" CHIP_ERROR_FORMAT, error.Format());
    self->Reset();
}

CHIP_ERROR MediaFileManagementBdxClient::BeginTransfer(chip::Messaging::ExchangeManager & exchangeMgr,
                                                       const SessionHandle & sessionHandle)
{
    mOutFile.open(mTargetPath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    VerifyOrReturnError(mOutFile.is_open(), CHIP_ERROR_OPEN_FAILED);

    TransferSession::TransferInitData initData;
    initData.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initData.MaxBlockSize     = kMaxBlockSize;
    initData.FileDesLength    = static_cast<uint16_t>(mDesignator.size());
    initData.FileDesignator   = reinterpret_cast<const uint8_t *>(mDesignator.data());

    chip::Messaging::ExchangeContext * exchangeCtx = exchangeMgr.NewContext(sessionHandle, this);
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);
    mExchangeCtx = exchangeCtx;

    CHIP_ERROR err = Initiator::InitiateTransfer(&chip::DeviceLayer::SystemLayer(), TransferRole::kReceiver, initData, kBdxTimeout,
                                                 kBdxPollIntervalMs);
    if (err != CHIP_NO_ERROR)
    {
        exchangeCtx->Close();
        mExchangeCtx = nullptr;
        return err;
    }
    return CHIP_NO_ERROR;
}

void MediaFileManagementBdxClient::HandleTransferSessionOutput(TransferSession::OutputEvent & event)
{
    if (event.EventType != TransferSession::OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "MediaFileManagementBdxClient: event %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case TransferSession::OutputEventType::kNone:
        break;
    case TransferSession::OutputEventType::kMsgToSend: {
        // As the receiver, the last message we send is BlockAckEOF; there is no
        // further reply, so the transfer is complete once it goes out.
        const bool isBlockAckEof = event.msgTypeData.HasMessageType(chip::bdx::MessageType::BlockAckEOF);
        chip::Messaging::SendFlags sendFlags;
        if (!event.msgTypeData.HasMessageType(chip::Protocols::SecureChannel::MsgType::StatusReport) && !isBlockAckEof)
        {
            sendFlags.Set(chip::Messaging::SendMessageFlags::kExpectResponse);
        }
        VerifyOrReturn(mExchangeCtx != nullptr);
        CHIP_ERROR err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType,
                                                   std::move(event.MsgData), sendFlags);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(BDX, "MediaFileManagementBdxClient: SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            Reset();
        }
        else if (isBlockAckEof)
        {
            ChipLogProgress(BDX, "MediaFileManagementBdxClient: download complete, %llu bytes -> %s",
                            static_cast<unsigned long long>(mBytesWritten), mTargetPath.c_str());
            mExchangeCtx = nullptr;
            Reset();
        }
        else if (!sendFlags.Has(chip::Messaging::SendMessageFlags::kExpectResponse))
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
            ChipLogError(BDX, "MediaFileManagementBdxClient: PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, err.Format());
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
                ChipLogError(BDX, "MediaFileManagementBdxClient: PrepareBlockAck failed: %" CHIP_ERROR_FORMAT, err.Format());
                Reset();
            }
        }
        else
        {
            CHIP_ERROR err = mTransfer.PrepareBlockQuery();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(BDX, "MediaFileManagementBdxClient: PrepareBlockQuery failed: %" CHIP_ERROR_FORMAT, err.Format());
                Reset();
            }
        }
        break;
    }
    case TransferSession::OutputEventType::kStatusReceived:
        ChipLogError(BDX, "MediaFileManagementBdxClient: StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        Reset();
        break;
    case TransferSession::OutputEventType::kInternalError:
        ChipLogError(BDX, "MediaFileManagementBdxClient: internal error");
        Reset();
        break;
    case TransferSession::OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "MediaFileManagementBdxClient: transfer timed out");
        Reset();
        break;
    case TransferSession::OutputEventType::kInitReceived:
    case TransferSession::OutputEventType::kQueryReceived:
    case TransferSession::OutputEventType::kQueryWithSkipReceived:
    case TransferSession::OutputEventType::kAckReceived:
    case TransferSession::OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "MediaFileManagementBdxClient: unexpected event");
        break;
    }
}

void MediaFileManagementBdxClient::Reset()
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

void MediaFileManagementBdxClient::AbortTransfer()
{
    if (mInitialized)
    {
        TEMPORARY_RETURN_IGNORED mTransfer.AbortTransfer(StatusCode::kUnknown);
        PollForOutput();
    }
}

} // namespace core
} // namespace casting
} // namespace matter
