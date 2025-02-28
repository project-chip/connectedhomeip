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

#include <protocols/bdx/StatusCode.h>
#include <system/SystemClock.h>

namespace chip {
namespace bdx {

AsyncTransferFacilitator::~AsyncTransferFacilitator() {}

CHIP_ERROR AsyncTransferFacilitator::Init(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx,
                                          System::Clock::Timeout timeout)
{
    VerifyOrReturnError(layer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!mExchange, CHIP_ERROR_INCORRECT_STATE);

    mSystemLayer = layer;
    mExchange.Grab(exchangeCtx);
    mTimeout                          = timeout;
    mProcessingOutputEvents           = false;
    mDestroySelfAfterProcessingEvents = false;
    return CHIP_NO_ERROR;
}

/**
 * Get events one by one from the TransferSession and process them,
 * until there are no more events to process.
 */
void AsyncTransferFacilitator::ProcessOutputEvents()
{
    if (mProcessingOutputEvents)
    {
        ChipLogDetail(BDX,
                      "ProcessOutputEvents: we are already in the middle of processing events, so nothing to do here; when we "
                      "unwind to the processing loop the events will get processed.");
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

        // If the transfer session state machine generates an event of type TransferSession::OutputEventType::kInternalError,
        // indicating that the session is in a bad state, it will keep doing that thereafter.
        //
        // So stop trying to process events, and go ahead and destroy ourselves to clean up the transfer.
        if (outEvent.EventType == TransferSession::OutputEventType::kInternalError)
        {
            mDestroySelfAfterProcessingEvents = true;
            break;
        }

        if (outEvent.EventType == TransferSession::OutputEventType::kMsgToSend)
        {
            CHIP_ERROR err = SendMessage(outEvent.msgTypeData, outEvent.MsgData);

            // If we failed to send the message across the exchange, just abort the transfer.
            // We have no way to notify our peer we are doing that (we can't send them a
            // message!) but eventually they will time out.
            if (err != CHIP_NO_ERROR)
            {
                DestroySelf();
                return;
            }

            // If we send out a status report across the exchange, that means there was an error.
            // We've sent our report about that error and can now abort the transfer.  Our peer
            // will respond to the status report by tearing down their side.
            if (outEvent.msgTypeData.HasMessageType(Protocols::SecureChannel::MsgType::StatusReport))
            {
                mDestroySelfAfterProcessingEvents = true;
                break;
            }
        }
        else
        {
            HandleTransferSessionOutput(outEvent);
        }
        mTransfer.GetNextAction(outEvent);
    }

    mProcessingOutputEvents = false;

    // If mDestroySelfAfterProcessingEvents is set (by our code above or by NotifyEventHandled), we need
    // to call DestroySelf() after processing all pending output events.
    if (mDestroySelfAfterProcessingEvents)
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

        // This should notify the transfer object to abort transfer so it can send a status report across the exchange
        // when we call ProcessOutputEvents below.
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(err));
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
    ReturnErrorOnFailure(AsyncTransferFacilitator::Init(layer, exchangeCtx, timeout));
    ReturnErrorOnFailure(mTransfer.WaitForTransfer(role, xferControlOpts, maxBlockSize, timeout));
    return CHIP_NO_ERROR;
}

void AsyncResponder::NotifyEventHandled(const TransferSession::OutputEventType eventType, CHIP_ERROR status)
{
    // If this is the end of the transfer (whether a clean end, or some sort of error condition), ensure
    // that we destroy ourselves after unwinding the processing loop in the ProcessOutputEvents API.
    // We can ignore the status for these output events because none of them are supposed to result in
    // us sending a StatusReport, and that's all we use the status for.
    //
    // In particular, for kTransferTimeout, kAckEOFReceived, and kStatusReceived per spec we
    // are not supposed to reply with a StatusReport.  And for kInternalError the state machine
    // is in an unrecoverable state of some sort, and we should stop trying to make use of it.
    if (eventType == TransferSession::OutputEventType::kAckEOFReceived ||
        eventType == TransferSession::OutputEventType::kInternalError ||
        eventType == TransferSession::OutputEventType::kTransferTimeout ||
        eventType == TransferSession::OutputEventType::kStatusReceived)
    {
        ChipLogProgress(BDX, "NotifyEventHandled : Event %s Error %" CHIP_ERROR_FORMAT,
                        TransferSession::OutputEvent::TypeToString(eventType), status.Format());
        mDestroySelfAfterProcessingEvents = true;
    }
    else if (status != CHIP_NO_ERROR)
    {
        // If there was an error handling the output event, this should notify the transfer object to abort transfer
        // so it can send a status report across the exchange when we call ProcessOutputEvents below.
        mTransfer.AbortTransfer(GetBdxStatusCodeFromChipError(status));
    }

    ProcessOutputEvents();
}

} // namespace bdx
} // namespace chip
