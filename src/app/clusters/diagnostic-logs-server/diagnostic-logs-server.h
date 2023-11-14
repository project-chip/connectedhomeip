/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <lib/core/CHIPConfig.h>

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
#include <app/bdx/DiagnosticLogsBDXTransferHandler.h>
#endif
#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandlerInterface.h>
#include <app/clusters/diagnostic-logs-server/diagnostic-logs-provider-delegate.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DiagnosticLogs {

// Spec mandated max file designator length
static constexpr uint8_t kLogFileDesignatorMaxLen = 32;

static constexpr const uint16_t kDiagnosticLogsEndpoint = 0;

// Spec mandated max size of the log content field in the Response paylod
static constexpr uint16_t kLogContentMaxSize = 1024;

/// A reference implementation for DiagnosticLogs source.
class DiagnosticLogsServer
{
public:
    static DiagnosticLogsServer & Instance();

    /**
     * Set the default delegate of the diagnostic logs cluster for the specified endpoint
     *
     * @param endpoint ID of the endpoint
     *
     * @param delegate The log provider delegate at the endpoint
     */
    void SetLogProviderDelegate(EndpointId endpoint, LogProviderDelegate * delegate);

    void HandleLogRequestForResponsePayload(chip::app::CommandHandler * commandHandler, chip::app::ConcreteCommandPath path,
                                            IntentEnum intent);

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

    void HandleBDXResponse(CHIP_ERROR error);

    CHIP_ERROR HandleLogRequestForBDXProtocol(chip::Messaging::ExchangeContext * exchangeCtx, chip::EndpointId endpointId,
                                              IntentEnum intent, chip::CharSpan fileDesignator);

    void SetAsyncCommandHandleAndPath(CommandHandler * commandObj, const ConcreteCommandPath & commandPath);

    bool HasValidFileDesignator(chip::CharSpan transferFileDesignator);

    bool IsBDXProtocolRequested(TransferProtocolEnum requestedProtocol);

    void SendErrorResponse(chip::app::CommandHandler * commandHandler, chip::app::ConcreteCommandPath path,
                                   StatusEnum status);

#endif

private:
#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

    DiagnosticLogsBDXTransferHandler * mDiagnosticLogsBDXTransferHandler;

#endif

    LogSessionHandle mLogSessionHandle;

    chip::app::CommandHandler::Handle mAsyncCommandHandle;
    chip::app::ConcreteCommandPath mRequestPath = chip::app::ConcreteCommandPath(0, 0, 0);
    IntentEnum mIntent;

    static DiagnosticLogsServer sInstance;
};

} // namespace DiagnosticLogs
} // namespace Clusters
} // namespace app
} // namespace chip
