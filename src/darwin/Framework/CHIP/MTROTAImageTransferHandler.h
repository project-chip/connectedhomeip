/**
 *
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

#import <Matter/MTROTAProviderDelegate.h>

#include <protocols/bdx/AsyncTransferFacilitator.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * This class inherits from the AsyncResponder class and handles the BDX messages for a BDX transfer session.
 * It overrides the HandleAsyncTransferSessionOutput virtual method and provides an implementation for it.
 *
 * For each BDX transfer, we will have an instance of MTROTAImageTransferHandler.
 */
class MTROTAImageTransferHandler : public chip::bdx::AsyncResponder
{
public:
    MTROTAImageTransferHandler();
    ~MTROTAImageTransferHandler();

    void HandleAsyncTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

private:
    CHIP_ERROR PrepareForTransfer(chip::Messaging::ExchangeContext * exchangeCtx, chip::FabricIndex fabricIndex,
                                  chip::NodeId nodeId);

    void ResetState();

    CHIP_ERROR ConfigureState(chip::FabricIndex fabricIndex, chip::NodeId nodeId);

    static void HandleBdxInitReceivedTimeoutExpired(chip::System::Layer * systemLayer, void * state);

    CHIP_ERROR OnMessageToSend(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnTransferSessionBegin(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnTransferSessionEnd(chip::bdx::TransferSession::OutputEvent & event);

    CHIP_ERROR OnBlockQuery(chip::bdx::TransferSession::OutputEvent & event);

    // Inherited from ExchangeContext
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
                                 chip::System::PacketBufferHandle && payload) override;

    // The fabric index of the peer node.
    chip::Optional<chip::FabricIndex> mFabricIndex;

    // The node id of the peer node.
    chip::Optional<chip::NodeId> mNodeId;

    // The OTA provider delegate used by the controller.
    id<MTROTAProviderDelegate> mDelegate = nil;

    // The OTA provider delegate queue used by the controller.
    dispatch_queue_t mDelegateNotificationQueue = nil;
};

NS_ASSUME_NONNULL_END
