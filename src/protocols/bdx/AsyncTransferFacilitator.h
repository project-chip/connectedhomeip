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

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeHolder.h>
#include <protocols/bdx/BdxTransferSession.h>

#pragma once

namespace chip {
namespace bdx {

/**
 * An abstract class with methods for handling BDX messages from an ExchangeContext and using an event driven
 * async approach to get the next action from the transfer session state machine.
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
    ~AsyncTransferFacilitator() override = default;

    /**
     * This method should be implemented to contain business-logic handling of BDX messages and other TransferSession events.
     *
     * @param[in] event An OutputEvent that contains output from the TransferSession object.
     */
    virtual void HandleTransferSessionOutput(TransferSession::OutputEvent & event) = 0;

    /**
     * This method returns the exchange on which the BDX transfer is happening.
     *
     * May return null if there is no such exchange (e.g. it has been closed due to the transfer completing
     * or some error condition).
     */
    Messaging::ExchangeContext * GetExchangeContext();

    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;
    void OnExchangeClosing(Messaging::ExchangeContext * ec) override;

protected:
    // The transfer session coresponding to this AsynTransferFacilitator object.
    TransferSession mTransfer;

    // The Exchange holder that holds the exchange context used for the BDX messages.
    Messaging::ExchangeHolder mExchange;
};

/**
 * An AsyncTransferFacilitator that is initialized to respond to an incoming BDX transfer request.
 *
 * Provides a method for initializing the TransferSession member but still needs to be extended to implement
 * HandleAsyncTransferSessionOutput. 
 *
 * This class, or a subclass of it, should be used as the exchange delegate for a BDX transfer.
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
    CHIP_ERROR PrepareForTransfer(Messaging::ExchangeContext * exchangeCtx, TransferRole role,
                                  BitFlags<TransferControlFlags> xferControlOpts, uint16_t maxBlockSize,
                                  System::Clock::Timeout timeout);

    void ResetTransfer();

    /**
     * Notifies the transfer facilitator that an output event has been handled by the delegate and passes any error(s) that occured
     * while handling the event. This is needed as the delegate calls async callbacks for handling the BDX messages and we need to
     * get the next action from the state machine based on how the delegate handled the message.
     *
     * @param[in] eventHandlingResult  The result of handling the event.  CHIP_NO_ERROR if
     *                                 it was handled successfully, otherwise the error that
     *                                 was encountered.
     */
    void NotifyEventHandled(CHIP_ERROR eventHandlingResult);

private:
    bdx::StatusCode GetBdxStatusCodeFromChipError(CHIP_ERROR err);
};
} // namespace bdx
} // namespace chip
