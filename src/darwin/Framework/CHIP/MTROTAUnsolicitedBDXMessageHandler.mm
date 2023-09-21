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

#import "MTROTAUnsolicitedBDXMessageHandler.h"
#import "MTROTAImageTransferHandler.h"

#include <protocols/bdx/BdxMessages.h>

using namespace chip;
using namespace chip::Messaging;
using namespace chip::bdx;

MTROTAUnsolicitedBDXMessageHandler::~MTROTAUnsolicitedBDXMessageHandler()
{
    assertChipStackLockedByCurrentThread();
    if (mExchangeMgr) {
        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
        mExchangeMgr = nullptr;
    }
    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates = 0;
}

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::Init(ExchangeManager * exchangeManager)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates = 0;

    mExchangeMgr = exchangeManager;
    return mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
}

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::OnUnsolicitedMessageReceived(
    const PayloadHeader & payloadHeader, ExchangeDelegate * _Nonnull & newDelegate)
{
    assertChipStackLockedByCurrentThread();
    ChipLogDetail(BDX, "%s: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId, __FUNCTION__,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // If we receive a ReceiveInit BDX message, create a new MTROTAImageTransferHandler and register it
    // as the handler for all BDX messages for the OTA file transfer that will come over this exchange.
    if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
        otaImageTransferHandler = new MTROTAImageTransferHandler();
        newDelegate = otaImageTransferHandler;
    }
    return CHIP_NO_ERROR;
}

uint8_t MTROTAUnsolicitedBDXMessageHandler::GetNumberOfDelegates()
{
    return MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates;
}

void MTROTAUnsolicitedBDXMessageHandler::IncrementNumberOfDelegates() { MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates++; }

void MTROTAUnsolicitedBDXMessageHandler::DecrementNumberOfDelegates() { MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates--; }
