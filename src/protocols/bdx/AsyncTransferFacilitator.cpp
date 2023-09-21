/*
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

#include <system/SystemClock.h>

namespace chip {
namespace bdx {

CHIP_ERROR AsyncTransferFacilitator::OnMessageReceived(Messaging::ExchangeContext * ec,
                                                       const PayloadHeader & payloadHeader,
                                                       System::PacketBufferHandle && payload)
{
    VerifyOrReturnError(ec == mExchange.Get(), CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(BDX, " %s: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId, __FUNCTION__,
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

    // Get the next output event and send it to the delegate.
    TransferSession::OutputEvent outEvent;
    do
    {
        mTransfer.GetNextAction(outEvent);

        if (outEvent.EventType == TransferSession::OutputEventType::kTransferTimeout)
        {
            OnResponseTimeout(ec);
        }
        else
        {
            HandleAsyncTransferSessionOutput(outEvent);
        }
    } while (outEvent.EventType != TransferSession::OutputEventType::kNone);

    return err;
}

Messaging::ExchangeContext * AsyncTransferFacilitator::GetExchangeContext()
{
    return mExchange.Get();
}

void AsyncTransferFacilitator::OnExchangeClosing(chip::Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "%s, ec: " ChipLogFormatExchange, __FUNCTION__, ChipLogValueExchange(ec));
    mExchange.Release();
}

void AsyncTransferFacilitator::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "%s, ec: " ChipLogFormatExchange, __FUNCTION__, ChipLogValueExchange(ec));
    mExchange.Release();
}

CHIP_ERROR AsyncResponder::PrepareForTransfer(Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                                              BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                              System::Clock::Timeout timeout)
{
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, timeout));

    VerifyOrReturnError(!mExchange, CHIP_ERROR_INCORRECT_STATE);
    mExchange.Grab(exchangeCtx);

    return CHIP_NO_ERROR;
}

void AsyncResponder::ResetTransfer()
{
    if (mExchange)
    {
        mExchange.Release();
    }
    mTransfer.Reset();
}

bdx::StatusCode AsyncResponder::GetBdxStatusCodeFromChipError(CHIP_ERROR err)
{
    if (err == CHIP_ERROR_INCORRECT_STATE)
    {
        return bdx::StatusCode::kUnexpectedMessage;
    }
    if (err == CHIP_ERROR_INVALID_ARGUMENT)
    {
        return bdx::StatusCode::kBadMessageContents;
    }
    return bdx::StatusCode::kUnknown;
}

void AsyncResponder::NotifyEventHandledWithError(CHIP_ERROR error)
{
    ChipLogDetail(BDX, "%s : error %d", __FUNCTION__, error.AsInteger());
    if (error != CHIP_NO_ERROR)
    {
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(error));
    }

    // Get the next output event  and send it to the delegate.
    TransferSession::OutputEvent outEvent;
    do
    {
        mTransfer.GetNextAction(outEvent);

        if (outEvent.EventType == TransferSession::OutputEventType::kTransferTimeout)
        {
            OnResponseTimeout(mExchange.Get());
        }
        else
        {
            HandleAsyncTransferSessionOutput(outEvent);
        }
    } while (outEvent.EventType != TransferSession::OutputEventType::kNone);
}

} // namespace bdx
} // namespace chip
