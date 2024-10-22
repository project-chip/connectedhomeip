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

#include <protocols/bdx/BdxMessages.h>

using namespace chip;
using namespace chip::Messaging;
using namespace chip::bdx;

MTROTAUnsolicitedBDXMessageHandler * MTROTAUnsolicitedBDXMessageHandler::sInstance = nullptr;

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::Init(ExchangeManager * exchangeManager)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INCORRECT_STATE);

    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates = 0;
    mExchangeMgr = exchangeManager;
    return mExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
}

MTROTAUnsolicitedBDXMessageHandler * MTROTAUnsolicitedBDXMessageHandler::GetInstance()
{
    return sInstance;
}

void MTROTAUnsolicitedBDXMessageHandler::Shutdown()
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mExchangeMgr != nullptr);

    mExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id);
    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates = 0;

    if (mOTAImageTransferHandler != nullptr)
    {
        mOTAImageTransferHandler->DestroySelf();
        delete mOTAImageTransferHandler;
    }

}

void MTROTAUnsolicitedBDXMessageHandler::ControllerShuttingDown(MTRDeviceController * controller)
{
    assertChipStackLockedByCurrentThread();

    // Since the OTA provider delegate only supports one BDX transfer at a time, calling ShutDown is fine for now.
    // TODO: #36181 - This class needs to keep a list of all MTROTAImageTransferHandlers mapped to the fabric index and only
    // delete the MTROTAImageTransferHandler objects with a fabric index matching the MTRDeviceController's fabric index.
    Shutdown();
}

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, const SessionHandle & session,
    ExchangeDelegate * _Nonnull & newDelegate)
{
    assertChipStackLockedByCurrentThread();

    ChipLogDetail(BDX, "MTROTAUnsolicitedBDXMessageHandler: OnUnsolicitedMessageReceived: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (GetNumberOfDelegates() >= 1) {
        return CHIP_ERROR_BUSY;
    }

    // Only proceed if there is a valid fabric index for the SessionHandle.
    if (session->IsSecureSession() && session->AsSecureSession() != nullptr && session->AsSecureSession()->GetFabricIndex() != kUndefinedFabricIndex) {

        // If we receive a ReceiveInit BDX message, create a new MTROTAImageTransferHandler and register it
        // as the handler for all BDX messages that will come over this exchange.
        if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
            mOTAImageTransferHandler = new MTROTAImageTransferHandler();
            newDelegate = mOTAImageTransferHandler;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

void MTROTAUnsolicitedBDXMessageHandler::OnExchangeCreationFailed(ExchangeDelegate * delegate)
{
    assertChipStackLockedByCurrentThread();
    auto * otaTransferHandler = static_cast<MTROTAImageTransferHandler *>(delegate);
    VerifyOrReturn(otaTransferHandler != nullptr);

    otaTransferHandler->DestroySelf();
}

uint8_t MTROTAUnsolicitedBDXMessageHandler::GetNumberOfDelegates()
{
    assertChipStackLockedByCurrentThread();
    return MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates;
}

void MTROTAUnsolicitedBDXMessageHandler::IncrementNumberOfDelegates()
{
    assertChipStackLockedByCurrentThread();
    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates++;
}

void MTROTAUnsolicitedBDXMessageHandler::DecrementNumberOfDelegates()
{
    assertChipStackLockedByCurrentThread();
    MTROTAUnsolicitedBDXMessageHandler::mNumberOfDelegates--;
}

void MTROTAUnsolicitedBDXMessageHandler::OnDelegateCreated(void * imageTransferHandler)
{
    assertChipStackLockedByCurrentThread();

    // TODO: #36181 - Store the imageTransferHandler in a set of MTROTAImageTransferHandler objects.
    mOTAImageTransferHandler = static_cast<MTROTAImageTransferHandler *>(imageTransferHandler);
    MTROTAUnsolicitedBDXMessageHandler::IncrementNumberOfDelegates();
}

void MTROTAUnsolicitedBDXMessageHandler::OnDelegateDestroyed(void * imageTransferHandler)
{
    assertChipStackLockedByCurrentThread();

    // TODO: #36181 - Remove the object matching imageTransferHandler in the set of MTROTAImageTransferHandler objects.
    if (mOTAImageTransferHandler == static_cast<MTROTAImageTransferHandler *>(imageTransferHandler))
    {
        mOTAImageTransferHandler = nullptr;
    }
    MTROTAUnsolicitedBDXMessageHandler::DecrementNumberOfDelegates();
}
