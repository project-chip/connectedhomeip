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

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

#pragma once

#include <app/clusters/diagnostic-logs-server/diagnostic-logs-provider-delegate.h>

#include <protocols/bdx/TransferFacilitator.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {
/**
 * An implementation of the handler than initiates a BDX transfer as a Sender using the synchronous Sender Drive
 * transfer mode. It gets the chunks of the log from the accessory and sends the block accross to the receiver until
 * all the blocks have been transferred and the accessory reports that end of file is reached.
 */
class DiagnosticLogsBDXTransferHandler : public chip::bdx::Initiator
{
public:
    DiagnosticLogsBDXTransferHandler() {}
    ~DiagnosticLogsBDXTransferHandler() {}

    CHIP_ERROR Init();

    CHIP_ERROR InitializeTransfer(chip::Messaging::ExchangeContext * exchangeCtx, chip::FabricIndex fabricIndex,
                                  chip::NodeId nodeId, LogProviderDelegate * delegate, IntentEnum intent,
                                  chip::CharSpan fileDesignator);

    void HandleTransferSessionOutput(chip::bdx::TransferSession::OutputEvent & event) override;

    void Reset();

private:
    chip::Optional<chip::FabricIndex> mFabricIndex;
    chip::Optional<chip::NodeId> mNodeId;

    chip::Messaging::ExchangeContext * mExchangeCtx;

    bool mInitialized;

    uint64_t mNumBytesSent;

    LogSessionHandle mLogSessionHandle;

    LogProviderDelegate * mDelegate;

    IntentEnum mIntent;

    char mFileDesignator[chip::bdx::kMaxFileDesignatorLen];
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
#endif
