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

#include <controller/python/chip/bdx/bdx-transfer-manager.h>

namespace chip {
namespace bdx {

BdxTransferManager::BdxTransferManager(BdxTransfer::Delegate * bdxTransferDelegate) : mBdxTransferDelegate(bdxTransferDelegate)
{
}

BdxTransferManager::~BdxTransferManager()
{
    mTransferPool.ReleaseAll();
}

CHIP_ERROR BdxTransferManager::Init(System::Layer * systemLayer)
{
    VerifyOrReturnError(systemLayer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mSystemLayer = systemLayer;
    return CHIP_NO_ERROR;
}

void BdxTransferManager::ExpectATransfer()
{
    ++mExpectedTransfers;
}

void BdxTransferManager::StopExpectingATransfer()
{
    if (mExpectedTransfers > 0)
    {
        --mExpectedTransfers;
    }
}

BdxTransfer * BdxTransferManager::Allocate()
{
    VerifyOrReturnValue(mExpectedTransfers != 0, nullptr);

    BdxTransfer * result = mTransferPool.CreateObject(mSystemLayer);
    VerifyOrReturnValue(result != nullptr, nullptr);
    result->SetDelegate(this);

    --mExpectedTransfers;
    return result;
}

// This method is only called by BdxTransferServer when creating an exchange context fails.
void BdxTransferManager::Release(BdxTransfer * bdxTransfer)
{
    mBdxTransferDelegate->TransferCompleted(bdxTransfer, CHIP_ERROR_CONNECTION_ABORTED);
    mTransferPool.ReleaseObject(bdxTransfer);
}

void BdxTransferManager::InitMessageReceived(BdxTransfer * transfer, TransferSession::TransferInitData init_data)
{
    mBdxTransferDelegate->InitMessageReceived(transfer, init_data);
}

void BdxTransferManager::DataReceived(BdxTransfer * transfer, const ByteSpan & block)
{
    mBdxTransferDelegate->DataReceived(transfer, block);
}

void BdxTransferManager::TransferCompleted(BdxTransfer * transfer, CHIP_ERROR result)
{
    mBdxTransferDelegate->TransferCompleted(transfer, result);
    // TODO: Schedule this to happen after the transfer completes.
    mTransferPool.ReleaseObject(transfer);
}

} // namespace bdx
} // namespace chip
