/*
 *    Copyright (c) 2024 Project CHIP Authors
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

/**
 * Releases the exchange, resets the transfer session and schedules calling the DestroySelf
 * virtual method implemented by the subclass to delete the subclass.
 */
void AsyncTransferFacilitator::CleanUp()
{
    mExchange.Release();
    mTransfer.Reset();
    VerifyOrReturn(mSystemLayer != nullptr, ChipLogError(BDX, "CleanUp: mSystemLayer is null"));

    mSystemLayer->ScheduleWork(
        [](auto * systemLayer, auto * appState) -> void {
            auto * _this = static_cast<AsyncTransferFacilitator *>(appState);
            _this->DestroySelf();
        },
        this);
}

AsyncTransferFacilitator::~AsyncTransferFacilitator() {}

bdx::StatusCode AsyncTransferFacilitator::GetBdxStatusCodeFromChipError(CHIP_ERROR err)
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

/**
 * Calls the GetNextAction on the TransferSession to get the next output events until it receives
 * TransferSession::OutputEventType::kNone If the output event is of type TransferSession::OutputEventType::kMsgToSend, it sends the
 * message over the exchange context, otherwise it calls the HandleTransferSessionOutput method implemented by the subclass to
 * handle the BDX message.
 */
void AsyncTransferFacilitator::HandleNextOutputEvents()
{
    if (mHandlingOutputEvents)
    {
        ChipLogDetail(BDX, "HandleNextOutputEvents: Still getting and processing output events from a previous call. Return.");
        return;
    }

    mHandlingOutputEvents = true;

    // Get the next output event and handle it based on the type of event.
    // If its of type kMsgToSend send it over the exchange, otherwise call the HandleTransferSessionOutput
    // virtual method that must be implemeted by the subclass of this class to handle the BDX message.
    TransferSession::OutputEvent outEvent;

    mTransfer.GetNextAction(outEvent);
    while (outEvent.EventType != TransferSession::OutputEventType::kNone)
    {
        if (outEvent.EventType == TransferSession::OutputEventType::kMsgToSend)
        {
            SendMessage(outEvent.msgTypeData, outEvent.MsgData);
        }
        else
        {
            HandleTransferSessionOutput(outEvent);
        }
        mTransfer.GetNextAction(outEvent);
    }
    mHandlingOutputEvents = false;
}

CHIP_ERROR AsyncTransferFacilitator::SendMessage(const TransferSession::MessageTypeData msgTypeData,
                                                 System::PacketBufferHandle & msgBuf)
{
    VerifyOrReturnError(mExchange, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages that are sent expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    Messaging::ExchangeContext * ec = mExchange.Get();

    // Set the response timeout on the exchange before sending the message.
    ec->SetResponseTimeout(mTimeout);

    CHIP_ERROR err = ec->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(msgBuf), sendFlags);

    // If we failed to send the message across the exchange, there is no way to let the other side know there was an error sending
    // the message so call CleanUp to release the exchange so the other side can get notified the exchange is closing
    // and clean up as needed. Also the CleanUp API resets the transfer session and destroys the subclass.
    if (err != CHIP_NO_ERROR)
    {
        CleanUp();
    }
    return err;
}

CHIP_ERROR AsyncTransferFacilitator::OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                       System::PacketBufferHandle && payload)
{
    VerifyOrReturnError(mExchange, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(ec == mExchange.Get(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err =
        mTransfer.HandleMessageReceived(payloadHeader, std::move(payload), System::SystemClock().GetMonotonicTimestamp());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "OnMessageReceived: Failed to handle message: %" CHIP_ERROR_FORMAT, err.Format());

        // This should notify the tranfer object to abort transfer so it can send a status report across the exchange
        // when we call HandleNextOutputEvents below.
        mTransfer.AbortTransfer(AsyncResponder::GetBdxStatusCodeFromChipError(err));
    }
    else if (!payloadHeader.HasMessageType(MessageType::BlockAckEOF))
    {

        // Almost every BDX message expect BlockAckEOF will follow up with a response on the exchange.
        ec->WillSendMessage();
    }

    HandleNextOutputEvents();
    return err;
}

void AsyncTransferFacilitator::OnExchangeClosing(Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "OnExchangeClosing, ec: " ChipLogFormatExchange, ChipLogValueExchange(ec));
    CleanUp();
}

void AsyncTransferFacilitator::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "OnResponseTimeout, ec: " ChipLogFormatExchange, ChipLogValueExchange(ec));
    CleanUp();
}

CHIP_ERROR AsyncResponder::PrepareForTransfer(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                                              BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                              System::Clock::Timeout timeout)
{
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mExchange, CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = layer;
    mTimeout     = timeout;

    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, mTimeout));

    mExchange.Grab(exchangeCtx);

    return CHIP_NO_ERROR;
}

void AsyncResponder::NotifyEventHandled(TransferSession::OutputEvent & event, CHIP_ERROR error)
{
    ChipLogDetail(BDX, "NotifyEventHandled : Event %s Error %" CHIP_ERROR_FORMAT, event.ToString(event.EventType), error.Format());

    // If it's a message indicating either the end of the transfer or a timeout reported by the transfer session
    // or an error occured, we need to call CleanUp.
    if (event.EventType == TransferSession::OutputEventType::kAckEOFReceived ||
        event.EventType == TransferSession::OutputEventType::kInternalError ||
        event.EventType == TransferSession::OutputEventType::kTransferTimeout ||
        event.EventType == TransferSession::OutputEventType::kStatusReceived)
    {
        CleanUp();
        return;
    }

    // If there was an error handling the output event, this should notify the tranfer object to abort transfer so it can send a
    // status report
    //  across the exchange when we call HandleNextOutputEvents below.
    if (error != CHIP_NO_ERROR)
    {
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(error));
    }

    HandleNextOutputEvents();
}

} // namespace bdx
} // namespace chip
