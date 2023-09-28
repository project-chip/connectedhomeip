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

AsyncTransferFacilitator::~AsyncTransferFacilitator()
{
    Reset();
}

void AsyncTransferFacilitator::Reset()
{
    mExchange.Release();
    mTransfer.Reset();
}

CHIP_ERROR AsyncTransferFacilitator::SendMessage(TransferSession::OutputEvent & event)
{

    VerifyOrReturnError(mExchange, CHIP_ERROR_INCORRECT_STATE);

    Messaging::SendFlags sendFlags;

    // All messages sent from the Sender expect a response, except for a StatusReport which would indicate an error and
    // the end of the transfer.
    if (!event.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
    {
        sendFlags.Set(Messaging::SendMessageFlags::kExpectResponse);
    }

    auto & msgTypeData = event.msgTypeData;

    Messaging::ExchangeContext * ec = mExchange.Get();
    VerifyOrReturnError(ec != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Set the response timeout on the exchange before sending the message.
    ec->SetResponseTimeout(mTimeout);

    CHIP_ERROR err = ec->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(event.MsgData), sendFlags);

    // If there's an error sending the message, release the exchange.
    if (err != CHIP_NO_ERROR)
    {
        mExchange.Release();
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
        ChipLogError(BDX, "failed to handle message: %" CHIP_ERROR_FORMAT, err.Format());
        mTransfer.AbortTransfer(AsyncResponder::GetBdxStatusCodeFromChipError(err));
    }
    else if (!payloadHeader.HasMessageType(MessageType::BlockAckEOF))
    {

        // Almost every BDX message expect BlockAckEOF will follow up with a response on the exchange.
        ec->WillSendMessage();
    }

    // Get the next output event and handle it based on the type of event.
    TransferSession::OutputEvent outEvent;

    mTransfer.GetNextAction(outEvent);
    while (outEvent.EventType != TransferSession::OutputEventType::kNone)
    {

        // If its a message of type kMsgToSend, send the message over the exchange.
        if (outEvent.EventType == TransferSession::OutputEventType::kMsgToSend)
        {
            SendMessage(outEvent);
        }
        else
        {
            // Call the HandleTransferSessionOutput virtual method that's implemeted by the subclass to handle
            // the output events.
            HandleTransferSessionOutput(outEvent);

            // If it's a message indicating either the end of the transfer or a timeout reported by the transfer session
            // or an error occured, we need to call the CleanUp virtual method implemented by the subclass to clean up and
            // delete both the sublcass and base class objects.
            if (outEvent.EventType == TransferSession::OutputEventType::kAckEOFReceived ||
                outEvent.EventType == TransferSession::OutputEventType::kInternalError ||
                outEvent.EventType == TransferSession::OutputEventType::kTransferTimeout ||
                outEvent.EventType == TransferSession::OutputEventType::kStatusReceived)
            {
                CleanUp();
                return err;
            }
        }
        mTransfer.GetNextAction(outEvent);
    };
    return err;
}

void AsyncTransferFacilitator::OnExchangeClosing(Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "OnExchangeClosing, ec: " ChipLogFormatExchange, ChipLogValueExchange(ec));
    mExchange.Release();
}

void AsyncTransferFacilitator::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "OnResponseTimeout, ec: " ChipLogFormatExchange, ChipLogValueExchange(ec));
    CleanUp();
}

CHIP_ERROR AsyncResponder::PrepareForTransfer(Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                                              BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                              System::Clock::Timeout timeout)
{
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mExchange, CHIP_ERROR_INCORRECT_STATE);

    mTimeout = timeout;

    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, mTimeout));

    mExchange.Grab(exchangeCtx);

    return CHIP_NO_ERROR;
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

void AsyncResponder::NotifyEventHandled(CHIP_ERROR eventHandlingResult)
{
    // We can only come here after we have handled a BDX message (like ReceiveInit/BlockQuery) asynchronously
    // in the subclass. The base class needs to notify us when the handling of the event is done and the event
    // handling result - success or failure.
    ChipLogDetail(BDX, "NotifyEventHandled : error %" CHIP_ERROR_FORMAT, eventHandlingResult.Format());

    // If there was an error, we abort the transfer.
    if (eventHandlingResult != CHIP_NO_ERROR)
    {
        CleanUp();
    }
    else
    {

        // We need to get the next output event from the state machine which is a response to the
        // BDX message handled by the subclass (ReceiveAccept/Block).
        TransferSession::OutputEvent outEvent;

        mTransfer.GetNextAction(outEvent);
        while (outEvent.EventType != TransferSession::OutputEventType::kNone)
        {
            if (outEvent.EventType == TransferSession::OutputEventType::kMsgToSend)
            {
                AsyncTransferFacilitator::SendMessage(outEvent);
                mTransfer.GetNextAction(outEvent);
            }
            else
            {
                break;
            }
        };
    }
}

} // namespace bdx
} // namespace chip
