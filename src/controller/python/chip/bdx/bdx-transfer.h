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

#include <vector>

#include <lib/support/Span.h>
#include <messaging/ExchangeContext.h>

#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/TransferFacilitator.h>

#pragma once

namespace chip {
namespace bdx {

// A class that represents a BDX transfer initiated by the other end of the transfer. This implements most of the transfer,
// but uses a delegate for transfer control and data handling. The delegate must call one of ReceiveData, SendData, or Reject
// after or during a call to InitMessageReceived.
class BdxTransfer : public Responder
{
public:
    // The delegate is informed when specific events occur during the transfer.
    class Delegate
    {
    public:
        virtual ~Delegate() = default;

        // Called when the SendInit or ReceiveInit message is received.
        virtual void InitMessageReceived(BdxTransfer * transfer, TransferSession::TransferInitData init_data) = 0;
        // Called when a data block arrives. This is only used when the transfer is sending data to this controller.
        virtual void DataReceived(BdxTransfer * transfer, const ByteSpan & block) = 0;
        // Called when the transfer completes. The outcome of the transfer (successful or otherwise) is indicated by result.
        virtual void TransferCompleted(BdxTransfer * transfer, CHIP_ERROR result) = 0;
    };

    BdxTransfer(System::Layer * systemLayer) : mSystemLayer(systemLayer) {}

    ~BdxTransfer() override = default;

    // Accepts the transfer with the intent of receiving data. This will send an AcceptSend message to the other end of the
    // transfer. When a block of data arrives the delegate is invoked with the block.
    CHIP_ERROR AcceptAndReceiveData();

    // Accepts the transfer with the intent of sending data. This will send an AcceptReceive message to the other end of the
    // transfer.
    CHIP_ERROR AcceptAndSendData(const ByteSpan & data_to_send);

    // Rejects the transfer.
    CHIP_ERROR Reject();

    void SetDelegate(Delegate * delegate);

    // Responder virtual method overrides.
    void HandleTransferSessionOutput(TransferSession::OutputEvent & event) override;
    void OnExchangeClosing(Messaging::ExchangeContext * exchangeContext) override;
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * exchangeContext, const chip::PayloadHeader & payloadHeader,
                                 chip::System::PacketBufferHandle && payload) override;

private:
    void EndSession(CHIP_ERROR result);
    CHIP_ERROR SendMessage(TransferSession::OutputEvent & event);
    CHIP_ERROR SendBlock();

    Delegate * mDelegate = nullptr;
    bool mAwaitingAccept = false;

    System::Layer * mSystemLayer = nullptr;

    std::vector<uint8_t> mData;
    size_t mDataCount            = 0;
    size_t mDataTransferredCount = 0;
};

} // namespace bdx
} // namespace chip
