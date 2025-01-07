/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <lib/core/DataModelTypes.h>
#include <lib/support/Pool.h>
#include <protocols/bdx/BdxTransferDiagnosticLog.h>
#include <protocols/bdx/BdxTransferServerDelegate.h>
#include <system/SystemLayer.h>

namespace chip {
namespace bdx {

class BdxTransferDiagnosticLogPoolDelegate
{
public:
    virtual ~BdxTransferDiagnosticLogPoolDelegate() {}

    virtual BdxTransferDiagnosticLog * Allocate(BDXTransferServerDelegate * delegate, System::Layer * systemLayer) = 0;

    virtual void Release(BdxTransferDiagnosticLog * client) = 0;
};

template <size_t N>
class BdxTransferDiagnosticLogPool : public BdxTransferDiagnosticLogPoolDelegate
{
public:
    ~BdxTransferDiagnosticLogPool() override { mTransferPool.ReleaseAll(); }

    BdxTransferDiagnosticLog * Allocate(BDXTransferServerDelegate * delegate, System::Layer * systemLayer) override
    {
        return mTransferPool.CreateObject(delegate, this, systemLayer);
    }

    void Release(BdxTransferDiagnosticLog * transfer) override { mTransferPool.ReleaseObject(transfer); }

    void AbortTransfersForFabric(FabricIndex fabricIndex)
    {
        mTransferPool.ForEachActiveObject([fabricIndex](BdxTransferDiagnosticLog * transfer) {
            if (transfer->IsForFabric(fabricIndex))
            {
                transfer->AbortTransfer();
            }
            return Loop::Continue;
        });
    }

private:
    ObjectPool<BdxTransferDiagnosticLog, N> mTransferPool;
};

} // namespace bdx
} // namespace chip
