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

#include "TransferFacilitator.h"

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <system/SystemClock.h>

namespace chip {
namespace bdx {

CHIP_ERROR TransferFacilitator::OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                                  chip::System::PacketBufferHandle && payload)
{
    if (mExchangeCtx == nullptr)
    {
        mExchangeCtx = ec;
    }

    ChipLogDetail(BDX, "%s: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId, __FUNCTION__,
                  payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));
    CHIP_ERROR err =
        mTransfer.HandleMessageReceived(payloadHeader, std::move(payload));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "failed to handle message: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Almost every BDX message except BlockAckEOF will follow up with a response on the exchange. Even messages that might signify the end of a
    // transfer could necessitate a response if they are received at the wrong time.
    // For this reason, it is left up to the application logic to call ExchangeContext::Close() when it has determined that the
    // transfer is finished.
    if (mExchangeCtx != nullptr && !payloadHeader.HasMessageType(chip::bdx::MessageType::BlockAckEOF)) {
    	mExchangeCtx->WillSendMessage();
    }

    return err;
}

void TransferFacilitator::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogError(BDX, "%s, ec: " ChipLogFormatExchange, __FUNCTION__, ChipLogValueExchange(ec));
    mExchangeCtx = nullptr;
    mTransfer.UnregisterOutputEventCallback(OnOutputEventReceived);
    mTransfer.Reset();
}

void TransferFacilitator::OnOutputEventReceived(void * context, TransferSession::OutputEvent & event)
{
	ChipLogProgress(BDX, "OnOutputEventReceived %hu", event.EventType);
	TransferFacilitator * facilitator = static_cast<TransferFacilitator *>(context);
	if (facilitator) {
		facilitator->HandleTransferSessionOutput(event);
	}
}

CHIP_ERROR Responder::PrepareForTransfer(System::Layer * layer, TransferRole role, BitFlags<TransferControlFlags> xferControlOpts,
                                         uint16_t maxBlockSize)
{
    VerifyOrReturnError(layer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(mTransfer.WaitForTransfer(layer, role, xferControlOpts, maxBlockSize));
    ChipLogProgress(BDX, "Register for output event");
    mTransfer.RegisterOutputEventCallback(this, OnOutputEventReceived);
    return CHIP_NO_ERROR;
}

void Responder::ResetTransfer()
{
    ChipLogProgress(BDX, "Unregister for output event");
	mTransfer.UnregisterOutputEventCallback(OnOutputEventReceived);
	ChipLogProgress(BDX, "calling problematic reset");
    mTransfer.Reset();
}

CHIP_ERROR Initiator::InitiateTransfer(TransferRole role, const TransferSession::TransferInitData & initData)
{
    ReturnErrorOnFailure(mTransfer.StartTransfer(role, initData));
    return CHIP_NO_ERROR;
}

} // namespace bdx
} // namespace chip
