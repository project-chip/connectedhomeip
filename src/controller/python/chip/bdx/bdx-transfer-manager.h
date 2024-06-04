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

#pragma once

#include <lib/support/Pool.h>

#include <controller/python/chip/bdx/bdx-transfer.h>
#include <controller/python/chip/bdx/bdx-transfer-pool.h>

namespace chip {
namespace bdx {

// This class implements the pool interface used to allocate BdxTransfer objects. It keeps track of the number of transfers
// that are expected to be created and only allocates a BdxTransfer object if a transfer is expected.
class BdxTransferManager : public BdxTransferPool
{
public:
    ~BdxTransferManager() override;

    // These keep track of the number of expected transfers.
    void ExpectATransfer();
    void StopExpectingATransfer();

    BdxTransfer * Allocate() override;
    void Release(BdxTransfer * bdxTransfer) override;

private:
    ObjectPool<BdxTransfer, 2> mTransferPool;
    size_t mExpectedTransfers = 0;
};

} // namespace bdx
} // namespace chip
