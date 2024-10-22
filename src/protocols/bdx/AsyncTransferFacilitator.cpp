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

CHIP_ERROR AsyncTransferFacilitator::Init(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx,
                                          System::Clock::Timeout timeout)
{
    VerifyOrReturnError(layer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!mExchange, CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = layer;
    mExchange.Grab(exchangeCtx);
    mTimeout = timeout;
    mProcessingOutputEvents = false;
    mDestroySelf = false;
    return CHIP_NO_ERROR;
}

/**
 * Calls the GetNextAction on the TransferSession to get the next output events until it receives
 * TransferSession::OutputEventType::kNone If the output event is of type TransferSession::OutputEventType::kMsgToSend, it sends the
 * message over the exchange context, otherwise it calls the HandleTransferSessionOutput method implemented by the subclass to
 * handle the BDX message.
 */
void AsyncTransferFacilitator::ProcessOutputEvents()
{
    if (mProcessingOutputEvents)
    {
        ChipLogDetail(BDX, "ProcessOutputEvents: Still getting and processing output events from a previous call. Return.");
        return;
    }

    mProcessingOutputEvents = true;

    // Get the next output event and handle it based on the type of event.
    // If its of type kMsgToSend send it over the exchange, otherwise call the HandleTransferSessionOutput
    // virtual method that must be implemeted by the subclass of this class to handle the BDX message.
    TransferSession::OutputEvent outEvent;

    mTransfer.GetNextAction(outEvent);
    while (outEvent.EventType != TransferSession::OutputEventType::kNone)
    {
        if (outEvent.EventType == TransferSession::OutputEventType::kMsgToSend)
        {
            CHIP_ERROR err = SendMessage(outEvent.msgTypeData, outEvent.MsgData);

            // If we failed to send the message across the exchange, there is no way to let the other side know there was an
            // error so call DestroySelf so the exchange can be released and the other side will be notified the exchange is
            // closing and will clean up.
            if (err != CHIP_NO_ERROR)
            {
                DestroySelf();
                return;
            }

            // If we send out a status report across the exchange, schedule a call to DestroySelf() at a little bit later time
            // since we want the message to be sent before we clean up.
            if (outEvent.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
            {
                DestroySelf();
                return;
            }
        }
        else
        {
            HandleTransferSessionOutput(outEvent);
        }
        mTransfer.GetNextAction(outEvent);
    }

    mProcessingOutputEvents = false;

    // If the mDestroySelf is set in NotifyEventHandled, we need to call DestroySelf() after processing all pending output events.
    if (mDestroySelf)
    {
        DestroySelf();
    }
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
    return ec->SendMessage(msgTypeData.ProtocolId, msgTypeData.MessageType, std::move(msgBuf), sendFlags);
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
        // when we call ProcessOutputEvents below.
        mTransfer.AbortTransfer(AsyncResponder::GetBdxStatusCodeFromChipError(err));
    }
    else if (!payloadHeader.HasMessageType(MessageType::BlockAckEOF))
    {

        // Almost every BDX message expect BlockAckEOF will follow up with a response on the exchange.
        ec->WillSendMessage();
    }

    ProcessOutputEvents();
    return err;
}

void AsyncTransferFacilitator::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogDetail(BDX, "OnResponseTimeout, ec: " ChipLogFormatExchange, ChipLogValueExchange(ec));
    DestroySelf();
}

CHIP_ERROR AsyncResponder::Init(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                                BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                System::Clock::Timeout timeout)
{
    AsyncTransferFacilitator::Init(layer, exchangeCtx, timeout);
    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, timeout));
    return CHIP_NO_ERROR;
}

void AsyncResponder::NotifyEventHandled(const TransferSession::OutputEvent & event, CHIP_ERROR status)
{
    ChipLogDetail(BDX, "NotifyEventHandled : Event %s Error %" CHIP_ERROR_FORMAT, TransferSession::OutputEvent::TypeToString(event.EventType), status.Format());

    // If it's a message indicating either the end of the transfer or a timeout reported by the transfer session
    // or an error occured, we need to call DestroySelf().
    if (event.EventType == TransferSession::OutputEventType::kAckEOFReceived ||
        event.EventType == TransferSession::OutputEventType::kInternalError ||
        event.EventType == TransferSession::OutputEventType::kTransferTimeout ||
        event.EventType == TransferSession::OutputEventType::kStatusReceived)
    {

        // Set the mDestroySelf flag to true so that when ProcessOutputEvents finishes processing all pending events, it
        // will call DestroySelf() to clean up.
        mDestroySelf = true;
    }

    // If there was an error handling the output event, this should notify the tranfer object to abort transfer so it can send a
    // status report across the exchange when we call ProcessOutputEvents below.
    if (status != CHIP_NO_ERROR)
    {
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(status));
    }

    ProcessOutputEvents();
}

} // namespace bdx
} // namespace chip
