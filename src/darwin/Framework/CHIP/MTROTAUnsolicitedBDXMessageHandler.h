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

#import "MTROTAImageTransferHandler.h"

#include <messaging/ExchangeMgr.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * This class creates a handler for listening to all unsolicited BDX messages and when a BDX ReceiveInit
 * message is received from a node, it creates a new MTROTAImageTransferHandler object as a delegate
 * that handles the OTA image file transfer for that node.
 *
 * Each MTROTAImageTransferHandler instance will handle one BDX transfer session.
 */
class MTROTAUnsolicitedBDXMessageHandler : public chip::Messaging::UnsolicitedMessageHandler
{
public:
    MTROTAUnsolicitedBDXMessageHandler() : mExchangeMgr(nullptr) {}
    ~MTROTAUnsolicitedBDXMessageHandler();

    CHIP_ERROR Init(chip::Messaging::ExchangeManager * exchangeManager);

    // Returns the number of delegates that are currently handling BDX transfers
    static uint8_t GetNumberOfDelegates();

    // Increase the number of delegates by 1.
    static void IncrementNumberOfDelegates();

    // Decrease the number of delegates by 1.
    static void DecrementNumberOfDelegates();

private:
    CHIP_ERROR OnUnsolicitedMessageReceived(const chip::PayloadHeader & payloadHeader,
                                            chip::Messaging::ExchangeDelegate * _Nonnull & newDelegate) override;

protected:
    chip::Messaging::ExchangeManager * mExchangeMgr;

    MTROTAImageTransferHandler * otaImageTransferHandler;

    static inline uint8_t mNumberOfDelegates = 0;
};

NS_ASSUME_NONNULL_END
