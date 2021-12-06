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

#include "OTAUpdater_esp32.h"
#include "platform/ESP32/ESP32Utils.h"
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

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
    return DeviceLayer::Internal::ESP32Utils::MapError(OTAUpdater::GetInstance().Apply());
}

inline CHIP_ERROR DFUManager::DiscardUpdate()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

inline void DFUManager::HandleTransferSessionOutput(bdx::TransferSession::OutputEvent & event)
{
    using OutputEventType  = bdx::TransferSession::OutputEventType;
    using MessageType      = bdx::MessageType;
    using SendMessageFlags = Messaging::SendMessageFlags;

    CHIP_ERROR err = CHIP_NO_ERROR;

    if (event.EventType != OutputEventType::kNone)
    {
        ChipLogDetail(BDX, "OutputEvent type: %s", event.ToString(event.EventType));
    }

    switch (event.EventType)
    {
    case OutputEventType::kNone:
        if (mIsTransferComplete)
        {
            ChipLogProgress(BDX, "Transfer complete!");
            OTAUpdater::GetInstance().End();
            mTransfer.Reset();
            mIsTransferComplete = false;
        }
        break;
    case OutputEventType::kMsgToSend: {
        Messaging::SendFlags sendFlags;
        VerifyOrReturn(mExchangeCtx != nullptr, ChipLogError(BDX, "mExchangeContext is null, cannot proceed"));
        sendFlags.Set(SendMessageFlags::kFromInitiator, event.msgTypeData.MessageType == to_underlying(MessageType::ReceiveInit));
        sendFlags.Set(SendMessageFlags::kExpectResponse, event.msgTypeData.MessageType != to_underlying(MessageType::BlockAckEOF));
        err = mExchangeCtx->SendMessage(event.msgTypeData.ProtocolId, event.msgTypeData.MessageType, std::move(event.MsgData),
                                        sendFlags);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(BDX, "SendMessage() failed: %" CHIP_ERROR_FORMAT, err.Format()));
        break;
    }
    case OutputEventType::kAcceptReceived: {
        VerifyOrReturn(CHIP_NO_ERROR == mTransfer.PrepareBlockQuery(), ChipLogError(BDX, "PrepareBlockQuery failed"));
        break;
    }
    case OutputEventType::kBlockReceived: {
        ChipLogDetail(BDX, "Got block length %zu", event.blockdata.Length);
        if (OTAUpdater::GetInstance().IsInProgress() == false)
        {
            OTAUpdater::GetInstance().Begin();
        }
        // TODO: Process/skip the Matter OTA header
        OTAUpdater::GetInstance().Write(reinterpret_cast<const void *>(event.blockdata.Data), event.blockdata.Length);
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
    case OutputEventType::kStatusReceived: {
        ChipLogError(BDX, "Got StatusReport %x", static_cast<uint16_t>(event.statusData.statusCode));
        OTAUpdater::GetInstance().Abort();
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    }
    case OutputEventType::kInternalError: {
        ChipLogError(BDX, "Transfer stopped due to internal error");
        OTAUpdater::GetInstance().Abort();
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    }
    case OutputEventType::kTransferTimeout: {
        ChipLogError(BDX, "Transfer timed out");
        OTAUpdater::GetInstance().Abort();
        mTransfer.Reset();
        mExchangeCtx->Close();
        break;
    }
    case OutputEventType::kInitReceived:
    case OutputEventType::kAckReceived:
    case OutputEventType::kQueryReceived:
    case OutputEventType::kAckEOFReceived:
    default:
        ChipLogError(BDX, "Unexpected BDX event type: %" PRIu16, static_cast<uint16_t>(event.EventType));
    }
}

} // namespace Shell
} // namespace chip
