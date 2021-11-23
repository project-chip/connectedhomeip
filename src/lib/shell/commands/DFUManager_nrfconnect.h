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

#pragma once

#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#include <dfu/dfu_target.h>
#include <dfu/dfu_target_mcuboot.h>

namespace chip {
namespace Shell {

class DFUManager : public bdx::Initiator
{
public:
    void SetInitialExchange(Messaging::ExchangeContext * ec) { mExchangeCtx = ec; }
    CHIP_ERROR ApplyUpdate();
    CHIP_ERROR DiscardUpdate();

private:
    void HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event) override;

    uint8_t mDfuBuffer[1024];
    bool mIsTransferComplete = false;
};

inline CHIP_ERROR DFUManager::ApplyUpdate()
{
    return System::MapErrorZephyr(dfu_target_done(true));
}

inline CHIP_ERROR DFUManager::DiscardUpdate()
{
    return System::MapErrorZephyr(dfu_target_reset());
}

inline void DFUManager::HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event)
{
    using OutputEventType  = bdx::TransferSession::OutputEventType;
    using MessageType      = bdx::MessageType;
    using SendMessageFlags = Messaging::SendMessageFlags;

    CHIP_ERROR error = CHIP_NO_ERROR;
    int status       = 0;

    switch (event.EventType)
    {
    case OutputEventType::kNone:
        if (mIsTransferComplete)
        {
            ChipLogProgress(BDX, "Transfer complete!");
            mTransfer.Reset();
            mIsTransferComplete = false;
        }
        break;
    case OutputEventType::kMsgToSend: {
        Messaging::SendFlags flags;
        flags.Set(SendMessageFlags::kFromInitiator, event.msgTypeData.MessageType == to_underlying(MessageType::ReceiveInit));
        flags.Set(SendMessageFlags::kExpectResponse, event.msgTypeData.MessageType != to_underlying(MessageType::BlockAckEOF));

        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "Exchange context is null"));
        error =
            mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData), flags);
        VerifyOrReturn(error == CHIP_NO_ERROR, ChipLogError(BDX, "SendMessage() failed: %" CHIP_ERROR_FORMAT, error.Format()));
        break;
    }
    case OutputEventType::kAcceptReceived:
        ChipLogProgress(BDX, "Starting transfer of %uB", static_cast<unsigned>(event.transferAcceptData.Length));

        dfu_target_mcuboot_set_buf(mDfuBuffer, sizeof(mDfuBuffer));
        dfu_target_reset();
        status = dfu_target_init(DFU_TARGET_IMAGE_TYPE_MCUBOOT, event.transferAcceptData.Length, nullptr);
        VerifyOrReturn(status == 0, ChipLogError(BDX, "dfu_target_init() failed: %d", status));

        error = mTransfer.PrepareBlockQuery();
        VerifyOrReturn(error == CHIP_NO_ERROR,
                       ChipLogError(BDX, "PrepareBlockQuery() failed: %" CHIP_ERROR_FORMAT, error.Format()));
        break;
    case OutputEventType::kBlockReceived:
        ChipLogProgress(BDX, "Received %uB (total: %ukB)", static_cast<unsigned>(event.blockdata.Length),
                        static_cast<unsigned>(mTransfer.GetNumBytesProcessed()) / 1024u);

        status = dfu_target_write(event.blockdata.Data, event.blockdata.Length);
        VerifyOrReturn(status == 0, ChipLogError(BDX, "dfu_target_write() failed: %d", status));

        if (event.blockdata.IsEof)
        {
            error = mTransfer.PrepareBlockAck();
            VerifyOrReturn(error == CHIP_NO_ERROR,
                           ChipLogError(BDX, "PrepareBlockAck() failed: %" CHIP_ERROR_FORMAT, error.Format()));
            mIsTransferComplete = true;
        }
        else
        {
            error = mTransfer.PrepareBlockQuery();
            VerifyOrReturn(error == CHIP_NO_ERROR,
                           ChipLogError(BDX, "PrepareBlockQuery() failed: %" CHIP_ERROR_FORMAT, error.Format()));
        }
        break;
    case OutputEventType::kStatusReceived:
        ChipLogError(BDX, "Received status %" PRIu16, to_underlying(event.statusData.statusCode));
        mTransfer.Reset();
        dfu_target_reset();
        break;
    case OutputEventType::kInternalError:
        ChipLogError(BDX, "Transfer stopped due to internal error");
        mTransfer.Reset();
        dfu_target_reset();
        break;
    case OutputEventType::kTransferTimeout:
        ChipLogError(BDX, "Transfer timed out");
        mTransfer.Reset();
        dfu_target_reset();
        break;
    default:
        ChipLogError(BDX, "Unexpected event type: %" PRIu16, to_underlying(event.EventType));
        break;
    }
}

} // namespace Shell
} // namespace chip
