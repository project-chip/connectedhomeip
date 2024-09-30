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

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeHolder.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <transport/raw/MessageHeader.h>

#pragma once

namespace chip {
namespace bdx {

/**
 * An abstract class with methods for handling BDX messages received from an ExchangeContext. Once a message is received, this
 * class passes the message to the TransferSession to process the received message and gets the resulting output events from the
 * TransferSession state machine and either sends a message accross the exchange or calls the HandleTransferSessionOutput virtual
 * method to notify the subclass of the event generated. It keeps getting the next output event until it recieves an output event
 * of type TransferSession::OutputEventType::kNone. For events that are handled via the HandleTransferSessionOutput method, the
 * subclass must call the NotifyEventHandled to notify the AsyncTransferFacilitator that the event has been handled and returns an
 * error code for error cases or success.
 *
 * This class does not define any methods for beginning a transfer or initializing the underlying TransferSession object.
 * See AsyncResponder for a class that does.
 * TODO: # 29334 - Add AsyncInitiator to handle the initiating side of a transfer.
 *
 * An AsyncTransferFacilitator is associated with a specific BDX transfer.
 */
class AsyncTransferFacilitator : public Messaging::ExchangeDelegate
{
public:
    AsyncTransferFacilitator() : mExchange(*this) {}
    ~AsyncTransferFacilitator() override;

    /**
     * This method should be implemented to contain business-logic handling of BDX messages
     * and other TransferSession events.
     *
     * @param[in] event An OutputEvent that contains the output from the TransferSession object.
     */
    virtual void HandleTransferSessionOutput(TransferSession::OutputEvent & event) = 0;

    /**
     * This method should be implemented to destroy the object subclassing AsyncTransferFacilitator.
     */
    virtual void DestroySelf() = 0;

protected:
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
    void OnExchangeClosing(Messaging::ExchangeContext * ec) override;

    CHIP_ERROR SendMessage(const TransferSession::MessageTypeData msgTypeData, System::PacketBufferHandle & msgBuf);

    static bdx::StatusCode GetBdxStatusCodeFromChipError(CHIP_ERROR err);

    void CleanUp();

    void HandleNextOutputEvents();

    // The transfer session coresponding to this AsyncTransferFacilitator object.
    TransferSession mTransfer;

    // The Exchange holder that holds the exchange context used for sending and receiving BDX messages.
    Messaging::ExchangeHolder mExchange;

    // The timeout for the BDX transfer session.
    System::Clock::Timeout mTimeout;

    System::Layer * mSystemLayer;

private:
    bool mHandlingOutputEvents;
};

/**
 * An AsyncTransferFacilitator that is initialized to respond to an incoming BDX transfer request.
 *
 * Provides a method for initializing the TransferSession members but still needs to be extended to implement
 * HandleTransferSessionOutput.
 *
 * An instance of some subclass of this class should be used as the exchange delegate for a BDX transfer.
 */
class AsyncResponder : public AsyncTransferFacilitator
{
public:
    /**
     * Initialize the TransferSession state machine to be ready for an incoming transfer request.
     *
     * @param[in] exchangeCtx     The exchange to use for the transfer.
     * @param[in] role            The role of the Responder: Sender or Receiver of BDX data
     * @param[in] xferControlOpts Supported transfer modes (see TransferControlFlags)
     * @param[in] maxBlockSize    The maximum supported size of BDX Block data
     * @param[in] timeout         The chosen timeout delay for the BDX transfer
     */
    CHIP_ERROR PrepareForTransfer(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                                  BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                  System::Clock::Timeout timeout);

    /**
     * This is called by the subclass implementing HandleTransferSessionOutput to notify the AsyncTransferFacilitator
     * that it has handled the OutputEvent specified in "event" and "status" is the result of handling the event.
     * Once this is called the AsyncTransferFacilitator either aborts the transfer if an error has ocurred or drives the
     * TransferSession state machine to generate the next output events to establish and continue the BDX session further.
     *
     *
     * @param[in] event The OutputEvent that was handled by the subclass.
     * @param[in] status The error code that occured when handling the event if an error occurs. Otherwise CHIP_NO_ERROR.
     */
    void NotifyEventHandled(TransferSession::OutputEvent & event, CHIP_ERROR error);
};

} // namespace bdx
} // namespace chip
