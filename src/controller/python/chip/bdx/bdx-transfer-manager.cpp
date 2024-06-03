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

BdxTransferManager::BdxTransferManager()
{
}

BdxTransferManager::~BdxTransferManager()
{
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

void * BdxTransferManager::Allocate()
{
    if (mExpectedTransfers == 0)
    {
        return nullptr;
    }

    // TODO: Change the type to BdxTransfer.
    int * result = mTransferPool.CreateObject();

    if (result)
    {
        --mExpectedTransfers;
    }

    return result;
}

void BdxTransferManager::Release(void * bdxTransfer)
{
    mTransferPool.ReleaseObject(bdxTransfer);
}

} // namespace bdx
} // namespace chip
