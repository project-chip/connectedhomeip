/*
 *
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

// TODO: Includes.
#include <lib/support/Span.h>
#include <messaging/ExchangeContext.h>

#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#pragma once

namespace chip {
namespace bdx {

// A class that represents a BDX transfer initiated by the other end of the transfer. This implements most of the transfer,
// but uses a delegate to determine whether the transfer should be accepted and provide the data source or sink.
class BdxTransfer : public Responder
{
public:
    // The delegate is informed when specific events occur during the transfer.
    class Delegate
    {
    public:
        // Called when the SendInit or ReceiveInit message is received.
        virtual void InitMessageReceived(BdxTransfer * transfer, TransferSession::TransferInitData init_data) = 0;
        // Called when the transfer completes. The outcome of the transfer (successful or otherwise) is indicated by result.
        virtual void TransferCompleted(BdxTransfer * transfer, CHIP_ERROR result) = 0;
    };

    // The callback used when a data block arrives. This is only used when the transfer is sending data to this controller.
    using DataCallback = void(*)(const ByteSpan&);

    // Accepts the transfer. When a block of data arrives the callback is invoked with the block. This must only be called if the
    // transfer sends data to this controller.
    CHIP_ERROR AcceptSend(DataCallback callback);

    // Accepts the transfer. The data provided here will be sent over the transfer. This must only be called if the transfer
    // receives data from this controller.
    // TODO: Should the parameter be const&?
    CHIP_ERROR AcceptReceive(const ByteSpan data_to_send);

    // Rejects the transfer.
    CHIP_ERROR Reject();

    void SetDelegate(Delegate * delegate);

    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override;
    void OnExchangeClosing(Messaging::ExchangeContext * exchangeContext) override;

private:
    void EndSession(CHIP_ERROR result);
    CHIP_ERROR SendMessage(TransferSession::OutputEvent & event);
    CHIP_ERROR SendBlock();

    Delegate * mDelegate = nullptr;
    bool mAwaitingAccept = false;
    DataCallback mDataCallback = nullptr;
    TransferSession::TransferInitData mInitData;

    // TODO: Request the data from a data source rather than copy the data here.
    uint8_t * mData = nullptr;
    size_t mDataCount = 0;
    size_t mDataTransferredCount = 0;
};

}  // namespace bdx
}  // namespace chip
