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

/**
 * @file BdxEndpoint.h
 *
 *  This file defines interfaces for connecting the BDX state machine (TransferSession) to the messaging layer.
 */

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <system/SystemLayer.h>

#pragma once

namespace chip {
namespace bdx {

/**
 * An abstract class with methods for handling BDX messages from an ExchangeContext and gets output events from a TransferSession state machine.
 *
 * This class does not define any methods for beginning a transfer or initializing the underlying TransferSession object (see
 * Initiator and Responder below).
 * A CHIP node may have many TransferFacilitator instances but only one TransferFacilitator should be used for each BDX transfer.
 */
class TransferFacilitator : public Messaging::ExchangeDelegate, public Messaging::UnsolicitedMessageHandler
{
public:
    TransferFacilitator() : mExchangeCtx(nullptr) {}
    ~TransferFacilitator() override = default;
  
    static void OnOutputEventReceived(void * context, TransferSession::OutputEvent & event);

private:
    //// UnsolicitedMessageHandler Implementation ////
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override
    {
        // TODO: Implement a bdx manager, which dispatch bdx messages to bdx transections.
        // directly.
        newDelegate = this;
        return CHIP_NO_ERROR;
    }

    // Inherited from ExchangeContext
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                 chip::System::PacketBufferHandle && payload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    /**
     * This method should be implemented to contain business-logic handling of BDX messages and other TransferSession events.
     *
     * NOTE: It is the responsiblity of the implementer to Close the underlying ExchangeContext when it has determined that the
     * transfer is finished. This class assumes that a response message will be sent for all received messages.
     *
     * @param[in] event An OutputEvent that contains output from the TransferSession object.
     */
    virtual void HandleTransferSessionOutput(TransferSession::OutputEvent & event) = 0;

protected:
    TransferSession mTransfer;
    Messaging::ExchangeContext * mExchangeCtx;
    System::Layer * mSystemLayer;
};

/**
 * A TransferFacilitator that is initialized to respond to an incoming BDX transfer request.
 *
 * Provides a method for initializing the TransferSession member but still needs to be extended to implement
 * HandleTransferSessionOutput. It is intended that this class will be used as a delegate for handling an unsolicited BDX message.
 */
class Responder : public TransferFacilitator
{
public:
    /**
     * Initialize the TransferSession state machine to be ready for an incoming transfer request and set the callback to get the output
     * event
     * @param layer               A System::Layer pointer to pass to the session to start the bdx idle timeout timer
     * @param[in] role            The role of the Responder: Sender or Receiver of BDX data
     * @param[in] xferControlOpts Supported transfer modes (see TransferControlFlags)
     * @param[in] maxBlockSize    The supported maximum size of BDX Block data
     */
    CHIP_ERROR PrepareForTransfer(System::Layer * layer, TransferRole role, BitFlags<TransferControlFlags> xferControlOpts,
                                  uint16_t maxBlockSize);

    void ResetTransfer();
};

/**
 * A TransferFacilitator that initiates a BDX transfer.
 *
 * Provides a method for initializing the TransferSession member (thus beginning the transfer) but still needs to be extended to
 * implement HandleTransferSessionOutput.
 */
class Initiator : public TransferFacilitator
{
public:
    /**
     * Initialize the TransferSession state machine to prepare a transfer request message and sends the outgoing message
     *
     * @param[in] role       The role of the Initiator: Sender or Receiver of BDX data
     * @param[in] initData   Data needed for preparing a transfer request BDX message
     */
    CHIP_ERROR InitiateTransfer(TransferRole role, const TransferSession::TransferInitData & initData);
};

} // namespace bdx
} // namespace chip
