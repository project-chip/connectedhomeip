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

#import <Foundation/Foundation.h>

#include <lib/core/CHIPError.h>
#include <messaging/ExchangeMgr.h>

@class MTRDeviceController;

NS_ASSUME_NONNULL_BEGIN

/**
 * This class creates an unsolicited handler for listening to all unsolicited BDX messages
 * and when it receives a BDX ReceiveInit message from a node, it creates a new
 * MTROTAImageTransferHandler object as a delegate that will prepare for transfer and
 * handle all BDX messages for the BDX transfer session with that node. If it receives an out of order
 * BDX message or if the message is received on a non-valid session, the OnUnsolicitedMessageReceived
 * returns CHIP_ERROR_INCORRECT_STATE.
 *
 */
class MTROTAUnsolicitedBDXMessageHandler : public chip::Messaging::UnsolicitedMessageHandler
{
public:
    MTROTAUnsolicitedBDXMessageHandler() : mExchangeMgr(nullptr) {}
    ~MTROTAUnsolicitedBDXMessageHandler() { mExchangeMgr = nullptr; }

    CHIP_ERROR Init(chip::Messaging::ExchangeManager * exchangeManager);

    // Returns the number of delegates that are currently handling BDX transfers.
    static uint8_t GetNumberOfDelegates();

    // Increase the number of delegates handling BDX transfers by 1.
    static void IncrementNumberOfDelegates();

    // Decrease the number of delegates handling BDX transfers by 1.
    static void DecrementNumberOfDelegates();

   void Shutdown();

   void ControllerShuttingDown(MTRDeviceController * controller);

private:

    CHIP_ERROR OnUnsolicitedMessageReceived(const chip::PayloadHeader & payloadHeader, const chip::SessionHandle & session,
                                            chip::Messaging::ExchangeDelegate * _Nonnull & newDelegate) override;

    void OnExchangeCreationFailed(chip::Messaging::ExchangeDelegate * _Nonnull delegate) override;

protected:
    chip::Messaging::ExchangeManager * mExchangeMgr;

    static inline uint8_t mNumberOfDelegates = 0;
};

NS_ASSUME_NONNULL_END
