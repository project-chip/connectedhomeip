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

#include <controller/python/chip/bdx/test-bdx-transfer-server.h>

namespace chip {
namespace bdx {

TestBdxTransferServer::TestBdxTransferServer(BdxTransfer::Delegate * bdxTransferDelegate) :
    mBdxTransferDelegate(bdxTransferDelegate)
{}

TestBdxTransferServer::~TestBdxTransferServer()
{
    mTransferPool.ReleaseAll();
}

CHIP_ERROR TestBdxTransferServer::Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeManager)
{
    VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exchangeManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mSystemLayer     = systemLayer;
    mExchangeManager = exchangeManager;
    // This removes the BdxTransferServer registered as part of CHIPDeviceControllerFactory.
    mExchangeManager->UnregisterUnsolicitedMessageHandlerForType(MessageType::SendInit);
    return mExchangeManager->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id, this);
}

void TestBdxTransferServer::Shutdown()
{
    VerifyOrReturn(mExchangeManager != nullptr);
    LogErrorOnFailure(mExchangeManager->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id));
    mExchangeManager = nullptr;
}

void TestBdxTransferServer::ExpectATransfer()
{
    ++mExpectedTransfers;
}

void TestBdxTransferServer::StopExpectingATransfer()
{
    if (mExpectedTransfers > 0)
    {
        --mExpectedTransfers;
    }
}

void TestBdxTransferServer::Release(BdxTransfer * bdxTransfer)
{
    mTransferPool.ReleaseObject(bdxTransfer);
}

CHIP_ERROR TestBdxTransferServer::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader,
                                                               Messaging::ExchangeDelegate *& delegate)
{
    VerifyOrReturnValue(mExpectedTransfers != 0, CHIP_ERROR_HANDLER_NOT_SET);

    BdxTransfer * transfer = mTransferPool.CreateObject(mSystemLayer);
    if (transfer == nullptr)
    {
        ChipLogError(BDX, "Failed to allocate BDX transfer. The pool (size %d) is exhausted.", static_cast<int>(kTransferPoolSize));
        return CHIP_ERROR_NO_MEMORY;
    }
    transfer->SetDelegate(mBdxTransferDelegate);
    delegate = transfer;

    --mExpectedTransfers;

    return CHIP_NO_ERROR;
}

void TestBdxTransferServer::OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate)
{
    BdxTransfer * bdxTransfer = static_cast<BdxTransfer *>(delegate);
    mBdxTransferDelegate->TransferCompleted(bdxTransfer, CHIP_ERROR_CONNECTION_ABORTED);
    Release(bdxTransfer);
}

} // namespace bdx
} // namespace chip
