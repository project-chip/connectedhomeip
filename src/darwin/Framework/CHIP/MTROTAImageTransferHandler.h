/**
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

#import <Matter/MTROTAProviderDelegate.h>

#include <protocols/bdx/AsyncTransferFacilitator.h>

NS_ASSUME_NONNULL_BEGIN

@class MTROTAImageTransferHandlerWrapper;

/**
 * This class handles the BDX events for a BDX transfer session.
 *
 * This object may be destroyed directly, when not processing some call it made, to interrupt the BDX
 * transfer.  Specifically, this has to be done if the fabric it's associated with is shut down. This
 * method of interrupting the BDX transfer will not notify the peer that we are interrupting and it will
 * keep waiting until it times out.
 *
 * If not otherwise destroyed, this object will destroy itself when the transfer completes.
 */
class MTROTAImageTransferHandler : public chip::bdx::AsyncResponder {
public:
    MTROTAImageTransferHandler(chip::System::Layer * layer);
    ~MTROTAImageTransferHandler();

    chip::FabricIndex GetPeerFabricIndex() { return mPeer.GetFabricIndex(); }

protected:
    CHIP_ERROR OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PayloadHeader & payloadHeader,
        chip::System::PacketBufferHandle && payload) override;

    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

private:
    CHIP_ERROR Init(chip::Messaging::ExchangeContext * exchangeCtx);

    CHIP_ERROR OnTransferSessionBegin(const chip::bdx::TransferSession::OutputEventType event);

    CHIP_ERROR OnTransferSessionEnd(const chip::bdx::TransferSession::OutputEventType eventType);

    CHIP_ERROR OnBlockQuery(const chip::bdx::TransferSession::OutputEventType eventType, uint64_t bytesToSkip);

    chip::ScopedNodeId GetPeerScopedNodeId(chip::Messaging::ExchangeContext * exchangeCtx);

    void InvokeTransferSessionEndCallback(CHIP_ERROR error);

    void DestroySelf() override;

    // The scoped node id of the node with which the BDX session is established.
    chip::ScopedNodeId mPeer;

    // The OTA provider delegate that is responding to the BDX transfer request.
    id<MTROTAProviderDelegate> mDelegate = nil;
    chip::System::Layer * mSystemLayer = nil;

    // The queue mDelegate must be called on.
    dispatch_queue_t mDelegateNotificationQueue = nil;

    MTROTAImageTransferHandlerWrapper * mOTAImageTransferHandlerWrapper;

    bool mNeedToCallTransferSessionEnd = false;

    bool mIsPeerNodeAKnownThreadDevice = NO;

    chip::System::Clock::Milliseconds32 mBDXThrottleIntervalForThreadDevices;
};

NS_ASSUME_NONNULL_END
