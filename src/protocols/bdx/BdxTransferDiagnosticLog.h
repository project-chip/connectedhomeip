/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <lib/core/DataModelTypes.h>
#include <protocols/bdx/BdxTransferProxyDiagnosticLog.h>
#include <protocols/bdx/BdxTransferServerDelegate.h>
#include <protocols/bdx/TransferFacilitator.h>
#include <system/SystemLayer.h>

namespace chip {
namespace bdx {

class BdxTransferDiagnosticLogPoolDelegate;

class BdxTransferDiagnosticLog : public Responder
{
public:
    BdxTransferDiagnosticLog(BDXTransferServerDelegate * delegate, BdxTransferDiagnosticLogPoolDelegate * poolDelegate,
                             System::Layer * systemLayer) :
        mSystemLayer(systemLayer),
        mDelegate(delegate), mPoolDelegate(poolDelegate){};

    ~BdxTransferDiagnosticLog() { Reset(); };

    /**
     * This method handles BDX messages and other TransferSession events.
     *
     * @param[in] event An OutputEvent that contains output from the TransferSession object.
     */
    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override;

    void OnExchangeClosing(Messaging::ExchangeContext * ec) override;

    /**
     * Lifetime management, to allow us to abort transfers when a fabric
     * identity is being shut down.
     */
    bool IsForFabric(FabricIndex fabricIndex) const;
    void AbortTransfer();

protected:
    /**
     * Called when a BDX message is received over the exchange context
     *
     * @param[in] ec The exchange context
     *
     * @param[in] payloadHeader The payload header of the message
     *
     * @param[in] payload The payload of the message
     */
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

private:
    /**
     * Called to send a BDX MsgToSend message over the exchange
     *
     *
     * @param[in] event The output event to be send
     */
    CHIP_ERROR OnMessageToSend(TransferSession::OutputEvent & event);

    /**
     * Called to begin the transfer session when an init message has been received
     *
     * @param[in] event The output event received
     */
    CHIP_ERROR OnTransferSessionBegin(TransferSession::OutputEvent & event);

    /**
     * Called to end the transfer session when a BlockAckEOF message has been sent over the exchange
     * or an error has occurred during the BDX session
     *
     * @param[in] error The error type
     */
    CHIP_ERROR OnTransferSessionEnd(CHIP_ERROR error);

    /**
     * Called when a block has been received from the Sender. The block is processed
     * and written to a file and a block ack is sent back to the sender.
     *
     * @param[in] event The output event received
     */
    CHIP_ERROR OnBlockReceived(TransferSession::OutputEvent & event);

    /**
     * This method is called to reset state. It resets the transfer and cleans up the
     * exchange and the fabric index and peer node id.
     */
    void Reset();

    void AbortTransferOnFailure(CHIP_ERROR error);

    BDXTransferProxyDiagnosticLog mTransferProxy;
    bool mIsExchangeClosing = false;

    System::Layer * mSystemLayer;

    BDXTransferServerDelegate * mDelegate;
    BdxTransferDiagnosticLogPoolDelegate * mPoolDelegate;
};

} // namespace bdx
} // namespace chip
