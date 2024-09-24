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

#import "MTROTAUnsolicitedBDXMessageHandler.h"
#import "MTROTAImageTransferHandler.h"

#include <protocols/bdx/BdxMessages.h>

using namespace chip;
using namespace chip::Messaging;
using namespace chip::bdx;

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::Init(ExchangeManager * exchangeManager)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates = 0;
    mExchangeMgr = exchangeManager;
    return mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
}

void MTROTAUnsolicitedBDXMessageHandler::Shutdown()
{
    assertChipStackLockedByCurrentThread();

    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates = 0;
    VerifyOrReturn(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
}

void MTROTAUnsolicitedBDXMessageHandler::ControllerShuttingDown(MTRDeviceController * controller)
{
    assertChipStackLockedByCurrentThread();

    // Since the OTA provider delegate only supports one BDX transfer at a time, calling ShutDown is fine for now.
    // TODO: This class needs to keep a list of all MTROTAImageTransferHandlers mapped to the fabric index and only
    // delete the MTROTAImageTransferHandler with a fabric index matching the MTRDeviceController's fabric index.
    Shutdown();
}

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, const SessionHandle & session,
    ExchangeDelegate *  _Nonnull & newDelegate)
{
    assertChipStackLockedByCurrentThread();

    ChipLogDetail(BDX, "MTROTAUnsolicitedBDXMessageHandler: OnUnsolicitedMessageReceived: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (GetNumberOfDelegates() >= 1)
    {
        return CHIP_ERROR_BUSY;
    }

    // Only proceed if there is a valid fabric index for the SessionHandle.
    if (session->IsSecureSession() && session->AsSecureSession() != nullptr && session->AsSecureSession()->GetFabricIndex() != kUndefinedFabricIndex)
    {
        // If we receive a ReceiveInit BDX message, create a new MTROTAImageTransferHandler and register it
        // as the handler for all BDX messages that will come over this exchange and increment the number of delegates.
        if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
            MTROTAImageTransferHandler * otaImageTransferHandler = new MTROTAImageTransferHandler();
            newDelegate = otaImageTransferHandler;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

void MTROTAUnsolicitedBDXMessageHandler::OnExchangeCreationFailed(ExchangeDelegate * delegate)
{
    auto * otaTransferHandler = static_cast<MTROTAImageTransferHandler *>(delegate);
    otaTransferHandler->DestroySelf();
}

uint8_t MTROTAUnsolicitedBDXMessageHandler::GetNumberOfDelegates()
{
    return MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates;
}

void MTROTAUnsolicitedBDXMessageHandler::IncrementNumberOfDelegates() { MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates++; }

void MTROTAUnsolicitedBDXMessageHandler::DecrementNumberOfDelegates() { MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates--; }
