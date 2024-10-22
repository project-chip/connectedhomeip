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

#pragma once

#include <protocols/bdx/BdxTransferDiagnosticLogPool.h>

#include <lib/core/DataModelTypes.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxTransferDiagnosticLog.h>

namespace chip {
namespace bdx {

class BdxTransferDiagnosticLog;

class BDXTransferServer : public Messaging::UnsolicitedMessageHandler
{
public:
    BDXTransferServer(){};

    ~BDXTransferServer() { Shutdown(); };

    CHIP_ERROR Init(System::Layer * systemLayer, Messaging::ExchangeManager * exchangeMgr);

    void Shutdown();

    void SetDelegate(BDXTransferServerDelegate * delegate) { mDelegate = delegate; }

    void AbortTransfersForFabric(FabricIndex fabricIndex) { mPoolDelegate.AbortTransfersForFabric(fabricIndex); }

protected:
    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader,
                                            Messaging::ExchangeDelegate *& newDelegate) override;
    void OnExchangeCreationFailed(Messaging::ExchangeDelegate * delegate) override;

private:
    System::Layer * mSystemLayer;
    Messaging::ExchangeManager * mExchangeMgr;

    BDXTransferServerDelegate * mDelegate;
    BdxTransferDiagnosticLogPool<CHIP_CONFIG_MAX_BDX_LOG_TRANSFERS> mPoolDelegate;
};

} // namespace bdx
} // namespace chip
