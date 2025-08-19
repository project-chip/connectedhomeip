/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/diagnostic-logs-server/DiagnosticLogsLogic.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include "BDXDiagnosticLogsProvider.h"

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Protocols::InteractionModel::Status;
using DiagnosticLogs::IntentEnum;

using chip::bdx::DiagnosticLogs::kMaxFileDesignatorLen;
using chip::bdx::DiagnosticLogs::kMaxLogContentSize;

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
// Global BDX provider instance
static BDXDiagnosticLogsProvider gBDXDiagnosticLogsProvider;
#endif

namespace chip {
namespace app {
namespace Clusters {

void AddResponse(CommandHandler * commandObj, const ConcreteCommandPath & path, StatusEnum status)
{
    Commands::RetrieveLogsResponse::Type response;
    response.status = status;
    commandObj->AddResponse(path, response);
}

void AddResponse(CommandHandler * commandObj, const ConcreteCommandPath & path, StatusEnum status, MutableByteSpan & logContent,
                 const Optional<uint64_t> & timeStamp, const Optional<uint64_t> & timeSinceBoot)
{
    Commands::RetrieveLogsResponse::Type response;
    response.status        = status;
    response.logContent    = ByteSpan(logContent);
    response.UTCTimeStamp  = timeStamp;
    response.timeSinceBoot = timeSinceBoot;

    commandObj->AddResponse(path, response);
}

std::optional<DataModel::ActionReturnStatus>
DiagnosticLogsProviderLogic::HandleLogRequestForResponsePayload(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                                                IntentEnum intent, StatusEnum status)
{
    // If there is no delegate, there is no mechanism to read the logs. Assume those are empty and return NoLogs
    VerifyOrReturnError(nullptr != mDelegate, std::nullopt, AddResponse(commandObj, path, StatusEnum::kNoLogs));
    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Alloc(kMaxLogContentSize), std::nullopt, AddResponse(commandObj, path, StatusEnum::kDenied));

    auto logContent = MutableByteSpan(buffer.Get(), kMaxLogContentSize);
    Optional<uint64_t> timeStamp;
    Optional<uint64_t> timeSinceBoot;

    auto size = mDelegate->GetSizeForIntent(intent);
    VerifyOrReturnError(size != 0, std::nullopt, AddResponse(commandObj, path, StatusEnum::kNoLogs));
    auto err = mDelegate->GetLogForIntent(intent, logContent, timeStamp, timeSinceBoot);
    VerifyOrReturnError(CHIP_ERROR_NOT_FOUND != err, std::nullopt, AddResponse(commandObj, path, StatusEnum::kNoLogs));
    VerifyOrReturnError(CHIP_NO_ERROR == err, std::nullopt, AddResponse(commandObj, path, StatusEnum::kDenied));
    AddResponse(commandObj, path, status, logContent, timeStamp, timeSinceBoot);
    return std::nullopt;
}

std::optional<DataModel::ActionReturnStatus>
DiagnosticLogsProviderLogic::HandleLogRequestForBdx(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                                    IntentEnum intent, Optional<CharSpan> transferFileDesignator)
{
    // If the RequestedProtocol is set to BDX and there is no TransferFileDesignator the command SHALL fail with a Status Code of
    // INVALID_COMMAND.
    VerifyOrReturnError(transferFileDesignator.HasValue(), std::nullopt, commandObj->AddStatus(path, Status::InvalidCommand));

    VerifyOrReturnError(transferFileDesignator.Value().size() <= kMaxFileDesignatorLen, std::nullopt,
                        commandObj->AddStatus(path, Status::ConstraintError));
    // If there is no delegate, there is no mechanism to read the logs. Assume those are empty and return NoLogs
    VerifyOrReturnError(nullptr != mDelegate, std::nullopt, AddResponse(commandObj, path, StatusEnum::kNoLogs));

    auto size = mDelegate->GetSizeForIntent(intent);
    // In the case where the size is 0 sets the Status field of the RetrieveLogsResponse to NoLogs and do not start a BDX session.
    VerifyOrReturnError(size != 0, std::nullopt, HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kNoLogs));

    // In the case where the Node is able to fit the entirety of the requested logs within the LogContent field, the Status field of
    // the RetrieveLogsResponse SHALL be set to Exhausted and a BDX session SHALL NOT be initiated.
    VerifyOrReturnError(size > kMaxLogContentSize, std::nullopt,
                        HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kExhausted));
// If the RequestedProtocol is set to BDX and either the Node does not support BDX or it is not possible for the Node
// to establish a BDX session, then the Node SHALL utilize the LogContent field of the RetrieveLogsResponse command
// to transfer as much of the current logs as it can fit within the response, and the Status field of the
// RetrieveLogsResponse SHALL be set to Exhausted.
#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
    VerifyOrReturnError(!gBDXDiagnosticLogsProvider.IsBusy(), std::nullopt, AddResponse(commandObj, path, StatusEnum::kBusy));
    auto err = gBDXDiagnosticLogsProvider.InitializeTransfer(commandObj, path, mDelegate, intent, transferFileDesignator.Value());
    VerifyOrReturnError(CHIP_NO_ERROR == err, std::nullopt, AddResponse(commandObj, path, StatusEnum::kDenied));
#else
    HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kExhausted);
#endif // CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER

    return std::nullopt;
}

} // namespace Clusters
} // namespace app
} // namespace chip
