/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
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

#include "BdxTransferServer.h"

namespace chip {
namespace bdx {

CHIP_ERROR BDXTransferServer::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr)
{
    VerifyOrReturnError(nullptr != systemLayer, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nullptr != exchangeMgr, CHIP_ERROR_INVALID_ARGUMENT);

    mSystemLayer = systemLayer;
    mExchangeMgr = exchangeMgr;
    return mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(MessageType::SendInit, this);
}

CHIP_ERROR BDXTransferServer::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader,
                                                           Messaging::ExchangeDelegate *& newDelegate)
{
    auto * logTransfer = mPoolDelegate.Allocate(mDelegate, mSystemLayer);
    VerifyOrReturnError(nullptr != logTransfer, CHIP_ERROR_NO_MEMORY);

    newDelegate = logTransfer;
    return CHIP_NO_ERROR;
}

void BDXTransferServer::OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate)
{
    auto * logTransfer = static_cast<BdxTransferDiagnosticLog *>(delegate);
    mPoolDelegate.Release(logTransfer);
}

void BDXTransferServer::Shutdown()
{
    VerifyOrReturn(nullptr != mSystemLayer);
    VerifyOrReturn(nullptr != mExchangeMgr);

    LogErrorOnFailure(mExchangeMgr->UnregisterUnsolicitedMessageHandlerForType(MessageType::SendInit));

    mSystemLayer = nullptr;
    mExchangeMgr = nullptr;
}

} // namespace bdx
} // namespace chip
