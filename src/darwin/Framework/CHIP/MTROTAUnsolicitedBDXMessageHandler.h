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

#import "MTRDeviceController_Concrete.h"
#import "MTROTAImageTransferHandler.h"
#import <Foundation/Foundation.h>

#include <lib/core/CHIPError.h>
#include <messaging/ExchangeMgr.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * This class defines a handler that can listen for all unsolicited BDX messages.
 *
 * When it receives a BDX ReceiveInit message from a node, it creates a new
 * MTROTAImageTransferHandler object as the exchange delegate that will
 * handle the transfer.
 *
 * In all other cases, it fails out so that there is no exchange delegate created,
 * which means the message is potentially acked, but does not get a response.
 */
class MTROTAUnsolicitedBDXMessageHandler : public chip::Messaging::UnsolicitedMessageHandler
{
public:
    MTROTAUnsolicitedBDXMessageHandler() : mExchangeMgr(nullptr) { sInstance = this; }

    ~MTROTAUnsolicitedBDXMessageHandler() {}

    static MTROTAUnsolicitedBDXMessageHandler * GetInstance();

    CHIP_ERROR Init(chip::System::Layer * systemLayer, chip::Messaging::ExchangeManager * exchangeManager);

    bool IsInAnOngoingTransfer();

    void OnTransferHandlerCreated(MTROTAImageTransferHandler * imageTransferHandler);

    void OnTransferHandlerDestroyed(MTROTAImageTransferHandler * imageTransferHandler);

    void Shutdown();

    void ControllerShuttingDown(MTRDeviceController_Concrete * controller);

private:
    CHIP_ERROR OnUnsolicitedMessageReceived(const chip::PayloadHeader & payloadHeader, const chip::SessionHandle & session,
                                            chip::Messaging::ExchangeDelegate * _Nonnull & newDelegate) override;

    void OnExchangeCreationFailed(chip::Messaging::ExchangeDelegate * _Nonnull delegate) override;

    // TODO: #36181 - Have a set of MTROTAImageTransferHandler objects.
    MTROTAImageTransferHandler * _Nullable mOTAImageTransferHandler = nullptr;

    chip::System::Layer * mSystemLayer;

    chip::Messaging::ExchangeManager * mExchangeMgr;

    static MTROTAUnsolicitedBDXMessageHandler * sInstance;
};

NS_ASSUME_NONNULL_END
