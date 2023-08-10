/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "AsyncTransferFacilitator.h"

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>
#include <unistd.h>

namespace chip {

namespace bdx {

CHIP_ERROR AsyncTransferFacilitator::OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                                  chip::System::PacketBufferHandle && payload)
{
        // Get the next output event 
    TransferSession::OutputEvent outEvent;
    
    ChipLogDetail(BDX, "OnMessageReceived %s: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId, __FUNCTION__,
                  payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));
    CHIP_ERROR err =
        mTransfer.HandleMessageReceived(payloadHeader, std::move(payload), System::SystemClock().GetMonotonicTimestamp());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "failed to handle message: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Almost every BDX message will follow up with a response on the exchange. Even messages that might signify the end of a
    // transfer could necessitate a response if they are received at the wrong time.
    // For this reason, it is left up to the application logic to call ExchangeContext::Close() when it has determined that the
    // transfer is finished.
    ec->WillSendMessage();
    
    do
    {
        mTransfer.GetNextAction(outEvent);
    
        ChipLogError(BDX, "AsyncTransferFacilitator outEvent %s", outEvent.ToString(outEvent.EventType));
        
        if (outEvent.EventType == TransferSession::OutputEventType::kTransferTimeout) {
            OnResponseTimeout(ec);
        } else {
            HandleAsyncTransferSessionOutput(ec, outEvent);
        }
        ChipLogError(BDX, "AsyncTransferFacilitator register");
        //mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
    } while (outEvent.EventType != TransferSession::OutputEventType::kNone);
    

    return err;
}

void AsyncTransferFacilitator::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogError(BDX, "%s, ec: " ChipLogFormatExchange, __FUNCTION__, ChipLogValueExchange(ec));
    mTransfer.Reset();
}

CHIP_ERROR AsyncResponder::PrepareForTransfer(chip::Messaging::ExchangeManager * exchangeManager, TransferRole role, BitFlags<TransferControlFlags> xferControlOpts,
                                         uint16_t maxBlockSize, System::Clock::Timeout timeout)
{
    mExchangeMgr = exchangeManager;
    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, timeout));
    
    
    if (role == TransferRole::kSender)
    {
        mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
        //mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::SecureChannel::Id, this);
        //mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(Protocols::BDX::Id, static_cast<uint8_t>(TransferSession::OutputEventType::kMsgToSend),this);
    }
    
    // create a manager for nodeid
    
   // register for init received on t

    return CHIP_NO_ERROR;
}

void AsyncResponder::ResetTransfer()
{
    mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
    mExchangeMgr = nullptr;
    mTransfer.Reset();
}

bdx::StatusCode AsyncResponder::GetBdxStatusCodeFromChipError(CHIP_ERROR err)
    {
        if (err == CHIP_ERROR_INCORRECT_STATE) {
            return bdx::StatusCode::kUnexpectedMessage;
        }
        if (err == CHIP_ERROR_INVALID_ARGUMENT) {
            return bdx::StatusCode::kBadMessageContents;
        }
        return bdx::StatusCode::kUnknown;
    }

void AsyncResponder::OnEventHandled(chip::Messaging::ExchangeContext * ec, CHIP_ERROR error)
{
    ChipLogError(BDX, "AsyncTransferFacilitator error %d", error.AsInteger());
    if (error != CHIP_NO_ERROR)
    {
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(error));
    }
 
    // Get the next output event 
    TransferSession::OutputEvent outEvent;
    do
    {
        mTransfer.GetNextAction(outEvent);
    
        ChipLogError(BDX, "AsyncTransferFacilitator OnEventHandled outEvent %s", outEvent.ToString(outEvent.EventType));
        
        if (outEvent.EventType == TransferSession::OutputEventType::kTransferTimeout) {
            OnResponseTimeout(ec);
        } else {
            HandleAsyncTransferSessionOutput(ec, outEvent);
        }
    } while (outEvent.EventType != TransferSession::OutputEventType::kNone);
}

CHIP_ERROR Initiator::InitiateTransfer(TransferRole role, const TransferSession::TransferInitData & initData,
                                       System::Clock::Timeout timeout)
{
    ReturnErrorOnFailure(mTransfer.StartTransfer(role, initData, timeout));


    return CHIP_NO_ERROR;
}

} // namespace bdx
} // namespace chip