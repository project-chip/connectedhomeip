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
#import "MTRDeviceController_Internal.h"

#include <protocols/bdx/BdxMessages.h>

using namespace chip;
using namespace chip::Messaging;
using namespace chip::bdx;

MTROTAUnsolicitedBDXMessageHandler * MTROTAUnsolicitedBDXMessageHandler::sInstance = nullptr;

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::Init(System::Layer * systemLayer, ExchangeManager * exchangeManager)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mSystemLayer = systemLayer;
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

    VerifyOrReturn(mOTAImageTransferHandler != nullptr);

    delete mOTAImageTransferHandler;
    mOTAImageTransferHandler = nullptr;
}

void MTROTAUnsolicitedBDXMessageHandler::ControllerShuttingDown(MTRDeviceController_Concrete * controller)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrReturn(mOTAImageTransferHandler != nullptr);

    FabricIndex peerFabricIndex = mOTAImageTransferHandler->GetPeerFabricIndex();
    VerifyOrReturn(peerFabricIndex != kUndefinedFabricIndex);

    if (controller.fabricIndex == peerFabricIndex) {
        delete mOTAImageTransferHandler;
        mOTAImageTransferHandler = nullptr;
    }
}

bool MTROTAUnsolicitedBDXMessageHandler::IsInAnOngoingTransfer()
{
    return (mOTAImageTransferHandler != nullptr);
}

CHIP_ERROR MTROTAUnsolicitedBDXMessageHandler::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, const SessionHandle & session,
    ExchangeDelegate * _Nonnull & newDelegate)
{
    assertChipStackLockedByCurrentThread();

    ChipLogDetail(BDX, "MTROTAUnsolicitedBDXMessageHandler: OnUnsolicitedMessageReceived: message " ChipLogFormatMessageType " protocol " ChipLogFormatProtocolId,
        payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()));

    if (IsInAnOngoingTransfer()) {
        ChipLogDetail(BDX, "Already in an ongoing transfer. Return");
        return CHIP_ERROR_BUSY;
    }

    VerifyOrReturnError(mExchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Only proceed if there is a valid fabric index for the SessionHandle.
    if (session->IsSecureSession() && session->AsSecureSession() != nullptr && session->AsSecureSession()->GetFabricIndex() != kUndefinedFabricIndex) {

        // If we receive a ReceiveInit BDX message, create a new MTROTAImageTransferHandler and register it
        // as the handler for all BDX messages that will come over this exchange.
        if (payloadHeader.HasMessageType(MessageType::ReceiveInit)) {
            newDelegate = new MTROTAImageTransferHandler(mSystemLayer);
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

    delete otaTransferHandler;
}

void MTROTAUnsolicitedBDXMessageHandler::OnTransferHandlerCreated(MTROTAImageTransferHandler * imageTransferHandler)
{
    assertChipStackLockedByCurrentThread();

    // TODO: #36181 - Store the imageTransferHandler in a set of MTROTAImageTransferHandler objects.
    mOTAImageTransferHandler = imageTransferHandler;
}

void MTROTAUnsolicitedBDXMessageHandler::OnTransferHandlerDestroyed(MTROTAImageTransferHandler * imageTransferHandler)
{
    assertChipStackLockedByCurrentThread();

    // TODO: #36181 - Remove the object matching imageTransferHandler in the set of MTROTAImageTransferHandler objects.
    if (mOTAImageTransferHandler == imageTransferHandler) {
        mOTAImageTransferHandler = nullptr;
    }
}
