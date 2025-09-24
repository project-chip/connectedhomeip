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
#include <app/util/attribute-storage.h>
#include <app/util/config.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/bdx/DiagnosticLogs.h>

#include "BDXDiagnosticLogsProvider.h"

#ifdef MATTER_DM_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT
static constexpr size_t kDiagnosticLogsDiagnosticLogsProviderDelegateTableSize =
    MATTER_DM_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;
static_assert(kDiagnosticLogsDiagnosticLogsProviderDelegateTableSize < kEmberInvalidEndpointIndex,
              "DiagnosticLogs: log provider delegate table size error");

using namespace chip::app::Clusters::DiagnosticLogs;
using chip::Protocols::InteractionModel::Status;

using chip::bdx::DiagnosticLogs::kMaxFileDesignatorLen;
using chip::bdx::DiagnosticLogs::kMaxLogContentSize;

// Global delegate table
static DiagnosticLogsProviderDelegate *
    gDiagnosticLogsProviderDelegateTable[kDiagnosticLogsDiagnosticLogsProviderDelegateTableSize];

#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
// Global BDX provider instance
static BDXDiagnosticLogsProvider gBDXDiagnosticLogsProvider;
#endif

namespace chip {
namespace app {
namespace Clusters {

void DiagnosticLogsProviderLogic::SetDelegate(EndpointId endpoint, DiagnosticLogsProviderDelegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, DiagnosticLogs::Id, MATTER_DM_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT);
    if (ep < kDiagnosticLogsDiagnosticLogsProviderDelegateTableSize)
    {
        gDiagnosticLogsProviderDelegateTable[ep] = delegate;
    }
}

DiagnosticLogsProviderDelegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, DiagnosticLogs::Id, MATTER_DM_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT);
    auto delegate =
        (ep >= MATTER_ARRAY_SIZE(gDiagnosticLogsProviderDelegateTable) ? nullptr : gDiagnosticLogsProviderDelegateTable[ep]);

    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Diagnosticlogs: no log provider delegate set for endpoint:%u", endpoint);
    }

    return delegate;
}

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

void DiagnosticLogsProviderLogic::HandleLogRequestForResponsePayload(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                                                     IntentEnum intent, StatusEnum status)
{
    // If there is no delegate, there is no mechanism to read the logs. Assume those are empty and return NoLogs
    auto * delegate = GetDelegate(path.mEndpointId);
    VerifyOrReturn(nullptr != delegate, AddResponse(commandObj, path, StatusEnum::kNoLogs));

    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturn(buffer.Alloc(kMaxLogContentSize), AddResponse(commandObj, path, StatusEnum::kDenied));

    auto logContent = MutableByteSpan(buffer.Get(), kMaxLogContentSize);
    Optional<uint64_t> timeStamp;
    Optional<uint64_t> timeSinceBoot;

    auto size = delegate->GetSizeForIntent(intent);
    VerifyOrReturn(size != 0, AddResponse(commandObj, path, StatusEnum::kNoLogs));

    auto err = delegate->GetLogForIntent(intent, logContent, timeStamp, timeSinceBoot);
    VerifyOrReturn(CHIP_ERROR_NOT_FOUND != err, AddResponse(commandObj, path, StatusEnum::kNoLogs));
    VerifyOrReturn(CHIP_NO_ERROR == err, AddResponse(commandObj, path, StatusEnum::kDenied));

    AddResponse(commandObj, path, status, logContent, timeStamp, timeSinceBoot);
}

void DiagnosticLogsProviderLogic::HandleLogRequestForBdx(CommandHandler * commandObj, const ConcreteCommandPath & path,
                                                         IntentEnum intent, Optional<CharSpan> transferFileDesignator)
{
    // If the RequestedProtocol is set to BDX and there is no TransferFileDesignator the command SHALL fail with a Status Code of
    // INVALID_COMMAND.
    VerifyOrReturn(transferFileDesignator.HasValue(), commandObj->AddStatus(path, Status::InvalidCommand));

    VerifyOrReturn(transferFileDesignator.Value().size() <= kMaxFileDesignatorLen,
                   commandObj->AddStatus(path, Status::ConstraintError));

    // If there is no delegate, there is no mechanism to read the logs. Assume those are empty and return NoLogs
    auto * delegate = GetDelegate(path.mEndpointId);
    VerifyOrReturn(nullptr != delegate, AddResponse(commandObj, path, StatusEnum::kNoLogs));

    auto size = delegate->GetSizeForIntent(intent);
    // In the case where the size is 0 sets the Status field of the RetrieveLogsResponse to NoLogs and do not start a BDX session.
    VerifyOrReturn(size != 0, HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kNoLogs));

    // In the case where the Node is able to fit the entirety of the requested logs within the LogContent field, the Status field of
    // the RetrieveLogsResponse SHALL be set to Exhausted and a BDX session SHALL NOT be initiated.
    VerifyOrReturn(size > kMaxLogContentSize, HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kExhausted));

// If the RequestedProtocol is set to BDX and either the Node does not support BDX or it is not possible for the Node
// to establish a BDX session, then the Node SHALL utilize the LogContent field of the RetrieveLogsResponse command
// to transfer as much of the current logs as it can fit within the response, and the Status field of the
// RetrieveLogsResponse SHALL be set to Exhausted.
#if CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
    VerifyOrReturn(!gBDXDiagnosticLogsProvider.IsBusy(), AddResponse(commandObj, path, StatusEnum::kBusy));
    auto err = gBDXDiagnosticLogsProvider.InitializeTransfer(commandObj, path, delegate, intent, transferFileDesignator.Value());
    VerifyOrReturn(CHIP_NO_ERROR == err, AddResponse(commandObj, path, StatusEnum::kDenied));
#else
    HandleLogRequestForResponsePayload(commandObj, path, intent, StatusEnum::kExhausted);
#endif // CHIP_CONFIG_ENABLE_BDX_LOG_TRANSFER
}

} // namespace Clusters
} // namespace app
} // namespace chip
#endif // MATTER_DM_DIAGNOSTIC_LOGS_CLUSTER_SERVER_ENDPOINT_COUNT
