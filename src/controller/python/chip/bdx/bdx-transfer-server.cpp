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

#include <controller/python/chip/bdx/bdx-transfer-server.h>

#include <messaging/ExchangeDelegate.h>
#include <transport/raw/MessageHeader.h>

#include <controller/python/chip/bdx/bdx-transfer.h>
#include <controller/python/chip/bdx/bdx-transfer-pool.h>

namespace chip {
namespace bdx {

CHIP_ERROR BdxTransferServer::Init(Messaging::ExchangeManager * exchangeManager)
{
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mExchangeManager = exchangeManager;
    ReturnOnFailure(mExchangeManager->RegisterUnsolicitedMessageHandlerForType(MessageType::ReceiveAccept, this));
    return mExchangeManager->RegisterUnsolicitedMessageHandlerForType(MessageType::SendAccept, this);
}

void BdxTransferServer::Shutdown()
{
    VerifyOrReturn(mExchangeManager != nullptr);
    LogErrorOnFailure(mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(MessageType::ReceiveAccept));
    LogErrorOnFailure(mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(MessageType::SendAccept));
    mExchangeManager = nullptr;
}

CHIP_ERROR BdxTransferServer::OnUnsolicitedMessageReceived(const PayloadHeader& payloadHeader,
                                                           Messaging::ExchangeDelegate *& delegate)
{
    // TODO: Verify details from the payload header.
    BdxTransfer * transfer = mBdxTransferPool->Allocate();
    if (!transfer)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    delegate = transfer;
    return CHIP_NO_ERROR;
}

void BdxTransferServer::OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate)
{
    mBdxTransferPool->Release(static_cast<BdxTransfer *>(delegate));
}

} // namespace bdx
} // namespace chip
