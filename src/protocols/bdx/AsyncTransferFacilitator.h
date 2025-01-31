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
 * An abstract class with methods for receiving and sending BDX messages on an ExchangeContext. It interacts with the Transfer
 * Session state machine to process the received messages and send any outgoing messages.
 * Note: If the relevant fabric shuts down, it is the responsibility of the subclass that implements HandleTransferSessionOutput
 * to destroy itself (and hence this object).
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

protected:
    CHIP_ERROR Init(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx, System::Clock::Timeout timeout);

    // If subclasses override this method and they call the superclass's OnMessageReceived, the superclass
    // may destroy the subclass's object before OnMessageReceived returns.
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    // If subclasses override this method and they call the superclass's OnResponseTimeout, the superclass
    // may destroy the subclass's object before OnResponseTimeout returns.
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    /**
     * This method should be implemented to contain business-logic handling of BDX messages
     * and other TransferSession events.
     *
     * @param[in] event An OutputEvent that contains the output from the TransferSession object.
     */
    virtual void HandleTransferSessionOutput(TransferSession::OutputEvent & event) = 0;

    /**
     * This method should be implemented to destroy the object subclassing AsyncTransferFacilitator.
     *
     * This is a hook that is expected to be called by AsyncTransferFacilitator and allows subclasses
     * to select an allocation strategy of their choice.
     */
    virtual void DestroySelf() = 0;

    /**
     * Calling ProcessOutputEvents can destroy this object before the call returns.
     */
    void ProcessOutputEvents();

    // The transfer session corresponding to this AsyncTransferFacilitator object.
    TransferSession mTransfer;

    bool mDestroySelfAfterProcessingEvents = false;

private:
    bool mProcessingOutputEvents = false;

    // The Exchange holder that holds the exchange context used for sending and receiving BDX messages.
    Messaging::ExchangeHolder mExchange;

    // The timeout for the BDX transfer session.
    System::Clock::Timeout mTimeout;

    System::Layer * mSystemLayer;

    CHIP_ERROR SendMessage(const TransferSession::MessageTypeData msgTypeData, System::PacketBufferHandle & msgBuf);
};

/**
 * An AsyncTransferFacilitator that is initialized to respond to an incoming BDX transfer request.
 * An AsyncResponder object is associated with an exchange and handles all BDX messages sent over that exchange.
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
    CHIP_ERROR Init(System::Layer * layer, Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                    BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize, System::Clock::Timeout timeout);

    /**
     * Method that must be called by the subclass implementing HandleTransferSessionOutput to notify the AsyncResponder
     * that it has handled the OutputEvent specified in "event" and "status" is the result of handling the event.
     *
     * Every call to HandleTransferSessionOutput must result in a call to NotifyEventHandled.  The call
     * to NotifyEventHandled may happen before HandleTransferSessionOutput returns, or may happen
     * later, asynchronously.

     * Note: NotifyEventHandled is allowed to destroy the AsyncResponder before the call returns.
     *          Callers must be careful about this.
     *
     * @param[in] eventType The OutputEventType that was handled by the subclass.
     * @param[in] status The error code that occured when handling the event if an error occurs. Otherwise CHIP_NO_ERROR.
     */
    void NotifyEventHandled(const TransferSession::OutputEventType eventType, CHIP_ERROR status);
};

} // namespace bdx
} // namespace chip
